#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "sd.h"
#include "gpio.h"
#include "eim.h"
#include "crc-16.h"

/* CHUMBY_BEND - 89
 * IR_DETECT - 102
 */

/* Pin connection:
 * SD  | MX233
 * 9   | 0
 * 1   | 1
 * 2   | 2
 * 3   | GND
 * DET | 3
 * 4   | [power switch]
 * 5   | 4
 * 6   | GND
 * 7   | 7
 * 8   | NC (was: 6)
 */

/** Definitions for Kovan test jig */
/*
#define CS_PIN 50
#define MISO_PIN 62
#define CLK_PIN 46
#define MOSI_PIN 48
#define POWER_PIN 55
*/

/** Definitions for Novena EIM interface */
#define CS_PIN    GPIO_IS_EIM | 3
#define MISO_PIN  GPIO_IS_EIM | 0
#define CLK_PIN   GPIO_IS_EIM | 4
#define MOSI_PIN  GPIO_IS_EIM | 5
#define DAT1_PIN  GPIO_IS_EIM | 1
#define DAT2_PIN  GPIO_IS_EIM | 2
#define POWER_PIN 17 //GPIO1_IO17


// R1 Response Codes (from SD Card Product Manual v1.9 section 5.2.3.1)
#define R1_IN_IDLE_STATE    (1<<0)  // The card is in idle state and running initializing process.
#define R1_ERASE_RESET      (1<<1)  // An erase sequence was cleared before executing because of an out of erase sequence command was received.
#define R1_ILLEGAL_COMMAND  (1<<2)  // An illegal command code was detected
#define R1_COM_CRC_ERROR    (1<<3)  // The CRC check of the last command failed.
#define R1_ERASE_SEQ_ERROR  (1<<4)  // An error in the sequence of erase commands occured.
#define R1_ADDRESS_ERROR    (1<<5)  // A misaligned address, which did not match the block length was used in the command.
#define R1_PARAMETER        (1<<6)  // The command's argument (e.g. address, block length) was out of the allowed range for this card.


static int load_rom_file(struct sd_state *state, char *file) {
    int i;
    int fd;

    fd = open(file, O_RDONLY);
    if (-1 == fd) {
        perror("Couldn't open ROM file");
        return 1;
    }

    for (i=0; i<65536; i+=2) {
        read(fd, eim_get(fpga_romulator_base+i), 2);
    }

    close(fd);
    return 0;
}



static int print_header(uint8_t *bfr) {
	printf(" CMD %2d {%02x %02x %02x %02x %02x %02x}  ", bfr[0]&0x3f, bfr[0], bfr[1], bfr[2], bfr[3], bfr[4], bfr[5]);
	return 0;
}



static int send_cmdX(struct sd_state *state, 
		uint8_t cmd,
		uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4,
		int print_size) {
	uint8_t bfr[6];
	uint8_t out_bfr[print_size];
	int result;
	static int run = 0;
	memset(out_bfr, 0, sizeof(out_bfr));
	cmd = (cmd&0x3f)|0x40;
	bfr[0] = cmd;
	bfr[1] = a1;
	bfr[2] = a2;
	bfr[3] = a3;
	bfr[4] = a4;
	bfr[5] = (crc7(bfr, 5)<<1)|1;
	result = sd_dump_rom(state, bfr, sizeof(bfr), out_bfr, print_size);
	if (result!=-1 && !(result&R1_ILLEGAL_COMMAND)) {
		out_bfr[0] = result;
		printf("Run %-4d  ", run);
		print_header(bfr);
		print_hex(out_bfr, print_size);
	}
	run++;
	return result;
}


static int do_get_csd_cid(struct sd_state *state) {
	sd_reset(state, 2);

	printf("CSD:\n");
	send_cmdX(state, 9, 0, 0, 0, 0, 32);
	printf("\n");

	printf("CID:\n");
	send_cmdX(state, 10, 0, 0, 0, 0, 32);
	printf("\n");
	return 0;
}

static int drain_nand_addrs(struct sd_state *state) {
    int i;
    int val;
    int addrs;
    for (i=0; addrs; i++) {
        val = *eim_get(fpga_r_nand_adr_hi);
        addrs = *eim_get(fpga_r_nand_adr_stat)&eim_nand_adr_bits;
    }
    return val;
}


