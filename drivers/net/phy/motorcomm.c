/*
 * drivers/net/phy/motorcomm.c
 *
 * Driver for Motorcomm PHYs
 *
 * Author: Leilei Zhao <leilei.zhao@motorcomm.com>
 *
 * Copyright (c) 2019 Motorcomm, Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * Support : Motorcomm Phys:
 *		Giga phys: yt8511, yt8521
 *		100/10 Phys : yt8512, yt8512b, yt8510
 *		Automotive 100Mb Phys : yt8010
 *		Automotive 100/10 hyper range Phys: yt8510
 */
#include <phy.h>
#include <common.h>
#include <miiphy.h>


#define REG_DEBUG_ADDR_OFFSET           0x1e
#define REG_DEBUG_DATA                  0x1f

#define YT8521_EXTREG_LED1 0xA00D
#define YT8521_EXTREG_LED2 0xA00E

#define YT8521_EN_SLEEP_SW_BIT 15

#define PHY_ID_YT8531S          0x0000091a

#define MOTORCOMM_PHY_ID_MASK   0x00000fff

#define YT8521_EXTREG_SLEEP_CONTROL1 0x27

static int ytphy_read_ext(struct phy_device *phydev, u32 regnum)
{
	int ret;
	int val;

	ret = phy_write(phydev, MDIO_DEVAD_NONE,REG_DEBUG_ADDR_OFFSET, regnum);
	if (ret < 0)
		return ret;

	val = phy_read(phydev, MDIO_DEVAD_NONE,REG_DEBUG_DATA);

	return val;
}

static int ytphy_write_ext(struct phy_device *phydev, u32 regnum, u16 val)
{
	int ret;

	ret = phy_write(phydev,MDIO_DEVAD_NONE, REG_DEBUG_ADDR_OFFSET, regnum);
	if (ret < 0)
		return ret;

	ret = phy_write(phydev, MDIO_DEVAD_NONE,REG_DEBUG_DATA, val);

	return ret;
}

static int yt8531_delay_init(struct phy_device *phydev)
{
        int ret;
        int val = 0;
        struct device_node *np;

        ret = ytphy_write_ext(phydev,0xa003,0x4f8);
        return ret;

}

static int yt8531_led_init(struct phy_device *phydev)
{
    int ret;
    int val;
    int mask;

    val = ytphy_read_ext(phydev, YT8521_EXTREG_LED1);
    if (val < 0)
        return val;

  	/* set when link up and speed is 10/100/1000 make led on  as link led */
	val = 0x180;
    ret = ytphy_write_ext(phydev, YT8521_EXTREG_LED1, val);
    if (ret < 0)
        return ret;

    val = ytphy_read_ext(phydev, YT8521_EXTREG_LED2);
    if (val < 0)
        return val;

	/* when rx and tx send or recive msg make led link  as stats led*/
    val = 0x70;

    ret = ytphy_write_ext(phydev, YT8521_EXTREG_LED2, val);

    return ret;
}

static int yt8521_config_init(struct phy_device *phydev)
{
	int ret;
	int val;
	ytphy_write_ext(phydev, 0xa000, 0);
	ret = genphy_config_aneg(phydev);
	if (ret < 0)
		return ret;
	if((phydev->phy_id&0xfff) == PHY_ID_YT8531S){
	printf("phyid=%x\n",phydev->phy_id&0xfff);
		ret = yt8531_led_init(phydev);
	}
    
	/* disable auto sleep */
	val = ytphy_read_ext(phydev, YT8521_EXTREG_SLEEP_CONTROL1);
	if (val < 0)
		return val;

	val &= (~BIT(YT8521_EN_SLEEP_SW_BIT));

	ret = ytphy_write_ext(phydev, YT8521_EXTREG_SLEEP_CONTROL1, val);
	if (ret < 0)
		return ret;

	/* enable RXC clock when no wire plug */
	ret = ytphy_write_ext(phydev, 0xa000, 0);
	if (ret < 0)
		return ret;

	val = ytphy_read_ext(phydev, 0xc);
	if (val < 0)
		return val;
	val &= ~(1 << 12);
	ret = ytphy_write_ext(phydev, 0xc, val);
	if (ret < 0)
		return ret;		
		
	if (yt8531_delay_init(phydev) < 0){
		printf("delay set failed\n");
	}
	printf ("yt8521_config_init, 8531 init call out.\n");
	return ret;
}

static struct phy_driver ytphy_drvs[] = {
	{
		/* same as 8531 */
        .uid         = PHY_ID_YT8531S,
        .name           = "YT8531S Ethernet",
        .mask        = MOTORCOMM_PHY_ID_MASK,
        .features       = PHY_BASIC_FEATURES | PHY_GBIT_FEATURES,
        .config       = yt8521_config_init,
        .shutdown        = &genphy_shutdown,
        .startup         = &genphy_startup,              
        }, 
};
int phy_motorcomm_init(void)
{
        phy_register(&ytphy_drvs);
        return 0;
}

