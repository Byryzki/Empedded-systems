/*
 * PushButtonLED.c
 *
 * Created: 27.1.2024 16.39.35
 * Author : smpyla
 */ 

#include <avr/io.h>
#include <util/delay.h>
#define LED_PIN		PORTB5
#define BUTTON_PIN  PIND2

void initIO()
{
	DDRB |= (1 << LED_PIN); // set D13 (LED) as output
	
	// set D2 (button) as input with pull-up resistor
	DDRD &= ~(1 << BUTTON_PIN);
	PORTD |= (1 << BUTTON_PIN);
}

int isButtonPressed()
{
	return !(PIND & (1 << BUTTON_PIN)); //active low
}

void blinkLED()
{
	PORTB ^= (1 << LED_PIN); //toggling LED state
}

int main(void)
{
    initIO();
	
    while (1) 
    {
		if(isButtonPressed())
		{
			blinkLED();
			_delay_ms(200);
		}
    }
	return 0;
}
