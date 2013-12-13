/*------------------------------------------------------------------------/
/  Bitbanging MMCv3/SDv1/SDv2 (in SPI mode) control module
/-------------------------------------------------------------------------/
/
/  Copyright (C) 2012, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/--------------------------------------------------------------------------/
 Features and Limitations:

 * Very Easy to Port
   It uses only 4 bit of GPIO port. No interrupt, no SPI port is used.

 * Platform Independent
   You need to modify only a few macros to control GPIO ports.

 * Low Speed
   The data transfer rate will be several times slower than hardware SPI.

/-------------------------------------------------------------------------*/


#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <ctype.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "crc-16.h"
#include "gpio.h"
#include "sd.h"
#include "eim.h"

enum disk_status {
    STA_NO_INIT,
    STA_NOINIT,
};

enum their_results {
    RES_OK,
    RES_PARERR,
    RES_NOTRDY,
    RES_ERROR,
};

enum disk_ioctl_arg {
    CTRL_SYNC,
    GET_SECTOR_COUNT,
    GET_BLOCK_SIZE,
};


struct sd_state {
    /* Pin numbers */
    union {
        uint32_t miso;
        uint32_t dat0;
    };
    union {
        uint32_t mosi;
        uint32_t cmd;
    };
    uint32_t dat1;
    uint32_t dat2;
    union {
        uint32_t cs;
        uint32_t dat3;
    };
    uint32_t clk;

    uint32_t power;

    uint32_t blklen;
    enum disk_status status;
};



/*-------------------------------------------------------------------------*/
/* Platform dependent macros and functions needed to be modified           */
/*-------------------------------------------------------------------------*/

#define READY_TRIES 10


#define DLY_US(n)   usleep(n)   /* Delay n microseconds */

#define CS_H()      gpio_set_value(state->cs, CS_DESEL) /* Set MMC CS "high" */
#define CS_L()      gpio_set_value(state->cs, CS_SEL) /* Set MMC CS "low" */
#define CK_H()      gpio_set_value(state->clk, 1) /* Set MMC CLK "high" */
#define CK_L()      gpio_set_value(state->clk, 0) /* Set MMC CLK "low" */
#define DI_H()      gpio_set_value(state->mosi, 1) /* Set MMC DI "high" */
#define DI_L()      gpio_set_value(state->mosi, 0) /* Set MMC DI "low" */
#define DO      gpio_get_value(state->miso) /* Test for MMC DO ('H':true, 'L':false) */


/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

/* MMC/SD command (SPI mode) */
#define CMD0    (0)         /* GO_IDLE_STATE */
#define CMD1    (1)         /* SEND_OP_COND */
#define ACMD41  (0x80+41)   /* SEND_OP_COND (SDC) */
#define CMD8    (8)         /* SEND_IF_COND */
#define CMD9    (9)         /* SEND_CSD */
#define CMD10   (10)        /* SEND_CID */
#define CMD12   (12)        /* STOP_TRANSMISSION */
#define CMD13   (13)        /* SEND_STATUS */
#define ACMD13  (0x80+13)   /* SD_STATUS (SDC) */
#define CMD16   (16)        /* SET_BLOCKLEN */
#define CMD17   (17)        /* READ_SINGLE_BLOCK */
#define CMD18   (18)        /* READ_MULTIPLE_BLOCK */
#define CMD23   (23)        /* SET_BLOCK_COUNT */
#define ACMD23  (0x80+23)   /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24   (24)        /* WRITE_BLOCK */
#define CMD25   (25)        /* WRITE_MULTIPLE_BLOCK */
#define CMD41   (41)        /* SEND_OP_COND (ACMD) */
#define CMD55   (55)        /* APP_CMD */
#define CMD58   (58)        /* READ_OCR */

/* Card type flags (CardType) */
#define CT_MMC      0x01        /* MMC ver 3 */
#define CT_SD1      0x02        /* SD ver 1 */
#define CT_SD2      0x04        /* SD ver 2 */
#define CT_SDC      0x06        /* SD */
#define CT_BLOCK    0x08        /* Block addressing */


static uint32_t Stat = STA_NO_INIT; /* Disk status */

static uint8_t CardType;            /* b0:MMC, b1:SDv1, b2:SDv2, b3:Block addressing */


