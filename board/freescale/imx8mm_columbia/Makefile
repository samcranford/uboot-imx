#
# Copyright 2018 NXP
#
# SPDX-License-Identifier:      GPL-2.0+
#

ccflags-y += -I./drivers/fastboot/fb_fsl
obj-y += imx8mm_columbia.o board_id.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_timing.o
obj-$(CONFIG_IMX8M_DDR4) += ddr4_timing.o
endif
