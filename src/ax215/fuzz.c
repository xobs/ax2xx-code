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

static int interesting_one_cycle(struct sd_state *state, int run, int seed) {
    uint8_t response[16];
    static int spots = 5;
    uint8_t sfrs[spots];
    uint8_t sfrs_val0[spots];
    uint8_t sfrs_val1[spots];
    uint8_t file[512+(4*sizeof(uint16_t))];
	int i = 0;
    int ret;

    memset(response, 0, sizeof(response));
    memset(file, 0, sizeof(file));

    srand(seed);
    printf("\n\nRun %-4d  Seed: %8d\n", run, seed);

    memset(file, 0, sizeof(file));
    int fd = open("fuzzer.bin", O_RDONLY);
    if (-1 == fd) {
        perror("Couldn't open binary program");
        exit(1);
    }
    read(fd, file, 512);
    close(fd);

    // Patch the program binary
    for (i=0; i<spots; i++) {
        sfrs[i] = (rand()&0x7f) | 0x80;
//        if (sfrs[i] == 0xef || sfrs[i] == 0xf2)
//            continue;

        sfrs_val0[i] = rand();
        sfrs_val1[i] = rand();
    }

    // Actually enter factory mode (sends CMD63/APPO and waits for response)
    ret = sd_enter_factory_mode(state, run);
    if (-1 == ret) {
        printf("Couldn't enter factory mode\n");
        return -1;
    }

    int matched = 0;
    int total = 1+(rand()%spots);
    for (i=0; i<512; i++) {
        if (file[i] != 0xa5)
            continue;

        if ((file[i+1] == file[i+2]) && ((file[i+1] & 0x7f) < spots)) {
            int reg = file[i+1]&0x7f;
            int oneorzero = (file[i+1]&0x80);
            if (reg < total) {

                file[i+0] = 0x75;           // mov SFR, #immediate
                file[i+1] = sfrs[reg];        // Dest register

                if (oneorzero)
                    file[i+2] = sfrs_val1[reg];   // Immediate value
                else
                    file[i+2] = sfrs_val0[reg];   // Immediate value
            }
            else {
                file[i+0] = 0;
                file[i+1] = 0;
                file[i+2] = 0;
                sfrs[reg] = 0;
                sfrs_val0[reg] = 0;
                sfrs_val1[reg] = 0;
            }
            matched++;
            i+=2;
        }
        else {
            printf("Error locating special opcode (%d %02x %02x / %02x)\n",
                    i, file[i+1], file[i+2], file[i+1]&0x7f);
            exit(0);
        }
    }

    if (matched != spots*2) {
        printf("Couldn't find %d matches, only found %d\n", spots*2, matched);
        exit(1);
    }

    fd = open("fuzzer-out.bin", O_WRONLY | O_CREAT | O_TRUNC, 0777);
    write(fd, file, 512);
    close(fd);

    // Transmit the file
    sd_mmc_dat4_crc16(file, file+(sizeof(file)-8), sizeof(file)-8);
    xmit_mmc_dat4(state, file, sizeof(file));
    rcvr_spi(state, response, sizeof(response));


    // Wait for some sign of life
    int sd_pins = sd_read_pins(state);
    int matches = 0;
    int sleeptime = 100*(rand()&0xf);
    for (i=0; i<10; i++) {
        usleep(sleeptime);
        int new_pins = sd_read_pins(state);
        if (new_pins != sd_pins) {
            matches++;
            sd_pins = new_pins;
        }
    }

    printf("\nResponse:\n");
    print_hex(response, sizeof(response));
    for (i=0; i<spots; i++)
        printf("SFR_%02x:  %02x / %02x\n", sfrs[i], sfrs_val0[i], sfrs_val1[i]);
    printf("%d pin matches (last: %x)\n", matches, sd_pins);
    printf("Finished up run: %-4d  Seed: %8d\n", run, seed);
    printf("--------------------------------------------------\n");


    if (matches >= 5) {
        printf("Potential match\n");
        return 1;
    }

    return 0;
}

int do_fuzz(struct sd_state *state, int *seed, int *loop) {
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