static int init_port(struct sd_state *state) {
    gpio_set_value(state->power, SD_OFF);
    gpio_set_value(state->mosi, 0);
    gpio_set_value(state->clk, 0);
    gpio_set_value(state->cs, 0);
    gpio_set_value(state->dat1, 0);
    gpio_set_value(state->dat2, 0);

    gpio_set_direction(state->clk, GPIO_OUT);

    eim_set(fpga_w_nand_pwr_ctl, 0);
    usleep(140000);
    CS_H();

    eim_set(fpga_w_nand_pwr_ctl, 1);

    gpio_set_value(state->power, SD_ON);
    usleep(8000);
    return 0;
}


int sd_toggle_clk(struct sd_state *state, int times) {
    while(times--) {
        CK_H(); CK_L();
    }
    return 0;
}


static int nsleep(int nsec) {
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = nsec;
    return nanosleep(&ts, NULL);
}


/*-----------------------------------------------------------------------*/
/* Transmit bytes to the card (bitbanging)                               */
/*-----------------------------------------------------------------------*/

void xmit_spi (
    struct sd_state *state,
    const uint8_t *buff,    /* Data to be sent */
    uint32_t bc     /* Number of bytes to send */
)
{
    uint8_t d;

    gpio_set_direction(state->mosi, GPIO_OUT);

    do {
        d = *buff++;    /* Get a byte to be sent */

        if (d & 0x80) DI_H(); else DI_L();  /* bit7 */
        nsleep(5); CK_H(); nsleep(5); CK_L();

        if (d & 0x40) DI_H(); else DI_L();  /* bit6 */
        nsleep(5); CK_H(); nsleep(5); CK_L();

        if (d & 0x20) DI_H(); else DI_L();  /* bit5 */
        nsleep(5); CK_H(); nsleep(5); CK_L();

        if (d & 0x10) DI_H(); else DI_L();  /* bit4 */
        nsleep(5); CK_H(); nsleep(5); CK_L();

        if (d & 0x08) DI_H(); else DI_L();  /* bit3 */
        nsleep(5); CK_H(); nsleep(5); CK_L();

        if (d & 0x04) DI_H(); else DI_L();  /* bit2 */
        nsleep(5); CK_H(); nsleep(5); CK_L();

        if (d & 0x02) DI_H(); else DI_L();  /* bit1 */
        nsleep(5); CK_H(); nsleep(5); CK_L();

        if (d & 0x01) DI_H(); else DI_L();  /* bit0 */
        nsleep(5); CK_H(); nsleep(5); CK_L();

    } while (--bc);
}

void xmit_mmc_cmd(struct sd_state *state, const uint8_t *buff, uint32_t bc) {
    xmit_spi(state, buff, bc);
    gpio_set_direction(state->cmd, GPIO_IN);
}


void xmit_mmc_dat4 (
    struct sd_state *state,
    const uint8_t *buff,    /* Data to be sent */
    uint32_t bc     /* Number of bytes to send */
)
{
    uint8_t d;

    gpio_set_value(state->dat0, 0);
    gpio_set_value(state->dat1, 0);
    gpio_set_value(state->dat2, 0);
    gpio_set_value(state->dat3, 0);
    gpio_set_direction(state->dat0, GPIO_OUT);
    gpio_set_direction(state->dat1, GPIO_OUT);
    gpio_set_direction(state->dat2, GPIO_OUT);
    gpio_set_direction(state->dat3, GPIO_OUT);

    /* Send start bit (0) */
    CK_H(); CK_L();

    do {
        d = *buff++;    /* Get a byte to be sent */

        /* High byte first */
        gpio_set_value(state->dat0, !!(d&0x10));
        gpio_set_value(state->dat1, !!(d&0x20));
        gpio_set_value(state->dat2, !!(d&0x40));
        gpio_set_value(state->dat3, !!(d&0x80));
        CK_H(); CK_L();

        /* Low byte next */
        gpio_set_value(state->dat0, !!(d&0x01));
        gpio_set_value(state->dat1, !!(d&0x02));
        gpio_set_value(state->dat2, !!(d&0x04));
        gpio_set_value(state->dat3, !!(d&0x08));
        CK_H(); CK_L();
    } while (--bc);

    gpio_set_value(state->dat0, 1);
    gpio_set_value(state->dat1, 1);
    gpio_set_value(state->dat2, 1);
    gpio_set_value(state->dat3, 1);
    CK_H(); CK_L();

    /*
     * Not sure why, but somehow this makes it more reliable
     * to enter factory mode.
     */
    sd_toggle_clk(state, 80);
}


