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
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"

#define PWM_FREQUENCY 55

uint8_t toggle_delay = 1;
uint8_t iterator = 0;
uint8_t sw1Pressed = 0;
uint8_t sw1Counts = 0;
uint8_t sw2Pressed = 0;
uint8_t longTimeout = 0;
uint8_t manualMode = 0;
uint8_t manualMode1 = 0;
uint8_t manualMode2 = 0;
uint8_t manualMode3 = 0;
uint32_t ui32Period;
uint32_t ui32Periodsw1;
uint32_t ui32PeriodLong;

void sw2LongPressISR();

void timer0_init(){
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_ONE_SHOT);
}

void timer1_init(){
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
	TimerConfigure(TIMER1_BASE, TIMER_CFG_ONE_SHOT);
}

void timer2_init(){
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
	TimerConfigure(TIMER2_BASE, TIMER_CFG_ONE_SHOT);
}

void portFISR()
{
	if(GPIOIntStatus(GPIO_PORTF_BASE, false) & GPIO_PIN_0){ // interrupt on PF0 i.e. SW2
		sw2Pressed = 1;
		longTimeout = 0;
		TimerEnable(TIMER0_BASE, TIMER_A); // enable the timer to debounce the key press
		GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0);
		TimerEnable(TIMER1_BASE, TIMER_A);
		GPIOIntRegister(GPIO_PORTF_BASE, sw2LongPressISR);
		GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_RISING_EDGE);
	}

	else if(GPIOIntStatus(GPIO_PORTF_BASE, false) & GPIO_PIN_4){ // interrupt on PF4 i.e. SW1
		sw1Pressed = 1;
		TimerEnable(TIMER0_BASE, TIMER_A); // enable the timer to debounce the key press
		GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4);
	}

	else {}

}

void sw2LongPressISR()
{
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0);
	TimerDisable(TIMER1_BASE, TIMER_A);
	if(longTimeout == 1) manualMode = 1;
	GPIOIntRegister(GPIO_PORTF_BASE, portFISR);
	GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE);
}

void timer0A_ISR(){
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT); // Clear the timer interrupt

	// Read the current state of the GPIO pin
	if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4)){
	}
	else{
		if(sw1Pressed == 1){
			if(manualMode == 0){
				if(toggle_delay == 1)
				{
				}
				else
				{
					toggle_delay = toggle_delay/2;
				}
			}

			else if(manualMode == 1){
				TimerDisable(TIMER2_BASE, TIMER_A);
				sw1Counts++;
				TimerEnable(TIMER2_BASE, TIMER_A);
			}

			else
			{
			}
		}
	}

	if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0)){
	}
	else{
		if(sw2Pressed == 1){
			if(toggle_delay == 4)
			{
			}
			else
			{
				toggle_delay = toggle_delay*2;
			}
		}
	}

	sw1Pressed = 0;
	sw2Pressed = 0;

}

void timer1A_ISR(){
	TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	longTimeout = 1;
}

void timer2A_ISR(){
	TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
	if(sw1Counts == 1){
		manualMode1 = 1;
		manualMode2 = 0;
		manualMode3 = 0;
	}
	else if(sw1Counts == 2){
		manualMode1 = 0;
		manualMode2 = 1;
		manualMode3 = 0;
	}
	else if(sw1Counts == 3){
		manualMode1 = 0;
		manualMode2 = 0;
		manualMode3 = 1;
	}
	sw1Counts = 0;
}

int main(void)
{
	volatile uint32_t ui32Load;
	volatile uint32_t ui32PWMClock;
	volatile uint8_t ui8AdjustR;
	volatile uint8_t ui8AdjustG;
	volatile uint8_t ui8AdjustB;
	// uint32_t loop_delay = 100000;
	ui8AdjustR = 255;
	ui8AdjustB = 1;
	ui8AdjustG = 1;

	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
	SysCtlPWMClockSet(SYSCTL_PWMDIV_64);

//	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);
	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2);
	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_3);
	GPIOPinConfigure(GPIO_PF1_M1PWM5);
	GPIOPinConfigure(GPIO_PF2_M1PWM6);
	GPIOPinConfigure(GPIO_PF3_M1PWM7);

	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
	GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_DIR_MODE_IN);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	ui32PWMClock = SysCtlClockGet() / 64;
	ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;
//	PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_DOWN);
	PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN);
	PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN);
//	PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, ui32Load);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, ui32Load);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, ui32Load);

	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8AdjustR * ui32Load / 1000);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8AdjustB * ui32Load / 1000);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8AdjustG * ui32Load / 1000);
	PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, true);
	PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, true);
	PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, true);
