/* SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 *
 * Copyright 2024-2025 NXP
 */
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/dma-mapping.h>
#include <la9310_base.h>
#include <asm/cacheflush.h>
#include <linux/dma-direct.h>
#include <linux/dma-map-ops.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/spinlock.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/usb/composite.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>
#include <linux/sdr-shared.h>
#include <rfnm-gpio.h>
#include "rfnm_fe_generic.h"
#include "sdr_fe_mt3812.h"
#include <linux/i2c.h>

void rfnm_tx_ch_get(struct rfnm_dgb *dgb_dt, struct rfnm_api_tx_ch *tx_ch)
{
	pr_debug("M inside %s probe\n", __func__);
}
void rfnm_rx_ch_get(struct rfnm_dgb *dgb_dt, struct rfnm_api_rx_ch *rx_ch)
{
	pr_debug("M inside %s probe\n", __func__);
}

int rfnm_tx_ch_set(struct rfnm_dgb *dgb_dt, struct rfnm_api_tx_ch *tx_ch)
{
	mt3812_priv_t *mt3812_priv = (mt3812_priv_t *)dgb_dt->priv_drv;

	//mt3812_priv->tx_gain_db = MT3812_TXGAIN_NO_PA + 1;
	//Remove hardcoding and handle through sysfs
	if (tx_ch->enable == RFNM_CH_ON || tx_ch->enable == RFNM_CH_ON_TDD) {
		if (mt3812_priv->tx_gain_db > MT3812_TXGAIN_NO_PA) {
			pr_debug(" M inside rfnm_Tx_ch_set high gain called\n");
			mt3812_tx_on(dgb_dt, tx_ch, 1);
			memcpy(&mt3812_priv->fe_tx_full, &dgb_dt->fe, sizeof(struct fe_s));
		} else {
			pr_debug(" M inside rfnm_Tx_ch_set else low gain called\n");
			mt3812_tx_on(dgb_dt, tx_ch, 0);
			memcpy(&mt3812_priv->fe_tx_bypass, &dgb_dt->fe, sizeof(struct fe_s));
		}
	} else if (tx_ch->enable == RFNM_CH_OFF) {
		mt3812_tx_off(dgb_dt, tx_ch);
	}
	if (tx_ch->enable == RFNM_CH_ON_TDD) {
		if (mt3812_priv->tx_gain_db > MT3812_TXGAIN_NO_PA)
			memcpy(&dgb_dt->fe_tdd[RFNM_TX], &mt3812_priv->fe_tx_full, sizeof(struct fe_s));
		else
			memcpy(&dgb_dt->fe_tdd[RFNM_TX], &mt3812_priv->fe_tx_bypass, sizeof(struct fe_s));
	}
	memcpy(dgb_dt->tx_s[0], dgb_dt->tx_ch[0], sizeof(struct rfnm_api_tx_ch));
	//based on gain , load appropriate low and high gain latches
	if (mt3812_priv->tx_gain_db > MT3812_TXGAIN_NO_PA)
		memcpy(&dgb_dt->fe, &mt3812_priv->fe_tx_full,  sizeof(struct fe_s));
	else
		memcpy(&dgb_dt->fe, &mt3812_priv->fe_tx_bypass, sizeof(struct fe_s));
	rfnm_fe_load_latches(dgb_dt);
	rfnm_fe_trigger_latches(dgb_dt);
	return 0;
}