// Wait for the start bit.
// Return the number of clock cycles we've waited.
int rcvr_mmc_dat0_start(struct sd_state *state, int tries) {
    int attempts = 0;
    while (attempts < tries && gpio_get_value(state->dat0)) {
        attempts++;
        CK_L(); CK_H();
        if (!gpio_get_value(state->dat0))
            return attempts;
    }
    return -1;
}

int rcvr_mmc_cmd_start(struct sd_state *state, int tries) {
    int attempts = 0;
    gpio_set_direction(state->cmd, GPIO_IN);
    while (attempts < tries) {
        if (!gpio_get_value(state->cmd)) 
            return attempts;
        attempts++;
        CK_L();
        usleep(1);
        CK_H();
        usleep(1);
        if (!gpio_get_value(state->cmd))
            return attempts;
    }
    return -1;
}


void rcvr_mmc_dat4 (
    struct sd_state *state,
    uint8_t *buff,  /* Pointer to read buffer */
    uint32_t bc     /* Number of bytes to receive */
)
{
    uint8_t r;


    gpio_set_direction(state->dat0, GPIO_IN);
    gpio_set_direction(state->dat1, GPIO_IN);
    gpio_set_direction(state->dat2, GPIO_IN);
    gpio_set_direction(state->dat3, GPIO_IN);

    do {
        r = 0;

        if (gpio_get_value(state->dat0))
            r |= 0x10;
        if (gpio_get_value(state->dat1))
            r |= 0x20;
        if (gpio_get_value(state->dat2))
            r |= 0x40;
        if (gpio_get_value(state->dat3))
            r |= 0x80;

        CK_L(); CK_H();
        if (gpio_get_value(state->dat0))
            r |= 0x01;
        if (gpio_get_value(state->dat1))
            r |= 0x02;
        if (gpio_get_value(state->dat2))
            r |= 0x04;
        if (gpio_get_value(state->dat3))
            r |= 0x08;

        CK_L(); CK_H();

        *buff++ = r;            /* Store a received byte */
    } while (--bc);
}


void rcvr_mmc_dat1 (
    struct sd_state *state,
    uint8_t *buff,  /* Pointer to read buffer */
    uint32_t bc     /* Number of bytes to receive */
)
{
    uint8_t r;


    gpio_set_direction(state->dat0, GPIO_IN);

    do {
        r = 0;   if (gpio_get_value(state->dat0)) r++;  /* bit7 */
        CK_H(); CK_L();
        r <<= 1; if (gpio_get_value(state->dat0)) r++;  /* bit6 */
        CK_H(); CK_L();
        r <<= 1; if (gpio_get_value(state->dat0)) r++;  /* bit5 */
        CK_H(); CK_L();
        r <<= 1; if (gpio_get_value(state->dat0)) r++;  /* bit4 */
        CK_H(); CK_L();
        r <<= 1; if (gpio_get_value(state->dat0)) r++;  /* bit3 */
        CK_H(); CK_L();
        r <<= 1; if (gpio_get_value(state->dat0)) r++;  /* bit2 */
        CK_H(); CK_L();
        r <<= 1; if (gpio_get_value(state->dat0)) r++;  /* bit1 */
        CK_H(); CK_L();
        r <<= 1; if (gpio_get_value(state->dat0)) r++;  /* bit0 */
        CK_L(); CK_H();
        *buff++ = r;            /* Store a received byte */
    } while (--bc);
}


/*-----------------------------------------------------------------------*/
/* Receive bytes from the card (bitbanging)                              */
/*-----------------------------------------------------------------------*/

static int gpio_really_get_value(int gpio) {
    int tries = 1;
    int try;
    int states[tries];
    for (try=0; try<tries; try++) {
        states[try] = gpio_get_value(gpio);
        if (try) {
            if (states[try-1] != states[try]) {
                fprintf(stderr, "GPIO state has changed! Try %d %d -> %d\n", try, states[try-1], states[try]);
            }
        }
    }
    return states[tries-1];
}

