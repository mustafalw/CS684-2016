#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"

#define PWM_FREQUENCY 55

uint8_t sw1Pressed = 0;
uint8_t sw2Pressed = 0;
volatile uint32_t ui32Load;
volatile uint32_t ui32PWMClock;
volatile uint8_t ui8Adjust;
uint32_t ui32Period;

void portFISR()
{
	if(GPIOIntStatus(GPIO_PORTF_BASE, false) & GPIO_PIN_0){ // interrupt on PF0 i.e. SW2
		sw2Pressed = 1;
		TimerEnable(TIMER0_BASE, TIMER_A); // enable the timer to debounce the key press
		GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0);
	}

	else if(GPIOIntStatus(GPIO_PORTF_BASE, false) & GPIO_PIN_4){ // interrupt on PF4 i.e. SW1
		sw1Pressed = 1;
		TimerEnable(TIMER0_BASE, TIMER_A); // enable the timer to debounce the key press
		GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4);
	}

	else {}

}

void timer0A_ISR(){
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT); // Clear the timer interrupt

	// Read the current state of the GPIO pin
	if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4)){
	}
	else{
		if(sw1Pressed == 1){
			if(ui8Adjust > 60) ui8Adjust = ui8Adjust - 3;
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, ui8Adjust * ui32Load / 1000);
		}
	}

	if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0)){
	}
	else{
		if(sw2Pressed == 1){
			if(ui8Adjust < 107) ui8Adjust = ui8Adjust + 3;
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, ui8Adjust * ui32Load / 1000);
		}
	}

	sw1Pressed = 0;
	sw2Pressed = 0;

}

int main(void)
{
	ui8Adjust = 83;

	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
	SysCtlPWMClockSet(SYSCTL_PWMDIV_64);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	GPIOPinTypePWM(GPIO_PORTD_BASE, GPIO_PIN_0);
	GPIOPinConfigure(GPIO_PD0_M1PWM0);

	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
	GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_DIR_MODE_IN);

	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU); // set the pad configuration for pins 0 and 4
	GPIOIntDisable(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4); // disable interrupts on pins 0 and 4
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4); // clear the interrupt on pins 0 and 4
	GPIOIntRegister(GPIO_PORTF_BASE, portFISR); // register portFISR() with the PORT F interrupt handler
	GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, GPIO_FALLING_EDGE); // set the interrupt type for pins 0 and 4 as falling edge
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4); // clear the interrupt on pins 0 and 4
	GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4); // enable interrupts on pins 0 and 4

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_ONE_SHOT);

	ui32Period = (SysCtlClockGet() / 100); // get the number of required clock cycles for 10 ms delay
	TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1); // load the timer
	TimerIntRegister(TIMER0_BASE, TIMER_A, timer0A_ISR); // register timerISR() timer A of timer base 0
	IntEnable(INT_TIMER0A); // enable interrupts on timer 0A
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();

	ui32PWMClock = SysCtlClockGet() / 64;
	ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;
	PWMGenConfigure(PWM1_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_0, ui32Load);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, ui8Adjust * ui32Load / 1000);
	PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, true);
	PWMGenEnable(PWM1_BASE, PWM_GEN_0);

	while(1)
	{
	}
}
