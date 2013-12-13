#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <wordexp.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sd.h"
#include "crc-16.h"

#define DBG_PROMPT "AX215> "
#define PROGRAM_OFFSET 0x4700

struct dbg {
    int             should_quit;
    int             ret;
    struct sd_state *sd;

    int             read_sfr_offset;
    int             write_sfr_offset;
    int             ext_op_offset;

    int             initialized; // False if uninitialized
    int             fixed_up; // True if fixups have been found
};

/* These are SD commands as they get sent to the card */
enum protocol_code {
    cmd_null = 0,
    cmd_hello = 1,
    cmd_peek = 2,
    cmd_poke = 3,
    cmd_jump = 4,
    cmd_nand = 5,
    cmd_sfr_get = 6,
    cmd_sfr_set = 7,
    cmd_ext_op  = 8,
    cmd_error = 9,
    cmd_irq = 10,
};

struct debug_command {
    const char    *name;
    const char    *desc;
    const char    *help;
    int (*func)(struct dbg *dbg, int argc, char **argv);
};

int disasm_8051(FILE *ofile, uint8_t *bfr, int size, int offset);

static int dbg_do_hello(struct dbg *dbg, int argc, char **argv);
static int dbg_do_null(struct dbg *dbg, int argc, char **argv);
static int dbg_do_peek(struct dbg *dbg, int argc, char **argv);
static int dbg_do_poke(struct dbg *dbg, int argc, char **argv);
static int dbg_do_jump(struct dbg *dbg, int argc, char **argv);
static int dbg_do_gpio(struct dbg *dbg, int argc, char **argv);
static int dbg_do_help(struct dbg *dbg, int argc, char **argv);
static int dbg_do_memset(struct dbg *dbg, int argc, char **argv);
static int dbg_do_disasm(struct dbg *dbg, int argc, char **argv);
static int dbg_do_nand(struct dbg *dbg, int argc, char **argv);
static int dbg_do_sfr(struct dbg *dbg, int argc, char **argv);
static int dbg_do_reset(struct dbg *dbg, int argc, char **argv);
static int dbg_do_dump_rom(struct dbg *dbg, int argc, char **argv);
static int dbg_do_irq(struct dbg *dbg, int argc, char **argv);
static int dbg_do_ext_op(struct dbg *dbg, int argc, char **argv);

