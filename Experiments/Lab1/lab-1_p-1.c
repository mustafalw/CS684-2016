/*

* Description: This program sequentially glows the red, blue and green LED while SW1 is pressed

* Filename: lab-1_p-1.c

* Functions: setup(), pin_config(), sw1DownISR(), sw1UpISR(), main()

* Global Variables: ui8LED

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

void setup();
void pin_config();
void sw1DownISR();
void sw1UpISR();

uint8_t ui8LED = 2; // this 8 bit value will be written on PORT F to change the colour of the LED

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

* Description: Pin configuration of PORT F; pins 1, 2 and 3 as output (LEDs) and pin 4 as input (switch)

* Example Call: pin_config();

*/

void pin_config()
{
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3); // set pins 1, 2 and 3 at PORT F as output
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4); // set pin4 as input
}

/*

* Function Name: sw1DownISR()

* Input: none

* Output: none

* Description: ISR for falling edge interrupt on pin 4 of PORT F i.e. SW1

* Example Call: not applicable

*/

void sw1DownISR()
{
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4); // clear the interrupt on SW1
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, ui8LED); // write the value of ui8LED to the corresponding pins of PORT F
	GPIOIntRegister(GPIO_PORTF_BASE, sw1UpISR); // register sw1UpISR() with the PORT F interrupt handler
	GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_RISING_EDGE); // set the interrupt type for SW1 as rising edge
	if (ui8LED == 8) // i.e. if the colour is green
	{
		ui8LED = 2; // then make it red the next time it glows
	}
	else
	{
		ui8LED = ui8LED*2; // else make it make it blue (green) if it is red (blue)
	}
}

/*

* Function Name: sw1UpISR()

* Input: none

* Output: none

* Description: ISR for rising edge interrupt on pin 4 of PORT F i.e. SW1

* Example Call: not applicable

*/

void sw1UpISR()
{
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0); // write 0 i.e. switch off the LED
	GPIOIntRegister(GPIO_PORTF_BASE, sw1DownISR); // register sw1DownISR() with the PORT F interrupt handler
	GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE); // set the interrupt type for SW1 as falling edge
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4); // clear the interrupt on SW1
}

int main(void)
{
	setup();
	pin_config();

	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU); // set the pad configuration for pin 4 of PORT F i.e. SW1
	GPIOIntDisable(GPIO_PORTF_BASE, GPIO_PIN_4); // disable interrupts on SW1
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4); // clear the interrupt on SW1
	GPIOIntRegister(GPIO_PORTF_BASE, sw1DownISR); // register sw1DownISR() with the PORT F interrupt handler
	GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE); // set the interrupt type for SW1 as falling edge
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4); // clear the interrupt on SW1
	GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_4); // enable SW1 interrupts

	while(1);
}
