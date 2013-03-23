
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

uint16_t read_adc(uint8_t chan);

int main(void)
{
	unsigned char slaveAddress, temp;

	sei();
  
	// enable the ADC circuitry, free-running mode, interrupt with /2 prescaler
	ADCSRA = (1<<ADEN) | (1<<ADSC) | (1<<ADPS2) | (1<<ADPS1);
	// wait for complete conversion
	while ( ADCSRA & ( 1 << ADSC ) );    

	slaveAddress = 0x26;		// This can be change to your own address
  
	usiTwiSlaveInit(slaveAddress);
	for(;;) {
		if(usiTwiDataInReceiveBuffer()) {
			temp = usiTwiReceiveByte();
			uint16_t v = read_adc(temp);
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