#ifndef __SD_H__
#define __SD_H__
#include <stdint.h>


enum sd_r1_states {
	SD_R1_IDLE_STATE = 0,
};

enum sd_cmds {
	SD_CMD0 = 0,
	SD_CMD1 = 1,
	SD_CMD8 = 8,
	SD_CMD9 = 9,
	SD_CMD12 = 12,
	SD_CMD13 = 13,
	SD_CMD10 = 10,
	SD_CMD16 = 16,
	SD_CMD17 = 17,
	SD_CMD41 = 41,
	SD_CMD55 = 55,
	SD_CMD58 = 58,
};

enum sd_value {
	SD_ON = 1,
	SD_OFF = 0,
};

enum sd_cs {
	CS_SEL = 0,
	CS_DESEL = 1,
};


struct sd_state;

struct sd_state *sd_init(uint32_t cmd_in, uint32_t cmd_out, uint32_t clk,
			 uint32_t cs, uint32_t dat1, uint32_t dat2, uint32_t power);
void sd_deinit(struct sd_state **state);

int sd_reset(struct sd_state *state, int send_cmd0);
int sd_get_ocr(struct sd_state *state, uint8_t ocr[4]);
int sd_get_cid(struct sd_state *state, uint8_t cid[16]);
int sd_get_csd(struct sd_state *state, uint8_t csd[16]);
int sd_get_sr(struct sd_state *state, uint8_t sr[6]);
int sd_set_blocklength(struct sd_state *state, uint32_t blklen);
int sd_read_block(struct sd_state *state, uint32_t offset, void *block, uint32_t count);
int sd_write_block(struct sd_state *state, uint32_t offset, const void *block, uint32_t count);
int sd_cmd60_command(struct sd_state *state, void *bfr);
int sd_some_bfr(struct sd_state *state, void *bytes, int count);

int sd_write_cmd8 (
        struct sd_state *state,
        uint32_t sector,                /* Start sector number (LBA) */
        const void *buff,       /* Pointer to the data to be written */
        uint32_t count                  /* Sector count (1..128) */
);
int sd_dump_rom(struct sd_state *state, void *bfr, int count, uint8_t *out, int getbytes);
int sd_write_file(struct sd_state *state, uint8_t *bfr1, int bfr1_sz, uint8_t *bfr2, int bfr2_sz);

int print_hex(uint8_t *block, int count);


void xmit_spi (
    struct sd_state *state,
    const uint8_t *buff,    /* Data to be sent */
    uint8_t *ibuff,
    uint32_t bc     /* Number of bytes to send */
);
void xmit_mmc_cmd (
    struct sd_state *state,
    const uint8_t *buff,    /* Data to be sent */
    uint32_t bc     /* Number of bytes to send */
);
void xmit_mmc_dat1 (
    struct sd_state *state,
    const uint8_t *buff,    /* Data to be sent */
    uint32_t bc     /* Number of bytes to send */
);
void xmit_mmc_dat4 (
    struct sd_state *state,
    const uint8_t *buff,    /* Data to be sent */
    uint32_t bc     /* Number of bytes to send */
);

void rcvr_spi (
    struct sd_state *state,
    uint8_t *buff,  /* Pointer to read buffer */
    uint32_t bc     /* Number of bytes to receive */
);
void rcvr_mmc_cmd (
    struct sd_state *state,
    uint8_t *buff,  /* Pointer to read buffer */
    uint32_t bc     /* Number of bytes to receive */
);
void rcvr_mmc_dat1 (
    struct sd_state *state,
    uint8_t *buff,  /* Pointer to read buffer */
    uint32_t bc     /* Number of bytes to receive */
);
void rcvr_mmc_dat4 (
    struct sd_state *state,
    uint8_t *buff,  /* Pointer to read buffer */
    uint32_t bc     /* Number of bytes to receive */
);


int sd_enter_factory_mode(struct sd_state *state, uint8_t type);

#endif /* __SD_H__ */