void rcvr_mmc_cmd (
    struct sd_state *state,
    uint8_t *buff,  /* Pointer to read buffer */
    uint32_t bc     /* Number of bytes to receive */
)
{
    uint8_t r;

    gpio_set_direction(state->cmd, GPIO_IN);

    do {
        r = 0;   if (gpio_really_get_value(state->cmd)) r++;    /* bit7 */
        CK_H(); CK_L();
        r <<= 1; if (gpio_really_get_value(state->cmd)) r++;    /* bit6 */
        CK_H(); CK_L();
        r <<= 1; if (gpio_really_get_value(state->cmd)) r++;    /* bit5 */
        CK_H(); CK_L();
        r <<= 1; if (gpio_really_get_value(state->cmd)) r++;    /* bit4 */
        CK_H(); CK_L();
        r <<= 1; if (gpio_really_get_value(state->cmd)) r++;    /* bit3 */
        CK_H(); CK_L();
        r <<= 1; if (gpio_really_get_value(state->cmd)) r++;    /* bit2 */
        CK_H(); CK_L();
        r <<= 1; if (gpio_really_get_value(state->cmd)) r++;    /* bit1 */
        CK_H(); CK_L();
        r <<= 1; if (gpio_really_get_value(state->cmd)) r++;    /* bit0 */
        CK_H(); CK_L();
        *buff++ = r;            /* Store a received byte */
    } while (--bc);
}


void rcvr_spi (
    struct sd_state *state,
    uint8_t *buff,  /* Pointer to read buffer */
    uint32_t bc     /* Number of bytes to receive */
)
{
    uint8_t r;

    gpio_set_direction(state->cs, GPIO_OUT);
    gpio_set_direction(state->clk, GPIO_OUT);
    gpio_set_direction(state->mosi, GPIO_OUT);
    gpio_set_direction(state->miso, GPIO_IN);
    gpio_set_value(state->cs, 0);

    DI_H(); /* Send 0xFF */

    do {
        r = 0;   if (DO) r++;   /* bit7 */
        CK_H(); CK_L();
        r <<= 1; if (DO) r++;   /* bit6 */
        CK_H(); CK_L();
        r <<= 1; if (DO) r++;   /* bit5 */
        CK_H(); CK_L();
        r <<= 1; if (DO) r++;   /* bit4 */
        CK_H(); CK_L();
        r <<= 1; if (DO) r++;   /* bit3 */
        CK_H(); CK_L();
        r <<= 1; if (DO) r++;   /* bit2 */
        CK_H(); CK_L();
        r <<= 1; if (DO) r++;   /* bit1 */
        CK_H(); CK_L();
        r <<= 1; if (DO) r++;   /* bit0 */
        CK_H(); CK_L();
        *buff++ = r;            /* Store a received byte */
    } while (--bc);
}



/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

static
int wait_ready (        /* 1:OK, 0:Timeout */
    struct sd_state *state
)
{
    uint8_t d;
    uint32_t tmr;


    for (tmr = 5000; tmr; tmr--) {  /* Wait for ready in timeout of 500ms */
        rcvr_spi(state, &d, 1);
        if (d == 0xFF) break;
        DLY_US(100);
    }

    return tmr ? 1 : 0;
}



/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

void sd_end (
    struct sd_state *state
)
{
    uint8_t d;

    CS_H();
    rcvr_spi(state, &d, 1); /* Dummy clock (force DO hi-z for multiple slave SPI) */
}



/*-----------------------------------------------------------------------*/
/* Select the card and wait for ready                                    */
/*-----------------------------------------------------------------------*/

int sd_begin (  /* 1:OK, 0:Timeout */
    struct sd_state *state
)
{
    uint8_t d;

    CS_L();
    rcvr_spi(state, &d, 1); /* Dummy clock (force DO enabled) */

    if (wait_ready(state)) return 1;    /* OK */
    fprintf(stderr, "Card never came ready\n");
    sd_end(state);
    return 0;           /* Failed */
}



/*-----------------------------------------------------------------------*/
/* Receive a data packet from the card                                   */
/*-----------------------------------------------------------------------*/

static
int rcvr_datablock (    /* 1:OK, 0:Failed */
    struct sd_state *state,
    uint8_t *buff,          /* Data buffer to store received data */
    uint32_t btr            /* Byte count */
)
{
    uint8_t d[2];
    uint32_t tmr;


    for (tmr = 1000; tmr; tmr--) {  /* Wait for data packet in timeout of 100ms */
        rcvr_spi(state, d, 1);
        if (d[0] != 0xFF) break;
        DLY_US(100);
    }
    if (d[0] != 0xFE) return 0;     /* If not valid data token, return with error */

    rcvr_spi(state, buff, btr);         /* Receive the data block into buffer */
    rcvr_spi(state, d, 2);                  /* Discard CRC */

    return 1;                       /* Return with success */
}



/*-----------------------------------------------------------------------*/
/* Send a data packet to the card                                        */
/*-----------------------------------------------------------------------*/