//	PWMGenEnable(PWM0_BASE, PWM_GEN_1);
	PWMGenEnable(PWM1_BASE, PWM_GEN_2);
	PWMGenEnable(PWM1_BASE, PWM_GEN_3);

	GPIOIntDisable(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4); // disable interrupts on pins 0 and 4
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4); // clear the interrupt on pins 0 and 4
	GPIOIntRegister(GPIO_PORTF_BASE, portFISR); // register portFISR() with the PORT F interrupt handler
	GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, GPIO_FALLING_EDGE); // set the interrupt type for pins 0 and 4 as falling edge
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4); // clear the interrupt on pins 0 and 4
	GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4); // enable interrupts on pins 0 and 4

	timer0_init();
	timer1_init();
	timer2_init();

	ui32Period = (SysCtlClockGet() / 100); // get the number of required clock cycles for 10 ms delay
	TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1); // load the timer
	TimerIntRegister(TIMER0_BASE, TIMER_A, timer0A_ISR); // register timerISR() timer A of timer base 0
	IntEnable(INT_TIMER0A); // enable interrupts on timer 0A
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	ui32PeriodLong = (SysCtlClockGet() * 2);
	TimerLoadSet(TIMER1_BASE, TIMER_A, ui32PeriodLong -1); // load the timer
	TimerIntRegister(TIMER1_BASE, TIMER_A, timer1A_ISR);
	IntEnable(INT_TIMER1A);
	TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

	ui32Periodsw1 = (SysCtlClockGet());
	TimerLoadSet(TIMER2_BASE, TIMER_A, ui32Periodsw1 -1); // load the timer
	TimerIntRegister(TIMER2_BASE, TIMER_A, timer2A_ISR);
	IntEnable(INT_TIMER2A);
	TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();

	while(1)
	{

		if(manualMode == 0){
			if(ui8AdjustR == 255 && ui8AdjustG < 255 && ui8AdjustB == 1)
			{
				ui8AdjustG++;
			}
			else if(ui8AdjustR > 1 && ui8AdjustG == 255 && ui8AdjustB == 1)
			{
				ui8AdjustR--;
			}
			else if(ui8AdjustR == 1 && ui8AdjustG == 255 && ui8AdjustB < 255)
			{
				ui8AdjustB++;
			}
			else if(ui8AdjustR == 1 && ui8AdjustG > 1 && ui8AdjustB == 255)
			{
				ui8AdjustG--;
			}
			else if(ui8AdjustR < 255 && ui8AdjustG == 1 && ui8AdjustB == 255)
			{
				ui8AdjustR++;
			}
			else if(ui8AdjustR == 255 && ui8AdjustG == 1 && ui8AdjustB > 1)
			{
				ui8AdjustB--;
			}
			else
			{
			}

			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8AdjustR * ui32Load / 1000);
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8AdjustB * ui32Load / 1000);
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8AdjustG * ui32Load / 1000);

			for(iterator = 0; iterator < toggle_delay; iterator++) SysCtlDelay(100000);
		}

		else if(manualMode == 1){
			if(manualMode1 == 1){
			GPIOIntDisable(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4); // disable interrupts on pins 0 and 4
				if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)
				{
					if(ui8AdjustR > 1) ui8AdjustR--;
					PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8AdjustR * ui32Load / 1000);
				}

				if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00)
				{
					if(ui8AdjustR < 255) ui8AdjustR++;
					PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8AdjustR * ui32Load / 1000);
				}
			}
			else if(manualMode2 == 1){
			GPIOIntDisable(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4); // disable interrupts on pins 0 and 4
				if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)
				{
					if(ui8AdjustB > 1) ui8AdjustB--;
					PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8AdjustB * ui32Load / 1000);
				}

				if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00)
				{
					if(ui8AdjustB < 255) ui8AdjustB++;
					PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8AdjustB * ui32Load / 1000);
				}
			}
			else if(manualMode3 == 1){
			GPIOIntDisable(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4); // disable interrupts on pins 0 and 4
				if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)
				{
					if(ui8AdjustG > 1) ui8AdjustG--;
					PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8AdjustG * ui32Load / 1000);
				}

				if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00)
				{
					if(ui8AdjustG < 255) ui8AdjustG++;
					PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8AdjustG * ui32Load / 1000);
				}
			}
			else
			{
			}

			SysCtlDelay(100000);
		}
	}
}