int rfnm_rx_ch_set(struct rfnm_dgb *dgb_dt, struct rfnm_api_rx_ch *rx_ch)
{
	mt3812_priv_t *mt3812_priv = (mt3812_priv_t *)dgb_dt->priv_drv;
	//mt3812_priv->rx_gain_db = MT3812_RXGAIN_NO_LNA + 1;
	//Remove hardcoding and handle through sysfs

	if (rx_ch->enable == RFNM_CH_ON || rx_ch->enable == RFNM_CH_ON_TDD) {
		if (rx_ch->dgb_ch_id == 0) {
			if (rx_ch->path == RFNM_PATH_SMA_B) {
				if (mt3812_priv->rx_gain_db > MT3812_RXGAIN_NO_LNA)
					mt3812_rx_ant_b_on(dgb_dt, 1);
				else
					mt3812_rx_ant_b_on(dgb_dt, 0);
				}
			else
				pr_debug("Error As CHannel channel is 0 and path is not B\n");
		} else if (rx_ch->dgb_ch_id == 1) {
			if (rx_ch->path == RFNM_PATH_SMA_A) {
				if (mt3812_priv->rx_gain_db > MT3812_RXGAIN_NO_LNA)
					mt3812_rx_ant_a_on(dgb_dt, 1);
				else
					mt3812_rx_ant_a_on(dgb_dt, 0);
				}
			else
				pr_debug("Error As CHannel channel is 1 and path is not A\n");
		}
		if (mt3812_priv->rx_gain_db > MT3812_RXGAIN_NO_LNA)
			memcpy(&mt3812_priv->fe_rx_full, &dgb_dt->fe, sizeof(struct fe_s));
		else
			memcpy(&mt3812_priv->fe_rx_bypass, &dgb_dt->fe, sizeof(struct fe_s));

	} else if (rx_ch->enable == RFNM_CH_OFF) {
		if (rx_ch->dgb_ch_id == 0) {
			if (rx_ch->path == RFNM_PATH_SMA_B)
				mt3812_rx_ant_b_off(dgb_dt);
			else
				pr_debug("Error As CHannel channel is 0 and path is not B\n");
		} else if (rx_ch->dgb_ch_id == 1) {
			if (rx_ch->path == RFNM_PATH_SMA_A)
				mt3812_rx_ant_a_off(dgb_dt);
			else
				pr_debug("Error As CHannel channel is 1 and path is not A\n");
		}
	}
	if (rx_ch->enable == RFNM_CH_ON_TDD) {
		if (mt3812_priv->rx_gain_db > MT3812_RXGAIN_NO_LNA)
			memcpy(&dgb_dt->fe_tdd[RFNM_RX], &mt3812_priv->fe_rx_full, sizeof(struct fe_s));
		else
			memcpy(&dgb_dt->fe_tdd[RFNM_RX], &mt3812_priv->fe_rx_bypass, sizeof(struct fe_s));
	}
	//based on gain , load appropriate low and high gain latches
	if (mt3812_priv->rx_gain_db > MT3812_RXGAIN_NO_LNA)
		memcpy(&dgb_dt->fe, &mt3812_priv->fe_rx_full,  sizeof(struct fe_s));
	else
		memcpy(&dgb_dt->fe, &mt3812_priv->fe_rx_bypass, sizeof(struct fe_s));
	rfnm_fe_load_latches(dgb_dt);
	rfnm_fe_trigger_latches(dgb_dt);
	return 0;
}

