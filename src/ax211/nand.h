#ifndef __NAND_H__
#define __NAND_H__

struct nand;
struct timespec;

enum nand_status {
    NAND_DAT_OVERFLOW = 0x01,
    NAND_CMD_OVERFLOW = 0x02,
    NAND_XCMD_UNDERFLOW = 0x04,
    NAND_XCMD_OVERFLOW = 0x08,
};

enum nand_log_state {
    NAND_LOG_STOP = 0,
    NAND_LOG_RESET = 1,
    NAND_LOG_RUN = 2,
};

struct nand *nand_init(void);
int nand_log_reset(struct nand *n);
int nand_log_enable(struct nand *n);
int nand_log_disable(struct nand *n);
int nand_log_gettime(struct nand *n, struct timespec *t);
int nand_log_count(struct nand *n);
int nand_log_status(struct nand *n);
int nand_log_max_cmd_depth(struct nand *n);
int nand_log_max_data_depth(struct nand *n);
int nand_log_is_full(struct nand *n);
int nand_log_dump(struct nand *n);

#endif /* __NAND_H__ */