static int calculate_mmc_crc16(uint8_t *bfr, uint8_t *crc_bfr, int size) {
    uint8_t sub_bfr[4][size/4];
    uint16_t crcs[4];
    int i;
    int bit;
    memset(sub_bfr, 0, sizeof(sub_bfr));
    // De-interleave bfr into four arrays.
    // Every four bytes of bfr get turned into one byte of buffer.
    for (i=0; i<size; ) {
        for (bit=7; bit>=0; bit-=2) {
            sub_bfr[0][i/4] |= (!!(bfr[i]&0x80))<<(bit-0);
            sub_bfr[0][i/4] |= (!!(bfr[i]&0x08))<<(bit-1);

            sub_bfr[1][i/4] |= (!!(bfr[i]&0x40))<<(bit-0);
            sub_bfr[1][i/4] |= (!!(bfr[i]&0x04))<<(bit-1);

            sub_bfr[2][i/4] |= (!!(bfr[i]&0x20))<<(bit-0);
            sub_bfr[2][i/4] |= (!!(bfr[i]&0x02))<<(bit-1);

            sub_bfr[3][i/4] |= (!!(bfr[i]&0x10))<<(bit-0);
            sub_bfr[3][i/4] |= (!!(bfr[i]&0x01))<<(bit-1);

            i++;
        }
    }

    for (i=0; i<4; i++)
        crcs[i] = crc16(sub_bfr[i], size/4);

    crc_bfr[0] = 
            ((!!((crcs[0]>>8)&0x80))<<7)
          | ((!!((crcs[1]>>8)&0x80))<<6)
          | ((!!((crcs[2]>>8)&0x80))<<5)
          | ((!!((crcs[3]>>8)&0x80))<<4)
          | ((!!((crcs[0]>>8)&0x40))<<3)
          | ((!!((crcs[1]>>8)&0x40))<<2)
          | ((!!((crcs[2]>>8)&0x40))<<1)
          | ((!!((crcs[3]>>8)&0x40))<<0);
    crc_bfr[1] = 
            ((!!((crcs[0]>>8)&0x20))<<7)
          | ((!!((crcs[1]>>8)&0x20))<<6)
          | ((!!((crcs[2]>>8)&0x20))<<5)
          | ((!!((crcs[3]>>8)&0x20))<<4)
          | ((!!((crcs[0]>>8)&0x10))<<3)
          | ((!!((crcs[1]>>8)&0x10))<<2)
          | ((!!((crcs[2]>>8)&0x10))<<1)
          | ((!!((crcs[3]>>8)&0x10))<<0);
    crc_bfr[2] = 
            ((!!((crcs[0]>>8)&0x8))<<7)
          | ((!!((crcs[1]>>8)&0x8))<<6)
          | ((!!((crcs[2]>>8)&0x8))<<5)
          | ((!!((crcs[3]>>8)&0x8))<<4)
          | ((!!((crcs[0]>>8)&0x4))<<3)
          | ((!!((crcs[1]>>8)&0x4))<<2)
          | ((!!((crcs[2]>>8)&0x4))<<1)
          | ((!!((crcs[3]>>8)&0x4))<<0);
    crc_bfr[3] = 
            ((!!((crcs[0]>>8)&0x2))<<7)
          | ((!!((crcs[1]>>8)&0x2))<<6)
          | ((!!((crcs[2]>>8)&0x2))<<5)
          | ((!!((crcs[3]>>8)&0x2))<<4)
          | ((!!((crcs[0]>>8)&0x1))<<3)
          | ((!!((crcs[1]>>8)&0x1))<<2)
          | ((!!((crcs[2]>>8)&0x1))<<1)
          | ((!!((crcs[3]>>8)&0x1))<<0);

    crc_bfr[4] = 
            ((!!((crcs[0])&0x80))<<7)
          | ((!!((crcs[1])&0x80))<<6)
          | ((!!((crcs[2])&0x80))<<5)
          | ((!!((crcs[3])&0x80))<<4)
          | ((!!((crcs[0])&0x40))<<3)
          | ((!!((crcs[1])&0x40))<<2)
          | ((!!((crcs[2])&0x40))<<1)
          | ((!!((crcs[3])&0x40))<<0);
    crc_bfr[5] = 
            ((!!((crcs[0])&0x20))<<7)
          | ((!!((crcs[1])&0x20))<<6)
          | ((!!((crcs[2])&0x20))<<5)
          | ((!!((crcs[3])&0x20))<<4)
          | ((!!((crcs[0])&0x10))<<3)
          | ((!!((crcs[1])&0x10))<<2)
          | ((!!((crcs[2])&0x10))<<1)
          | ((!!((crcs[3])&0x10))<<0);
    crc_bfr[6] = 
            ((!!((crcs[0])&0x8))<<7)
          | ((!!((crcs[1])&0x8))<<6)
          | ((!!((crcs[2])&0x8))<<5)
          | ((!!((crcs[3])&0x8))<<4)
          | ((!!((crcs[0])&0x4))<<3)
          | ((!!((crcs[1])&0x4))<<2)
          | ((!!((crcs[2])&0x4))<<1)
          | ((!!((crcs[3])&0x4))<<0);
    crc_bfr[7] = 
            ((!!((crcs[0])&0x2))<<7)
          | ((!!((crcs[1])&0x2))<<6)
          | ((!!((crcs[2])&0x2))<<5)
          | ((!!((crcs[3])&0x2))<<4)
          | ((!!((crcs[0])&0x1))<<3)
          | ((!!((crcs[1])&0x1))<<2)
          | ((!!((crcs[2])&0x1))<<1)
          | ((!!((crcs[3])&0x1))<<0);
    return 0;
}

