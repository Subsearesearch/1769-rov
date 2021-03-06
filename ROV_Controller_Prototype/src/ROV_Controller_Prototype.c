/*
===============================================================================
 Name        : ROV_Controller_Prototype.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#include "chip.h"
#include "board.h"

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
#define LX	0
#define LY	1
#define LZ	2
#define RX	3
#define RY	4
#define RZ	5

//Board LED.
#define REDLED		0
#define GREENLED	1
#define BLUELED		2

#define PCLK_PWM1_BY8	((1<<13)|(1<<12))
#define PCLK_PWM1_BY4	~(PCLK_PWM1_BY8)

#define UART_SELECTION 	LPC_UART3
#define IRQ_SELECTION 	UART3_IRQn
#define HANDLER_NAME 	UART3_IRQHandler

uint16_t movement[] = {0, 0, 0, 0, 0, 0};
uint16_t button = 0;
uint32_t thruster[6];
uint8_t byteIn;
uint8_t byteIndex;
int bytes;

char start[] = "Hello to Raspberry Pi from LPC1769.\r\n\r\nReady to run.\r\n\r\n";

/* Transmit and receive ring buffers */
STATIC RINGBUFF_T txring, rxring;

/* Transmit and receive ring buffer sizes */
#define UART_SRB_SIZE 128	/* Send */
#define UART_RRB_SIZE 32	/* Receive */

/* Transmit and receive buffers */
static uint8_t rxbuff[UART_RRB_SIZE], txbuff[UART_SRB_SIZE];

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

/**
 * @brief	UART 3 interrupt handler using ring buffers
 * @return	Nothing
 */
void HANDLER_NAME(void)
{
	/* Want to handle any errors? Do it here. */

	/* Use default ring buffer handler. Override this with your own
	   code if you need more capability. */
	Chip_UART_IRQRBHandler(UART_SELECTION, &rxring, &txring);
}

