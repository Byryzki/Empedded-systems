#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdio.h>
#include <util/delay.h>

// Constants
#define F 16000000
#define USART_BAUDRATE 9600 // predefined BAUD rate between virtual terminal and a mikrocontroller
#define LED_PIN PD3
#define POWER_SWITCH PD2
#define POTENTIOMETER PC0
#define SOUNDER PB1

#define UBRR_VALUE      ((F_CPU/16/USART_BAUDRATE)-1)
#define RX_BUFFER_SIZE  128
#define RX_LINE_SIZE    128


// Global variables
volatile uint8_t BRIGHTNESS = 0;
volatile uint8_t POWER_SWITCH_STATE = 0;
volatile uint16_t FREQ = 500;
volatile uint16_t ADC_VALUE = 0;

void PORT_SETUP() {
	// Set LED pin as output
	DDRD |= (1 << LED_PIN);

	// Set button pin as input with pull-up resistor
	DDRD &= ~(1 << POWER_SWITCH);
	PORTD |= (1 << POWER_SWITCH); // Enable pull-up resistor

  // Set button pin as input with pull-up resistor
	DDRC &= ~(1 << POTENTIOMETER);
	PORTC |= (1 << POTENTIOMETER); // Enable pull-up resistor

	// Enable pin change interrupt on button pin
	PCICR |= (1 << PCIE2);
	PCMSK2 |= (1 << PCINT18);

	// Enable external interrupt to INT0
	EICRA |= (1 << ISC00);
	EIMSK |= (1 << INT0);

	// Enable global interrupts
	sei();
}
// Setup for USART

void USART_init(unsigned int ubrr) {
	// Set baud rate and 16bit register
	UBRR0H = (unsigned char)( ubrr >> 8 );
	UBRR0L = (unsigned char)ubrr;
	
	// Enable receiver and transmitter
	UCSR0B = (1 << RXEN0)|(1 << TXEN0);
	
	// Set frame format: 8 data bits, 1 stop bit
	UCSR0C = (1 << UCSZ01)|(1 << UCSZ00);
	
}


void USART_Transmit_16bit(uint16_t DATA) {
	unsigned char High = (DATA >> 8) & 0xFF;
	unsigned char Low = (DATA) & 0xFF;
	
	USART_Transmit(High);
	USART_Transmit(Low);
}
// Transmitting to a virtual terminal
void USART_Transmit( unsigned char data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) );
	/* Put data into buffer, sends the data */
	UDR0 = data;
}


// ADC conf
void ADC_init()
{
  ADMUX = 0b01000000;   //external VCC and ADC 0 selected
  ADCSRA = 0b10000111;  //ADEN enabled and 128 prescalar
  ADCSRB = 0x00;        // no need for ACME, free running mode
}

uint16_t ADC_Convert (void)
{
	ADCSRA |= 1<<ADSC;  // start conversion
	while ((ADCSRA&(1<<ADSC)) == 1){}  // wait for the conversion to finish
	uint8_t adcl = ADCL; // read ADCL register
	uint8_t adch = ADCH; // read ADCH Register
	uint16_t val = ((adch<<8)|adcl)&0x3FF;  // combine into single 10 bit value, 0x3FF-> 0b11 1111 1111	 
	return val;
}

void VirtualTerminal_on()
// The frequency can also be tuned by sending a ‘+’ or a ‘-‘ characters from the virtual terminal. 
// E.g., ‘+’ means adding 10 Hz to the frequency. Also, as a non- mandatory extra, 
// you can send the frequency from the terminal as a numerical value (like 440).
//ei vielä toteutusta tässä
{
  while(1)
  {   
      uint16_t ADC_val = ADC_Convert(); //this to FREQ?
      USART_Transmit_16bit(FREQ);
      _delay_ms(200);
  }

}

// Low power mode when the system is off. Using Power save mode
void LowPower() {
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	cli();
	sleep_enable();
	sei();
	sleep_cpu();
	sleep_disable();
}


int main(void)
{
	// setup ports
	PORT_SETUP();
	// setup receive and transmit of USART
  USART_init(UBRR_VALUE);
  ADC_init();

	while (1)
	{
		// If system turns in then execute these
		if(POWER_SWITCH_STATE)
		{
			PORTD |= (1 << LED_PIN); // Just to show if the system is on
			VirtualTerminal_on();
			_delay_ms(2000);
			
		} else
		{
			PORTD &= ~(1 << LED_PIN); // Turn off LED
			LowPower();
		}
	}

	return 0;
}

// Interrupt service for pressing the button and setting the system on and off
ISR(PCINT2_vect)
{
	// Debounce the button input
	_delay_ms(50);

	// If the button is pressed, toggle the power switch state
	if ((PIND & (1 << POWER_SWITCH)) == 0)
	{
		POWER_SWITCH_STATE ^= 1;
	}
	// If the button is released, do nothing
	else
	{
		return;
	}
}

// Interrupt service for waking up the system from sleep mode
ISR(INT0_vect)
{
	// Clear INT0 flag
	EIFR |= (1 << INTF0);

	// Wake up from sleep mode
	sleep_disable();
}