static struct debug_command debug_commands[] = {
    {
        .name = "hello",
        .func = dbg_do_hello,
        .desc = "Make sure the card is there",
        .help = "Takes up to four arguments.  All four arguments will "
                "be echoed back by the card.\n",
    },
    {
        .name = "peek",
        .func = dbg_do_peek,
        .desc = "Read an area of memory",
        .help = "Accepts two arguments:\n"
                "\tR1: Byte to peek from\n"
                "\tR2: Number of bytes to peek (up to 255)\n"
    },
    {
        .name = "poke",
        .func = dbg_do_poke,
        .desc = "Write to an area of memory",
        .help = "Accepts two arguments:\n"
                "\tR1: Address to poke\n"
                "\tR2: Byte to poke\n"
    },
    {
        .name = "jump",
        .func = dbg_do_jump,
        .desc = "Jump to an area of memory",
    },
    {
        .name = "dumprom",
        .func = dbg_do_dump_rom,
        .desc = "Dump all of ROM to a file",
        .help = "Dumps all of the AX215's 16 kB to a file.\n"
                "Note that, for some reason, the first 0x200 bytes "
                "cannot be read.\n",
    },
    {
        .name = "memset",
        .func = dbg_do_memset,
        .desc = "Set a range of memory to a single value",
        .help = "Usage: memset [addr] [value] [count]\n"
                "   Useful for generating test patterns for file transfer\n"
    },
    {
        .name = "null",
        .func = dbg_do_null,
        .desc = "Do nothing and return all zeroes",
        .help = "Make sure the card is responding.  Takes no commands,\n"
                "and returns all zeroes.\n",
    },
    {
        .name = "disasm",
        .func = dbg_do_disasm,
        .desc = "Disassemble an area of memory",
        .help = "Disassemble a number of bytes at the given offset.\n"
                "Usage: disasm [address] [bytes]\n",
    },
    {
        .name = "ram",
        .func = dbg_do_sfr,
        .desc = "Manipulate internal RAM",
        .help = "Usage: ram [-d] [-w ram:val] [-r ram] [-x ram]\n"
                "   -d          Dump internal ram\n"
                "   -w addr:val Set RAM [addr] to value [val]\n"
                "   -i addr     Invert RAM contents\n"
                "   -r addr     Read RAM [addr]\n"
                "   -x addr     Read 32-bit register [addr]\n"
                ,
    },
    {
        .name = "sfr",
        .func = dbg_do_sfr,
        .desc = "Manipulate special function registers",
        .help = "Usage: sfr [-d] [-w sfr:val] [-r sfr] [-x sfr]\n"
                "   -d          Dump special function registers\n"
                "   -w sfr:val  Set SFR [sfr] to value [val]\n"
                "   -i addr     Invert RAM contents\n"
                "   -r sfr      Read SFR [sfr]\n"
                "   -x sfr      Read extended (quad-wide-wide) sfr\n"
                ,
    },
    {
        .name = "nand",
        .func = dbg_do_nand,
        .desc = "Operate on the NAND in some fashion",
        .help = "Usage: nand [-r] [-l] [-s] [-d] [-v] [-c] [-t cmd:addr] -w [src:addr]\n"
                "   -c [reg]      Try every possible value in register [reg]\n"
                "   -w [src:dst]  Write RAM at address [src] to NAND addr [addr]\n"
                "   -t [type]     Test NAND command.  Specify an address on the cmdline\n"
                ,
    },
    {
        .name = "extop",
        .func = dbg_do_ext_op,
        .desc = "Execute an extended opcode on the chip",
        .help = "Usage: extop [opcode]    or   extop [op1] [op2]\n"
                "   Extended opcodes are one or two bytes.  Two-byte opcodes \n"
                "   have the upper nybble of op1 set as 0xf0, e.g. 0xf5 0x61\n"
                ,
    },
    {
        .name = "irq",
        .func = dbg_do_irq,
        .desc = "Manipulate IRQs on the AX215\n",
        .help = "Usage: irq [-r] [-m mask] [-p]\n"
                "   -r      Reset IRQ statistics\n"
                "   -p      Print IRQ statistics\n"
                "   -m=mask Set IRQ enable mask\n"
                ,
    },
    {
        .name = "gpio",
        .func = dbg_do_gpio,
        .desc = "Set/query GPIO values\n",
        .help = "Usage: gpio [-d] [-i DATpin] [-o DATpin] [-s/-c DATpin]\n"
                "   -d      Dump GPIO values\n"
                "   -i x    Make DATx a GPIO input\n"
                "   -o x    Make DATx a GPIO output\n"
                "   -s x    Set bit on DATx (e.g. write 1 out DATx)\n"
                "   -c x    Clearit on DATx (e.g. write 0 out DATx)\n"
                ,
    },
    {
        .name = "reset",
        .func = dbg_do_reset,
        .desc = "Reset the AX215 card",
        .help = "Call to reset/restart the AX215 CPU\n",
    },
    {
        .name = "help",
        .func = dbg_do_help,
        .desc = "Print this help",
    },
    { },
};




static int dbg_txrx(struct dbg *dbg, enum protocol_code code,
                    uint8_t args[4], uint8_t *out, int outsize) {
    uint8_t bfr[6];
    int tries;

    memset(out, 0, outsize);

    /* Construct the SD packet, with CRC7 at the end */
    bfr[0] = (code & 0x3f) | 0x40;
    bfr[1] = args[0];
    bfr[2] = args[1];
    bfr[3] = args[2];
    bfr[4] = args[3];
    bfr[5] = (crc7(bfr, 5) << 1) | 1;

    xmit_mmc_cmd(dbg->sd, bfr, sizeof(bfr));

    /* Get the response */
    tries = rcvr_mmc_cmd_start(dbg->sd, 1024);
    if (tries == -1) {
        printf("Never got start!\n");
        return -1;
    }
    rcvr_mmc_cmd(dbg->sd, out, outsize);

    if ((out[0] & 0x3f) != code && dbg->fixed_up)
        printf("Warning: Command stream seems to have changed "
                "(expected 0x%02x, got 0x%02x)\n", code, out[0] & 0x3f);
    return 0;
}


// Set a value in XRAM
int xram_set(struct dbg *dbg, int offset, uint8_t val) {
    int src_lo, src_hi;
    uint8_t args[4];
    uint8_t bfr[5];

    src_hi = (offset >> 8) & 0xff;
    src_lo = (offset) & 0xff;

    args[0] = src_hi;
    args[1] = src_lo;
    args[2] = val;
    args[3] = 0;
    if (dbg_txrx(dbg, cmd_poke, args, bfr, sizeof(bfr)))
        return -1;
    return bfr[1];
}

// Set a value in RAM (or SFR)
static int ram_set(struct dbg *dbg, uint8_t offset, uint8_t val) {
    uint8_t cmd[4];
    uint8_t bfr[5];
    xram_set(dbg, dbg->write_sfr_offset, offset);
    cmd[0] = val;
    return dbg_txrx(dbg, cmd_sfr_set, cmd, bfr, sizeof(bfr));
}