int main(void) {

	SystemCoreClockUpdate();
	Board_Init();
	Board_UART_Init(UART_SELECTION);

	//Initialize PWM
	InitPWM();

	//Initialize UART
	/* Setup UART for 115.2K8N1 */
	Chip_UART_Init(UART_SELECTION);
	Chip_UART_SetBaud(UART_SELECTION, 115200);
	Chip_UART_ConfigData(UART_SELECTION, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT));
	Chip_UART_SetupFIFOS(UART_SELECTION, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	Chip_UART_TXEnable(UART_SELECTION);

	/* Before using the ring buffers, initialize them using the ring
	   buffer init function */
	RingBuffer_Init(&rxring, rxbuff, 1, UART_RRB_SIZE);
	RingBuffer_Init(&txring, txbuff, 1, UART_SRB_SIZE);

	/* Reset and enable FIFOs, FIFO trigger level 3 (14 chars) */
	Chip_UART_SetupFIFOS(UART_SELECTION, (UART_FCR_FIFO_EN | UART_FCR_RX_RS |
							UART_FCR_TX_RS | UART_FCR_TRG_LEV3));

	/* Enable receive data and line status interrupt */
	Chip_UART_IntEnable(UART_SELECTION, (UART_IER_RBRINT | UART_IER_RLSINT));

	/* preemption = 1, sub-priority = 1 */
	NVIC_SetPriority(IRQ_SELECTION, 1);
	NVIC_EnableIRQ(IRQ_SELECTION);

	//Send start message through UART.
	Chip_UART_SendRB(UART_SELECTION, &txring, start, sizeof(start) - 1);

	//Turn on LED to indicate that the uC is alive.
	Board_LED_Set(REDLED, TRUE);

	//Wait until a button is pressed.
	while (button == 0) {

		//Read data from UART.
		bytes = Chip_UART_ReadRB(UART_SELECTION, &rxring, &byteIn , 1);

		//Check to see if anything was sent.
		if (bytes > 0) {
			//Read button and joystick.
			if (byteIndex < 12) {
				if ((byteIndex % 2) == 0) {
					movement[byteIndex>>1] = (uint16_t) byteIn<<8;
				} else {
					movement[byteIndex>>1] |= (uint16_t) byteIn;
				}
			} else {
				//Get button values.
				if ((byteIndex % 2) == 0) {
					button = (uint16_t) byteIn<<8;
				} else {
					button |= (uint16_t) byteIn;
				}
			}
			byteIndex++;

			//There is a total of 14 bytes in a command.
			if (byteIndex >= 14) {
				byteIndex = 0;
			}
		}
	}

	Board_LED_Set(REDLED, FALSE);

	//Main loop
	while (1) {

		//Get command if available.
		//Read data from UART
		bytes = Chip_UART_ReadRB(UART_SELECTION, &rxring, &byteIn , 1);

		//Check to see if anything was sent.
		if (bytes > 0) {
			//Read button and joystick.
			if (byteIndex < 12) {
				if ((byteIndex % 2) == 0) {
					movement[byteIndex>>1] = (uint16_t) byteIn<<8;
				} else {
					movement[byteIndex>>1] |= (uint16_t) byteIn;
				}
			} else {
				//Get button values.
				if ((byteIndex % 2) == 0) {
					button = (uint16_t) byteIn<<8;
				} else {
					button |= (uint16_t) byteIn;
				}
			}
			byteIndex++;

			//There is a total of 14 bytes in a command.
			if (byteIndex >= 14) {
				byteIndex = 0;
			}
		}

		//convert data.
		thruster[FRT] = 1500u + multDiv(multDiv((neg(to32(movement[LX])) + to32(movement[LY]) + neg(to32(movement[RZ]))), 200u, '*'), (0x10000u * 3u), '/');
		thruster[FLT] = 1500u + multDiv(multDiv((to32(movement[LX]) + to32(movement[LY]) + to32(movement[RZ])), 200u, '*'), (0x10000u * 3u), '/');
		thruster[BRT] = 1500u + multDiv(multDiv((to32(movement[LX]) + neg(to32(movement[LY])) + neg(to32(movement[RZ]))), 200u, '*'), (0x10000u * 3u), '/');
		thruster[BLT] = 1500u + multDiv(multDiv((neg(to32(movement[LX])) + neg(to32(movement[LY])) + to32(movement[RZ])), 200u, '*'), (0x10000u * 3u), '/');
		thruster[RVT] = 1500u + multDiv(multDiv((neg(to32(movement[LZ])) + to32(movement[RY])), 200u, '*'), (0x10000u * 2u), '/');
		thruster[LVT] = 1500u + multDiv(multDiv((neg(to32(movement[LZ])) + neg(to32(movement[RY]))), 200u, '*'), (0x10000u * 2u), '/');

		/*	Update PWM to ESCs.	*/
//		Chip_PWM_SetPulseWidth(LPC_PWM1, FRT + 1, thruster[FRT]);
//		Chip_PWM_SetPulseWidth(LPC_PWM1, FLT + 1, thruster[FLT]);
//		Chip_PWM_SetPulseWidth(LPC_PWM1, BRT + 1, thruster[BRT]);
//		Chip_PWM_SetPulseWidth(LPC_PWM1, BLT + 1, thruster[BLT]);
//		Chip_PWM_SetPulseWidth(LPC_PWM1, RVT + 1, thruster[RVT]);
//		Chip_PWM_SetPulseWidth(LPC_PWM1, LVT + 1, thruster[LVT]);

		Chip_PWM_SetPulseWidth(LPC_PWM1, FRT + 1, LPC_PWM1->MR0 - thruster[FRT]);
		Chip_PWM_SetPulseWidth(LPC_PWM1, FLT + 1, LPC_PWM1->MR0 - thruster[FLT]);
		Chip_PWM_SetPulseWidth(LPC_PWM1, BRT + 1, LPC_PWM1->MR0 - thruster[BRT]);
		Chip_PWM_SetPulseWidth(LPC_PWM1, BLT + 1, LPC_PWM1->MR0 - thruster[BLT]);
		Chip_PWM_SetPulseWidth(LPC_PWM1, RVT + 1, LPC_PWM1->MR0 - thruster[RVT]);
		Chip_PWM_SetPulseWidth(LPC_PWM1, LVT + 1, LPC_PWM1->MR0 - thruster[LVT]);

		//Wait for interrupt.
		__WFI();
	}
}
