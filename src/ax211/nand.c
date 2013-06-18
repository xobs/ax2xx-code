#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include "eim.h"
#include "nand.h"

#define LOGENTRY_LEN 8
#define FIFOWIDTH 4
#define DDR3_SIZE (1024 * 1024 * 1) // in words (4 bytes per word)
#define DDR3_FIFODEPTH 64
#define PULSE_GATE_MASK 0x1000

struct nand_state {
    int res;
};

struct nand_event {
    uint8_t data;
    uint8_t ctrl;
    uint8_t unk[2];
    struct timespec ts;
} __attribute__((__packed__));

enum nand_ctrl_pins {
    nand_ctrl_ale = 0x1,
    nand_ctrl_cle = 0x2,
    nand_ctrl_re  = 0x4,
    nand_ctrl_we  = 0x8,
    nand_ctrl_cs  = 0x10,
};

struct nand_state *nand_init(void) {
    struct nand_state *n;

    n = malloc(sizeof(struct nand_state));
    memset(n, 0, sizeof(*n));
    return n;
}

int nand_log_reset(struct nand_state *n) {
    // Reset the NAND log
    eim_set(fpga_w_log_cmd, NAND_LOG_RESET);
    eim_set(fpga_w_log_cmd, NAND_LOG_STOP);
    return 0;
}

int nand_log_enable(struct nand_state *n) {
    usleep(10000);
    eim_set(fpga_w_log_cmd, NAND_LOG_RUN); // run the log
    return 0;
}

int nand_log_disable(struct nand_state *n) {
    eim_set(fpga_w_log_cmd, NAND_LOG_STOP); // stop the log
    return 0;
}

int nand_log_gettime(struct nand_state *n, struct timespec *t) {
    t->tv_nsec  = eim_get(fpga_r_log_time_nsl) & 0xFFFF;
    t->tv_nsec |= eim_get(fpga_r_log_time_nsh) << 16;
    t->tv_sec   = eim_get(fpga_r_log_time_sl) & 0xFFFF;
    t->tv_sec  |= eim_get(fpga_r_log_time_sh) << 16;
    return 0;
}

int nand_log_count(struct nand_state *n) {
    int entries;
    entries  = eim_get(fpga_r_log_entry_l) & 0xFFFF;
    entries |= eim_get(fpga_r_log_entry_h) << 16;

    // The actual count is one less than the listed value.
    if (entries)
        entries--;
    return entries;
}

int nand_log_status(struct nand_state *n) {
    int stat;
    stat = eim_get(fpga_r_log_stat);
    return stat;
}

int nand_log_max_cmd_depth(struct nand_state *n) {
    int stat;
    stat = nand_log_status(n);
    return (stat>>4)&0x1f;
}

int nand_log_max_data_depth(struct nand_state *n) {
    int stat;
    stat = nand_log_status(n);
    return (stat>>9)&0x7f;
}

int nand_log_is_full(struct nand_state *n) {
    return eim_get(fpga_r_log_debug)&1;
}


static void trace_printline(int num, struct nand_event *evt) {
    printf("Packet %6d: %4s %4s %4s %4s %4s %02x . %lu.%lu\n",
            num,
            evt->ctrl&nand_ctrl_ale?"ALE":"",
            evt->ctrl&nand_ctrl_cle?"CLE":"",
            evt->ctrl&nand_ctrl_we ?"WE" :"",
            evt->ctrl&nand_ctrl_re ?"RE" :"",
            evt->ctrl&nand_ctrl_cs ?"CS" :"",
            evt->data, evt->ts.tv_sec, evt->ts.tv_nsec);
}

int nand_log_dump(struct nand_state *n) {
    unsigned int readback[DDR3_FIFODEPTH];
    int i;
    int line=0;
    int burstaddr = 0;
    int offset;
    unsigned int rv;
    unsigned int arg = 0;
    unsigned int log_start;
    unsigned int records;
    int to_print;
    int verbose;
    struct nand_event evt;

    records = nand_log_count(n);
    verbose = 1;

    offset = 0x10; // accessing port 3 (read port)
    burstaddr = 0x0F000000 / 4; // log starts at 16MB for non-sandisk version
    log_start = 0x0F000000 / 4;

    eim_set(fpga_w_ddr3_p2_ladr + offset, ((burstaddr * 4) & 0xFFFF));
    eim_set(fpga_w_ddr3_p2_hadr + offset, ((burstaddr * 4) >> 16) & 0xFFFF);

    to_print = records;
    printf("dumping %u record%s", to_print, to_print==1?"":"s");
    if (verbose)
        printf( "\n" );

    while (burstaddr < (records * LOGENTRY_LEN / FIFOWIDTH + log_start)) {
        arg = ((DDR3_FIFODEPTH - 1) << 4) | 1;
        eim_set(fpga_w_ddr3_p3_cmd, arg | PULSE_GATE_MASK);
        arg |= 0x8;
        eim_set(fpga_w_ddr3_p3_cmd, arg | PULSE_GATE_MASK);
        arg &= ~0x8;
        eim_set(fpga_w_ddr3_p3_cmd, arg | PULSE_GATE_MASK);
        for (i = 0; i < DDR3_FIFODEPTH; i++) {
            uint32_t raw_pkt;
            while ((eim_get(fpga_r_ddr3_p3_stat) & 4)) {
                putchar('i'); fflush(stdout);// wait for queue to become full before reading
            }
            rv = eim_get(fpga_r_ddr3_p3_ldat);
            raw_pkt = ((uint32_t) rv) & 0xFFFF;
            rv = eim_get(fpga_r_ddr3_p3_hdat);
            raw_pkt |= (rv << 16);
            readback[i] = raw_pkt;
        }

        while( !(eim_get(fpga_r_ddr3_p3_stat) & 0x4) ) {
            putchar('x'); fflush(stdout); // error, should be empty now
            eim_set(fpga_w_ddr3_p3_ren, 0x10);
            eim_set(fpga_w_ddr3_p3_ren, 0x00);
        }
        for( i = 0; i < DDR3_FIFODEPTH; i += 2 ) {
            evt.data = readback[i] & 0xFF;
            evt.ctrl = (readback[i] >> 8) & 0x1F; // control is already lined up by FPGA mapping
            evt.unk[0] = ((readback[i] >> 13) & 0xFF);
            evt.unk[1] = ((readback[i] >> 21) & 0x3);

            // now prepare nsec, sec values:
            // bits 31-23 are LSB of nsec
            // 1111 1111 1_111 1111 . 1111 1111 1111 1111
            evt.ts.tv_nsec = ((readback[i+1] & 0x7FFFFF) << 9) | ((readback[i] >> 23) & 0x1FF);
            evt.ts.tv_sec = (readback[i+1] >> 23) & 0x1FF;
//            if( (burstaddr + i + 1) < (records * LOGENTRY_LEN / FIFOWIDTH) )
//                write(ofd, &evt, sizeof(evt));

            if(verbose && to_print>0)
                trace_printline(line++, &evt);
            to_print--;
        }
        if( !verbose ) {
            if( (burstaddr % (1024 * 128)) == 0 ) {
                printf( "." );
                fflush(stdout);
            }
        }

        burstaddr += DDR3_FIFODEPTH;
        eim_set( fpga_w_ddr3_p2_ladr + offset, ((burstaddr * 4) & 0xFFFF));
        eim_set( fpga_w_ddr3_p2_hadr + offset, ((burstaddr * 4) >> 16) & 0xFFFF);
    }
    return 0;
}
