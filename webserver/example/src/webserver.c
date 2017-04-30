/*
 * @brief no-RTOS LWIP HTTP Webserver example
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2014
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "lwip/init.h"
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/memp.h"
#include "lwip/tcpip.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/timers.h"
#include "netif/etharp.h"

#if LWIP_DHCP
#include "lwip/dhcp.h"
#endif

#include "board.h"
#include "lpc_phy.h"
#include "arch/lpc17xx_40xx_emac.h"
#include "arch/lpc_arch.h"
#include "httpd.h"
#include "lwip_fs.h"

#include <string.h>
#include <ctype.h>

//#define MICRO2 1

#define redT		0
#define orangeT		1
#define whiteT		2
#define greenT		3
#define lightBlueT	4
#define darkBlueT	5

#define FRT	orangeT
#define FLT	whiteT
#define BRT	redT
#define BLT	darkBlueT
#define RVT	greenT
#define LVT	lightBlueT

//Command value indexes.
#define TX	0
#define TY	1
#define TZ	2
#define RX	3
#define RY	4
#define RZ	5
#define BTN 6

//Board LED.
#define REDLED		0
#define GREENLED	1
#define BLUELED		2

#define PCLK_PWM1_BY8	((1<<13)|(1<<12))
#define PCLK_PWM1_BY4	~(PCLK_PWM1_BY8)

#define MAXPW	200u

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
#if !defined(LWIP_FATFS_SUPPORT)
#endif

/* NETIF data */
static struct netif lpc_netif;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

extern char ROVparams[];
extern char ROVparam_vals[];
extern char http_index_html[];
extern Bool toggle;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Sets up system hardware */
static void prvSetupHardware(void)
{
	/* LED0 is used for the link status, on = PHY cable detected */
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED state is off to show an unconnected cable state */
	Board_LED_Set(0, false);
	Board_LED_Set(1, false);
	/* Setup a 1mS sysTick for the primary time base */
	SysTick_Enable(1);

}

//Twos complement.
uint32_t neg(uint32_t var) {
	return ~var + 1;
}

//PWM initialization code.
void InitPWM(void) {

	Chip_PWM_Init(LPC_PWM1);

	//Enable PWM1 power.
	LPC_SYSCTL->PCONP |= SYSCTL_CLOCK_PWM1;

	//PWM peripheral clock = PCLK.
	LPC_SYSCTL->PCLKSEL[0] &= PCLK_PWM1_BY4;

	//First alternate pin function (PWM1-PWM2).
	LPC_IOCON->PINSEL[4] = (IOCON_FUNC1<<0) | (IOCON_FUNC1<<2) | (IOCON_FUNC1<<4) | (IOCON_FUNC1<<6) | (IOCON_FUNC1<<8) | (IOCON_FUNC1<<10);

	//Disable pullup resistors.
	LPC_IOCON->PINMODE[4] = (IOCON_MODE_INACT<<0) | (IOCON_MODE_INACT<<2)| (IOCON_MODE_INACT<<4) | (IOCON_MODE_INACT<<6) | (IOCON_MODE_INACT<<8) | (IOCON_MODE_INACT<<10);

	//One clock pulse every microsecond.
	LPC_PWM1->PR = SystemCoreClock / (4 * 1000000) - 1;

	//Frequency of 50Hz or 20ms.
	Chip_PWM_SetTime(LPC_PWM1, 20000);

	//Set pulseWidth to 1500us.
	for (uint8_t i = 1; i <= 6; i++) {
//		Chip_PWM_SetPulseWidth(LPC_PWM1, i, 1500);
		Chip_PWM_SetPulseWidth(LPC_PWM1, i, LPC_PWM1->MR0 - 1500);
	}

	for (uint8_t i = 1; i <= 6; i++) {
		Chip_PWM_SetControlMode(LPC_PWM1, i, PWM_SINGLE_EDGE_CONTROL_MODE, PWM_OUT_ENABLED);
	}

	LPC_PWM1->TCR |= 0b1001;

	Chip_PWM_Reset(LPC_PWM1);
}

