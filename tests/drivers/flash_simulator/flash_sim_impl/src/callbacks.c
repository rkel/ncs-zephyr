/*
 * Copyright (c) 2025 Koppel Electronic
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/ztest.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/drivers/flash/flash_simulator.h>
#include <zephyr/device.h>

/* Test the flash simulator callbacks to modify the behaviour of the
 * flash simulator on the fly.
 */

/* configuration derived from DT */
#ifdef CONFIG_ARCH_POSIX
#define SOC_NV_FLASH_NODE DT_CHILD(DT_INST(0, zephyr_sim_flash), flash_0)
#else
#define SOC_NV_FLASH_NODE DT_CHILD(DT_INST(0, zephyr_sim_flash), flash_sim_0)
#endif /* CONFIG_ARCH_POSIX */
#define FLASH_SIMULATOR_BASE_OFFSET DT_REG_ADDR(SOC_NV_FLASH_NODE)
#define FLASH_SIMULATOR_ERASE_UNIT DT_PROP(SOC_NV_FLASH_NODE, erase_block_size)
#define FLASH_SIMULATOR_PROG_UNIT DT_PROP(SOC_NV_FLASH_NODE, write_block_size)
#define FLASH_SIMULATOR_FLASH_SIZE DT_REG_SIZE(SOC_NV_FLASH_NODE)

#define FLASH_SIMULATOR_ERASE_VALUE \
		DT_PROP(DT_PARENT(SOC_NV_FLASH_NODE), erase_value)

#if (defined(CONFIG_ARCH_POSIX) || defined(CONFIG_BOARD_QEMU_X86))
static const struct device *const flash_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_flash_controller));
#else
static const struct device *const flash_dev = DEVICE_DT_GET(DT_NODELABEL(sim_flash_controller));
#endif


/* We are simulating the broken FLASH memory.
 * Simulate different types of errors depending on the erase page.
 * Page 0: behaves normal
 * Page 1: erase works as expected, all writes fails (no write, -EIO returned)
 * Page 2: erase works as expected, all writes silently corrupt data (bits 0, 1, 20 in 32 bit word sticks to 1)
 * Page 3: erase fails (no erase, -EIO returned), all writes fails (no write, -EIO returned)
 * Page 4: erase fails silently (erases writes 0x55 to all bytes), all writes fails (no write, -EIO returned)
 */
