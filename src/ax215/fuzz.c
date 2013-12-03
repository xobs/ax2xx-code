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

#define MAX_POOLS 16
#define MAX_SLOTS 32
#define SFR_SPOTS 8

enum pool_types {
    pool_sfr,
    pool_sleep,
    pool_misc,
    pool_val0,
    pool_val1,
    pool_valtype,
};

enum pool_misc_types {
    pool_misc_totals,
    pool_misc_sleeptime,
};

union rand_seed {
    struct {
        uint32_t seed;
        uint16_t run;
    } fields;
    uint16_t seed[3];
};

static int fill_rand_pools(int run, int seed,
        uint8_t pools[MAX_POOLS][MAX_SLOTS])
{
    int i, j;
    union rand_seed rand_seed;
    rand_seed.fields.seed = seed;
    rand_seed.fields.run = run;

    printf("\nRun %-4d  Seed: %8d\n", run, seed);
    for (i = 0; i < MAX_POOLS; i++)
        for (j = 0; j < MAX_SLOTS; j++)
            pools[i][j] = nrand48(rand_seed.seed);
    return 0;
}

static int patch_fuzzer(struct sd_state *state, uint8_t *file, int filesize,
        uint8_t pools[MAX_POOLS][MAX_SLOTS])
{
    int matched = 0;
    int total;
    int i;

    total = 1 + (pools[pool_misc][pool_misc_totals] % SFR_SPOTS);

    // Patch the binary, and at the same time disassemble the code.
    // We look for 0xa5 opcodes, and patch these with our own custom commands.
    printf("start:\n");
    for (i = 0; i < filesize; i++) {

        if (file[i] >= 0x78 && file[i] <= 0x7f) {
            uint8_t reg = file[i] & 7;
            printf("    mov R%d, #0x%02x\n", reg, pools[pool_sleep][reg]);
            file[i + 1] = pools[pool_sleep][reg];
            i++;
        }

        else if (file[i] >= 0xd8 && file[i] <= 0xdf) {
            printf("    djnz R%d, %d\n", file[i] & 7, (int8_t)file[i + 1]);
            i++;
        }

        else if (file[i] == 0xa5) {

            if ((file[i + 1] == file[i + 2])
            && ((file[i + 1] & 0x7f) < SFR_SPOTS)) {
                int reg = file[i + 1] & 0x7f;

                // The first instance will have this bit set to 0.  The second
                // instance will have it set to 1.  Thus, you can do interesting
                // things here.
                int is_anti = (file[i + 1] & 0x80);
                if (reg < total) {
                    uint8_t val;

                    if (is_anti) {
                        if ((pools[pool_valtype][reg] & 3) == 0)
                            val = pools[pool_val0][reg];
                        else if ((pools[pool_valtype][reg] & 3) == 1)
                            val = ~pools[pool_val0][reg];
                        else
                            val = pools[pool_val1][reg];
                    }
                    else
                        val = pools[pool_val0][reg];

                    file[i + 0] = 0x75;                 // mov SFR, #immediate
                    file[i + 1] = pools[pool_sfr][reg]; // Dest register
                    file[i + 2] = val;                  // Immediate value

                    printf("    mov SFR_%02X, #0x%02x\n",
                            pools[pool_sfr][reg], val);
                }
                else {
                    printf("    nop\n");
                    /* Fill with NOPs */
                    file[i + 0] = 0;
                    file[i + 1] = 0;
                    file[i + 2] = 0;
                }
                matched++;
                i += 2;
            }
            else {
                printf("Error locating special opcode (%d %02x %02x / %02x)\n",
                        i, file[i+1], file[i+2], file[i+1]&0x7f);
                return -1;
            }
        }

        else if (file[i] == 0x80) {
            printf("    sjmp %d\n", (int8_t)file[i + 1]);
            i++;
        }

        else if (file[i] == 0x00)
            printf("    nop\n");

        else
            printf("Unrecognized opcode: 0x%02x\n", file[i]);
    }

    if (matched != SFR_SPOTS * 2) {
        printf("Couldn't find %d matches, only found %d\n",
                SFR_SPOTS * 2, matched);
        return -2;
    }

    return 0;
}


static int interesting_one_cycle(struct sd_state *state, int run, int seed) {
    uint8_t pools[MAX_POOLS][MAX_SLOTS];
    uint8_t response[1];
    uint8_t slow_response[64];
    uint8_t file[512+(4*sizeof(uint16_t))];
    int filesize;
    int i = 0;
    int ret;

    memset(response, 0, sizeof(response));
    memset(file, 0, sizeof(file));

    fill_rand_pools(run, seed, pools);

    memset(file, 0, sizeof(file));
    int fd = open("fuzzer.bin", O_RDONLY);
    if (-1 == fd) {
        perror("Couldn't open binary program");
        exit(1);
    }
    filesize = read(fd, file, 512);
    close(fd);

    if (filesize < 0) {
        perror("Couldn't read file");
        return -1;
    }

    ret = patch_fuzzer(state, file, filesize, pools);
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
    int sleeptime = 10 *(1 + (pools[pool_misc][pool_misc_sleeptime] & 0x3f));
    for (i = 0; i < sizeof(slow_response); i++) {
        usleep(sleeptime);
        slow_response[i] = sd_read_pins(state);
        if (slow_response[i] != sd_pins) {
            changes++;
        }
        sd_pins = slow_response[i];
    }

    printf("\n");
    printf("Waiting %d usec between samples\n", sleeptime);
    printf("Response:\n");
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
            if (val > 0) {
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
