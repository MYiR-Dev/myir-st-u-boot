/* SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause */
/*
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 *
 * Configuration settings for the STMicroelectronics STM32MP13x boards
 */

#ifndef __CONFIG_STM32MP13_ST_COMMON_H__
#define __CONFIG_STM32MP13_ST_COMMON_H__

#define STM32MP_BOARD_EXTRA_ENV \
	"usb_pgood_delay=2000\0" \
	"mtdids=spi-nand0=spi-nand0\0" \
	"mtdparts=mtdparts=spi-nand0:512k(fsbl1),512k(fsbl2),512k(metadata1),512k(metadata2),4m(fip-a1),4m(fip-a2),4m(fip-b1),4m(fip-b2),-(UBI);\0" \
	"console=ttySTM0\0"

#include <configs/stm32mp13_common.h>

/* uart with on-board st-link */
#define CFG_SYS_BAUDRATE_TABLE      { 9600, 19200, 38400, 57600, 115200, \
					 230400, 460800, 921600, \
					 1000000, 2000000, 4000000}

#ifdef CFG_EXTRA_ENV_SETTINGS
/*
 * default bootcmd for stm32mp13 STMicroelectronics boards:
 * for serial/usb: execute the stm32prog command
 * for mmc boot (eMMC, SD card), distro boot on the same mmc device
 * for nand or spi-nand boot, distro boot with ubifs on UBI partition or
 * sdcard
 * for nor boot, distro boot on SD card = mmc0 ONLY !
 */
#ifdef CONFIG_MYIR_512N512D
#define ST_STM32MP13_FUSE_PROG "fuse prog -y 0 9 20400000;"
#else
#define ST_STM32MP13_FUSE_PROG 
#endif
#define ST_STM32MP13_BOOTCMD "bootcmd_stm32mp=" \
	"echo \"Boot over ${boot_device}${boot_instance}!\";" \
	"if test ${boot_device} = serial || test ${boot_device} = usb;" \
	"then stm32prog ${boot_device} ${boot_instance}; " \
	"else " \
		"run env_check;" \
		"if test ${boot_device} = mmc;" \
		"then env set boot_targets \"mmc${boot_instance}\"; fi;" \
		"if test ${boot_device} = nand ||" \
		  " test ${boot_device} = spi-nand ;" \
		"then env set boot_targets ubifs0 mmc0;" \
		ST_STM32MP13_FUSE_PROG \
		"fi;" \
		"if test ${boot_device} = nor;" \
		"then env set boot_targets mmc0; fi;" \
		"run distro_bootcmd;" \
	"fi;\0"

#undef CFG_EXTRA_ENV_SETTINGS
#define CFG_EXTRA_ENV_SETTINGS \
	STM32MP_MEM_LAYOUT \
	ST_STM32MP13_BOOTCMD \
	BOOTENV \
	STM32MP_EXTRA \
	STM32MP_BOARD_EXTRA_ENV

#endif

#endif