static
int xmit_datablock (    /* 1:OK, 0:Failed */
    struct sd_state *state,
    const uint8_t *buff,    /* 512 byte data block to be transmitted */
    uint8_t token           /* Data/Stop token */
)
{
    uint8_t d[2];


    if (!wait_ready(state)) return 0;

    d[0] = token;
    xmit_spi(state, d, 1);              /* Xmit a token */
    if (token != 0xFD) {        /* Is it data token? */
        xmit_spi(state, buff, 512); /* Xmit the 512 byte data block to MMC */
        rcvr_spi(state, d, 2);          /* Xmit dummy CRC (0xFF,0xFF) */
        rcvr_spi(state, d, 1);          /* Receive data response */
        if ((d[0] & 0x1F) != 0x05)  /* If not accepted, return with error */
            return 0;
    }

    return 1;
}



/*-----------------------------------------------------------------------*/
/* Send a command packet to the card                                     */
/*-----------------------------------------------------------------------*/

static
uint8_t send_cmd (      /* Returns command response (bit7==1:Send failed)*/
    struct sd_state *state,
    uint8_t cmd,        /* Command byte */
    int32_t arg     /* Argument */
)
{
    uint8_t n, d, buf[6];

    if (cmd & 0x80) {   /* ACMD<n> is the command sequense of CMD55-CMD<n> */
        cmd &= 0x7F;
        n = send_cmd(state, CMD55, 0);
        if (n > 1) return n;
    }

    /* Select the card and wait for ready */
    sd_end(state);
    if (!sd_begin(state)) return 0xFF;

    /* Send a command packet */
    buf[0] = 0x40 | cmd;            /* Start + Command index */
    buf[1] = (uint8_t)(arg >> 24);      /* Argument[31..24] */
    buf[2] = (uint8_t)(arg >> 16);      /* Argument[23..16] */
    buf[3] = (uint8_t)(arg >> 8);       /* Argument[15..8] */
    buf[4] = (uint8_t)arg;              /* Argument[7..0] */
    n = 0x01;                       /* Dummy CRC + Stop */
    if (cmd == CMD0) n = 0x95;      /* (valid CRC for CMD0(0)) */
    if (cmd == CMD8) n = 0x87;      /* (valid CRC for CMD8(0x1AA)) */
    buf[5] = n;

    xmit_spi(state, buf, 6);

    /* Receive command response */
    if (cmd == CMD12) rcvr_spi(state, &d, 1);   /* Skip a stuff byte when stop reading */
    n = 10;                             /* Wait for a valid response in timeout of 10 attempts */
    do
        rcvr_spi(state, &d, 1);
    while ((d & 0x80) && --n);

        printf("Sending CMD%d {0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x}: %x\n",
                buf[0]&0x3f, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], d);
    return d;           /* Return with the response value */
}



/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

int disk_status (
    struct sd_state *state
)
{
    int s;
    uint8_t d;


    /* Check if the card is kept initialized */
    s = Stat;
    if (!(s & STA_NOINIT)) {
        if (send_cmd(state, CMD13, 0))  /* Read card status */ {
            s = STA_NOINIT;
            fprintf(stderr, "Card status returned STA_NOINIT\n");
        }
        rcvr_spi(state, &d, 1);     /* Receive following half of R2 */
        sd_end(state);
    }
    Stat = s;

    return s;
}



/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

