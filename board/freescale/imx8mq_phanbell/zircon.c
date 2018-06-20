// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <common.h>
#include <part.h>
#include <zircon/zircon.h>

#define PDEV_VID_GOOGLE             3
#define PDEV_PID_MADRONE            4

#define NVRAM_LENGTH                (1024 * 1024)

static const zbi_cpu_config_t cpu_config = {
    .cluster_count = 1,
    .clusters = {
        {
            .cpu_count = 4,
        },
    },
};

static const zbi_mem_range_t mem_config[] = {
    {
        .type = ZBI_MEM_RANGE_RAM,
        .paddr = 0x40000000,
        .length = 0xc0000000, // 3GB
    },
    {
        .type = ZBI_MEM_RANGE_PERIPHERAL,
        .paddr = 0,
        .length = 0x40000000,
    },
};

static const dcfg_simple_t uart_driver = {
    .mmio_phys = 0x30860000,
    .irq = 58,
};

static const dcfg_arm_gicv3_driver_t gicv3_driver = {
    .mmio_phys = 0x38800000,
    .gicd_offset = 0x00000,
    .gicr_offset = 0x80000,
    .gicr_stride = 0x20000,
    .ipi_base = 9,
    // Used for Errata e11171
    .mx8_gpr_phys = 0x30340000,
};

static const dcfg_arm_psci_driver_t psci_driver = {
    .use_hvc = false,
};

static const dcfg_arm_generic_timer_driver_t timer_driver = {
    .irq_phys = 30,
    .irq_virt = 27,
    .freq_override = 8333333,
};

static const zbi_platform_id_t platform_id = {
    .vid = PDEV_VID_GOOGLE,
    .pid = PDEV_PID_MADRONE,
    .board_name = "madrone",
};

enum {
    PART_ZIRCON_A,
    PART_ZIRCON_B,
    PART_ZIRCON_R,
    PART_SYS_CONFIG,
    PART_FACTORY_CONFIG,
    PART_FVM,
    PART_COUNT,
};

static zbi_partition_map_t partition_map = {
    // .block_count filled in below
    // .block_size filled in below
    .guid = {},
    .partition_count = PART_COUNT,
    .partitions = {
        {
            .type_guid = GUID_ZIRCON_A_VALUE,
            .uniq_guid = {},
            // .first_block filled in below
            // .last_block filled in below
            .flags = 0,
            .name = "zircon-a",
        },
        {
            .type_guid = GUID_ZIRCON_B_VALUE,
            .uniq_guid = {},
            // .first_block filled in below
            // .last_block filled in below
            .flags = 0,
            .name = "zircon-b",
        },
        {
            .type_guid = GUID_ZIRCON_R_VALUE,
            .uniq_guid = {},
            // .first_block filled in below
            // .last_block filled in below
            .flags = 0,
            .name = "zircon-r",
        },
        {
            .type_guid = GUID_SYS_CONFIG_VALUE,
            .uniq_guid = {},
            // .first_block filled in below
            // .last_block filled in below
            .flags = 0,
            .name = "sys-config",
        },
        {
            .type_guid = GUID_FACTORY_CONFIG_VALUE,
            .uniq_guid = {},
            // .first_block filled in below
            // .last_block filled in below
            .flags = 0,
            .name = "factory",
        },
        {
            .type_guid = GUID_FVM_VALUE,
            .uniq_guid = {},
            // .first_block filled in below
            // .last_block filled in below
            .flags = 0,
            .name = "fvm",
        },
    },
};

