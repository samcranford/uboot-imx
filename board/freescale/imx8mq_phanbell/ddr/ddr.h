/*
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/io.h>
#include <common.h>

enum fw_type {
	FW_1D_IMAGE,
	FW_2D_IMAGE,
};

#ifdef CONFIG_ENABLE_DDR_TRAINING_DEBUG
#define ddr_printf(args...) printf(args)
#else
#define ddr_printf(args...)
#endif

void ddr_init_micron_3gb(void);
void lpddr4_800M_cfg_phy(void);
void ddr_init_micron_1gb(void);
void ddr_init_kingston_2gb(void);

void ddr_load_train_code(enum fw_type type);
void wait_ddrphy_training_complete(void);

static inline void reg32_write(unsigned long addr, u32 val)
{
	writel(val, addr);
}

static inline uint32_t reg32_read(unsigned long addr) { return readl(addr); }

static void inline dwc_ddrphy_apb_wr(unsigned long addr, u32 val)
{
	writel(val, addr);
}

static inline void reg32setbit(unsigned long addr, u32 bit)
{
	setbits_le32(addr, (1 << bit));
}