static int xram_get(struct dbg *dbg, uint8_t *buf,
                        int offset, int count) {
    int ret;
    while (count > 0) {
        uint8_t args[4];
        uint8_t out[5];
        args[0] = (offset >> 8) & 0xff;
        args[1] = (offset >> 0) & 0xff;
        ret = dbg_txrx(dbg, cmd_peek, args, out, sizeof(out));
        if (ret)
            return ret;
        memcpy(buf, out + 1, (count > 4 ? 4 : count));
        buf    += 4;
        offset += 4;
        count  -= 4;
    }
    return 0;
}

static int ram_get(struct dbg *dbg, int offset) {
    uint8_t cmd[4];
    uint8_t bfr[5];
    xram_set(dbg, dbg->read_sfr_offset, offset);
    memset(cmd, 0, sizeof(cmd));
    dbg_txrx(dbg, cmd_sfr_get, cmd, bfr, sizeof(bfr));
    return bfr[1];
}

static int dbg_do_ext_op(struct dbg *dbg, int argc, char **argv) {
    uint8_t cmd[4];
    uint8_t bfr[5];
    int i;
    int ret;

    if (argc < 2) {
        printf("Usage: %s op1 [op2 [op3 [op4]]]\n", argv[0]);
        return -EINVAL;
    }

    xram_set(dbg, dbg->ext_op_offset + 0, 0xa5);
    for (i = 1; i <= 4; i++) {
        if (argc > i)
            xram_set(dbg, dbg->ext_op_offset + i, strtoul(argv[i], NULL, 0));
        else
            xram_set(dbg, dbg->ext_op_offset + i, 0);
    }

    memset(cmd, 0, sizeof(cmd));
    ret = dbg_txrx(dbg, cmd_ext_op, cmd, bfr, sizeof(bfr));

    return ret;
}

static int dbg_do_sfr(struct dbg *dbg, int argc, char **argv) {
    int ch;
    int sfr;
    uint8_t sfr_table[128];
    int offset = 0;

    if (!strcmp(argv[0], "sfr"))
        offset = 0x80;

    while ((ch = getopt(argc, argv, "di:w:r:x:")) != -1) {
        switch(ch) {
        case 'd':
            for (sfr = 0; sfr <= 127; sfr++) {
                uint8_t cmd[4];
                uint8_t bfr[5];
                xram_set(dbg, dbg->read_sfr_offset, sfr + offset);
                memset(cmd, 0, sizeof(cmd));
                dbg_txrx(dbg, cmd_sfr_get, cmd, bfr, sizeof(bfr));
                sfr_table[sfr] = bfr[1];
            }
            print_hex_offset(sfr_table, sizeof(sfr_table), offset);
            break;

        case 'w': {
                char *endptr;
                int sfr = strtoul(optarg, &endptr, 0);
                int val;
                if (offset && (sfr < 0x80 || sfr > 0xff)) {
                    printf("Invalid SFR.  "
                            "SFR addresses go between 0x80 and 0xff\n");
                    return -EINVAL;
                }
                if (!offset && (sfr < 0x00 || sfr > 0x7f)) {
                    printf("Invalid RAM address.  "
                            "RAM addresses go between 0x00 and 0x7f\n");
                    return -EINVAL;
                }
                if (!endptr) {
                    printf("No value specified\n");
                    return -EINVAL;
                }
                val = strtoul(endptr + 1, NULL, 0);

                printf("Setting %s_%02x -> %02x\n",
                        offset?"SFR":"RAM", sfr, val);

                ram_set(dbg, sfr, val);
            }
            break;

        case 'i': {
                int sfr = strtoul(optarg, NULL, 0);
                int val;
                if (offset && (sfr < 0x80 || sfr > 0xff)) {
                    printf("Invalid SFR.  "
                            "SFR addresses go between 0x80 and 0xff\n");
                    return -EINVAL;
                }
                if (!offset && (sfr < 0x00 || sfr > 0x7f)) {
                    printf("Invalid RAM address.  "
                            "RAM addresses go between 0x00 and 0x7f\n");
                    return -EINVAL;
                }

                val = ram_get(dbg, sfr);
                printf("Setting %s_%02x 0x%02x -> %02x\n",
                        offset?"SFR":"RAM", sfr, val, 0xff & (~val));
                ram_set(dbg, sfr, ~val);
            }
            break;

        case 'x':
        case 'r': {
                uint8_t cmd[4];
                uint8_t bfr[5];
                sfr = strtoul(optarg, NULL, 0);
                if (offset && (sfr < 0x80 || sfr > 0xff)) {
                    printf("Invalid SFR.  "
                            "SFR addresses go between 0x80 and 0xff\n");
                    return -EINVAL;
                }
                if (!offset && (sfr < 0x00 || sfr > 0x7f)) {
                    printf("Invalid RAM address.  "
                            "RAM addresses go between 0x00 and 0x7f\n");
                    return -EINVAL;
                }
                xram_set(dbg, dbg->read_sfr_offset, sfr);
                memset(cmd, 0, sizeof(cmd));
                dbg_txrx(dbg, cmd_sfr_get, cmd, bfr, sizeof(bfr));
                if (ch == 'r') {
                    printf("%s_%02X: %02x\n",
                            offset?"SFR":"RAM", sfr, bfr[1]);
                }
                else if (ch == 'x') {
                    uint8_t num[4];

                    num[0] = bfr[1];

                    xram_set(dbg, dbg->read_sfr_offset, sfr+1);
                    dbg_txrx(dbg, cmd_sfr_get, cmd, bfr, sizeof(bfr));
                    num[1] = bfr[2];

                    xram_set(dbg, dbg->read_sfr_offset, sfr+2);
                    dbg_txrx(dbg, cmd_sfr_get, cmd, bfr, sizeof(bfr));
                    num[2] = bfr[3];

                    xram_set(dbg, dbg->read_sfr_offset, sfr+3);
                    dbg_txrx(dbg, cmd_sfr_get, cmd, bfr, sizeof(bfr));
                    num[3] = bfr[4];

                    printf("%s_%02X: %02x%02x%02x%02x\n", offset?"SFR":"RAM",
                            sfr, num[3], num[2], num[1], num[0]);
                }
            }
            break;

        default:
            printf("Usage: %s [-d] [-w offset:val] [-r offset]\n", argv[0]);
            return -EINVAL;
        }
    }
    return 0;
}

