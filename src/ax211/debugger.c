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
#include "nand.h"

#define DBG_PROMPT "AX211> "

struct dbg {
    int             should_quit;
    int             ret;
    struct sd_state *sd;
    struct nand     *nand;

    int             read_sfr_offset;
    int             write_sfr_offset;
    int             ext_op_offset;

    int             initialized; // False if uninitialized
};

/* These are SD commands as they get sent to the card */
enum protocol_code {
    cmd_null = 0,
    cmd_hello = 1,
    cmd_peek = 2,
    cmd_poke = 3,
    cmd_jump = 4,
    cmd_nand = 5,
    cmd_sfr_set = 6,
    cmd_sfr_get = 7,
    cmd_ext_op  = 8,
};

struct debug_command {
    char    *name;
    char    *desc;
    char    *help;
    int (*func)(struct dbg *dbg, int argc, char **argv);
};

int disasm_8051(FILE *ofile, uint8_t *bfr, int size, int offset);

static int dbg_do_hello(struct dbg *dbg, int argc, char **argv);
static int dbg_do_null(struct dbg *dbg, int argc, char **argv);
static int dbg_do_peek(struct dbg *dbg, int argc, char **argv);
static int dbg_do_poke(struct dbg *dbg, int argc, char **argv);
static int dbg_do_jump(struct dbg *dbg, int argc, char **argv);
static int dbg_do_help(struct dbg *dbg, int argc, char **argv);
static int dbg_do_memset(struct dbg *dbg, int argc, char **argv);
static int dbg_do_disasm(struct dbg *dbg, int argc, char **argv);
static int dbg_do_nand(struct dbg *dbg, int argc, char **argv);
static int dbg_do_sfr(struct dbg *dbg, int argc, char **argv);
static int dbg_do_reset(struct dbg *dbg, int argc, char **argv);
static int dbg_do_dump_rom(struct dbg *dbg, int argc, char **argv);
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
        .help = "Dumps all of the AX211's 16 kB to a file.\n"
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
                "   -r sfr      Read SFR [sfr]\n"
                "   -x sfr      Read extended (quad-wide-wide) sfr\n"
                ,
    },
    {
        .name = "nand",
        .func = dbg_do_nand,
        .desc = "Operate on the NAND in some fashion",
        .help = "Usage: nand [-r] [-l] [-s] [-d] [-v] [-c] [-t cmd:addr] -w [src:addr]\n"
                "   -r            Reset NAND logging\n"
                "   -l            Begin NAND logging\n"
                "   -s            Stop NAND logging\n"
                "   -d            Dump NAND events\n"
                "   -v            Print NAND status\n"
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
        .name = "reset",
        .func = dbg_do_reset,
        .desc = "Reset the AX211 card",
        .help = "Call to reset/restart the AX211 CPU\n",
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
    uint8_t ret[outsize+1]; // Two extra bytes for start and crc7
    uint8_t bfr[6];
    int tries;

    memset(ret, 0, sizeof(ret));
    memset(out, 0, outsize);

    code = (code&0x3f)|0x40;
    bfr[0] = code;
    bfr[1] = args[0];
    bfr[2] = args[1];
    bfr[3] = args[2];
    bfr[4] = args[3];
    bfr[5] = (crc7(bfr, 5)<<1)|1;

    xmit_mmc_cmd(dbg->sd, bfr, sizeof(bfr));
    tries = rcvr_mmc_cmd_start(dbg->sd, 32);
    if (tries == -1) {
        printf("Never got start!\n");
        return -1;
    }
    rcvr_mmc_cmd(dbg->sd, ret, sizeof(ret));
    memcpy(out, ret+1, outsize);
    return 0;
}


// Set a value in XRAM
int xram_set(struct dbg *dbg, int offset, uint8_t val) {
    int src_lo, src_hi;
    uint8_t args[4];
    uint8_t bfr[5];

    src_hi = (offset>>8)&0xff;
    src_lo = (offset)&0xff;

    args[0] = src_hi;
    args[1] = src_lo;
    args[2] = val;
    args[3] = 0;
    if (dbg_txrx(dbg, cmd_poke, args, bfr, sizeof(bfr)))
        return -1;
    return bfr[0];
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
    while (count>0) {
        uint8_t args[4];
        uint8_t out[5];
        args[0] = (offset>>8)&0xff;
        args[1] = (offset>>0)&0xff;
        ret = dbg_txrx(dbg, cmd_peek, args, out, sizeof(out));
        if (ret)
            return ret;
        memcpy(buf, out, (count>4?4:count));
        buf+=4;
        offset+=4;
        count-=4;
    }
    return 0;
}