struct sd_state *sd_init(uint32_t miso, uint32_t mosi,
                         uint32_t clk, uint32_t cs,
                         uint32_t dat1, uint32_t dat2,
                         uint32_t power) {
    struct sd_state *state;


    state = malloc(sizeof(struct sd_state));
    if (!state) {
        perror("Couldn't allocate memory for sd_state");
        return NULL;
    }

    state->miso = miso;
    state->mosi = mosi;
    state->clk = clk;
    state->cs = cs;
    state->dat1 = dat1;
    state->dat2 = dat2;
    state->power = power;

    if (gpio_export(state->miso)) {
        perror("Unable to export DATA IN pin");
        sd_deinit(&state);
        return NULL;
    }
    gpio_set_direction(state->miso, GPIO_IN);


    if (gpio_export(state->mosi)) {
        perror("Unable to export DATA OUT pin");
        sd_deinit(&state);
        return NULL;
    }
    gpio_set_direction(state->mosi, GPIO_OUT);
    gpio_set_value(state->mosi, 1);


    if (gpio_export(state->clk)) {
        perror("Unable to export CLK pin");
        sd_deinit(&state);
        return NULL;
    }
    gpio_set_direction(state->clk, GPIO_OUT);
    gpio_set_value(state->clk, 1);


    /* Grab the chip select pin and deassert it */
    if (gpio_export(state->cs)) {
        perror("Unable to export CS pin");
        sd_deinit(&state);
        return NULL;
    }
    gpio_set_direction(state->cs, GPIO_OUT);
    gpio_set_value(state->cs, CS_DESEL);

    /* Power down the card */
    if (gpio_export(state->power)) {
        perror("Unable to export power pin");
        sd_deinit(&state);
        return NULL;
    }
    gpio_set_direction(state->power, GPIO_OUT);
    gpio_set_value(state->power, SD_OFF);



    if (gpio_export(state->dat1)) {
        perror("Unable to export DAT1 pin");
        sd_deinit(&state);
        return NULL;
    }
    gpio_set_direction(state->dat1, GPIO_OUT);
    gpio_set_value(state->dat1, 0);


    if (gpio_export(state->dat2)) {
        perror("Unable to export DAT2 pin");
        sd_deinit(&state);
        return NULL;
    }
    gpio_set_direction(state->dat2, GPIO_OUT);
    gpio_set_value(state->dat2, 0);

    return state;
}


