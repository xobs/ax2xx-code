#include <stdio.h>
#include <stdint.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "sd.h"


static int patch_fuzzer(struct sd_state *state, uint8_t *file, int filesize) {
    static int spots = 8;
    uint8_t sfrs[spots];
    uint8_t sfrs_val0[spots];
    uint8_t sfrs_val1[spots];
    int i;

    // Patch the program binary
    for (i = 0; i < spots; i++) {
        sfrs[i] = (rand() & 0x7f) | 0x80;
        sfrs_val0[i] = rand();

        // Do something interesting with the value.  Invert it, don't change
        // it, or generate a new value.
        switch (rand() & 3) {
        case 0:
            sfrs_val1[i] = ~sfrs_val0[i];
            break;

        case 1:
            sfrs_val1[i] = sfrs_val0[i];
            break;

        default:
            sfrs_val1[i] = rand();
            break;
        }
    }

    int matched = 0;
    int total = 1 + (rand() % spots);
    for (i=0; i<512; i++) {
        if (file[i] != 0xa5)
            continue;

        if ((file[i+1] == file[i+2]) && ((file[i+1] & 0x7f) < spots)) {
            int reg = file[i+1]&0x7f;

            // The first instance will have this bit set to 0.  The second
            // instance will have it set to 1.  Thus, you can do interesting
            // things here.
            int is_anti = (file[i+1]&0x80);
            if (reg < total) {

                file[i+0] = 0x75;           // mov SFR, #immediate
                file[i+1] = sfrs[reg];        // Dest register

                if (is_anti)
                    file[i+2] = sfrs_val1[reg];   // Immediate value
                else
                    file[i+2] = sfrs_val0[reg];   // Immediate value
            }
            else {
                /* Fill with NOPs */
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
            return -1;
        }
    }

    if (matched != spots * 2) {
        printf("Couldn't find %d matches, only found %d\n", spots*2, matched);
        return -2;
    }

    for (i = 0; i < total; i++)
        printf("    SFR_%02x:  %02x / %02x\n", sfrs[i], sfrs_val0[i], sfrs_val1[i]);

    return 0;
}

static int interesting_one_cycle(struct sd_state *state, int run, int seed) {
    uint8_t response[1];
    uint8_t slow_response[64];
    uint8_t file[512+(4*sizeof(uint16_t))];
    int i = 0;
    int ret;

    memset(response, 0, sizeof(response));
    memset(file, 0, sizeof(file));

    srand(seed);
    printf("\nRun %-4d  Seed: %8d\n", run, seed);

    memset(file, 0, sizeof(file));
    int fd = open("fuzzer.bin", O_RDONLY);
    if (-1 == fd) {
        perror("Couldn't open binary program");
        exit(1);
    }
    read(fd, file, 512);
    close(fd);

    ret = patch_fuzzer(state, file, sizeof(file));
    if (ret < 0)
        return ret;

    // Actually enter factory mode (sends CMD63/APPO and waits for response)
    ret = sd_enter_factory_mode(state, run);
    if (-1 == ret) {
        printf("Couldn't enter factory mode\n");
        return ret;
    }

    // Transmit the file
    sd_mmc_dat4_crc16(file, file+(sizeof(file)-8), sizeof(file)-8);
    xmit_mmc_dat4(state, file, sizeof(file));
    rcvr_spi(state, response, sizeof(response));


    // Wait for some sign of life
    int sd_pins = sd_read_pins(state);
    int changes = 0;
    int sleeptime = 10 *(1 + (rand() & 0xff));
    for (i = 0; i < sizeof(slow_response); i++) {
        usleep(sleeptime);
        slow_response[i] = sd_read_pins(state);
        if (slow_response[i] != sd_pins) {
            changes++;
        }
        sd_pins = slow_response[i];
    }

    printf("\nResponse:\n");
    print_hex(response, sizeof(response));
    printf("%d state changes, slow response:\n", changes);
    print_hex(slow_response, sizeof(slow_response));
    printf("Finished up run: %-4d  Seed: %8d\n", run, seed);
    printf("\n==================================================\n");


    if (changes >= 5) {
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
