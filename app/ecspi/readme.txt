How to Build SPI Application
=============================
iMX8MP has three ecspi instances. User mode driver has set of api's 
which can be used to build application using ecspi libarry.

Directory Structure
--------------------
Below is the directory structure,showing how api, app and lib directory's are structured. 

api
 diora_ecspi_api.h
 imx_ecspi_api.h
 la9310_wdog_api.h
app
 ecspi
  ecspi_test_app.c
  Makefile
  readme.txt
 freq_correction
  freq_correction.c
  Makefile
 Makefile
lib
 ecspi
  imx_ecspi.h
  imx_ecspi_lib.c
  Makefile
 Makefile

List of API'
-----------
API's which can be used to make application are mentioned in file imx_ecspi_api.h.
USer can make application using these api's.

Clock Configuartion
===================
ECSPI clock is configurable as per application need.There are two level of clock divisor configuartion.
At top level root clock is being selected  as shown below 

ROOT CLOCK
-------------
Below are the various root clock options for ECSPI block to work. 
101 (Peripheral) ECSPI1_CLK_ROOT 0xB280 80 
------------------------------------------
	000 - 24M_REF_CLK
	100 - SYSTEM_PLL1_CLK
	011 - SYSTEM_PLL1_DIV5
	010 - SYSTEM_PLL1_DIV20
	110 - SYSTEM_PLL2_DIV4
	001 - SYSTEM_PLL2_DIV5
	101 - SYSTEM_PLL3_CLK
	111 - AUDIO_PLL2_CLK

Note: Application has been tested with only 24M_REF_CLK and SYSTEM_PLL1_CLK clock sources.
root clock has both pre and post divisior to further reduce the clock as mentioned in register "Target Register (CCM_TARGET_ROOTn)".

Root Clock Pre and Post Divider
--------------------------------
(I) Pre divider divide the number

	Divider value is n+1
	000 Divide by 1
	001 Divide by 2
	010 Divide by 3
	011 Divide by 4
	100 Divide by 5
	101 Divide by 6
	110 Divide by 7
	111 Divide by 8
(II) Post divider divide number

Divider value is n + 1.
000000 Divide by 1
000001 Divide by 2
000010 Divide by 3
000011 Divide by 4
000100 Divide by 5
000101 Divide by 6
:
111111 Divide by 64

SPI Controller Level Divisor
---------------------------

There is next level clock divisor in "Control Register (ECSPIx_CONREG)" 
namely PRE_DIVIDER and  POST_DIVIDER.

(I) PRE_DIVIDER
	ECSPI uses a two-stage divider to generate the SPI clock.
	This field defines the predivider of the reference clock.
	0000 Divide by 1.
	0001 Divide by 2.
	0010 Divide by 3.
	...
	1101 Divide by 14.
	1110 Divide by 15.
	1111 Divide by 16.

(II) POST_DIVIDER

SPI Post Divider. ECSPI uses a two-stage divider to generate the SPI clock.
This field defines the post divider of the reference clock.
0000 Divide by 1.
0001 Divide by 2.
0010 Divide by 4.
1110 Divide by 2 14 .
1111 Divide by 2 15 .

API' to configure clock
======================
Both root clock and control register pre and post divider along with
root clock can be configured using api imx_spi_init_with_clk.
 
Example: 
    ecspi_clk_t clk;
    clk.ecspi_root_clk = IMX8MP_ECSPI_CLK_SYSTEM_PLL1_CLK;
    clk.ecspi_ccm_target_root_pre_podf_div_clk = 0x0; /* Pre divider, Target Register (CCM_TARGET_ROOTn) bit 16 to 18 */
    clk.ecspi_ccm_target_root_post_podf_div_clk = 0x0; /* Post divider, Target Register (CCM_TARGET_ROOTn) bit 0 to 5  */
    clk.ecspi_ctrl_pre_div_clk = 0x5; /* 0x5 Pre divider, Control Register (ECSPIx_CONREG) bit 12 to 15. */
    clk.ecspi_ctrl_post_div_clk = 0x3; /* 0x3 Post divider Control Register (ECSPIx_CONREG) bit 8 to 11. */
    ecspi_base = imx_spi_init_with_clk(ecspi_chan, clk);

How to build Application
========================

Below is the steps to build application.
Which build everything in host software along with ecspi application.

make clean;CONFIG_ENABLE_FLOAT_BYPASS=y make IMX_RFNM=1 IMX_RFGR=1

user can disable IMX_RFNM and IMX_RFGR by setting them to IMX_RFNM=0 IMX_RFGR=0,
if not needed.

How to Invoke Test
=================
root@imx8mp-rfnm:~# ./ecspi_test_app                                                                                                                                                                                                      
Usage: ./ecspi_test_app ecspi_ch_id(1..3)  -c(cpu number to affine(1..4))

User need to pass the valid ecspi channel number between 1 to 3.
Optionally user can pass cpu to affine if needed by passing number 1 to 4.

 