static void add_partition_map(zbi_header_t* zbi) {
    struct blk_desc* dev_desc;
    disk_partition_t boot_a_info;
    disk_partition_t boot_b_info;
    disk_partition_t system_a_info;
    disk_partition_t misc_info;
    disk_partition_t factory_info;
    disk_partition_t userdata_info;

    dev_desc = blk_get_dev("mmc", 0);
    if (!dev_desc || dev_desc->type == DEV_TYPE_UNKNOWN) {
        printf("could not find MMC device for partition map\n");
        return;
    }

    if (part_get_info_by_name(dev_desc, "boot_a", &boot_a_info)) {
        printf("could not find boot_a partition\n");
        return;
    }
    if (part_get_info_by_name(dev_desc, "boot_b", &boot_b_info)) {
        printf("could not find boot_b partition\n");
        return;
    }
    if (part_get_info_by_name(dev_desc, "system_a", &system_a_info)) {
        printf("could not find system_a partition\n");
        return;
    }
    if (part_get_info_by_name(dev_desc, "misc", &misc_info)) {
        printf("could not find misc partition\n");
        return;
    }
    if (part_get_info_by_name(dev_desc, "factory", &factory_info)) {
        printf("could not find factory partition\n");
        return;
    }
    if (part_get_info_by_name(dev_desc, "userdata", &userdata_info)) {
        printf("could not find userdata partition\n");
        return;
    }

    // map boot_a partition to ZIRCON_A
    partition_map.partitions[PART_ZIRCON_A].first_block = boot_a_info.start;
    partition_map.partitions[PART_ZIRCON_A].last_block = boot_a_info.start + boot_a_info.size - 1;

    // map boot_b partition to ZIRCON_B
    partition_map.partitions[PART_ZIRCON_B].first_block = boot_b_info.start;
    partition_map.partitions[PART_ZIRCON_B].last_block = boot_b_info.start + boot_b_info.size - 1;

    // map system_a partition to ZIRCON_R
    partition_map.partitions[PART_ZIRCON_R].first_block = system_a_info.start;
    partition_map.partitions[PART_ZIRCON_R].last_block = system_a_info.start +
                                                         system_a_info.size - 1;

    // map misc partition to SYS_CONFIG
    partition_map.partitions[PART_SYS_CONFIG].first_block = misc_info.start;
    partition_map.partitions[PART_SYS_CONFIG].last_block = misc_info.start + misc_info.size - 1;

    // map factory partition to FACTORY_CONFIG
    partition_map.partitions[PART_FACTORY_CONFIG].first_block = factory_info.start;
    partition_map.partitions[PART_FACTORY_CONFIG].last_block = factory_info.start +
                                                               factory_info.size - 1;

    // map userdata partition to FVM
    partition_map.partitions[PART_FVM].first_block = userdata_info.start;
    partition_map.partitions[PART_FVM].last_block = userdata_info.start +
                                                    userdata_info.size - 1;

    partition_map.block_count = userdata_info.start + userdata_info.size;
    partition_map.block_size = userdata_info.blksz;

    zircon_append_boot_item(zbi, ZBI_TYPE_DRV_PARTITION_MAP, 0, &partition_map,
                            sizeof(zbi_partition_map_t) +
                            partition_map.partition_count * sizeof(zbi_partition_t));
}

#if 0
static int hex_digit(char ch) {
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    } else if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    } else if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    } else {
        return -1;
    }
}

static void add_eth_mac_address(zbi_header_t* zbi) {
    char* str = getenv("eth_mac");
    uint8_t addr[6];

    // this would be easier with sscanf
    int i;
    for (i = 0; i < 6; i++) {
        unsigned left, right;
        if (str[0] && str[1] && (left = hex_digit(*str++)) >= 0 &&
                (right = hex_digit(*str++)) >= 0) {
            addr[i] = (left << 4) | right;
        } else {
            goto failed;
        }
        if (i < 5 && *str++ != ':') {
            goto failed;
        }
    }

    zircon_append_boot_item(zbi, ZBI_TYPE_DRV_MAC_ADDRESS, 0, addr, sizeof(addr));
    return;

failed:
    printf("MAC address parsing failed for \"%s\"\n", getenv("eth_mac"));
}
#endif

int zircon_preboot(zbi_header_t* zbi) {
    // add CPU configuration
    zircon_append_boot_item(zbi, ZBI_TYPE_CPU_CONFIG, 0, &cpu_config,
                            sizeof(zbi_cpu_config_t) +
                            sizeof(zbi_cpu_cluster_t) * cpu_config.cluster_count);

    // allocate crashlog save area at end of RAM
    zbi_nvram_t nvram;
    nvram.base = mem_config[0].length - NVRAM_LENGTH;
    nvram.length = NVRAM_LENGTH;
    zircon_append_boot_item(zbi, ZBI_TYPE_NVRAM, 0, &nvram, sizeof(nvram));

    // add memory configuration
    zircon_append_boot_item(zbi, ZBI_TYPE_MEM_CONFIG, 0, &mem_config, sizeof(mem_config));

    // add kernel drivers
    zircon_append_boot_item(zbi, ZBI_TYPE_KERNEL_DRIVER, KDRV_NXP_IMX_UART, &uart_driver,
                    sizeof(uart_driver));
    zircon_append_boot_item(zbi, ZBI_TYPE_KERNEL_DRIVER, KDRV_ARM_GIC_V3, &gicv3_driver,
                    sizeof(gicv3_driver));
    zircon_append_boot_item(zbi, ZBI_TYPE_KERNEL_DRIVER, KDRV_ARM_PSCI, &psci_driver,
                    sizeof(psci_driver));
    zircon_append_boot_item(zbi, ZBI_TYPE_KERNEL_DRIVER, KDRV_ARM_GENERIC_TIMER, &timer_driver,
                    sizeof(timer_driver));

    // add platform ID
    zircon_append_boot_item(zbi, ZBI_TYPE_PLATFORM_ID, 0, &platform_id, sizeof(platform_id));

    add_partition_map(zbi);
//    add_eth_mac_address(zbi);
    return 0;
 }