static int irq_print_statistics(struct dbg *dbg) {
    uint8_t *ptr;
    uint32_t counter;

    ptr = (uint8_t *)&counter;

    printf("IRQ Statistics:\n");

    printf("\tirq 0: %d\n", ram_get(dbg, 0x35));

    ptr[0] = ram_get(dbg, 0xc0);
    ptr[1] = ram_get(dbg, 0xc1);
    ptr[2] = ram_get(dbg, 0xc2);
    ptr[3] = ram_get(dbg, 0xc3);
    printf("\tirq 1: %d\n", counter);

    ptr[0] = ram_get(dbg, 0xc8);
    ptr[1] = ram_get(dbg, 0xc9);
    ptr[2] = ram_get(dbg, 0xca);
    ptr[3] = ram_get(dbg, 0xcb);
    printf("\tirq 2: %d\n", counter);

    ptr[0] = ram_get(dbg, 0xd8);
    ptr[1] = ram_get(dbg, 0xd9);
    ptr[2] = ram_get(dbg, 0xda);
    ptr[3] = ram_get(dbg, 0xdb);
    printf("\tirq 3: %d\n", counter);

    ptr[0] = ram_get(dbg, 0xf8);
    ptr[1] = ram_get(dbg, 0xf9);
    ptr[2] = ram_get(dbg, 0xfa);
    ptr[3] = ram_get(dbg, 0xfb);
    printf("\tirq 4: %d\n", counter);
    return 0;
}

static int dbg_do_irq(struct dbg *dbg, int argc, char **argv) {
    int ch;
    int handled = 0;

    while ((ch = getopt(argc, argv, "rpm:")) != -1) {
        handled = 1;
        switch(ch) {
        case 'm': {
                uint8_t cmd[4];
                uint8_t bfr[5];
                cmd[0] = 1; /* Set mask command */
                cmd[1] = strtoul(optarg, NULL, 0);
                dbg_txrx(dbg, cmd_irq, cmd, bfr, sizeof(bfr));
                printf("Previous IRQ mask: 0x%02x\n", bfr[1]);
            }
            break;

        case 'p':
            irq_print_statistics(dbg);
            break;

        case 'r':
            ram_set(dbg, 0x35, 0);

            ram_set(dbg, 0xc0, 0);
            ram_set(dbg, 0xc1, 0);
            ram_set(dbg, 0xc2, 0);
            ram_set(dbg, 0xc3, 0);

            ram_set(dbg, 0xc8, 0);
            ram_set(dbg, 0xc9, 0);
            ram_set(dbg, 0xca, 0);
            ram_set(dbg, 0xcb, 0);

            ram_set(dbg, 0xd8, 0);
            ram_set(dbg, 0xd9, 0);
            ram_set(dbg, 0xda, 0);
            ram_set(dbg, 0xdb, 0);

            ram_set(dbg, 0xf8, 0);
            ram_set(dbg, 0xf9, 0);
            ram_set(dbg, 0xfa, 0);
            ram_set(dbg, 0xfb, 0);
            printf("IRQ statistics reset\n");
            break;

        default:
            printf("Usage: %s [-p] [-r] [-m mask]\n", argv[0]);
            return -EINVAL;
        }
    }

    if (!handled)
        irq_print_statistics(dbg);

    return 0;
}

static int gpio_print_state(struct dbg *dbg) {
    uint8_t values = sd_read_pins(dbg->sd);
    printf("GPIO pin state:\n");
    printf("\tDAT0: %d\n", !!(values & 1));
    printf("\tDAT1: %d\n", !!(values & 2));
    printf("\tDAT2: %d\n", !!(values & 4));
    printf("\tDAT3: %d\n", !!(values & 8));
    return 0;
}

