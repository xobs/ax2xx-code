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

static int look_for_known_state(struct sd_state *state, int sleeptime) {
    // Wait for some sign of life
    int sd_pins = sd_read_pins(state);
    int i;
    uint8_t slow_response[512];
    int changes = 0;
    for (i = 0; i < sizeof(slow_response); i++) {
        usleep(sleeptime);
        slow_response[i] = sd_read_pins(state);
        if (slow_response[i] != sd_pins) {
            changes++;
        }
        sd_pins = slow_response[i];
    }

    printf("Observed %d changes:\n", changes);
    print_hex(slow_response, sizeof(slow_response));
    return changes;
}

int do_execute_file(struct sd_state *state,
                           int run,
                           int seed,
                           char *filename) {
    uint8_t response[560];
    uint8_t file[512+(4*sizeof(uint16_t))];
    uint8_t cmdsize = 6;
    uint8_t cmd[cmdsize];
    int i;
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
    ret = sd_enter_factory_mode(state, run);
    if (-1 == ret) {
        printf("Couldn't enter factory mode\n");
        return ret;
    }

    // Transmit the file
    sd_mmc_dat4_crc16(file, file+(sizeof(file)-8), sizeof(file)-8);
    xmit_mmc_dat4(state, file, sizeof(file));

    ret = rcvr_mmc_dat0_start(state, 100);
    if (-1 == ret)
        printf("DAT0 never started\n");
    else
        printf("Entered factory mode after %d tries\n", ret);
    rcvr_spi(state, response, 1);


    return look_for_known_state(state, 720);

    printf("Immediate code-load response:\n");
    print_hex(response, 1);

    for (i=0; i<cmdsize; i++)
        cmd[i] = rand();
    cmd[0] &= 0x3f;
    cmd[0] |= 0x40;
    cmd[0] = 0x40;
    cmd[5] = (crc7(cmd, 5)<<1)|1;

    printf("Transmitting data:\n");
    print_hex(cmd, sizeof(cmd));
    xmit_mmc_cmd(state, cmd, sizeof(cmd));
    rcvr_spi(state, response, 1);


    if (-1 == ret)
        printf("No response\n");
    else {
        printf("Response after %d tries\n", ret);
        rcvr_mmc_dat4(state, response, sizeof(response));
        printf("Result of factory mode:\n");
        print_hex(response, sizeof(response));
    }

    return 0;
}
