/*

* Description: This program increments a variable sw2Status on pressing SW2 (with software debouncing)

* Filename: lab-2_p-2.c

* Functions: setup(), pin_config(), timer0_init(), sw2ISR(), timerISR(), main()

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
#include "inc/hw_gpio.h" // for unlocking NMI on PF0
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"

void setup();
void pin_config();

uint8_t sw2Status = 0; // counter for number of key press on SW2
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

	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0); // set pin 0 as input
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

* Function Name: sw2ISR()

* Input: none

* Output: none

* Description: ISR for falling edge interrupt on pin 0 of PORT F i.e. SW2.

* Example Call: not applicable;

*/

void sw2ISR()
{
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0); // clear the interrupt on SW1
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
		sw2Status++;
	}
}

int main(void)
{
	setup();
	pin_config();
	timer0_init();

	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU); // set the pad configuration for pin 0 of PORT F i.e. SW2
	GPIOIntDisable(GPIO_PORTF_BASE, GPIO_PIN_0); // disable interrupts on SW2
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0); // clear the interrupt on SW2
	GPIOIntRegister(GPIO_PORTF_BASE, sw2ISR); // register sw2ISR() with the PORT F interrupt handler
	GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE); // set the interrupt type for SW2 as falling edge
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0); // clear the interrupt on SW2
	GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_0); // enable SW2 interrupts

	ui32Period = (SysCtlClockGet() / 100); // get the number of required clock cycles for 10 ms delay
	TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1); // load the timer
	TimerIntRegister(TIMER0_BASE, TIMER_A, timerISR); // register timerISR() timer A of timer base 0
	IntEnable(INT_TIMER0A); // enable interrupts on timer 0A
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();

	while(1);
}