static int dbg_do_ext_op(struct dbg *dbg, int argc, char **argv) {
    uint8_t op1;
    uint8_t op2;
    uint8_t cmd[4];
    uint8_t bfr[5];
    int ret;

    if (argc < 2) {
        printf("Usage: %s op1 [op2]\n", argv[0]);
        return -EINVAL;
    }

    op1 = strtoul(argv[1], NULL, 0);
    op2 = 0x00; // nop opcode
    if (argc > 2) {
        op2 = strtoul(argv[2], NULL, 0);
        op1 |= 0xf0;
    }

    xram_set(dbg, dbg->ext_op_offset, 0xa5);
    xram_set(dbg, dbg->ext_op_offset+1, op1);
    xram_set(dbg, dbg->ext_op_offset+2, op2);

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

    while ((ch = getopt(argc, argv, "dw:r:x:")) != -1) {
        switch(ch) {
            case 'd':
                for (sfr=0; sfr<=127; sfr++) {
                    uint8_t cmd[4];
                    uint8_t bfr[5];
                    xram_set(dbg, dbg->read_sfr_offset, sfr+offset);
                    memset(cmd, 0, sizeof(cmd));
                    dbg_txrx(dbg, cmd_sfr_get, cmd, bfr, sizeof(bfr));
                    sfr_table[sfr] = bfr[0];
                }
                print_hex_offset(sfr_table, sizeof(sfr_table), offset);
                break;

            case 'w': {
                    char *endptr;
                    int sfr = strtoul(optarg, &endptr, 0);
                    int val;
                    if (offset && (sfr<0x80 || sfr>0xff)) {
                        printf("Invalid SFR.  SFR values go between 0x80 and 0xff\n");
                        return -EINVAL;
                    }
                    if (!offset && (sfr<0x00 || sfr>0x7f)) {
                        printf("Invalid RAM address.  RAM values go between 0x00 and 0x7f\n");
                        return -EINVAL;
                    }
                    if (!endptr) {
                        printf("No value specified\n");
                        return -EINVAL;
                    }
                    val = strtoul(endptr+1, NULL, 0);

                    printf("Setting %s_%02x -> %02x\n", offset?"SFR":"RAM", sfr, val);

                    ram_set(dbg, sfr, val);
                }
                break;

            case 'x':
            case 'r': {
                    uint8_t cmd[4];
                    uint8_t bfr[5];
                    sfr = strtoul(optarg, NULL, 0);
                    if (offset && (sfr<0x80 || sfr>0xff)) {
                        printf("Invalid SFR.  SFR values go between 0x80 and 0xff\n");
                        return -EINVAL;
                    }
                    if (!offset && (sfr<0x00 || sfr>0x7f)) {
                        printf("Invalid RAM address.  RAM values go between 0x00 and 0x7f\n");
                        return -EINVAL;
                    }
                    xram_set(dbg, dbg->read_sfr_offset, sfr);
                    memset(cmd, 0, sizeof(cmd));
                    dbg_txrx(dbg, cmd_sfr_get, cmd, bfr, sizeof(bfr));
                    if (ch == 'r') {
                        printf("%s_%02X: %02x\n", offset?"SFR":"RAM", sfr, bfr[0]);
                    }
                    else if (ch == 'x') {
                        uint8_t num[4];

                        num[0] = bfr[0];

                        xram_set(dbg, dbg->read_sfr_offset, sfr+1);
                        dbg_txrx(dbg, cmd_sfr_get, cmd, bfr, sizeof(bfr));
                        num[1] = bfr[0];

                        xram_set(dbg, dbg->read_sfr_offset, sfr+2);
                        dbg_txrx(dbg, cmd_sfr_get, cmd, bfr, sizeof(bfr));
                        num[2] = bfr[0];

                        xram_set(dbg, dbg->read_sfr_offset, sfr+3);
                        dbg_txrx(dbg, cmd_sfr_get, cmd, bfr, sizeof(bfr));
                        num[3] = bfr[0];

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

static int dbg_do_nand(struct dbg *dbg, int argc, char **argv) {
    int ch;
    uint8_t cmd[4];
    uint8_t bfr[5];
    int ret = 0;
    while ((ch = getopt(argc, argv, "rlsdt:vc:w:")) != -1) {
        switch(ch) {
            case 'r':
                nand_log_reset(dbg->nand);
                break;

            case 'l':
                nand_log_enable(dbg->nand);
                break;

            case 's':
                nand_log_disable(dbg->nand);
                break;

            case 'd':
                nand_log_dump(dbg->nand);
                break;

            case 'w': {
                    uint32_t src; // Source address in the AX211
                    uint32_t dst; // Destination address in NAND
                    uint8_t ram_buffer[512+3];
                    int ram_bytes = sizeof(ram_buffer)-3;
                    uint16_t crc;
                    char *next;

                    src = strtoul(optarg, &next, 0);
                    dst = 0;
                    if (next)
                        dst = strtoul(next+1, NULL, 0);

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
                    nand_log_reset(dbg->nand);
                    nand_log_enable(dbg->nand);
                    ret = dbg_txrx(dbg, cmd_nand, cmd, bfr, sizeof(bfr));
                    nand_log_dump(dbg->nand);
                }
                break;

            case 'c': {
                    uint32_t addr = 0x1000;
                    int i;
                    int sfr = strtoul(optarg, NULL, 0);
                    for (i=255; i>=0; i--) {

                        ram_set(dbg, sfr, i);

                        memset(cmd, 0, sizeof(cmd));
                        cmd[0] = 0xaf;
                        cmd[1] = ((addr/8)>>0)&0xff;
                        cmd[2] = ((addr/8)>>8)&0xff;
                        nand_log_reset(dbg->nand);
                        nand_log_enable(dbg->nand);
                        ret = dbg_txrx(dbg, cmd_nand, cmd, bfr, sizeof(bfr));
                        printf("NCMD 0x%02x: ", i);
                        nand_log_summarize(dbg->nand);
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
                    nand_log_reset(dbg->nand);
                    nand_log_enable(dbg->nand);
                    ret = dbg_txrx(dbg, cmd_nand, cmd, bfr, sizeof(bfr));
                    nand_log_dump(dbg->nand);
                }
                break;
                

            case 'v': {
                struct timespec ts;
                nand_log_gettime(dbg->nand, &ts);
                printf("FPGA time: %ld.%09ld\n", ts.tv_sec, ts.tv_nsec);
                printf("%d records sitting in log buffer\n", nand_log_count(dbg->nand));
                printf("Log %s full\n", nand_log_is_full(dbg->nand)?"is":"is not");
                printf("Log status is: %x\n", nand_log_status(dbg->nand));
                printf("Max data depth: %d\n", nand_log_max_data_depth(dbg->nand));
                printf("Max command depth: %d\n", nand_log_max_cmd_depth(dbg->nand));
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

    printf("CPU -> AX211: {%02x %02x %02x %02x}\n", args[0], args[1], args[2], args[3]);
    printf("CPU <- AX211: {%02x %02x %02x %02x}\n", response[0], response[1], response[2], response[3]);

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
    write(fd, ram, sizeof(ram));
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

    if (src > 16384) {
        printf("AX211 only has 16384 bytes of RAM\n");
        return -ERANGE;
    }
    if ((src+cnt) > 16384) {
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
                    printf(cmd->help);
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
    ret = xram_get(dbg, program_memory, 0x2900, sizeof(program_memory));
    if (ret) {
        printf("Failed\n");
        return ret;
    }

    for (i=0; i<sizeof(program_memory); i++) {
        // Special charachter for our detection.
        if (program_memory[i] == 0xa5
            && program_memory[i+1] == 0x60
            && program_memory[i+2] == 0x61) {
            found_sfr_get = 1;
            xram_set(dbg, 0x2900+i, 0x85);  // mov
            dbg->read_sfr_offset = 0x2900+i+1;
            xram_set(dbg, 0x2900+i+2, 0x20);  // Destination register
        }
        if (program_memory[i] == 0xa5
            && program_memory[i+1] == 0x62
            && program_memory[i+2] == 0x63) {
            found_sfr_set = 1;
            xram_set(dbg, 0x2900+i, 0x85);  // mov
            xram_set(dbg, 0x2900+i+1, 0x20);  // Source register
            dbg->write_sfr_offset = 0x2900+i+2;
        }
        if (program_memory[i] == 0xa5
            && program_memory[i+1] == 0x64
            && program_memory[i+2] == 0x65) {
            found_ext_op = 1;
            dbg->ext_op_offset = 0x2900+i;
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
    return ret;
}


int dbg_main(struct sd_state *sd) {
    static struct dbg dbg;

    if (!dbg.initialized) {
        memset(&dbg, 0, sizeof(dbg));

        dbg.nand = nand_init();

        rl_attempted_completion_function = cmd_completion;
        rl_bind_key('\t', rl_complete);

        dbg.initialized = 1;
    }

    dbg.sd = sd;
    dbg.should_quit = 0;
    dbg.ret = 0;

    if (find_fixups(&dbg))
        return -EAGAIN;

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