static int interesting_one_cycle(struct sd_state *state, int run, int seed) {
    uint8_t response[16];
    uint8_t file[512+(4*sizeof(uint16_t))];
	int i = 0;
    int ret;

    memset(response, 0, sizeof(response));
    memset(file, 0, sizeof(file));

    srand(seed);
    printf("\n\nRun %-4d  Seed: %8d\n", run, seed);

    // Actually enter factory mode (sends CMD63/APPO and waits for response)
    ret = sd_enter_factory_mode(state, run);
    if (-1 == ret) {
        printf("Couldn't enter factory mode\n");
        return -1;
    }

    int fd = open("TestBoot.bin", O_RDONLY);
    read(fd, file, 512);
    close(fd);

    // Randomly permute the areas we're setting
    file[0x4a] = rand()|0x80;
    file[0x4b] = (rand()&1)?0xFF:0x00;

    file[0x4d] = rand()|0x80;
    file[0x4e] = (rand()&1)?0xFF:0x00;

    file[0x56] = rand()|0x80;
    file[0x59] = rand()|0x80;
    file[0x5c] = rand()|0x80;
    file[0x5f] = rand()|0x80;
    file[0x65] = file[0x56];
    file[0x68] = file[0x59];
    file[0x6a] = file[0x5c];
    file[0x6d] = file[0x5f];

    /*
    file[0x50] = rand()|0x80;
    file[0x51] = rand();//(rand()&1)?0xFF:0x00;

    file[0x53] = rand()|0x80;
    file[0x54] = rand();//(rand()&1)?0xFF:0x00;
    */

    calculate_mmc_crc16(file, file+(sizeof(file)-8), sizeof(file)-8);
    xmit_mmc_dat4(state, file, sizeof(file));
    //rcvr_mmc_dat1_start(state, 256);
    rcvr_mmc_cmd(state, response, sizeof(response));

    printf("Result of factory mode: %d\n", ret);

    printf("File:\n");
    print_hex(file, sizeof(file));

    printf("\nResponse:\n");
    print_hex(response, sizeof(response));
    printf("mov FSR_%02x, #0x%02x\n", file[0x4a], file[0x4b]);
    printf("mov FSR_%02x, #0x%02x\n", file[0x4d], file[0x4e]);
    //printf("mov FSR_%02x, #0x%02x\n", file[0x50], file[0x51]);
    //printf("mov FSR_%02x, #0x%02x\n", file[0x53], file[0x54]);
    printf("mov FSR_%02x, #0xff/00\n", file[0x56]);
    printf("mov FSR_%02x, #0xff/00\n", file[0x59]);
    printf("mov FSR_%02x, #0xff/00\n", file[0x5c]);
    printf("mov FSR_%02x, #0xff/00\n", file[0x5f]);
    printf("Finished up run: %-4d  Seed: %8d\n", run, seed);
    usleep(1000000);

    return 0;
}

