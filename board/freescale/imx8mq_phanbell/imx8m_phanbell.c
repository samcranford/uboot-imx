/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <asm/io.h>
#include <miiphy.h>
#include <netdev.h>
#include <dm.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/boot_mode.h>
#include <asm-generic/gpio.h>
#include <fsl_esdhc.h>
#include <i2c.h>
#include <mmc.h>
#include <asm/arch/imx8mq_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/imx-common/gpio.h>
#include <asm/imx-common/mxc_i2c.h>
#include <asm/arch/clock.h>
#include <spl.h>
#include <power/pmic.h>
#include <usb.h>
#include <dwc3-uboot.h>
#include "board_id.h"

DECLARE_GLOBAL_DATA_PTR;

#define QSPI_PAD_CTRL	(PAD_CTL_DSE2 | PAD_CTL_HYS)

#define UART_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_FSEL1)

#define WDOG_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE)

static iomux_v3_cfg_t const wdog_pads[] = {
	IMX8MQ_PAD_GPIO1_IO02__WDOG1_WDOG_B | MUX_PAD_CTRL(WDOG_PAD_CTRL),
};

#ifdef CONFIG_FSL_QSPI
static iomux_v3_cfg_t const qspi_pads[] = {
	IMX8MQ_PAD_NAND_ALE__QSPI_A_SCLK | MUX_PAD_CTRL(QSPI_PAD_CTRL),
	IMX8MQ_PAD_NAND_CE0_B__QSPI_A_SS0_B | MUX_PAD_CTRL(QSPI_PAD_CTRL),

	IMX8MQ_PAD_NAND_DATA00__QSPI_A_DATA0 | MUX_PAD_CTRL(QSPI_PAD_CTRL),
	IMX8MQ_PAD_NAND_DATA01__QSPI_A_DATA1 | MUX_PAD_CTRL(QSPI_PAD_CTRL),
	IMX8MQ_PAD_NAND_DATA02__QSPI_A_DATA2 | MUX_PAD_CTRL(QSPI_PAD_CTRL),
	IMX8MQ_PAD_NAND_DATA03__QSPI_A_DATA3 | MUX_PAD_CTRL(QSPI_PAD_CTRL),
};

int board_qspi_init(void)
{
	imx_iomux_v3_setup_multiple_pads(qspi_pads, ARRAY_SIZE(qspi_pads));

	set_clk_qspi();

	return 0;
}
#endif

static iomux_v3_cfg_t const uart_pads[] = {
	IMX8MQ_PAD_UART1_RXD__UART1_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	IMX8MQ_PAD_UART1_TXD__UART1_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

int board_early_init_f(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));

	set_wdog_reset(wdog);

	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));

	return 0;
}

#ifdef CONFIG_BOARD_POSTCLK_INIT
int board_postclk_init(void)
{
	/* TODO */
	return 0;
}
#endif

int dram_init(void)
{
	const size_t ram_size = get_ddr_size();

	/* rom_pointer[1] contains the size of TEE occupies */
	if (rom_pointer[1]) {
		gd->ram_size = ram_size - rom_pointer[1];
	} else {
		gd->ram_size = ram_size;
	}

	return 0;
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
	return 0;
}
#endif

#ifdef CONFIG_FEC_MXC
#define FEC_RST_PAD IMX_GPIO_NR(1, 9)
static iomux_v3_cfg_t const fec1_rst_pads[] = {
	IMX8MQ_PAD_GPIO1_IO09__GPIO1_IO9 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void setup_iomux_fec(void)
{
	imx_iomux_v3_setup_multiple_pads(fec1_rst_pads, ARRAY_SIZE(fec1_rst_pads));

	gpio_request(IMX_GPIO_NR(1, 9), "fec1_rst");
	gpio_direction_output(IMX_GPIO_NR(1, 9), 0);
	udelay(500);
	gpio_direction_output(IMX_GPIO_NR(1, 9), 1);
}

static int setup_fec(void)
{
	setup_iomux_fec();

	/* Use 125M anatop REF_CLK1 for ENET1, not from external */
	clrsetbits_le32(IOMUXC_GPR1,
			BIT(13) | BIT(17), 0);
	return set_clk_enet(ENET_125MHz);
}


int board_phy_config(struct phy_device *phydev)
{
	/* enable rgmii rxc skew and phy mode select to RGMII copper */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x1f);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x8);

	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x05);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x100);

	if (phydev->drv->config)
		phydev->drv->config(phydev);
	return 0;
}
#endif

