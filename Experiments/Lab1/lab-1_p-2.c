/*

* Description: This program increments (rather intends to) the value of a variable sw2Status on SW2 key press

* Filename: lab-1_p-2.c

* Functions: setup(), pin_config(), sw2ISR(), main()

* Global Variables: sw2Status

* Mustafa Lokhandwala (13D070043)

*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_ints.h"
#include <time.h>
#include "inc/hw_gpio.h" // for unlocking NMI on PF0

void setup();
void pin_config();
void sw2ISR();

uint8_t sw2Status = 0; // counter for number of key press on SW2

/*

* Function Name: setup()

* Input: none

* Output: none

* Description: Set crystal frequency and enable GPIO Peripherals

* Example Call: setup();

*/

void setup()
{
	SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
}

/*

* Function Name: pin_config()

* Input: none

* Output: none

* Description: Pin configuration of PORT F; unlock NMI on PF0 and set it as input

* Example Call: pin_config();

*/

void pin_config()
{
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY; // unlock NMI on PF0 i.e. SW2
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;

	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0); // set pin 0 as input
}

/*

* Function Name: sw2ISR()

* Input: none

* Output: none

* Description: ISR for falling edge interrupt on pin 0 of PORT F i.e. SW2

* Example Call: not applicable;

*/

void sw2ISR()
{
	sw2Status++; // increment sw2Status by 1
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0); // clear the interrupt on SW2
}

int main(void)
{
	setup();
	pin_config();

	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU); // set the pad configuration for pin 0 of PORT F i.e. SW2
	GPIOIntDisable(GPIO_PORTF_BASE, GPIO_PIN_0); // disable interrupts on SW2
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0); // clear the interrupt on SW2
	GPIOIntRegister(GPIO_PORTF_BASE, sw2ISR); // register sw2ISR() with the PORT F interrupt handler
	GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE); // set the interrupt type for SW2 as falling edge
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0); // clear the interrupt on SW2
	GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_0); // enable SW2 interrupts

	while(1);
}