static int dbg_do_gpio(struct dbg *dbg, int argc, char **argv) {
    int ch;
    int handled = 0;

    while ((ch = getopt(argc, argv, "di:o:s:c:")) != -1) {
        uint8_t dat_n;
        handled = 1;
        switch(ch) {
        case 'i':
            dat_n = strtoul(optarg, NULL, 0);
            if (dat_n > 3) {
                printf("Must specify 0 - 3\n");
                return -EINVAL;
            }
            sd_set_dat_input(dbg->sd, dat_n);
            break;

        case 'o':
            dat_n = strtoul(optarg, NULL, 0);
            if (dat_n > 3) {
                printf("Must specify 0 - 3\n");
                return -EINVAL;
            }
            sd_set_dat_output(dbg->sd, dat_n);
            break;

        case 's':
            dat_n = strtoul(optarg, NULL, 0);
            if (dat_n > 3) {
                printf("Must specify 0 - 3\n");
                return -EINVAL;
            }
            sd_set_dat_value(dbg->sd, dat_n, 1);
            break;

        case 'c':
            dat_n = strtoul(optarg, NULL, 0);
            if (dat_n > 3) {
                printf("Must specify 0 - 3\n");
                return -EINVAL;
            }
            sd_set_dat_value(dbg->sd, dat_n, 0);
            break;

        case 'd':
            gpio_print_state(dbg);
            break;

        default:
            printf("Try 'help %s'\n", argv[0]);
            return -EINVAL;
        }
    }

    if (!handled)
        gpio_print_state(dbg);

    return 0;
}

static int dbg_do_nand(struct dbg *dbg, int argc, char **argv) {
    int ch;
    uint8_t cmd[4];
    uint8_t bfr[5];
    int ret = 0;
    while ((ch = getopt(argc, argv, "rlsdt:vc:w:")) != -1) {
        switch(ch) {
        case 'w': {
            uint32_t src; // Source address in the AX215
            uint8_t ram_buffer[512+3];
            int ram_bytes = sizeof(ram_buffer)-3;
            uint16_t crc;
            char *next;

            src = strtoul(optarg, &next, 0);

            xram_get(dbg, ram_buffer, src, ram_bytes);
            crc = crc16(ram_buffer, ram_bytes);
            crc = htons(crc);
            memcpy(&ram_buffer[ram_bytes], &crc, sizeof(crc));
            ram_buffer[ram_bytes+2] = 0x0e;

            xram_set(dbg, src+ram_bytes+0, ram_buffer[ram_bytes+0]);
            xram_set(dbg, src+ram_bytes+1, ram_buffer[ram_bytes+1]);
            xram_set(dbg, src+ram_bytes+2, ram_buffer[ram_bytes+2]);

            cmd[0] = 5;
            cmd[1] = ((src/8)>>0)&0xff;
            cmd[2] = ((src/8)>>8)&0xff;
            ret = dbg_txrx(dbg, cmd_nand, cmd, bfr, sizeof(bfr));
            }
            break;

        case 'r': {
            }
            break;

        case 'c': {
                uint32_t addr = 0x1000;
                int i;
                int sfr = strtoul(optarg, NULL, 0);
                for (i = 255; i >= 0; i--) {

                    ram_set(dbg, sfr, i);

                    memset(cmd, 0, sizeof(cmd));
                    cmd[0] = 0xaf;
                    cmd[1] = ((addr/8)>>0)&0xff;
                    cmd[2] = ((addr/8)>>8)&0xff;
                    ret = dbg_txrx(dbg, cmd_nand, cmd, bfr, sizeof(bfr));
                    printf("NCMD 0x%02x: ", i);
                }
            }
            break;

        case 't': {
                uint32_t addr = 0;
                char *addr_str;
                memset(cmd, 0, sizeof(cmd));
                cmd[0] = strtoul(optarg, &addr_str, 0);
                if (addr_str)
                    addr = strtoul(addr_str+1, NULL, 0)/8;
                cmd[1] = (addr>>0)&0xff;
                cmd[2] = (addr>>8)&0xff;
                ret = dbg_txrx(dbg, cmd_nand, cmd, bfr, sizeof(bfr));
            }
            break;
            
        default:
            break;
        }
    }
    return ret;
}