static int rfnm_mt3812_probe(struct spi_device *spi)
{
	struct spi_master *spi_master = spi->master;
	int dgb_id = spi_master->bus_num - 1;
	struct device *dev = &spi->dev;
	struct rfnm_dgb *dgb_dt;
	struct rfnm_api_tx_ch *tx_ch, *tx_s;
	struct rfnm_api_rx_ch *rx_ch[2], *rx_s[2];
	mt3812_priv_t *mt3812_priv; 

	pr_debug("M inside %s probe\n", __func__);

	dgb_dt = devm_kzalloc(dev, sizeof(struct rfnm_dgb), GFP_KERNEL);
	if (!dgb_dt) {
		return -ENOMEM;
	}
	mt3812_priv = devm_kzalloc(dev, sizeof(struct mt3812_priv_t *), GFP_KERNEL);
	if (!mt3812_priv) {
		return -ENOMEM;
	}
	mt3812_priv->tx_gain_db = MT3812_TXGAIN_NO_PA;
	mt3812_priv->rx_gain_db = MT3812_RXGAIN_NO_LNA;
	dgb_dt->priv_drv = mt3812_priv;
	dgb_dt->dgb_id = dgb_id;
	dgb_dt->rx_ch_set = rfnm_rx_ch_set;
	dgb_dt->rx_ch_get = rfnm_rx_ch_get;
	dgb_dt->tx_ch_set = rfnm_tx_ch_set;
	dgb_dt->tx_ch_get = rfnm_tx_ch_get;
	dgb_dt->reset = sdr_reset;
	spi_set_drvdata(spi, dgb_dt);
	spi->max_speed_hz = 1000000;
	spi->bits_per_word = 8;
	spi->mode = SPI_CPHA;
	rfnm_fe_generic_init(dgb_dt, SDR_MT3812_NUM_LATCHES);
	tx_ch = devm_kzalloc(dev, sizeof(struct rfnm_api_tx_ch), GFP_KERNEL);
	rx_ch[0] = devm_kzalloc(dev, sizeof(struct rfnm_api_rx_ch), GFP_KERNEL);
	rx_ch[1] = devm_kzalloc(dev, sizeof(struct rfnm_api_rx_ch), GFP_KERNEL);
	tx_s = devm_kzalloc(dev, sizeof(struct rfnm_api_tx_ch), GFP_KERNEL);
	rx_s[0] = devm_kzalloc(dev, sizeof(struct rfnm_api_rx_ch), GFP_KERNEL);
	rx_s[1] = devm_kzalloc(dev, sizeof(struct rfnm_api_rx_ch), GFP_KERNEL);
	if (!tx_ch || !rx_ch[0] || !rx_ch[1] || !tx_s || !rx_s[0] || !rx_s[1]) {
		return -ENOMEM;
	}
	//1 TX channel
	tx_ch->freq_max = MHZ_TO_HZ(3800);
	tx_ch->freq_min = MHZ_TO_HZ(3300);
	tx_ch->path_preferred = RFNM_PATH_SMA_B;
	tx_ch->path_possible[0] = RFNM_PATH_SMA_B;
	tx_ch->path_possible[1] = RFNM_PATH_SMA_A;
	tx_ch->path_possible[2] = RFNM_PATH_NULL;
	tx_ch->dac_id = 0;
	//Register TX channel
	rfnm_dgb_reg_tx_ch(dgb_dt, tx_ch, tx_s);
	//2 RX channels
	rx_ch[0]->freq_max = MHZ_TO_HZ(3800);
	rx_ch[0]->freq_min = MHZ_TO_HZ(3300);
	rx_ch[0]->path_preferred = RFNM_PATH_SMA_A;
	rx_ch[0]->path_possible[0] = RFNM_PATH_SMA_A;
	rx_ch[0]->path_possible[1] = RFNM_PATH_SMA_B;
	rx_ch[0]->path_possible[2] = RFNM_PATH_EMBED_ANT;
	rx_ch[0]->path_possible[3] = RFNM_PATH_NULL;
	rx_ch[0]->adc_id = 1;
	//Register RX channel 1
	rfnm_dgb_reg_rx_ch(dgb_dt, rx_ch[0], rx_s[0]);
	rx_ch[1]->freq_max = MHZ_TO_HZ(3800);
	rx_ch[1]->freq_min = MHZ_TO_HZ(3300);
	rx_ch[1]->path_preferred = RFNM_PATH_SMA_B;
	rx_ch[1]->path_possible[0] = RFNM_PATH_SMA_B;
	rx_ch[1]->path_possible[1] = RFNM_PATH_SMA_A;
	rx_ch[1]->path_possible[2] = RFNM_PATH_EMBED_ANT;
	rx_ch[1]->path_possible[3] = RFNM_PATH_NULL;
	rx_ch[1]->adc_id = 0;
	//Register RX channel 2
	rfnm_dgb_reg_rx_ch(dgb_dt, rx_ch[1], rx_s[1]);
	dgb_dt->dac_ifs = 0xf;
	dgb_dt->dac_iqswap[0] = 1;
	dgb_dt->dac_iqswap[1] = 0;
	dgb_dt->adc_iqswap[0] = 0;
	dgb_dt->adc_iqswap[1] = 0;
	//Register with RFNM daughter board
	rfnm_dgb_reg(dgb_dt);
	return 0;
}

