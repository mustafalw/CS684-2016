/*

* Description: This program controls the RGB LEDs using SW1 and SW2
* Pressing SW1 toggles blinking delay between 0.5s, 1s and 2s
* Pressing SW2 toggles the colour between red, green and blue

* Filename: lab-1_p-3.c

* Functions: setup(), pin_config(), portFISR(), main()

* Global Variables: ui8LED, toggle_delay, iterator

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
void portFISR();

uint8_t ui8LED = 2; // this 8 bit value will be written on PORT F to change the colour of the LED
uint8_t toggle_delay = 1; // this int variable is the multiplier for 0.5s of delay
uint8_t iterator = 0;

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

* Description: Pin configuration of PORT F; pins 1, 2 and 3 as output (LEDs), unlock NMI on PF0 and pins 0 and 4 as input

* Example Call: pin_config();

*/

void pin_config()
{
	// unlock NMI on PF0 i.e. SW2
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;

	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3); // set pins 1, 2 and 3 at PORT F as output
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4); // set pins 0 and 4 as input
}

/*

* Function Name: portFISR()

* Input: none

* Output: none

* Description: ISR for various interrupts on GPIO port F

* Example Call: not applicable

*/

void portFISR()
{
	if(GPIOIntStatus(GPIO_PORTF_BASE, false) & GPIO_PIN_0){ // interrupt on PF0 i.e. SW2
		if(ui8LED == 8) // i.e. if the colour is green
		{
			ui8LED = 2; // then make it red the next time it glows
		}
		else
		{
			ui8LED = ui8LED*2; // else make it make it blue (green) if it is red (blue)
		}
	}

	else if(GPIOIntStatus(GPIO_PORTF_BASE, false) & GPIO_PIN_4){ // interrupt on PF4 i.e. SW1
		if(toggle_delay == 4) // i.e. if the colour is green
		{
			toggle_delay = 1; // then make it red the next time it glows
		}
		else
		{
			toggle_delay = toggle_delay*2; // else make it make it blue (green) if it is red (blue)
		}
	}

	else {}

	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4); // clear the interrupt on SW2
}

int main(void)
{
	setup();
	pin_config();

	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU); // set the pad configuration for pins 0 and 4
	GPIOIntDisable(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4); // disable interrupts on pins 0 and 4
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4); // clear the interrupt on pins 0 and 4
	GPIOIntRegister(GPIO_PORTF_BASE, portFISR); // register portFISR() with the PORT F interrupt handler
	GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, GPIO_FALLING_EDGE); // set the interrupt type for pins 0 and 4 as falling edge
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4); // clear the interrupt on pins 0 and 4
	GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4); // enable interrupts on pins 0 and 4

	while(1){
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, ui8LED); // write the value of ui8LED to the corresponding pins of PORT F
		for(iterator = 0; iterator < toggle_delay; iterator++) SysCtlDelay(6700000); // delay as decided by the value of toggle_delay, either 0.5s or 1s or 2s
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0); // write the value of ui8LED to the corresponding pins of PORT F
		for(iterator = 0; iterator < toggle_delay; iterator++) SysCtlDelay(6700000);
	}
}
