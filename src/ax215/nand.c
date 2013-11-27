#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>

#include "eim.h"
#include "nand.h"
#include "sd.h"

#define LOGENTRY_LEN 8
#define FIFOWIDTH 4
#define DDR3_SIZE (1024 * 1024 * 1) // in words (4 bytes per word)
#define DDR3_FIFODEPTH 64
#define PULSE_GATE_MASK 0x1000

struct nand {
    int         res;
    uint8_t     *data_buffer;
    int         data_buffer_size;
    uint8_t     data_buffer_ctrl;
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

struct nand *nand_init(void) {
    struct nand *n;

    n = malloc(sizeof(struct nand));
    memset(n, 0, sizeof(*n));
    return n;
}

int nand_log_reset(struct nand *n) {
    // Reset the NAND log
    eim_set(fpga_w_log_cmd, NAND_LOG_RESET);
    eim_set(fpga_w_log_cmd, NAND_LOG_STOP);
    return 0;
}

int nand_log_enable(struct nand *n) {
    usleep(10000);
    eim_set(fpga_w_log_cmd, NAND_LOG_RUN); // run the log
    return 0;
}

int nand_log_disable(struct nand *n) {
    eim_set(fpga_w_log_cmd, NAND_LOG_STOP); // stop the log
    return 0;
}

int nand_log_gettime(struct nand *n, struct timespec *t) {
    t->tv_nsec  = eim_get(fpga_r_log_time_nsl) & 0xFFFF;
    t->tv_nsec |= eim_get(fpga_r_log_time_nsh) << 16;
    t->tv_sec   = eim_get(fpga_r_log_time_sl) & 0xFFFF;
    t->tv_sec  |= eim_get(fpga_r_log_time_sh) << 16;
    return 0;
}

int nand_log_count(struct nand *n) {
    int entries;
    entries  = eim_get(fpga_r_log_entry_l) & 0xFFFF;
    entries |= eim_get(fpga_r_log_entry_h) << 16;

    // The actual count is one less than the listed value.
//    if (entries)
//        entries--;
    return entries;
}

int nand_log_status(struct nand *n) {
    int stat;
    stat = eim_get(fpga_r_log_stat);
    return stat;
}

int nand_log_max_cmd_depth(struct nand *n) {
    int stat;
    stat = nand_log_status(n);
    return (stat>>4)&0x1f;
}

int nand_log_max_data_depth(struct nand *n) {
    int stat;
    stat = nand_log_status(n);
    return (stat>>9)&0x7f;
}

int nand_log_is_full(struct nand *n) {
    return eim_get(fpga_r_log_debug)&1;
}



// Dump the NAND log, and call func() for each encountered event.
// Call func() at tne end with a NULL evt to terminate.
static int nand_log_foreach(struct nand *n,
                            int (*func)(struct nand *n,
                                        struct nand_event *evt,
                                        void *ctx),
                            void *ctx) {
    unsigned int readback[DDR3_FIFODEPTH];
    int i;
    int burstaddr = 0;
    int offset;
    unsigned int rv;
    unsigned int arg = 0;
    unsigned int log_start;
    unsigned int records;
    int to_print;
    struct nand_event evt;

    records = nand_log_count(n);

    offset = 0x10; // accessing port 3 (read port)
    burstaddr = 0x0F000000 / 4; // log starts at 16MB for non-sandisk version
    log_start = 0x0F000000 / 4;

    eim_set(fpga_w_ddr3_p2_ladr + offset, ((burstaddr * 4) & 0xFFFF));
    eim_set(fpga_w_ddr3_p2_hadr + offset, ((burstaddr * 4) >> 16) & 0xFFFF);

    to_print = records;

    while (burstaddr < (records * LOGENTRY_LEN / FIFOWIDTH + log_start)) {
        arg = ((DDR3_FIFODEPTH - 1) << 4) | 1;
        eim_set(fpga_w_ddr3_p3_cmd, arg | PULSE_GATE_MASK);
        arg |= 0x8;
        eim_set(fpga_w_ddr3_p3_cmd, arg | PULSE_GATE_MASK);
        arg &= ~0x8;
        eim_set(fpga_w_ddr3_p3_cmd, arg | PULSE_GATE_MASK);
        for (i = 0; i < DDR3_FIFODEPTH; i++) {
            uint32_t raw_pkt;
            while ((eim_get(fpga_r_ddr3_p3_stat) & 4))
                ;
            rv = eim_get(fpga_r_ddr3_p3_ldat);
            raw_pkt = ((uint32_t) rv) & 0xFFFF;
            rv = eim_get(fpga_r_ddr3_p3_hdat);
            raw_pkt |= (rv << 16);
            readback[i] = raw_pkt;
        }

        while( !(eim_get(fpga_r_ddr3_p3_stat) & 0x4) ) {
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

            if(to_print>0)
                func(n, &evt, ctx);
            to_print--;
        }

        burstaddr += DDR3_FIFODEPTH;
        eim_set( fpga_w_ddr3_p2_ladr + offset, ((burstaddr * 4) & 0xFFFF));
        eim_set( fpga_w_ddr3_p2_hadr + offset, ((burstaddr * 4) >> 16) & 0xFFFF);
    }
    func(n, NULL, ctx);
    return 0;
}


static int trace_printbuffer(struct nand *n) {
    if (!n->data_buffer_size)
        return 0;

    if (n->data_buffer_ctrl&nand_ctrl_we)
        printf("Write buffer:\n");
    else if (n->data_buffer_ctrl&nand_ctrl_re)
        printf("Read buffer:\n");
    else
        printf("No-flags buffer (bug?):\n");
    print_hex(n->data_buffer, n->data_buffer_size);
    free(n->data_buffer);
    n->data_buffer = NULL;
    n->data_buffer_size = 0;
    n->data_buffer_ctrl = 0;
    return 0;
}


static int trace_printline(struct nand *n, struct nand_event *evt, void *ctx) {
    int *num = ctx;

    // End-of-stream
    if (!evt) {
        // Drain the buffer if necessary
        if (n->data_buffer)
            return trace_printbuffer(n);
        return 0;
    }

    // If the packet type has changed, empty out the old buffer
    if (n->data_buffer_ctrl && (evt->ctrl != n->data_buffer_ctrl))
        trace_printbuffer(n);

    // If it's a data packet, stuff it in a buffer
    if ((!(evt->ctrl&nand_ctrl_ale)) && (!(evt->ctrl&nand_ctrl_cle))) {
        n->data_buffer_ctrl = evt->ctrl;
        n->data_buffer = realloc(n->data_buffer, ++(n->data_buffer_size));
        n->data_buffer[n->data_buffer_size-1] = evt->data;
    }
    else {
        printf("Packet %6d: %4s %4s %4s %4s %4s %02x . %lu.%lu\n",
            (*num)++,
            evt->ctrl&nand_ctrl_ale?"ALE":"",
            evt->ctrl&nand_ctrl_cle?"CLE":"",
            evt->ctrl&nand_ctrl_we ?"WE" :"",
            evt->ctrl&nand_ctrl_re ?"RE" :"",
            evt->ctrl&nand_ctrl_cs ?"CS" :"",
            evt->data, evt->ts.tv_sec, evt->ts.tv_nsec);
    }
    return 0;
}

static char *event_types[] = {
    "ADDR",
    "COMMAND",
    "READ",
    "WRITE",
    "UNKNOWN",
};

enum nand_event_type {
    nand_address,
    nand_command,
    nand_read,
    nand_write,
    nand_unknown,
};

struct nand_summary_event {
    enum nand_event_type type;
    int count;
    uint8_t data;
};


struct nand_summary {
    uint8_t last_ctrl;
    struct nand_summary_event **events;
    int event_count;
};

static int summarize_add(struct nand *n, struct nand_event *evt, void *ctx) {
    struct nand_summary *s = ctx;
    // Print out summary
    if (!evt) {
        int i;
        for (i=0; i<s->event_count; i++) {
            if (i)
                printf(", ");
            if (s->events[i]->count>1)
                printf("%s[%d]", event_types[s->events[i]->type], s->events[i]->count);
            else if (s->events[i]->type == nand_command)
                printf("%s %02x", event_types[s->events[i]->type], s->events[i]->data);
            else
                printf("%s", event_types[s->events[i]->type]);
            free(s->events[i]);
        }
        free(s->events);
        return 0;
    }

    if (evt->ctrl == s->last_ctrl) {
        s->events[s->event_count-1]->count++;
        return 0;
    }

    s->events = realloc(s->events, sizeof(struct nand_summary_event *)*s->event_count+1);
    s->events[s->event_count++] = malloc(sizeof(struct nand_summary_event));
    s->last_ctrl = evt->ctrl;

    memset(s->events[s->event_count-1], 0, sizeof(struct nand_summary_event));
    s->events[s->event_count-1]->count = 1;
    if ( 
            (evt->ctrl&nand_ctrl_cle)
        && !(evt->ctrl&nand_ctrl_ale)
       )
        s->events[s->event_count-1]->type = nand_command;
    else if ( 
           !(evt->ctrl&nand_ctrl_cle)
        &&  (evt->ctrl&nand_ctrl_ale)
       )
        s->events[s->event_count-1]->type = nand_address;
    else if ( 
            (evt->ctrl&nand_ctrl_cle)
        &&  (evt->ctrl&nand_ctrl_ale)
       )
        s->events[s->event_count-1]->type = nand_unknown;
    else if ( 
            (evt->ctrl&nand_ctrl_we)
        && !(evt->ctrl&nand_ctrl_re)
       )
        s->events[s->event_count-1]->type = nand_write;
    else if ( 
           !(evt->ctrl&nand_ctrl_we)
        &&  (evt->ctrl&nand_ctrl_re)
       )
        s->events[s->event_count-1]->type = nand_read;
    else
        s->events[s->event_count-1]->type = nand_unknown;

    s->events[s->event_count-1]->data = evt->data;

    return 0;
}


int nand_log_dump(struct nand *n) {
    int num = 0;
    return nand_log_foreach(n, trace_printline, &num);
}

// print CMD / ADDR[x] / CMD[x] / DATA[x]
int nand_log_summarize(struct nand *n) {
    struct nand_summary s;
    memset(&s, 0, sizeof(s));
    nand_log_foreach(n, summarize_add, &s);
    printf("\n");
    return 0;
}
