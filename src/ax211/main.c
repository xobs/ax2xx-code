#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "sd.h"
#include "gpio.h"
#include "eim.h"
#include "crc-16.h"

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

int dbg_main(struct sd_state *state);

static int print_header(uint8_t *bfr) {
	printf(" CMD %2d {%02x %02x %02x %02x %02x %02x}  ",
            bfr[0]&0x3f, bfr[0], bfr[1], bfr[2], bfr[3], bfr[4], bfr[5]);
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
	result = sd_txrx(state, bfr, sizeof(bfr), out_bfr, print_size);
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
        val = eim_get(fpga_r_nand_adr_hi);
        addrs = eim_get(fpga_r_nand_adr_stat)&eim_nand_adr_bits;
    }
    return val;
}

static int read_file(char *filename, uint8_t *bfr, int size) {
    int fd = open(filename, O_RDONLY);
    if (-1 == fd) {
        printf("Unable to load rom file %s: %s\n", filename, strerror(errno));
        return 1;
    }
    read(fd, bfr, size);
    close(fd);
    return 0;
}


static int load_and_enter_debugger(struct sd_state *state, char *filename) {
    uint8_t response1[6];
    uint8_t file[512+(4*sizeof(uint16_t))];
    memset(file, 0xff, sizeof(file));

    // Load in the debugger stub
    if (read_file(filename, file, 512))
        return 1;
    sd_mmc_dat4_crc16(file, file+(sizeof(file)-8), sizeof(file)-8);

    while(1) {
        int ret;
        int tries;
        // Actually enter factory mode (sends CMD63/APPO and waits for response)
        ret = -1;
        for (tries=0; ret<0 && tries<10; tries++) {
            ret = sd_enter_factory_mode(state, 0);
            if (-1 == ret)
                printf("Couldn't enter factory mode, trying again (%d/10)\n",
                        tries+1);
        }
        // Couldn't enter factory mode, abort
        if (-1 == ret)
            return 1;

        xmit_mmc_dat4(state, file, sizeof(file));
        rcvr_mmc_cmd(state, response1, 1);
        printf("Immediate code-load response: %02x\n", response1[0]);

        for (tries=0; tries<2; tries++) {
            if (-1 != rcvr_mmc_cmd_start(state, 50))
                break;
            usleep(50000);
        }
        // Couldn't enter debugger, try again
        if (-1 == rcvr_mmc_cmd_start(state, 32))
            continue;

        rcvr_mmc_cmd(state, response1, sizeof(response1));

        printf("Result of factory mode: %d\n", ret);
        printf("\nResponse1 (%02x):\n", response1[0]);
        print_hex(response1+1, sizeof(response1)-1);

        if (response1[0] != 0 || response1[1] != 0x0a)
            continue;

        break;
    }
    return 0;
}


