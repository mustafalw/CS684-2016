/*

* Description: This program toggles the colour of RGB LED on pressing SW1 (with software debouncing)

* Filename: lab-2_p-1.c

* Functions: setup(), pin_config(), timer0_init(), sw1ISR(), timerISR(), main()

* Global Variables: ui8LED, ui32Period

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
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"

void setup();
void pin_config();
void timer0_init();
void sw1ISR();
void timerISR();

uint8_t ui8LED = 2; // this 8 bit value will be written on PORT F to change the colour of the LED
uint32_t ui32Period; // 32 bit int to store the number of periods required for 10 ms delay

/*

* Function Name: setup()

* Input: none

* Output: none

* Description: Set crystal frequency and enable GPIO Peripherals

* Example Call: setup();

*/

void setup()
{
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
}

/*

* Function Name: pin_config()

* Input: none

* Output: none

* Description: Pin configuration of PORT F; pins 1, 2 and 3 as output (LEDs) and pin 4 as input

* Example Call: pin_config();

*/

void pin_config()
{
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4); // set pin 4 as input
}

/*

* Function Name: timer0_init()

* Input: none

* Output: none

* Description: Enable and configure timer 0 in one shot mode of operation

* Example Call: timer0_init();

*/

void timer0_init(){
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_ONE_SHOT);
}

/*

* Function Name: sw1ISR()

* Input: none

* Output: none

* Description: ISR for falling edge interrupt on SW1

* Example Call: not applicable

*/

void sw1ISR(){
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4); // clear interrupt on SW1
	TimerEnable(TIMER0_BASE, TIMER_A); // enable the timer to debounce the key press
}

/*

* Function Name: timerISR()

* Input: none

* Output: none

* Description: ISR for interrupt on timer 0A

* Example Call: not applicable

*/

void timerISR(){
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT); // Clear the timer interrupt

	// Read the current state of the GPIO pin
	if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4)){
		// switch has gone back to high, do nothing
	}
	else
	{
		if(ui8LED == 8) // i.e. if the colour is green
		{
			ui8LED = 2; // then make it red the next time it glows
		}
		else
		{
			ui8LED = ui8LED*2; // else make it make it blue (green) if it is red (blue)
		}
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, ui8LED); // write the value of ui8LED to the corresponding pins of PORT F
	}
}

int main(){

	setup();
	pin_config();
	timer0_init();

	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU); // set the pad configuration for pin 4 of PORT F i.e. SW1
	GPIOIntDisable(GPIO_PORTF_BASE, GPIO_PIN_4); // disable interrupts on SW1
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4); // clear the interrupt on SW1
	GPIOIntRegister(GPIO_PORTF_BASE, sw1ISR); // register sw1ISR() with the PORT F interrupt handler
	GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE); // set the interrupt type for SW1 as falling edge
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4); // clear the interrupt on SW1
	GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_4); // enable SW1 interrupts
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, ui8LED); // write the value of ui8LED to the corresponding pins of PORT F

	ui32Period = (SysCtlClockGet() / 100); // get the number of required clock cycles for 10 ms delay
	TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1); // load the timer
	TimerIntRegister(TIMER0_BASE, TIMER_A, timerISR); // register timerISR() timer A of timer base 0
	IntEnable(INT_TIMER0A); // enable interrupts on timer 0A
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();

	while(1);
}
