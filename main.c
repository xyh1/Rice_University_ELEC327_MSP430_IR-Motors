/*
 *
 * MSP430G2553 IR Motor Control
 * Rice University ELEC 327
 * Xin Huang
 * Jonathan Nguyen
 * Michael Tsehaie
 *
 */

//---------------------------------------------------
// Include
//---------------------------------------------------
#include "msp430g2553.h"

//---------------------------------------------------
// Define
//---------------------------------------------------
#define TRUE  1
#define FALSE 0

#define NUM_BITS     32     // 32 bits for IR signal
#define ZERO_LIMIT   225    // zero/one threshold
#define TIMEOUT      15000  // set for 120ms

#define LED1 BIT0

//---------------------------------------------------
// IR Remote NEC Hex Values
// Remote: http://www.adafruit.com/products/389
//---------------------------------------------------
#define VOL_DOWN   'FF00DF00'
#define PLAY       'F708DF00'
#define VOL_UP     'FB04DF00'
#define SETUP      'FD02DF00'
#define UP         'F50ADF00'
#define STOP       'F90GDF00'
#define LEFT       'FE01DF00'
#define ENTER      'F609DF00'
#define RIGHT      'FA05DF00'
#define PLUS       'FC03DF00'
#define DOWN       'F40BDF00'
#define BACK       'F807DF00'

#define ONE        '7F80DF00'
#define TWO        '7788DF00'
#define THREE      '7B84DF00'
#define FOUR       '7D82DF00'
#define FIVE       '758ADF00'
#define SIX        '7986DF00'
#define SEVEN      '7E81DF00'
#define EIGHT      '7689DF00'
#define NINE       '7A85DF00'

//---------------------------------------------------
// Preallocate Function Memory
//---------------------------------------------------
void display_HEX(unsigned long bits);
void display_BRD(unsigned short bits);
void init(void);

//---------------------------------------------------
// Global Variables
//---------------------------------------------------
unsigned short Count, DataReceived, WhichIRQ;
unsigned long DataBits, DataBitsReversed;
unsigned short time[128];

char ir_display[8];
const char hex_values[16] = "0123456789ABCDEF";

//---------------------------------------------------
// IR Signal Conversion Routines
//---------------------------------------------------
/* Input:
 *    enter with bits = 32-bit unsigned integer
 * Output:
 *    fill global array ir_display[]
 */
void display_HEX(unsigned long bits)
{
  unsigned short i;
  for (i = 0; i < 8; i++)
  {
    ir_display[i] = hex_values[bits & 0x000F];
    bits = bits >> 4; // hex_values values
  }
}

/* Input:
 *    enter with bits = 16-bit unsigned integer
 * Output:
 *    fill global array ir_display[]
 */
void display_BRD(unsigned short bits)
{
  unsigned short i;
  for (i = 0; i < 5; i++)
  {
    ir_display[i] = hex_values[bits % 10];
    bits = bits/10; // bit reversed order
  }
}

//---------------------------------------------------
// Hardware Initialization
//---------------------------------------------------
void init(void)
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;

  // set DCO - Digitally-Controlled Oscillator frequency
  // change from 1MHz to 16MHz
  // BCSCTL1 = CALBC1_8MHZ;
  // DCOCTL  = CALDCO_8MHZ;

  P1DIR |= LED1 + BIT6;
  P1OUT &= ~LED1;
  P1SEL2 &= BIT1;   // TIM0 input capture function
  P1SEL  |= BIT1;   // TIM0 input capture function

  P2DIR = BIT5 + BIT4;
//  P2OUT = BIT5;
//  P2REN = BIT4;

  // Initialize Timer0_A
  TA0CTL = TASSEL_2 + ID_3 + MC_0;
  // set up TAC0CTL1 for compare mode
  TA0CCTL1 = CCIE;
  TA0CCR1 = TIMEOUT;       // end of data timeout
  // set up TAC0CTL0 for capture mode
  TA0CCTL0 = CM_2 + CCIS_0 + CAP + CCIE;

  __enable_interrupt();    // set GIE in SR
}

//-----------------------------------
//  Main
//-----------------------------------

void main(void)
{
  unsigned short i, j;
  unsigned long thisbit;
  init();
  DataReceived = FALSE;
  Count = 0;

  while(1) {
    if (DataReceived == TRUE) {
      DataBits = 0;
      DataBitsReversed = 0;

      for (i = 0, j = 2; i < NUM_BITS; i++, j++) {
        thisbit = time[j] > ZERO_LIMIT;
        DataBits = (DataBits << 1) | thisbit;
        DataBitsReversed = (DataBitsReversed >> 1) | (thisbit * 0x80000000);
      } // end for loop

      display_HEX(DataBits);
      //display_BRD(DataBitsReversed);

      DataReceived = FALSE;

      // Case statement here using HEX IR codes
      // Switch ir_display
      // Case
      // etc
      switch(ir_display[1]){
      // Go backwards
      case '4':
    	  P2OUT = BIT5 + BIT2;
    	  P2REN = BIT4 + BIT1;

    	  break;
      // Go forwards
      case '5':
    	  P2OUT = BIT4 + BIT1;
    	  P2REN = BIT5 + BIT2;
    	  break;
      case 'E':
      // Turn left
    	  P2OUT = BIT2 + BIT4;
    	  P2REN = BIT1 + BIT5;
    	  break;
      case 'A':
      // Turn right
    	  P2OUT = BIT1 + BIT5;
    	  P2REN = BIT2 + BIT4;
    	  break;
      case '6':
    	  P2OUT = 0;
    	  P2REN = 0;
      }

      _BIS_SR(LPM0_bits + GIE);
    }   // end data received
  }     // end while loop
}       // end main

//-----------------------------------
//  Interrupt Vectors
//-----------------------------------

#pragma vector = TIMER0_A0_VECTOR
__interrupt void capture_IRQ(void)
// interrupts from CCR0
{
//  P1OUT |= LED1;
  TA0CTL = TASSEL_2 + ID_3 + TACLR + MC_2;  // start timer in continuous mode
  time[Count]  = TA0CCR0;                   // read capture time
  Count = ++Count % 128;
  time[Count] = 0;
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void timeout_IRQ(void)
// interrupt from CCIFG in TA0CCTL1 - highest priority
// interrupt from CCIFG in TA0CCTL2
// interrupt from TAIFG in TA0CTL
{
  WhichIRQ = TA0IV;  // must read to clear flags
  TA0CTL = TACLR;    // stop and clear timer
//  P1OUT &= ~LED1;
  Count = 0;
  DataReceived = TRUE;
  _BIC_SR(LPM0_EXIT);
}