#ifdef CONFIG_USB_DWC3

#define USB_PHY_CTRL0			0xF0040
#define USB_PHY_CTRL0_REF_SSP_EN	BIT(2)

#define USB_PHY_CTRL1			0xF0044
#define USB_PHY_CTRL1_RESET		BIT(0)
#define USB_PHY_CTRL1_COMMONONN		BIT(1)
#define USB_PHY_CTRL1_ATERESET		BIT(3)
#define USB_PHY_CTRL1_VDATSRCENB0	BIT(19)
#define USB_PHY_CTRL1_VDATDETENB0	BIT(20)

#define USB_PHY_CTRL2			0xF0048
#define USB_PHY_CTRL2_TXENABLEN0	BIT(8)

#define PTN5150A_BUS				2
#define PTN5150A_ADDR				0x3D
#define PTN5150A_CONTROL			0x2
#define PTN5150A_CONTROL_PORT_MASK	0x6


static struct dwc3_device dwc3_device_data = {
	.maximum_speed = USB_SPEED_HIGH,
	.base = USB1_BASE_ADDR,
	.dr_mode = USB_DR_MODE_PERIPHERAL,
	.index = 0,
	.power_down_scale = 2,
};

int usb_gadget_handle_interrupts(void)
{
	dwc3_uboot_handle_interrupt(0);
	return 0;
}

static void dwc3_nxp_usb_phy_init(struct dwc3_device *dwc3)
{
	u32 RegData;

	RegData = readl(dwc3->base + USB_PHY_CTRL1);
	RegData &= ~(USB_PHY_CTRL1_VDATSRCENB0 | USB_PHY_CTRL1_VDATDETENB0 |
			USB_PHY_CTRL1_COMMONONN);
	RegData |= USB_PHY_CTRL1_RESET | USB_PHY_CTRL1_ATERESET;
	writel(RegData, dwc3->base + USB_PHY_CTRL1);

	RegData = readl(dwc3->base + USB_PHY_CTRL0);
	RegData |= USB_PHY_CTRL0_REF_SSP_EN;
	writel(RegData, dwc3->base + USB_PHY_CTRL0);

	RegData = readl(dwc3->base + USB_PHY_CTRL2);
	RegData |= USB_PHY_CTRL2_TXENABLEN0;
	writel(RegData, dwc3->base + USB_PHY_CTRL2);

	RegData = readl(dwc3->base + USB_PHY_CTRL1);
	RegData &= ~(USB_PHY_CTRL1_RESET | USB_PHY_CTRL1_ATERESET);
	writel(RegData, dwc3->base + USB_PHY_CTRL1);
}

static void configure_cc_switch() {
	uint8_t value;
	int ret;
	struct udevice *bus, *dev;

	ret = uclass_get_device_by_seq(UCLASS_I2C, PTN5150A_BUS, &bus);
	if (ret) {
		printf("%s: No bus %d\n", __func__, PTN5150A_BUS);
		return;
	}

	ret = dm_i2c_probe(bus, PTN5150A_ADDR, 0, &dev);
	if (ret) {
		printf("%s: Can't find device id=0x%x, on bus %d\n",
			__func__, PTN5150A_ADDR, PTN5150A_BUS);
		return;
	}

	if (dm_i2c_read(dev, PTN5150A_CONTROL, &value, 1)) {
		printf("Unable to configure USB switch");
		return;
	}

	value &= ~PTN5150A_CONTROL_PORT_MASK;
	if (dm_i2c_write(dev, PTN5150A_CONTROL, &value, 1)) {
		printf("Unable to configure USB switch");
		return;
	}
	printf("Configured USB Switch for UFP\n");
}
#endif