//Twos complement multiply and divide.
uint32_t multDiv(uint32_t twosComp, uint32_t positive, char function) {
	if (function == '*') {
		if ((twosComp>>31) == 0) {
			return twosComp * positive;
		} else {
			return neg(neg(twosComp) * positive);
		}
	} else {
		if ((twosComp>>31) == 0) {
			return twosComp / positive;
		} else {
			return neg(neg(twosComp) / positive);
		}
	}
}

//Convert uint16_t to uint32_t.
uint32_t to32(uint16_t twoByte) {
	if ((twoByte>>15) == 0) {
		return (uint32_t) twoByte;
	} else {
		return (uint32_t) twoByte | 0xFFFF0000;
	}
}

Bool button(uint16_t *cmd, uint8_t bit) {
	return (cmd[BTN] & (1<<bit)) != 0;
}

uint8_t ToInt(char *byte, const uint8_t index) {
	uint8_t data;

	if (isxdigit(byte[index])) {
		if (isdigit(byte[index])) {
			data = (uint8_t) (byte[index] - '0')<<4u;
		} else if (isupper(byte[index])) {
			data = (uint8_t) (byte[index] - 'A' + 10u)<<4u;
		} else if (islower(byte[index])) {
			data = (uint8_t) (byte[index] - 'a' + 10u)<<4u;
		}
	} else {
		return 0;
	}

	if (isxdigit(byte[index + 1])) {
		if (isdigit(byte[index + 1])) {
			data |= (uint8_t) (byte[index + 1] - '0');
		} else if (isupper(byte[index + 1])) {
			data |= (uint8_t) (byte[index + 1] - 'A' + 10u);
		} else if (islower(byte[index + 1])) {
			data |= (uint8_t) (byte[index + 1] - 'a' + 10u);
		}
	} else {
		return 0;
	}

	return data;
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	main routine for example_lwip_tcpecho_sa_17xx40xx
 * @return	Function should not exit.
 */
int main(void)
{
	extern int fs_init(void);
	uint32_t physts;
	ip_addr_t ipaddr, netmask, gw;
	static int prt_ip = 0;

	uint16_t cmd[7];
	Bool started = false;
	uint32_t thruster[6];


//	ROVparams[0] = (char *) malloc(sizeof(char *) * 16);
//	ROVparam_vals[0] = (char *) malloc(sizeof(char *) * 16);
//
//	memset(ROVparams, 0, sizeof(char *) * 16);
//	memset(ROVparam_vals, 0, sizeof(char *) * 16);

	ROVparams[0] = '\0';
	ROVparam_vals[0] = '\0';
	http_index_html[0] = '\0';

	prvSetupHardware();

	InitPWM();

	/* In hitex evaluation boards, the Ethernet interface and 
	 * SDCARD interface are mutually exclusive
	 */
#if defined(LWIP_FATFS_SUPPORT)
	/* Initialize the file system */
	fs_init();
#endif

	/* Initialize LWIP */
	lwip_init();

	LWIP_DEBUGF(LWIP_DBG_ON, ("Starting LWIP TCP echo server...\n"));

	/* Static IP assignment */
#if LWIP_DHCP
	IP4_ADDR(&gw, 0, 0, 0, 0);
	IP4_ADDR(&ipaddr, 0, 0, 0, 0);
	IP4_ADDR(&netmask, 0, 0, 0, 0);
#else
#if MICRO2
	IP4_ADDR(&gw, 192, 168, 1, 1);
	IP4_ADDR(&ipaddr, 192, 168, 1, 123);
#else
	IP4_ADDR(&gw, 192, 168, 1, 1);
	IP4_ADDR(&ipaddr, 192, 168, 1, 61);
#endif
	IP4_ADDR(&netmask, 255, 255, 255, 0);
	//APP_PRINT_IP(&ipaddr);
#endif

	/* Add netif interface for lpc17xx_8x */
	netif_add(&lpc_netif, &ipaddr, &netmask, &gw, NULL, lpc_enetif_init,
			  ethernet_input);
	netif_set_default(&lpc_netif);
	netif_set_up(&lpc_netif);

#if LWIP_DHCP
	dhcp_start(&lpc_netif);
#endif

	/* Initialize and start application */
	httpd_init();

	/* This could be done in the sysTick ISR, but may stay in IRQ context
	   too long, so do this stuff with a background loop. */
	while (1) {
		/* Handle packets as part of this loop, not in the IRQ handler */
		lpc_enetif_input(&lpc_netif);

		/* lpc_rx_queue will re-qeueu receive buffers. This normally occurs
		   automatically, but in systems were memory is constrained, pbufs
		   may not always be able to get allocated, so this function can be
		   optionally enabled to re-queue receive buffers. */
#if 0
		while (lpc_rx_queue(&lpc_netif)) {}
#endif

		/* Free TX buffers that are done sending */
		lpc_tx_reclaim(&lpc_netif);

		/* LWIP timers - ARP, DHCP, TCP, etc. */
		sys_check_timeouts();

		/* Call the PHY status update state machine once in a while
		   to keep the link status up-to-date */
		physts = lpcPHYStsPoll();

		/* Only check for connection state when the PHY status has changed */
		if (physts & PHY_LINK_CHANGED) {
			if (physts & PHY_LINK_CONNECTED) {
				Board_LED_Set(0, true);
				prt_ip = 0;

				/* Set interface speed and duplex */
				if (physts & PHY_LINK_SPEED100) {
					Chip_ENET_Set100Mbps(LPC_ETHERNET);
					NETIF_INIT_SNMP(&lpc_netif, snmp_ifType_ethernet_csmacd, 100000000);
				}
				else {
					Chip_ENET_Set10Mbps(LPC_ETHERNET);
					NETIF_INIT_SNMP(&lpc_netif, snmp_ifType_ethernet_csmacd, 10000000);
				}
				if (physts & PHY_LINK_FULLDUPLX) {
					Chip_ENET_SetFullDuplex(LPC_ETHERNET);
				}
				else {
					Chip_ENET_SetHalfDuplex(LPC_ETHERNET);
				}

				netif_set_link_up(&lpc_netif);
			}
			else {
				Board_LED_Set(0, false);
				Board_LED_Set(1, false);
				netif_set_link_down(&lpc_netif);
			}

			DEBUGOUT("Link connect status: %d\r\n", ((physts & PHY_LINK_CONNECTED) != 0));
		}

		/* Print IP address info */
		if (!prt_ip) {
			if (lpc_netif.ip_addr.addr) {
				static char tmp_buff[16];
				DEBUGOUT("IP_ADDR    : %s\r\n", ipaddr_ntoa_r((const ip_addr_t *) &lpc_netif.ip_addr, tmp_buff, 16));
				DEBUGOUT("NET_MASK   : %s\r\n", ipaddr_ntoa_r((const ip_addr_t *) &lpc_netif.netmask, tmp_buff, 16));
				DEBUGOUT("GATEWAY_IP : %s\r\n", ipaddr_ntoa_r((const ip_addr_t *) &lpc_netif.gw, tmp_buff, 16));
				prt_ip = 1;
				Board_LED_Set(1, true);
			}
		}

		//Turn character string into an integer array.
		for (uint8_t i = 0; i < 7; i++) {
			cmd[i] = ToInt(ROVparam_vals, i * 4)<<8;
			cmd[i] |= ToInt(ROVparam_vals, (i * 4) + 2);
		}

		//Buttons
		if ((strcmp(ROVparams, "ROV") == 0) && toggle) {

			//Start ROV.
			if (button(cmd, 0)) {
				started = true;

				strcpy(http_index_html,     "Started.            ");
			}

			//LED
			if (!button(cmd, 2) && button(cmd, 1)) {
				Board_LED_Toggle(BLUELED);

				if (Board_LED_Test(BLUELED)) {
					strcpy(http_index_html, "The blue LED is on. ");
				} else {
					strcpy(http_index_html, "The blue LED is off.");
				}

			} else if (button(cmd, 2) && button(cmd, 1)) {
				Board_LED_Set(BLUELED, true);
				strcpy(http_index_html, "The blue LED is on. ");

			} else if (button(cmd, 2) && !button(cmd, 1)) {
				Board_LED_Set(BLUELED, false);
				strcpy(http_index_html, "The blue LED is off.");
			}
			toggle = false;
		}

		//convert data.
		thruster[FRT] = 1500u + multDiv(multDiv((neg(to32(cmd[TX])) + to32(cmd[TY]) + neg(to32(cmd[RZ]))), MAXPW, '*'), (0x8000u), '/');
		thruster[FLT] = 1500u + multDiv(multDiv((to32(cmd[TX]) + to32(cmd[TY]) + to32(cmd[RZ])), MAXPW, '*'), (0x8000u), '/');
		thruster[BRT] = 1500u + multDiv(multDiv((to32(cmd[TX]) + to32(cmd[TY]) + neg(to32(cmd[RZ]))), MAXPW, '*'), (0x8000u), '/');
		thruster[BLT] = 1500u + multDiv(multDiv((neg(to32(cmd[TX])) + to32(cmd[TY]) + to32(cmd[RZ])), MAXPW, '*'), (0x8000u), '/');

		thruster[RVT] = 1500u + neg(multDiv(multDiv((neg(to32(cmd[TZ])) + to32(cmd[RY])), MAXPW, '*'), (0x8000u), '/'));
		thruster[LVT] = 1500u + neg(multDiv(multDiv((neg(to32(cmd[TZ])) + neg(to32(cmd[RY]))), MAXPW, '*'), (0x8000u), '/'));

		if (started) {

			//Check and correct for over-current.
			if (thruster[FRT] > 1700) thruster[FRT] = 1500u + MAXPW;
			if (thruster[FRT] < 1300) thruster[FRT] = 1500u - MAXPW;

			if (thruster[FLT] > 1700) thruster[FLT] = 1500u + MAXPW;
			if (thruster[FLT] < 1300) thruster[FLT] = 1500u - MAXPW;

			if (thruster[BRT] > 1700) thruster[BRT] = 1500u + MAXPW;
			if (thruster[BRT] < 1300) thruster[BRT] = 1500u - MAXPW;

			if (thruster[BLT] > 1700) thruster[BLT] = 1500u + MAXPW;
			if (thruster[BLT] < 1300) thruster[BLT] = 1500u - MAXPW;

			if (thruster[RVT] > 1700) thruster[RVT] = 1500u + MAXPW;
			if (thruster[RVT] < 1300) thruster[RVT] = 1500u - MAXPW;

			if (thruster[LVT] > 1700) thruster[LVT] = 1500u + MAXPW;
			if (thruster[LVT] < 1300) thruster[LVT] = 1500u - MAXPW;

			/*	Update PWM to ESCs.	*/
			Chip_PWM_SetPulseWidth(LPC_PWM1, FRT + 1, LPC_PWM1->MR0 - thruster[FRT]);
			Chip_PWM_SetPulseWidth(LPC_PWM1, FLT + 1, LPC_PWM1->MR0 - thruster[FLT]);
			Chip_PWM_SetPulseWidth(LPC_PWM1, BRT + 1, LPC_PWM1->MR0 - thruster[BRT]);
			Chip_PWM_SetPulseWidth(LPC_PWM1, BLT + 1, LPC_PWM1->MR0 - thruster[BLT]);
			Chip_PWM_SetPulseWidth(LPC_PWM1, RVT + 1, LPC_PWM1->MR0 - thruster[RVT]);
			Chip_PWM_SetPulseWidth(LPC_PWM1, LVT + 1, LPC_PWM1->MR0 - thruster[LVT]);
		}
	}

	/* Never returns, for warning only */
	return 0;
}