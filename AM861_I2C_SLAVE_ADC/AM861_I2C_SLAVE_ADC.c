
/***************************************************************************
*
* Dan Gates
*
* File              : I2C_Slave.c
* Compiler          : AVRstudio 4
* Revision          : 1.0
* Date              : Tuesday, June 11, 2007
* Revised by        : Dan Gates
*
*
* Target device		: ATtiny45
*
* AppNote           : AVR312 - Using the USI module as a I2C slave.
*
* Description       : Program for returning Analog data over an I2C port.
*                   : This program assumes that you have an analog sensor
*					: attached to PB4 (ADC2). Other inputs are digital and
*					: were used to set the slave address (not used in this sample).
*
* Connections
*
*                             AT tiny 45
*                 +--------------------------------+
* Address select1 | 1 pb5 reset              VCC 8 |
* Address select2 | 2 pb3                SCL pb2 7 | SCL
*  In from analog | 3 pb4 ADC2               pb1 6 | Address select3
*                 | 4 GND                SDA pb0 5 | SDA
*                 +--------------------------------+
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
			//usiTwiTransmitByte((uint8_t)(v >> 8));
		}
	asm volatile ("NOP" ::);
	}
}

//	read selected channel with 10-bit precision
uint16_t read_adc(uint8_t chan) {
	//	if 8-bit precision only is required, then set the ADLAR bit and just read ADCH
	//ADMUX = (chan & ~0b00001111) | (1<<REFS0);
	ADMUX = (1<<ADLAR) | chan;
	//ADMUX = (1<<ADLAR) | (1<<MUX1);
	ADCSRA = (1<<ADEN) | (1<<ADSC);
	asm volatile ("NOP" ::);
    asm volatile ("NOP" ::);
	while ( ADCSRA & ( 1 << ADSC ) );
	return ADCH;
	//uint8_t result_l = ADCL;
	//uint8_t result_h = ADCH;
	//return (result_h << 8) | result_l;
}