static int do_interestingness(struct sd_state *state, int *seed, int *loop) {
	int run;
    int val;
    int s;

    // Reset the card...
    //sd_reset(state, 1);
    //blind_reset(state, NULL, NULL, 0);

    if (seed)
        val = interesting_one_cycle(state, loop?*loop:0, *seed);
    else if (loop)
        val = interesting_one_cycle(state, *loop, rand());
    else {
        val = 0;
        run = 0;
        for (; !val && run<24576-512; run++) {
            s = rand();
            val = interesting_one_cycle(state, run, s);

            // If we get an interesting value, try running it again.
            if (val>0) {
                printf("Potentially interesting.  Trying seed %d again...\n", s);
                val = interesting_one_cycle(state, run, s);

                // If we get an interesting value, try running it again.
                if (val>0) {
                    printf("Still potentially interesting.  Trying seed %d one last time...\n", s);
                    val = interesting_one_cycle(state, run, s);
                }
            }

            // Re-run when val<0, as it means the card didn't go ready
            if (val<0) {
                run--;
                val = 0;
            }
        }
    }

    return val;
}

static int do_one_ecc_knock(struct sd_state *state, int seed, int run) {
    int i;
    int addrs;
    uint8_t ecc_region[16] = {
        //0xc1, 0x77, 0xdb, 0xf5, 0xda, 0x4a, 0x4c, 0xfe,
        //0xd6, 0x1c, 0x02, 0xec, 0x9f, 0x8a, 0xca, 0xa1,
        0x7f, 0xa1, 0x8e, 0xbe, 0x74, 0x9d, 0x5f, 0x07,
        0xd7, 0xf6, 0xd1, 0x81, 0x12, 0x59, 0x5e, 0xf9
    };

    printf("\n\nRun %-4d  Seed: %8d\n", run, seed);
    srand(seed);

    for (i=0; i<sizeof(ecc_region); i++)
        ecc_region[i] = rand();
    ecc_region[2] &= ~0x0f;
    ecc_region[2] |= 0x0e;
    memcpy(eim_get(fpga_romulator_base+0x200), ecc_region, sizeof(ecc_region));

    printf("ECC data: ");
    print_hex(ecc_region, sizeof(ecc_region));

    // Drain address data
    addrs = *eim_get(fpga_r_nand_adr_stat)&eim_nand_adr_bits;
    printf("Pre-draining %d addresses\n", addrs);
    drain_nand_addrs(state);

    printf("Resetting...\n");
    sd_reset(state, 1);
    printf("Sending CMD9...\n");
    send_cmdX(state, 9, 0, 0, 0, 0, 32);
    usleep(50000);

    addrs = *eim_get(fpga_r_nand_adr_stat)&eim_nand_adr_bits;
    printf("Draining %d addresses\n", addrs);
    drain_nand_addrs(state);

    usleep(50000);
    addrs = *eim_get(fpga_r_nand_adr_stat)&eim_nand_adr_bits;
    if (addrs) {
        printf("No workey.  There are %d addrs left\n", addrs);
        return 0;
    }
    else {
        printf("Works well!\n");
        return 1;
    }
}

static int do_bang_on_ecc(struct sd_state *state) {
    int run;
    int seed;
    int i;
    int val;

    for (i=0; i<0x200; i+=2)
        *eim_get(fpga_romulator_base+i) = 0xfe80;

    run = 0;
    while (1) {
        seed = rand();
        val = do_one_ecc_knock(state, seed, run);
        if (val) {
            printf("Got a match.  Trying again...\n");
            val = do_one_ecc_knock(state, seed, run);
            if (val) {
                printf("Yay!  Confirmed.\n");
                return val;
            }
            printf("Nope.  Continuing.\n");
        }
        run++;
    }

    return val;
}


static int do_validate_file(struct sd_state *state) {
    int addrs;

    // Drain address data
    addrs = *eim_get(fpga_r_nand_adr_stat)&eim_nand_adr_bits;
    printf("Pre-draining %d addresses\n", addrs);
    drain_nand_addrs(state);

    sd_reset(state, 1);
    send_cmdX(state, 9, 0, 0, 0, 0, 32);
    usleep(900000);

    addrs = *eim_get(fpga_r_nand_adr_stat)&eim_nand_adr_bits;
    printf("Draining %d addresses\n", addrs);
    drain_nand_addrs(state);

    usleep(50000);
    addrs = *eim_get(fpga_r_nand_adr_stat)&eim_nand_adr_bits;
    if (addrs) {
        printf("Invalid firmware.  There are %d addrs left\n", addrs);
    }
    else {
        printf("Firmware okay.\n");
        return 0;
    }

    return 1;
}



