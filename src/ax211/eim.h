#ifndef __eim_h__
#define __eim_h__

enum eim_type {

    fpga_w_test0 = 0x40000,
    fpga_w_test1 = 0x40002,
    fpga_w_gpioa_dout = 0x40010,
    fpga_w_gpioa_dir = 0x40012,
    fpga_w_gpiob_dout = 0x40014,
    fpga_w_gpiob_dir = 0x40016,

    fpga_w_ddr3_p2_cmd = 0x40020,
    fpga_w_ddr3_p2_ladr = 0x40022,
    fpga_w_ddr3_p2_hadr = 0x40024,
    fpga_w_ddr3_p2_wen = 0x40026,
    fpga_w_ddr3_p2_ldat = 0x40028,
    fpga_w_ddr3_p2_hdat = 0x4002a,

    fpga_w_ddr3_p3_cmd = 0x40030,
    fpga_w_ddr3_p3_ladr = 0x40032,
    fpga_w_ddr3_p3_hadr = 0x40034,
    fpga_w_ddr3_p3_ren = 0x40036,

    fpga_w_nand_uk_ctl = 0x40100,
    fpga_w_nand_pwr_ctl = 0x40102,

    fpga_w_log_cmd = 0x40200,
    fpga_w_log_config = 0x40202,
    fpga_w_mcudrv = 0x40210,

    fpga_r_test0 = 0x41000,
    fpga_r_test1 = 0x41002,
    fpga_r_ddr3_cal = 0x41004,
    fpga_r_gpioa_din = 0x41010,
    fpga_r_gpiob_din = 0x41012,

    fpga_r_ddr3_p2_stat = 0x41020,
    fpga_r_ddr3_p3_stat = 0x41030,
    fpga_r_ddr3_p3_ldat = 0x41032,
    fpga_r_ddr3_p3_hdat = 0x41034,

    // every time I read it auto-advances the queue
    fpga_r_nand_uk_data = 0x41100,
    fpga_r_nand_uk_stat = 0x41102,

    // every time I read it auto-advances the queue
    fpga_r_nand_cmd_data = 0x41104,
    fpga_r_nand_cmd_stat = 0x41106,

    fpga_r_nand_adr_stat = 0x41108,
    fpga_r_nand_adr_low = 0x4110a,
    // every time I read it auto-advances the queue
    fpga_r_nand_adr_hi = 0x4110c,

    fpga_r_nand_ddr_stat = 0x4110e,

    fpga_r_log_stat = 0x41200,
    fpga_r_log_entry_l = 0x41202,
    fpga_r_log_entry_h = 0x41204,
    // this auto-loads time_t
    fpga_r_log_time_nsl = 0x41206,
    fpga_r_log_time_nsh = 0x41208,
    fpga_r_log_time_sl = 0x4120a,
    fpga_r_log_time_sh = 0x4120c,
    fpga_r_log_san_flsh = 0x41210,
    fpga_r_log_san_uk = 0x41212,
    fpga_r_log_debug = 0x41220,

    fpga_r_ddr3_v_minor = 0x41ffc,
    fpga_r_ddr3_v_major = 0x41ffe,

    fpga_romulator_base = 0,
};

#define eim_nand_adr_bits 0x3fff

uint16_t eim_get(enum eim_type type);
uint16_t eim_set(enum eim_type type, uint16_t value);

#endif // __eim_h__
