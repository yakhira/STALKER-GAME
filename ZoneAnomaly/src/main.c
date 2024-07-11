#include <Arduino.h>
#include "utils/uart.h"

#define SOUND PB0
#define PIR PB1
#define SWITCH PB2

#define COOLDOWN 5e3

bool activated = false;

void collapse();

ISR(PCINT0_vect)
{
   _delay_ms(50); 

	if (activated) {
		if ((digitalRead(SOUND) == HIGH) || (digitalRead(PIR) == HIGH)){
			collapse();
			_delay_ms(COOLDOWN);
		}
	}
}

void collapse()
{
	for (int j =  0; j < 1000; j++) {
		for (int i = 0; i < 25; i++){
			digitalWrite(SWITCH, HIGH);
			delayMicroseconds(750);
			digitalWrite(SWITCH, LOW);
			delayMicroseconds(750);
		}

		_delay_ms(50);

		for (int i = 0; i < 25; i++){
			digitalWrite(SWITCH, HIGH);
			delayMicroseconds(500);
			digitalWrite(SWITCH, LOW);
			delayMicroseconds(500);
		}
		_delay_ms(100);
	}
}

void setup(void)
{
	uart_puts("AT+NAMBZoneAnomaly\r\n");
	uart_puts("OK!\r\n");

	pinMode(SOUND, INPUT);
	pinMode(PIR, INPUT);
	pinMode(SWITCH, OUTPUT);

	digitalWrite(SWITCH, LOW);

	GIMSK |= (1<<PCIE); 
	PCMSK |= (1<<SOUND)|(1<<PIR);

	sei(); 
}

void loop()
{
	if (!activated) {
		if (uart_getc() == '1'){
			uart_puts("READY!\r\n");
			activated = true;
		}
	}
}