int sd_reset(struct sd_state *state, int reset_type) {
    uint8_t n, ty, cmd, buf[4];
    uint32_t tmr;
    int s;

    init_port(state);               /* Initialize control port */
    CS_H();
    for (n = 10; n; n--) rcvr_spi(state, buf, 1);   /* 80 dummy clocks */

    ty = 0;
    if (reset_type == 0)
        return 0;
    if (send_cmd(state, CMD0, 0) == 1) {            /* Enter Idle state */
        if (reset_type == 1)
            return 0;
        if (send_cmd(state, CMD8, 0x1AA) == 1) {    /* SDv2? */
            rcvr_spi(state, buf, 4);                            /* Get trailing return value of R7 resp */
            if (buf[2] == 0x01 && buf[3] == 0xAA) {     /* The card can work at vdd range of 2.7-3.6V */
                for (tmr = READY_TRIES; tmr; tmr--) {           /* Wait for leaving idle state (ACMD41 with HCS bit) */
                    if (send_cmd(state, ACMD41, 1UL << 30) == 0) break;
                    DLY_US(1000);
                }
                if (tmr && send_cmd(state, CMD58, 0) == 0) {    /* Check CCS bit in the OCR */
                    rcvr_spi(state, buf, 4);
                    ty = (buf[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;  /* SDv2 */
                }
            }
        } else {                            /* SDv1 or MMCv3 */
            if (send_cmd(state, ACMD41, 0) <= 1)    {
                ty = CT_SD1; cmd = ACMD41;  /* SDv1 */
            } else {
                ty = CT_MMC; cmd = CMD1;    /* MMCv3 */
            }
            for (tmr = 1000; tmr; tmr--) {          /* Wait for leaving idle state */
                if (send_cmd(state, cmd, 0) == 0) break;
                DLY_US(1000);
            }
            if (!tmr || send_cmd(state, CMD16, 512) != 0)   /* Set R/W block length to 512 */
                ty = 0;
        }
    }
    CardType = ty;
    s = ty ? 0 : STA_NOINIT;
    if (s == STA_NOINIT)
        fprintf(stderr, "Type of %d, not initted\n", ty);
    Stat = s;

    sd_end(state);

    return s;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

int sd_read_block (
    struct sd_state *state, /* Physical drive nmuber (0) */
    uint32_t sector,    /* Start sector number (LBA) */
    void *buff,     /* Pointer to the data buffer to store read data */
    uint32_t count      /* Sector count (1..128) */
)
{
    bzero(buff, count*512);
    if (disk_status(state) & STA_NOINIT) return RES_NOTRDY;
    if (!count) return RES_PARERR;
    if (!(CardType & CT_BLOCK)) sector *= 512;  /* Convert LBA to byte address if needed */

    if (count == 1) {   /* Single block read */
        if ((send_cmd(state, CMD17, sector) == 0)   /* READ_SINGLE_BLOCK */
            && rcvr_datablock(state, buff, 512))
            count = 0;
    }
    else {              /* Multiple block read */
        if (send_cmd(state, CMD18, sector) == 0) {  /* READ_MULTIPLE_BLOCK */
            do {
                if (!rcvr_datablock(state, buff, 512)) break;
                buff += 512;
            } while (--count);
            send_cmd(state, CMD12, 0);              /* STOP_TRANSMISSION */
        }
    }
    sd_end(state);

    return count ? RES_ERROR : RES_OK;
}

void sd_deinit(struct sd_state **state) {
    gpio_set_value((*state)->cs, CS_DESEL);
    gpio_set_value((*state)->power, SD_OFF);

    gpio_unexport((*state)->miso);
    gpio_unexport((*state)->mosi);
    gpio_unexport((*state)->clk);
    gpio_unexport((*state)->cs);
    gpio_unexport((*state)->power);
    free(*state);
    *state = NULL;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

int sd_write_block (
    struct sd_state *state,
    uint32_t sector,        /* Start sector number (LBA) */
    const void *buff,   /* Pointer to the data to be written */
    uint32_t count          /* Sector count (1..128) */
)
{
    if (disk_status(state) & STA_NOINIT) return RES_NOTRDY;
    if (!count) return RES_PARERR;
    if (!(CardType & CT_BLOCK)) sector *= 512;  /* Convert LBA to byte address if needed */

    if (count == 1) {   /* Single block write */
        if ((send_cmd(state, CMD24, sector) == 0)   /* WRITE_BLOCK */
            && xmit_datablock(state, buff, 0xFE))
            count = 0;
    }
    else {              /* Multiple block write */
        if (CardType & CT_SDC) send_cmd(state, ACMD23, count);
        if (send_cmd(state, CMD25, sector) == 0) {  /* WRITE_MULTIPLE_BLOCK */
            do {
                if (!xmit_datablock(state, buff, 0xFC)) break;
                buff += 512;
            } while (--count);
            if (!xmit_datablock(state, 0, 0xFD))    /* STOP_TRAN token */
                count = 1;
        }
    }
    sd_end(state);

    return count ? RES_ERROR : RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

int sd_get_csd(struct sd_state *state, uint8_t *csd) {
    int ret;
    bzero(csd, 16);
    ret = send_cmd(state, CMD9, 0);
    if (ret)
        return ret;
    return !rcvr_datablock(state, csd, 16);
}

int sd_get_cid(struct sd_state *state, uint8_t *cid) {
    int ret;
    bzero(cid, 16);
    ret = send_cmd(state, CMD10, 0);
    if (ret)
        return ret;
    return !rcvr_datablock(state, cid, 16);
}


int disk_ioctl (
    struct sd_state *state,
    uint8_t ctrl,       /* Control code */
    void *buff      /* Buffer to send/receive control data */
)
{
    int res;
    uint8_t n, csd[16];
    int32_t cs;


    if (disk_status(state) & STA_NOINIT) return RES_NOTRDY; /* Check if card is in the socket */

    res = RES_ERROR;
    switch (ctrl) {
        case CTRL_SYNC :        /* Make sure that no pending write process */
            if (sd_begin(state)) {
                sd_end(state);
                res = RES_OK;
            }
            break;

        case GET_SECTOR_COUNT : /* Get number of sectors on the disk
(int32_t) */
            if ((send_cmd(state, CMD9, 0) == 0) && rcvr_datablock(state, csd, 16)) {
                if ((csd[0] >> 6) == 1) {   /* SDC ver 2.00 */
                    cs = csd[9] + ((uint16_t)csd[8] << 8) +
((int32_t)(csd[7] & 63) << 8) + 1;
                    *(int32_t*)buff = cs << 10;
                } else {                    /* SDC ver 1.XX or MMC */
                    n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
                    cs = (csd[8] >> 6) +
((uint16_t)csd[7] << 2) + ((uint16_t)(csd[6] & 3) << 10) + 1;
                    *(int32_t*)buff = cs << (n - 9);
                }
                res = RES_OK;
            }
            break;

        case GET_BLOCK_SIZE :   /* Get erase block size in unit of
sector (int32_t) */
            *(int32_t*)buff = 128;
            res = RES_OK;
            break;

        default:
            res = RES_PARERR;
    }

    sd_end(state);

    return res;
}



int print_hex_offset(uint8_t *block, int count, int offset) {
    int byte;
    count += offset;
    block -= offset;
    for ( ; offset<count; offset+=16) {
        printf("%08x ", offset);

        for (byte=0; byte<16; byte++) {
            if (byte == 8)
                printf(" ");
            if (offset+byte < count)
                printf(" %02x", block[offset+byte]&0xff);
            else
                printf("   ");
        }

        printf("  |");
        for (byte=0; byte<16 && byte+offset<count; byte++)
            printf("%c", isprint(block[offset+byte]) ?
                                    block[offset+byte] :
                                    '.');
        printf("|\n");
    }
    return 0;
}

int print_hex(uint8_t *block, int count) {
    return print_hex_offset(block, count, 0);
}



int sd_txrx(struct sd_state *state,
                void *bfr,
                int count,
                uint8_t *outbfr, int outsize) {
    int result = -1;
    int tries;

    sd_end(state);
    if (!sd_begin(state)) return 0xFF;
    xmit_spi(state, bfr, count);

    /* Wait for the card to respond, positive or negative */
    for (tries=0; tries<16; tries++) {
        uint8_t val;
        rcvr_spi(state, &val, 1);
        if (val != 0xff) {
            result = val;
            break;
        }
    }

    if (result != -1 && outbfr && outsize) {
        outbfr[0] = result;
        if (outsize > 1)
            rcvr_spi(state, outbfr+1, outsize-1);
    }

    sd_end(state);
    return result;
}


int sd_enter_factory_mode(struct sd_state *state, uint8_t type) {
    uint8_t bfr[6];
    int i;
    uint8_t response[6];
    static int run = 0;

    init_port(state);
    gpio_set_direction(state->cmd, GPIO_OUT);

    gpio_set_direction(state->dat0, GPIO_OUT);
    gpio_set_direction(state->dat1, GPIO_OUT);
    gpio_set_direction(state->dat2, GPIO_OUT);
    gpio_set_direction(state->dat3, GPIO_OUT);

    /* dat1 and dat2 don't matter, but dat0 and dat3 must be high */
    gpio_set_value(state->dat0, 1);
    gpio_set_value(state->dat1, 0);
    gpio_set_value(state->dat2, 0);
    gpio_set_value(state->dat3, 1);
    run++;

    // Send magical knock sequence
    bfr[0] = 0|0x40;
    bfr[1] = 0;
    bfr[2] = 0;
    bfr[3] = 0;
    bfr[4] = 0;
    bfr[5] = (crc7(bfr, 5)<<1)|1;
    rcvr_mmc_cmd(state, response, 1);
    xmit_mmc_cmd(state, bfr, sizeof(bfr));

    // Send magical knock sequence
    bfr[0] = 63|0x40;
    bfr[1] = 'A';
    bfr[2] = 'P';
    bfr[3] = 'P';
    bfr[4] = 'O';
    bfr[5] = (crc7(bfr, 5)<<1)|1;
    rcvr_mmc_cmd(state, response, 1);
    xmit_mmc_cmd(state, bfr, sizeof(bfr));

    /* Wait for the response */
    gpio_set_direction(state->cmd, GPIO_IN);

    i = rcvr_mmc_cmd_start(state, 256);
    if (i < 0)
        return -1;

    rcvr_mmc_cmd(state, response, sizeof(response));
    if (((crc7(response, 5)<<1)|1) != response[5])
        printf("Knock CRC7 differs.  Got %02x, calculated %02x\n",
               response[5], ((crc7(response, 5)<<1)|1)); 

    // Response should be {0x3f 0x00 0x00 0x00 0x53 0x6b}
    if (response[0] != 0x3f || response[1] != 0x00
     || response[2] != 0x00 || response[3] != 0x00
     || response[4] != 0x53)
        return -1;

    return 0;
}


int sd_mmc_dat4_crc16(uint8_t *bfr, uint8_t *crc_bfr, int size) {
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

    for (i=0; i<4; i++) {
        crcs[i] = crc16(sub_bfr[i], size/4);
    }

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

int sd_read_pins(struct sd_state *state) {
    gpio_set_direction(state->dat0, GPIO_IN);
    gpio_set_direction(state->dat1, GPIO_IN);
    gpio_set_direction(state->dat2, GPIO_IN);
    gpio_set_direction(state->dat3, GPIO_IN);
    gpio_set_direction(state->cmd, GPIO_IN);
    gpio_set_direction(state->clk, GPIO_IN);

    return
        ((!!gpio_get_value(state->dat0)))
      | ((!!gpio_get_value(state->dat1))<<1)
      | ((!!gpio_get_value(state->dat2))<<2)
      | ((!!gpio_get_value(state->dat3))<<3)
      | ((!!gpio_get_value(state->cmd))<<4)
      | ((!!gpio_get_value(state->clk))<<5)
      | 0x40
      ;
}