static int dbg_do_hello(struct dbg *dbg, int argc, char **argv) {
    uint8_t args[4];
    uint8_t response[5];
    memset(args, 0, sizeof(args));
    if (argc > 1)
        args[0] = strtoul(argv[1], NULL, 0);
    if (argc > 2)
        args[1] = strtoul(argv[2], NULL, 0);
    if (argc > 3)
        args[2] = strtoul(argv[3], NULL, 0);
    if (argc > 4)
        args[3] = strtoul(argv[4], NULL, 0);

    dbg_txrx(dbg, cmd_hello, args, response, sizeof(response));

    printf("CPU -> AX215: {%02x %02x %02x %02x}\n", args[0], args[1], args[2], args[3]);
    printf("CPU <- AX215: {%02x %02x %02x %02x}\n", response[0], response[1], response[2], response[3]);

    return 0;
}

static int dbg_do_null(struct dbg *dbg, int argc, char **argv) {
    uint8_t args[4];
    uint8_t bfr[5];
    memset(args, 0, sizeof(args));
    dbg_txrx(dbg, cmd_null, args, bfr, sizeof(bfr));
    printf("Card response: {%02x %02x %02x %02x}\n", bfr[0], bfr[1], bfr[2], bfr[3]);
    return 0;
}


static int dbg_do_dump_rom(struct dbg *dbg, int argc, char **argv) {
    uint8_t ram[16384];
    memset(ram, 0, sizeof(ram));

    if (argc != 2) {
        printf("Usage: %s [romfile]\n", argv[0]);
        return -EINVAL;
    }

    int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (-1 == fd) {
        perror("Unable to open output file");
        return -errno;
    }

    xram_get(dbg, ram, 0, sizeof(ram));
    if (-1 == write(fd, ram, sizeof(ram)))
        perror("Unable to write to dumpfile");
    close(fd);

    return 0;
}


static int dbg_do_peek(struct dbg *dbg, int argc, char **argv) {
    int src;
    int cnt;

    if (argc != 3) {
        printf("Usage: %s [offset] [count]\n", argv[0]);
        return -EINVAL;
    }

    src = strtoul(argv[1], NULL, 0);
    cnt = strtoul(argv[2], NULL, 0);

    if ((src+cnt) > 65536) {
        printf("Attempt to read past end of RAM\n");
        return -ERANGE;
    }

    if (cnt < 1) {
        printf("Can't read negative bytes\n");
        return -ERANGE;
    }
    if (src < 0) {
        printf("Source address is negative\n");
        return -ERANGE;
    }

    uint8_t bfr[cnt+1];
    xram_get(dbg, bfr, src, cnt);
    print_hex_offset(bfr, cnt, src);

    return 0;
}

static int dbg_do_disasm(struct dbg *dbg, int argc, char **argv) {
    int count;
    int offset;
    int ret;

    if (argc != 3) {
        printf("Usage: %s [offset] [byte_count]\n", argv[0]);
        return -EINVAL;
    }

    offset = strtoul(argv[1], NULL, 0);
    count = strtoul(argv[2], NULL, 0);

    if (count <= 0 || count > 16384) {
        printf("Attempted to read too many bytes\n");
        return -ENOSPC;
    }

    uint8_t bfr[count];
    ret = xram_get(dbg, bfr, offset, count);
    if (ret)
        return ret;

    disasm_8051(stdout, bfr, count, offset);
    return 0;
}

static int dbg_do_memset(struct dbg *dbg, int argc, char **argv) {
    int offset;
    uint8_t pattern;
    int count;

    if (argc != 4) {
        printf("Usage: %s [start] [pattern] [count]\n", argv[0]);
        return -EINVAL;
    }

    offset = strtoul(argv[1], NULL, 0);
    pattern = strtoul(argv[2], NULL, 0);
    count = strtoul(argv[3], NULL, 0);

    if (offset < 0 || offset >= 16384) {
        printf("Start must be between 0 and 16383 bytes\n");
        return -ERANGE;
    }
    if (count<0 || offset+count >= 16384) {
        printf("Number of bytes is out of range\n");
        return -ERANGE;
    }

    while (count--)
        xram_set(dbg, offset++, pattern);
    return 0;
}

static int dbg_do_poke(struct dbg *dbg, int argc, char **argv) {
    int offset;
    uint8_t value;
    int old_value;

    if (argc < 3) {
        printf("Not enough arguments!  Usage: %s [addr] [count]\n",
                argv[0]);
        return -EINVAL;
    }

    offset = strtoul(argv[1], NULL, 0);
    value = strtoul(argv[2], NULL, 0);
    old_value = xram_set(dbg, offset, value);
    printf("Offset 0x%04x 0x%02x -> 0x%02x\n", offset, old_value, value);
    return 0;
}

static int dbg_do_jump(struct dbg *dbg, int argc, char **argv) {
    printf("Jumping...\n");
    return 0;
}

