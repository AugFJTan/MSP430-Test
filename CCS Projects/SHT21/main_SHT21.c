// main_SHT21.c
//
//****************************************************************************************************
// Author:
// 	Nipun Gunawardena
//
// Credits:
//	Borrows from Texas Instruments' sht21 library.
//
// Requirements:
//
//
// Description:
// 	Interface with Sensirion SHT21 to measure humidity without driverlib
//
// Notes:
//
// Todo:
//	Make more durable, timeouts, testing, etc.
//****************************************************************************************************



// Includes ------------------------------------------------------------------------------------------
#include <stdint.h>
#include <msp430.h>
#include "shtLib.h"



// Main ----------------------------------------------------------------------------------------------
int main(void) {
	  WDTCTL = WDTPW | WDTHOLD;       // Stop WDT

	  // Configure GPIO
	  P1OUT &= ~BIT0;                           // Clear P1.0 output latch
	  P1DIR |= BIT0;                            // For LED
	  P1SEL1 |= BIT6 | BIT7;                    // I2C pins
	  PJSEL0 |= BIT4 | BIT5;					// Set J.4 & J.5 to accept crystal input for ACLK

	  // Disable the GPIO power-on default high-impedance mode to activate
	  // previously configured port settings
	  PM5CTL0 &= ~LOCKLPM5;

	  // Clock setup
	  CSCTL0_H = CSKEY >> 8;                    // Unlock CS registers
	  CSCTL1 = DCOFSEL_0;                       // Set DCO to 1MHz
	  CSCTL2 = SELA__LFXTCLK | SELS__DCOCLK | SELM__DCOCLK; // Set ACLK = LFXTCLK; MCLK = DCO
	  CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers to 1
	  CSCTL4 &= ~LFXTOFF;						// Turn on LFXT

	  // Lock CS registers - Why isn't PUC created?
	  CSCTL0_H = 0;

	  // Enable interrupts
	  __bis_SR_register(GIE);

	  // Initialize SHT21 Variables
	  SHT21Init();

	  // Read Temperature
	  SHT21ReadTemperature();

	  // Read Humidity
	  SHT21ReadHumidity();

	  // Stop
	  __no_operation();
}




// Interrupts ----------------------------------------------------------------------------------------

// EUSCI_B I2C Interrupt
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B0_VECTOR))) USCI_B0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCB0IV, USCI_I2C_UCBIT9IFG))
  {
    case USCI_NONE:          break;         // Vector 0: No interrupts
    case USCI_I2C_UCALIFG:   break;         // Vector 2: ALIFG
    case USCI_I2C_UCNACKIFG: break;         // Vector 4: NACKIFG
    case USCI_I2C_UCSTTIFG:  break;         // Vector 6: STTIFG
    case USCI_I2C_UCSTPIFG:  break;         // Vector 8: STPIFG
    case USCI_I2C_UCRXIFG3:  break;         // Vector 10: RXIFG3
    case USCI_I2C_UCTXIFG3:  break;         // Vector 12: TXIFG3
    case USCI_I2C_UCRXIFG2:  break;         // Vector 14: RXIFG2
    case USCI_I2C_UCTXIFG2:  break;         // Vector 16: TXIFG2
    case USCI_I2C_UCRXIFG1:  break;         // Vector 18: RXIFG1
    case USCI_I2C_UCTXIFG1:  break;         // Vector 20: TXIFG1
    case USCI_I2C_UCRXIFG0:  		        // Vector 22: RXIFG0
		g_shtRxArr[g_shtRxCount] = UCB0RXBUF;	// Read byte from SHT21
		g_shtRxCount++;
    	if(g_shtRxCount == 2){
    		__bic_SR_register_on_exit(LPM0_bits); 	// Exit LPM0
    	}
    	break;
    case USCI_I2C_UCTXIFG0:                 // Vector 24: TXIFG0
    	break;
    default: break;
  }
}


// Timer B1 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer0_B0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_B0_VECTOR))) Timer0_B0_ISR (void)
#else
#error Compiler not supported!
#endif
{
	TB0CTL &= ~MC__STOP;
	__bic_SR_register_on_exit(LPM3_bits); 	// Exit LPM0
}