static int do_execute_file(struct sd_state *state,
                           int run,
                           int seed,
                           char *filename) {
    uint8_t response[560];
    uint8_t file[512+(4*sizeof(uint16_t))];
    int ret;
    int tries;

    memset(response, 0, sizeof(response));
    memset(file, 0xff, sizeof(file));
    srand(seed);

    // Load in the specified file
    {
        int fd = open(filename, O_RDONLY);
        if (-1 == fd) {
            perror("Unable to load rom file");
            return 1;
        }
        read(fd, file, 512);
        close(fd);
    }

    printf("\n\nRun %-4d  Seed: %8d\n", run, seed);

    // Actually enter factory mode (sends CMD63/APPO and waits for response)
    ret = -1;
    for (tries=0; ret<0 && tries<10; tries++) {
        ret = sd_enter_factory_mode(state, run);
        if (-1 == ret)
            printf("Couldn't enter factory mode, trying again (%d/10)\n", tries+1);
    }
    if (-1 == ret)
        return -1;

    calculate_mmc_crc16(file, file+(sizeof(file)-8), sizeof(file)-8);
    xmit_mmc_dat4(state, file, sizeof(file));
    //rcvr_mmc_dat1_start(state, 256);
    rcvr_mmc_cmd(state, response, sizeof(response));

    printf("Result of factory mode: %d\n", ret);

    printf("File:\n");
    print_hex(file, sizeof(file));
    static int romfile;
    if (!romfile)
        romfile = open("ax211rom.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    write(romfile, response, 512);

    printf("\nResponse:\n");
    print_hex(response, sizeof(response));

    return 0;
}


static int print_help(char *name) {
    printf("Usage:\n"
        "Modes:\n"
        "\t%s -f         Runs a fuzz test in factory mode\n"
        "\t%s -c         Boots normally and prints CSD/CID\n"
        "\t%s -e         Tries banging on ECC\n"
        "\t%s -e         Validate the firmware file\n"
        "\t%s -x [file]  Enters factory mode and executes the file\n"
        "Options:\n"
        " -s [seed]    Specifies a seed for random values\n"
        " -r [file]    Writes the specified ROM file to NAND\n"
        " -l [loop]    Execute the loop with run=[loop]\n"
        ,
        name, name, name, name, name);
    return 0;
}

int main(int argc, char **argv) {
    int ret = 0;
	struct sd_state *state;
	int mode = -1;
	int ch;

    int seed;
    int have_seed = 0;

    int loop;
    int have_loop = 0;

    int rom_file_written = 0;

    char prog_filename[512];


	srand(time(NULL));

	state = sd_init(MISO_PIN, MOSI_PIN, CLK_PIN, CS_PIN, DAT1_PIN, DAT2_PIN, POWER_PIN);
    if (!state)
        return 1;


	while ((ch = getopt(argc, argv, "vefchs:r:l:x:")) != -1) {
		switch(ch) {
		case 'c':
			mode = 1;
			break;

        case 'f':
            mode = 0;
            break;

        case 'e':
            mode = 2;
            break;

        case 's':
            have_seed = 1;
            seed = strtoul(optarg, NULL, 0);
            break;

        case 'l':
            have_loop = 1;
            loop = strtoul(optarg, NULL, 0);
            break;

        case 'r':
            load_rom_file(state, optarg);
            rom_file_written=1;
            break;

        case 'v':
            mode = 3;
            break;

        case 'x':
            strncpy(prog_filename, optarg, sizeof(prog_filename)-1);
            mode = 4;
            break;

		case 'h':
        default:
            print_help(argv[0]);
			return 0;
			break;
		}
	}

    if (!rom_file_written)
        load_rom_file(state, "bot64k-ecc-d.bin");

    if (mode == -1)
        ret = print_help(argv[0]);
    else if (mode == 0)
        ret = do_interestingness(state,
                                 have_seed?&seed:NULL,
                                 have_loop?&loop:NULL);
    else if (mode == 1)
		ret = do_get_csd_cid(state);
    else if (mode == 2)
        ret = do_bang_on_ecc(state);
    else if (mode == 3)
        ret = do_validate_file(state);
    else if (mode == 4)
        ret = do_execute_file(state, loop, seed, prog_filename);

	//sd_deinit(&state);

	return ret;
}
