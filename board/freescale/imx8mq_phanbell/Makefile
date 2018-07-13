#
# Copyright 2016 Freescale Semiconductor
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += imx8m_phanbell.o board_id.o
obj-$(CONFIG_ZIRCON_BOOT_IMAGE) += zircon.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-y += ddr/helper.o ddr/wait_ddrphy_training_complete.o
obj-y += ddr/ddr_init_micron_3gb.o ddr/ddrphy_train_micron_3gb.o
obj-y += ddr/ddr_init_micron_1gb.o ddr/ddrphy_train_micron_1gb.o
obj-y += ddr/ddr_init_hynix_1gb.o ddr/ddrphy_train_hynix_1gb.o
endif
