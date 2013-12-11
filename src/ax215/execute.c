#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "sd.h"
#include "gpio.h"
#include "eim.h"
#include "crc-16.h"

#define ROM_FILENAME "data-ax215.rom"
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

enum execute_mode {
    dump_rom,
    enter_debugger,
    known_state,
};


static int read_file(char *filename, uint8_t *bfr, int size) {
    int ret;
    int fd;
    
    fd = open(filename, O_RDONLY);
    if (-1 == fd) {
        printf("Unable to load rom file %s: %s\n", filename, strerror(errno));
        return 1;
    }
    ret = read(fd, bfr, size);
    if (-1 == ret)
        perror("Couldn't read from file");

    ret = close(fd);
    if (-1 == ret)
        perror("Couldn't close file");
    return 0;
}

int look_for_known_state(struct sd_state *state, int sleeptime)
{
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
    return 0;
}

int do_dump_rom(struct sd_state *state, int sleeptime) {
    int fd;
    int offset = 0;

    // Wait for some sign of life
    uint8_t sd_pins;
    uint8_t slow_response;
    int seeks;
    uint8_t byte, bit;
    int dbg = 0;
    uint8_t *rom;
    int rom_size = 65536;
    int first_bit = 1;

    fd = open(ROM_FILENAME, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (-1 == fd) {
        perror("Unable to open output rom file");
        return -1;
    }

    if (-1 == lseek(fd, rom_size - 1, SEEK_SET)) {
        perror("Couldn't seek to grow output file");
        return -1;
    }
    byte = 0;
    if (-1 == write(fd, &byte, 1)) {
        perror("Couldn't grow output rom file");
        return -1;
    }
    if (-1 == lseek(fd, 0, SEEK_SET)) {
        perror("Couldn't seek to beginning of file");
        return -1;
    }

    rom = mmap(0, rom_size - 1, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if( -1 == (int)rom ) {
            perror("Unable to mmap output rom file");

            if( -1 == close(fd) )
                    perror("Also couldn't close file");

            return -1;
    }

    memset(rom, 0, rom_size);
    for (offset = 0; offset < rom_size; offset++) {
        byte = 0;
        for (bit = 0; bit < 8; bit++) {
            uint8_t values[256];
            memset(values, 0, sizeof(values));

            seeks = 0;
            do {
                usleep(sleeptime);
                slow_response = sd_read_pins(state);
                if (dbg)
                    printf("%02x ", slow_response);
                values[slow_response]++;
                if (seeks++ > 300) {
                    printf("Card hung\n");
                    goto bail;
                }
            } while (slow_response == 0x48); 

            /* Give the other pin a little more time to settle */
            for (seeks = 0; seeks < 3; seeks++) {
                usleep(sleeptime);
                slow_response = sd_read_pins(state);
                if (dbg)
                    printf("%02x ", slow_response);
                values[slow_response]++;
            }

            seeks = 0;
            do {
                usleep(sleeptime);
                slow_response = sd_read_pins(state);
                if (dbg)
                    printf("%02x ", slow_response);
                values[slow_response]++;
                if (seeks++ > 300) {
                    printf("Card hung\n");
                    goto bail;
                }
            } while (slow_response != 0x48); 

            /* Count which byte "wins" */
            int max = 0;
            for (seeks = 0; seeks < 256; seeks++)
                if (values[seeks] > values[max] && seeks != 0x48)
                    max = seeks;

            sd_pins = max;
            if (dbg)
                printf("  [%02x] ", sd_pins);

            if (first_bit) {
                printf(" First bit, ignoring\n");
                first_bit = 0;
                bit--;
                continue;
            }

            if ((sd_pins & 0x10)) {
                byte |= (1 << (7 - bit));
                if (dbg)
                    printf(" - 1\n");
            }
            else {
                if (dbg)
                    printf(" - 0\n");
            }
        }
        rom[offset] = byte;
        if (!dbg)
            fprintf(stderr, "\r%d%% %d/%d bytes [0x%02x]",
                    (offset * 100) / 65536,offset, 65536, byte);
        else
            printf("~~ Byte: [0x%02x]\n", byte);

        /*
        if (offset >= 2) {
            if (rom[0] == 1 && rom[1] == 2 && rom[2] == 4) {
                printf("No dice\n");
                goto bail;
            }
            else {
                printf("Success?\n");
                munmap(rom, rom_size);
                close(fd);
                return 0;
            }
        }
        */
    }

    return 0;

bail:
    munmap(rom, rom_size);
    close(fd);
    return 1;
}

static int do_enter_debugger(struct sd_state *state)
{
    uint8_t response[5];
    uint8_t cmdsize = 6;
    uint8_t cmd[cmdsize];
    int i;
    int ret = 0;

    memset(response, 0, sizeof(response));

    for (i=0; i<cmdsize; i++)
        cmd[i] = rand();
    cmd[0] &= 0x3f;
    cmd[0] |= 0x40;
    cmd[0] = 0x40;
    cmd[5] = (crc7(cmd, 5)<<1)|1;

    printf("Transmitting data:\n");
    print_hex(cmd, sizeof(cmd));
    xmit_mmc_cmd(state, cmd, sizeof(cmd));

    ret = rcvr_mmc_dat0_start(state, 1024);
    if (-1 == ret)
        printf("No response\n");
    else {
        printf("Response after %d tries\n", ret);
        rcvr_spi(state, response, sizeof(response));
        //rcvr_mmc_dat4(state, response, sizeof(response));
        printf("SD card said:\n");
        print_hex(response, sizeof(response));
    }

    return 0;
}

static void patch_file(uint8_t *file, int size) {
    int i;
    for (i = 0; i < size; i++) {
        if (file[i] == 0xa5) {
            uint8_t sfr;
            uint8_t val;
            do {
                sfr = rand() | 0x80;
            } while (sfr == 0x80);
            val = rand();

            printf("    mov     0x%02x, #0x%02x\n", sfr, val);
            file[i + 0] = 0x75;     // mov SFR, #immediate
            file[i + 1] = sfr;      // Dest register
            file[i + 2] = val;      // Immediate value
            i += 2;
        }
    }
}

int do_one_execute_file(struct sd_state *state,
                           int run,
                           int seed,
                           enum execute_mode mode,
                           char *filename) {
    uint8_t response[1];
    uint8_t file[512+(4*sizeof(uint16_t))];
    int ret;

    memset(response, 0, sizeof(response));
    memset(file, 0xff, sizeof(file));
    srand(seed);

    // Load in the specified file
    if (read_file(filename, file, 512))
        return 1;


    printf("\n\nRun %-4d  Seed: %8d\n", run, seed);
    patch_file(file, 512);

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

    printf("Immediate code-load response:\n");
    print_hex(response, 1);


    if (mode == dump_rom)
        return do_dump_rom(state, 2000);
    else if (mode == enter_debugger)
        return do_enter_debugger(state);
    else if (mode == known_state) {
        do_enter_debugger(state);
        return look_for_known_state(state, 2000);
    }

    return 0;
}

int do_execute_file(struct sd_state *state,
                           int run,
                           int seed,
                           char *filename) {
    int ret;
    do {
        ret = do_one_execute_file(state, run++, rand(),
                known_state, filename);
    } while (ret != 0);
    return 0;
}