static int dbg_do_help(struct dbg *dbg, int argc, char **argv) {
    struct debug_command *cmd = debug_commands;

    if (argc > 1) {
        int found = 0;
        while (cmd->func) {
            if (!strcmp(argv[1], cmd->name)) {
                found = 1;

                if (cmd->help) {
                    printf("Help for %s:\n", cmd->name);
                    printf("%s", cmd->help);
                    printf("\n");
                }
                else {
                    printf("No help for \"%s\"\n", cmd->name);
                }
            }
            cmd++;
        }

        if (!found) {
            printf("Help not found for \"%s\"\n", argv[1]);
        }
    }

    else {
        printf("List of available commands:\n");
        while (cmd->func) {
            printf("%8s  %s\n", cmd->name, cmd->desc);
            cmd++;
        }
        printf("For more information on a specific command, type 'help [command]'\n");
    }

    return 0;
}

static int dbg_do_reset(struct dbg *dbg, int argc, char **argv) {
    dbg->should_quit = 1;
    dbg->ret = -EAGAIN;
    return 0;
}



static char *cmd_completion_generator(const char *text, int state) {
    static int index;
    static int text_len;
    if (!state) {
        index = 0;
        text_len = strlen(text);
    }

    while (debug_commands[index].func) {
        if (!strncmp(debug_commands[index].name, text, text_len))
            return strdup(debug_commands[index++].name);
        index++;
    }
    return NULL;
}

char **cmd_completion(const char *text, int start, int end) {
    if (start != 0)
        return NULL;
    return rl_completion_matches(text, cmd_completion_generator);
}

/* Look through RAM to find our sentinals.  They are patterns of code
 * that indicate where to find various offsets.
 * Because the 8051 can't dynamically read from IRAM, we actually patch
 * the code just before we call it in order to be able to read from an
 * arbitrary area of memory.
 */
static int find_fixups(struct dbg *dbg) {
    uint8_t program_memory[512];
    int i;
    int ret = 0;
    int found_sfr_get = 0;
    int found_sfr_set = 0;
    int found_ext_op = 0;

    printf("Locating fixup hooks... "); fflush(stdout);
    memset(program_memory, 0, sizeof(program_memory));
    ret = xram_get(dbg, program_memory, PROGRAM_OFFSET, sizeof(program_memory));
    if (ret) {
        printf("Failed to dump program memory\n");
        return ret;
    }

    for (i = 0; i < sizeof(program_memory); i++) {
        // Special charachter for our detection.
        if (program_memory[i] == 0xa5
            && program_memory[i + 1] == 0x60
            && program_memory[i + 2] == 0x61) {
            found_sfr_get = 1;
            /*
             * Patch program to read:
             *      mov     [source_sfr], 0x21
             */
            xram_set(dbg, PROGRAM_OFFSET + i, 0x85);        // mov opcode
            dbg->read_sfr_offset = PROGRAM_OFFSET + i + 1;  // Source RAM
            xram_set(dbg, PROGRAM_OFFSET + i + 2, 0x21);    // Target address
        }
        if (program_memory[i] == 0xa5
            && program_memory[i + 1] == 0x62
            && program_memory[i + 2] == 0x63) {
            found_sfr_set = 1;
            /*
             * Patch program to read:
             *      mov     0x31, [destination_sfr]
             */
            xram_set(dbg, PROGRAM_OFFSET + i, 0x85);        // mov opcode
            xram_set(dbg, PROGRAM_OFFSET + i + 1, 0x31);    // Source register
            dbg->write_sfr_offset = PROGRAM_OFFSET + i + 2; // Value
        }
        if (program_memory[i] == 0xa5
            && program_memory[i + 1] == 0x64
            && program_memory[i + 2] == 0x65) {
            found_ext_op = 1;
            dbg->ext_op_offset = PROGRAM_OFFSET + i;
        }
    }

    if (!found_sfr_get) {
        ret = -ERANGE;
        printf(" [couldn't find sfr_get opcodes] ");
    }
    if (!found_sfr_set) {
        ret = -ERANGE;
        printf(" [couldn't find sfr_set opcodes] ");
    }
    if (!found_ext_op) {
        ret = -ERANGE;
        printf(" [couldn't find ext_op opcodes] ");
    }
    printf("Done\n");

    printf("Program memory (0x%x - 0x%x):\n",
            PROGRAM_OFFSET, PROGRAM_OFFSET + sizeof(program_memory));
    print_hex(program_memory, sizeof(program_memory));

    return ret;
}


static int validate_communication(struct dbg *dbg) {
    int i;
    printf("Checking card communication... ");

    for (i = 1; i <= 2; i++) {
        uint8_t args[4] = {'H', 'E', 'L', 'O'};
        uint8_t response[5];

        printf("%d... ", i);
        dbg_txrx(dbg, cmd_hello, args, response, sizeof(response));

        if (response[1] != args[0]
                || response[2] != args[1]
                || response[3] != args[2]
                || response[4] != args[3]) {
            printf("Error: sent 0x%02x%02x%02x%02x, ",
                args[0], args[1], args[2], args[3]);
            printf("received 0x%02x%02x%02x%02x\n",
                response[1], response[2], response[3], response[4]);
            return -1;
        }
    }
    printf("Okay\n");

    return 0;
}