#if defined(CONFIG_USB_DWC3) || defined(CONFIG_USB_XHCI_IMX8M)
int board_usb_init(int index, enum usb_init_type init)
{
	imx8m_usb_power(index, true);

	/* Explicitly set USB switch to UFP (device) to ensure proper
	 * fastboot operation on type-C ports. */
	configure_cc_switch();

	if (index == 0 && init == USB_INIT_DEVICE) {
		dwc3_nxp_usb_phy_init(&dwc3_device_data);
		return dwc3_uboot_init(&dwc3_device_data);
	}

	return 0;
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	if (index == 0 && init == USB_INIT_DEVICE) {
		dwc3_uboot_exit(index);
	}

	imx8m_usb_power(index, false);

	return 0;
}
#endif

int board_init(void)
{
	printf("Board id: %i\n", get_board_id());
	printf("Baseboard id: %i\n", get_baseboard_id());

	board_qspi_init();

#ifdef CONFIG_FEC_MXC
	setup_fec();
#endif

	return 0;
}

/* For boot configuration, we need to use the bootstrap info from the ROM.
 * This function already exists in soc.c but relies on board_mmc_get_env_dev.
 * For Phanbell, this will always yield the eMMC. To ensure we boot the
 * bootstrapped device, use this in board_late_init.
 */
static int board_get_rom_mmc_dev(void) {
	struct bootrom_sw_info **p =
		is_soc_rev(CHIP_REV_1_0)? (struct bootrom_sw_info **)ROM_SW_INFO_ADDR_A0 :
		(struct bootrom_sw_info **)ROM_SW_INFO_ADDR;
	int devno = (*p)->boot_dev_instance;
	u8 boot_type = (*p)->boot_dev_type;

	/* If not boot from sd/mmc, use default value */
	if ((boot_type != BOOT_TYPE_SD) && (boot_type != BOOT_TYPE_MMC))
		return CONFIG_SYS_MMC_ENV_DEV;

	return devno;
}

int board_mmc_get_env_dev(int devno)
{
	/*
	 * This is used for determining which MMC fastboot will flash.
	 * For Phanbell, we should always flash the eMMC even if booting
	 * SD. This way SD can be used for recovery.
	 */
	return CONFIG_SYS_MMC_ENV_DEV;
}

int board_late_init(void)
{
	char s[32] = {0};
	int mmc_dev = board_get_rom_mmc_dev();
	struct mmc *fastboot_mmc = find_mmc_device(CONFIG_SYS_MMC_ENV_DEV);

	if (fastboot_mmc) {
		snprintf(s, sizeof(s), "%llu", fastboot_mmc->capacity_user);
		setenv("fastboot.mmc_size", s);
	}

	snprintf(s, sizeof(s), "%i", get_baseboard_id());
	setenv("baseboardid", s);

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	setenv("board_name", "Phanbell");
	setenv("board_rev", "iMX8MQ");
#endif

/* If we aren't supporting Android, set the default boot command
 * to run boot.scr from partition one of the boot device.
 */
#ifdef CONFIG_SD_BOOT
	static char bootdev[32];
	static char bootcmd[128];
	snprintf(bootdev, sizeof(bootdev), "%d", mmc_dev);
	setenv("bootdev", bootdev);
	/* For Zircon, try booting from mmc if ext2load fails */
	snprintf(bootcmd, sizeof(bootcmd), "ext2load mmc %d:1 ${loadaddr} boot.scr; source; boota mmc0 boot_a;", mmc_dev);
	setenv("bootcmd", bootcmd);
#endif

#ifdef CONFIG_ENV_IS_IN_MMC
	board_late_mmc_env_init();
#endif

	return 0;
}

#ifdef CONFIG_FSL_FASTBOOT
#ifdef CONFIG_ANDROID_RECOVERY
int is_recovery_key_pressing(void)
{
	return 0; /*TODO*/
}
#endif /*CONFIG_ANDROID_RECOVERY*/
#endif /*CONFIG_FSL_FASTBOOT*/