//Reset sequence called. The steps include setting GPIO4_5, wait for 1ms, configure , load, and trigger LATCH3_Q0 value 0,Clear GPIO4_5
int sdr_reset(struct rfnm_dgb *dgb_dt)
{
	pr_debug("Inside %s Entered\n", __func__);

#ifdef FE_TEST  //This part of Code is under review
	rfnm_api_failcode ecode = RFNM_API_OK;

	rfnm_gpio_set(dgb_dt->dgb_id, RFNM_DGB_GPIO4_5);
	usleep_range(500, 1000);
	//1000 microseconds =1 millisecond
	rfnm_fe_srb(dgb_dt, RFNM_MT3812_MT_TRX, 0);
	rfnm_fe_load_latches(dgb_dt);
	rfnm_fe_trigger_latches(dgb_dt);
	rfnm_gpio_clear(dgb_dt->dgb_id, RFNM_DGB_GPIO4_5);
#else//Working code - Turn around way
	pr_debug("Inside else condition, setting gpio output, clear gpio4_5, mt_trx 0, set gpio4_5\n");
	rfnm_gpio_output(dgb_dt->dgb_id, RFNM_DGB_GPIO5_16);
	rfnm_gpio_output(dgb_dt->dgb_id, RFNM_DGB_GPIO3_22);
	rfnm_gpio_output(dgb_dt->dgb_id, RFNM_DGB_GPIO4_5);
	rfnm_gpio_output(dgb_dt->dgb_id, RFNM_DGB_GPIO4_8);
	rfnm_gpio_set(dgb_dt->dgb_id, RFNM_DGB_GPIO5_16);
	rfnm_gpio_clear(dgb_dt->dgb_id, RFNM_DGB_GPIO3_22);
	rfnm_gpio_clear(dgb_dt->dgb_id, RFNM_DGB_GPIO4_8);
	//power
	udelay(1000);
	rfnm_gpio_set(dgb_dt->dgb_id, RFNM_DGB_GPIO3_22);
	rfnm_gpio_set(dgb_dt->dgb_id, RFNM_DGB_GPIO4_8);
	udelay(1000);
	//MT RST
	rfnm_gpio_clear(dgb_dt->dgb_id, RFNM_DGB_GPIO4_5);
	udelay(1000);
	rfnm_fe_srb(dgb_dt, SDR_MT3812_MT_TRX, 0);
	rfnm_fe_load_latches(dgb_dt);
	rfnm_fe_trigger_latches(dgb_dt);
	udelay(1000);
	rfnm_gpio_set(dgb_dt->dgb_id, RFNM_DGB_GPIO4_5);
#endif
	pr_debug("Inside rfnm_RESET Exiting\n");
	return 0;
}

static int rfnm_mt3812_remove(struct spi_device *spi)
{
	struct rfnm_dgb *dgb_dt;

	dgb_dt = spi_get_drvdata(spi);
	kfree(dgb_dt->tx_ch);
	kfree(dgb_dt->rx_ch[0]);
	kfree(dgb_dt->rx_ch[1]);
	kfree(dgb_dt->tx_s);
	kfree(dgb_dt->rx_s[0]);
	kfree(dgb_dt->rx_s[0]);
	kfree(dgb_dt->priv_drv);
	rfnm_dgb_unreg(dgb_dt);
	kfree(dgb_dt);
	return 0;
}

static const struct spi_device_id rfnm_mt3812_ids[] = {
	{ "rfnm,daughterboard" },
	{},
};
MODULE_DEVICE_TABLE(spi, rfnm_mt3812_ids);


static const struct of_device_id rfnm_mt3812_match[] = {
	{ .compatible = "rfnm,daughterboard" },
	{},
};
MODULE_DEVICE_TABLE(of, rfnm_mt3812_match);

static struct spi_driver rfnm_mt3812_spi_driver = {
	.driver = {
		.name = "rfnm_mt3812",
		.of_match_table = rfnm_mt3812_match,
	},
	.probe = rfnm_mt3812_probe,
	.remove = rfnm_mt3812_remove,
	.id_table = rfnm_mt3812_ids,
};

module_spi_driver(rfnm_mt3812_spi_driver);

MODULE_PARM_DESC(device, "RFNM MT3812 Daughterboard Driver");

MODULE_LICENSE("GPL");