static int install_isrs(struct dbg *dbg) {
    printf("Installing interrupt service routines... [ ");
    xram_set(dbg, 0x00, 0x05);  // INC [iram_addr]
    xram_set(dbg, 0x01, 0x35);  // iram_addr = 0x35
    xram_set(dbg, 0x02, 0x32);  // RETI
    ram_set(dbg, 0x35, 0);      // Reset count
    printf("RESET ");

    xram_set(dbg, 0x03, 0xa5);  // ext_op
    xram_set(dbg, 0x04, 0x12);  // INC32 ER0
    xram_set(dbg, 0x05, 0x32);  // RETI
    ram_set(dbg, 0xc0, 0);      // Reset count
    ram_set(dbg, 0xc1, 0);      // Reset count
    ram_set(dbg, 0xc2, 0);      // Reset count
    ram_set(dbg, 0xc3, 0);      // Reset count
    printf("SDI ");

    xram_set(dbg, 0x0b, 0xa5);  // ext_op
    xram_set(dbg, 0x0c, 0x16);  // INC32 ER1
    xram_set(dbg, 0x0d, 0x32);  // RETI
    ram_set(dbg, 0xc8, 0);      // Reset count
    ram_set(dbg, 0xc9, 0);      // Reset count
    ram_set(dbg, 0xca, 0);      // Reset count
    ram_set(dbg, 0xcb, 0);      // Reset count
    printf("SDIX ");

    xram_set(dbg, 0x13, 0xa5);  // ext_op
    xram_set(dbg, 0x14, 0x1a);  // INC32 ER2
    xram_set(dbg, 0x15, 0x32);  // RETI
    ram_set(dbg, 0xd8, 0);      // Reset count
    ram_set(dbg, 0xd9, 0);      // Reset count
    ram_set(dbg, 0xda, 0);      // Reset count
    ram_set(dbg, 0xdb, 0);      // Reset count
    printf("NAND ");

    xram_set(dbg, 0x1b, 0xa5);  // ext_op
    xram_set(dbg, 0x1c, 0x1e);  // INC32 ER3
    xram_set(dbg, 0x1d, 0x32);  // RETI
    ram_set(dbg, 0xf8, 0);      // Reset count
    ram_set(dbg, 0xf9, 0);      // Reset count
    ram_set(dbg, 0xfa, 0);      // Reset count
    ram_set(dbg, 0xfb, 0);      // Reset count
    printf("UNK ");

    printf("] Okay\n");
    return 0;
}

int dbg_main(struct sd_state *sd) {
    static struct dbg dbg;

    if (!dbg.initialized) {
        memset(&dbg, 0, sizeof(dbg));

        rl_attempted_completion_function = cmd_completion;
        rl_bind_key('\t', rl_complete);

        dbg.initialized = 1;
    }

    dbg.sd = sd;
    dbg.should_quit = 0;
    dbg.ret = 0;
    dbg.fixed_up = 0;

    if (validate_communication(&dbg) < 0)
        return -EAGAIN;

    if (find_fixups(&dbg))
        return -EAGAIN;

    if (install_isrs(&dbg))
        return -EAGAIN;

    dbg.fixed_up = 1;

    while (!dbg.should_quit) {
        char *cmd = readline(DBG_PROMPT);
        wordexp_t cmdline;
        int i;
        int ret = -ENOENT;
        int exp_res;

        // EOF (or ^D)
        if (!cmd) {
            printf("\n");
            return 0;
        }

        if (!*cmd)
            continue;

        add_history(cmd);
        exp_res = wordexp(cmd, &cmdline, 0);
        optind = 0;

        if (exp_res == WRDE_BADCHAR) {
            printf("Illegal character found in command\n");
            continue;
        }
        else if (exp_res == WRDE_BADVAL) {
            printf("Undefined variable in commaind\n");
            continue;
        }
        else if (exp_res == WRDE_CMDSUB) {
            printf("Did command substitution, and we weren't supposed to\n");
            continue;
        }
        else if (exp_res == WRDE_NOSPACE) {
            printf("Out of memory\n");
            continue;
        }
        else if (exp_res == WRDE_SYNTAX) {
            printf("Syntax error in command\n");
            continue;
        }
        else if (exp_res) {
            printf("Unrecognized error occurred in command parsing\n");
            continue;
        }


        for (i=0; debug_commands[i].func; i++)
            if (!strcmp(cmdline.we_wordv[0], debug_commands[i].name))
                ret = debug_commands[i].func(&dbg,
                        cmdline.we_wordc,
                        cmdline.we_wordv);

        if (ret == -ENOENT)
            printf("Command not found.  Type 'help' for a list of commands.\n");

        wordfree(&cmdline);
    }
    return dbg.ret;
}
