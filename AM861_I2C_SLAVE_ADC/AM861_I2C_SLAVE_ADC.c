
/***************************************************************************
*
* Alan K Duncan
*
* File              : AM861_I2C_SLAVE_ADC.c
* Compiler          : AVRStudio 6
* Revision          : 1.0
* Date              : March 22, 2013
* Revised by        : Alan Duncan, original code by Dan Gates.  Adapted for ATtiny861
*
*
* Target device		: ATtiny861
*
* AppNote           : AVR312 - Using the USI module as a I2C slave.
*
* Description       : Program for returning Analog data over an I2C port.
*
* Connections
*
****************************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "usiTwiSlave.h"

#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

//	OPERATIONAL CODES

enum { RADC0 = 0, RADC1, RADC2, RADC3, RADC4, RADC5, RADC6, RADC7, RADC8, RADC9, RADCT = 0x3F };
typedef uint8_t adc_code_t;


uint16_t read_adc(uint8_t chan);
uint16_t read_temp(void);

int main(void)
{
	DDRB |= (1<<PB6);
	
	PORTB |= (1<<PB6);
	unsigned char slaveAddress, temp;

	sei();
  
	// enable the ADC circuitry, free-running mode, interrupt with /64 prescaler
	ADCSRA = (1<<ADEN) | (1<<ADSC) | (1<<ADPS2) | (1<<ADPS1);
	// wait for complete conversion
	while ( ADCSRA & ( 1 << ADSC ) );    

	slaveAddress = 0x26;		// This can be change to your own address
  
	usiTwiSlaveInit(slaveAddress);
	for(;;) {
		if(usiTwiDataInReceiveBuffer()) {
			uint16_t v;
			adc_code_t code = (adc_code_t)usiTwiReceiveByte();
			if( code == RADCT ) {
				PORTB &= ~(1<<PB6);
				v = read_temp();
			}
			else {
				v = read_adc((uint8_t)code);
			}
			usiTwiTransmitByte((uint8_t)v);
			usiTwiTransmitByte((uint8_t)(v >> 8));
		}
	asm volatile ("NOP" ::);
	}
}

//	read selected channel with 10-bit precision
uint16_t read_adc(uint8_t chan) {
	//	if 8-bit precision only is required, then set the ADLAR bit and just read ADCH
	ADMUX = chan;
	ADCSRA = (1<<ADEN) | (1<<ADSC);
	asm volatile ("NOP" ::);
    asm volatile ("NOP" ::);
	while ( ADCSRA & ( 1 << ADSC ) );
	uint8_t result_l = ADCL;
	uint8_t result_h = ADCH;
	return (result_h << 8) | result_l;
}

uint16_t read_temp(void) {
	//	MUX5..0 set to 0b111111 to enable special ADC11 channel
	//	set 1.1V internal reference
	ADMUX = (1<<REFS1) | (1<<MUX4) | (1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (1<<MUX0) ;
	ADCSRB |= (1<<MUX5);
	asm volatile ("NOP" ::);
    asm volatile ("NOP" ::);
	while ( ADCSRA & ( 1 << ADSC ) );
	uint8_t result_l = ADCL;
	uint8_t result_h = ADCH;
	ADCSRB &= ~(1<<MUX5);
	return (result_h << 8) | result_l;
}