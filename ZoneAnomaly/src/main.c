#include <Arduino.h>
#include "utils/uart.h"

#define SOUND PB0
#define PIR PB1
#define SWITCH PB2

#define COOLDOWN 7e3

bool activated = false;

void collapse()
{
	for (int j =  0; j < 100; j++) {
		for (int i = 0; i < 25; i++){
			digitalWrite(SWITCH, HIGH);
			delayMicroseconds(750);
			digitalWrite(SWITCH, LOW);
			delayMicroseconds(750);
		}
		_delay_ms(50);
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

 
}

void loop()
{
	if (!activated) {
		if (uart_getc() == '1'){
			uart_puts("READY!\r\n");
			activated = true;
		}
	} else {
		if ((digitalRead(SOUND) == HIGH) || (digitalRead(PIR) == HIGH)){
			collapse();
			_delay_ms(COOLDOWN);
		}
	}
}