static int interesting_one_cycle(struct sd_state *state, int run, int seed) {
    uint8_t response[512];
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

    int fd = open("dump-rom.bin", O_RDONLY);
    read(fd, file, 512);
    close(fd);

    file[0x75] = run;
    file[0x78] = run>>8;

    static int rom_fd;
    if (!rom_fd)
        rom_fd = open("ax211-rom.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    sd_mmc_dat4_crc16(file, file+(sizeof(file)-8), sizeof(file)-8);
    xmit_mmc_dat4(state, file, sizeof(file));
    rcvr_mmc_cmd(state, response, 2);
    for (i=0; i<3; i++) {
        if (-1 != rcvr_mmc_cmd_start(state, 50))
            break;
        usleep(50000);
    }
    if (-1 != rcvr_mmc_cmd_start(state, 5))
        return -1;
    rcvr_mmc_cmd(state, response, sizeof(response));

    printf("Result of factory mode: %d\n", ret);

    printf("\nResponse:\n");
    print_hex(response, sizeof(response));
    printf("SDL: %02x   SDH: %02x\n", file[0x75], file[0x78]);
    printf("Finished up run: %-4d  Seed: %8d\n", run, seed);

    uint8_t cmp[] = {0xfd, 0x9a, 0xd1, 0x42, 0xf6, 0x20, 0xcc, 0x51,
                     0xd3, 0xc6, 0xec, 0x47, 0x77, 0x31, 0x71, 0x01};
    int matches = 0;
    for (i=0; i<sizeof(cmp); i++) {
        if (response[i+1] == cmp[i])
            matches++;
    }
    if (matches > 10) {
        printf("Got a match!\n");
        exit(0);
    }
    for (i=0; i<sizeof(response); i++) {
        if (response[i] != 0xff)
            break;
    }
    if (i >= sizeof(response)) {
        printf("Got no response at all!\n");
        return -1;
    }

    return 0;
}

static int do_interestingness(struct sd_state *state, int *seed, int *loop) {
	int run;
    int val;
    int s;

    if (seed)
        val = interesting_one_cycle(state, loop?*loop:0, *seed);
    else if (loop)
        val = interesting_one_cycle(state, *loop, rand());
    else {
        val = 0;
        run = 0;
        for (; !val; run++) {
            s = rand();
            val = interesting_one_cycle(state, run, s);

            // If we get an interesting value, try running it again.
            if (val>0) {
                printf("Potentially interesting.  "
                       "Trying seed %d again...\n", s);
                val = interesting_one_cycle(state, run, s);

                // If we get an interesting value, try running it again.
                if (val>0) {
                    printf("Still potentially interesting.  "
                           "Trying seed %d one last time...\n", s);
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


static int do_validate_file(struct sd_state *state) {
    int addrs;

    // Drain address data
    addrs = eim_get(fpga_r_nand_adr_stat)&eim_nand_adr_bits;
    printf("Pre-draining %d addresses\n", addrs);
    drain_nand_addrs(state);

    sd_reset(state, 1);
    send_cmdX(state, 9, 0, 0, 0, 0, 32);
    usleep(900000);

    addrs = eim_get(fpga_r_nand_adr_stat)&eim_nand_adr_bits;
    printf("Draining %d addresses\n", addrs);
    drain_nand_addrs(state);

    usleep(50000);
    addrs = eim_get(fpga_r_nand_adr_stat)&eim_nand_adr_bits;
    if (addrs) {
        printf("Invalid firmware.  There are %d addrs left\n", addrs);
    }
    else {
        printf("Firmware okay.\n");
        return 0;
    }

    return 1;
}


static int do_debugger(struct sd_state *state, char *filename) {

    if (load_and_enter_debugger(state, filename))
        return 1;
    printf("Loaded debugger\n");


    return dbg_main(state);
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
    if (read_file(filename, file, 512))
        return 1;

    printf("\n\nRun %-4d  Seed: %8d\n", run, seed);

    // Actually enter factory mode (sends CMD63/APPO and waits for response)
    ret = -1;
    for (tries=0; ret<0 && tries<10; tries++) {
        ret = sd_enter_factory_mode(state, run);
        if (-1 == ret)
            printf("Couldn't enter factory mode, trying again (%d/10)\n",
                    tries+1);
    }
    if (-1 == ret)
        return -1;

    sd_mmc_dat4_crc16(file, file+(sizeof(file)-8), sizeof(file)-8);
    xmit_mmc_dat4(state, file, sizeof(file));
    rcvr_mmc_cmd(state, response, 1);
    printf("Immediate code-load response: %02x\n", response[0]);

    for (tries=0; tries<2; tries++) {
        if (-1 != rcvr_mmc_cmd_start(state, 50))
            break;
        usleep(50000);
    }
    rcvr_mmc_cmd(state, response, sizeof(response));

    printf("Result of factory mode: %d\n", ret);

    printf("\nResponse (%02x):\n", response[0]);
    print_hex(response+1, sizeof(response)-1);

    return 0;
}


static int print_help(char *name) {
    printf("Usage:\n"
        "Modes:\n"
        "\t%s -f         Runs a fuzz test in factory mode\n"
        "\t%s -c         Boots normally and prints CSD/CID\n"
        "\t%s -v         Validate the firmware file\n"
        "\t%s -d [dbgr]  Enters debugger, launching the specified filename\n"
        "\t%s -x [file]  Enters factory mode and executes the file\n"
        "Options:\n"
        " -s [seed]    Specifies a seed for random values\n"
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

    char prog_filename[512];
    char debugger_filename[512];


	srand(time(NULL));

	state = sd_init(MISO_PIN, MOSI_PIN, CLK_PIN, CS_PIN,
                    DAT1_PIN, DAT2_PIN, POWER_PIN);
    if (!state)
        return 1;


	while ((ch = getopt(argc, argv, "vfchs:l:x:d:")) != -1) {
		switch(ch) {
		case 'c':
			mode = 1;
			break;

        case 'f':
            mode = 0;
            break;

        case 's':
            have_seed = 1;
            seed = strtoul(optarg, NULL, 0);
            break;

        case 'l':
            have_loop = 1;
            loop = strtoul(optarg, NULL, 0);
            break;

        case 'v':
            mode = 3;
            break;

        case 'x':
            strncpy(prog_filename, optarg, sizeof(prog_filename)-1);
            mode = 4;
            break;

        case 'd':
            strncpy(debugger_filename, optarg, sizeof(debugger_filename)-1);
            mode = 5;
            break;

		case 'h':
        default:
            print_help(argv[0]);
			return 0;
			break;
		}
	}

    if (mode == -1)
        ret = print_help(argv[0]);
    else if (mode == 0)
        ret = do_interestingness(state,
                                 have_seed?&seed:NULL,
                                 have_loop?&loop:NULL);
    else if (mode == 1)
		ret = do_get_csd_cid(state);
    else if (mode == 3)
        ret = do_validate_file(state);
    else if (mode == 4)
        ret = do_execute_file(state, loop, seed, prog_filename);
    else if (mode == 5)
        ret = do_debugger(state, debugger_filename);

	//sd_deinit(&state);

	return ret;
}
