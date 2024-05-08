#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/delay.h>

// Constants
#define F 16000000
#define USART_BAUDRATE 9600 // predefined BAUD rate between virtual terminal and a mikrocontroller
#define UBRR_VALUE      ((F_CPU/16/USART_BAUDRATE)-1)
#define RX_BUFFER_SIZE  128
#define RX_LINE_SIZE    128

//  Define inputs and outputs
#define BUTTON  PD2
#define LED     PB1
#define SOUNDER PB2

// Global variables
volatile uint8_t BUTTON_STATE = 0;
volatile uint16_t FREQ = 500;
volatile uint16_t set_time = 0;
volatile uint16_t OverFlow_count = 0;

void PORT_SETUP() {
	// Set button pin as input with pull-up resistor
	DDRD &= ~(1 << BUTTON);
	PORTD |= (1 << BUTTON); // Enable pull-up resistor
  
  // Set output pins
	DDRB |= (1 << LED);
  DDRB |= (1 << SOUNDER);

	// Enable pin change interrupt on button pin
	PCICR |= (1 << PCIE2);  //For enabling interrupts for port range with our button
	PCMSK2 |= (1 << PCINT18);

	// Enable external interrupt to INT0
	EICRA |= (1 << ISC00);
	EIMSK |= (1 << INT0);

}

void init_timer()
{
  // Configuring Timer1 to overflow every 1 seconds
    TCCR1B |= (1 << CS12) | (1 << CS10); // Set prescaler to 1024
    TIMSK1 |= (1 << TOIE1); // Enable Timer1 overflow interrupt

    // Enable global interrupts
    sei();
}

void init_USART(unsigned int ubrr) {
	// Set baud rate and 16bit register
	UBRR0H = (unsigned char)( ubrr >> 8 );
	UBRR0L = (unsigned char)ubrr;
	
	// Enable receiver and transmitter
	UCSR0B = (1 << RXEN0)|(1 << TXEN0);
	
	// Set frame format: 8 data bits, 1 stop bit
	UCSR0C = (1 << UCSZ01)|(1 << UCSZ00);
}

void BeepBuzzer()
{
    // Configure Timer2 for generating a square wave for the buzzer
    TCCR2A |= (1 << WGM21); // Set CTC mode
    TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20); // Set prescaler to 1024
    OCR2A = (16000000 / 1024 / FREQ) - 1; // Set TOP value for FREQ times 1 kHz frequency
    TCCR2A |= (1 << COM2B0); // Toggle OC2B on compare match

    _delay_ms(2000);  //Beep for 2seconds

    // Turn off Timer2 and OC2B (buzzer pin)
    TCCR2A = 0;
    TCCR2B = 0;
    PORTB &= ~(1 << PB2); // Set buzzer pin low
}

void USART_Transmit(unsigned char data)
{
    while (!(UCSR0A & (1<<UDRE0)));
    UDR0 = data;
}

void USART_Transmit_String(char* str)
{
    while (*str)
    {
        USART_Transmit(*str++);
    }
}

char USART_Receive(void)
{
    while (!(UCSR0A & (1<<RXC0)));
    return UDR0;
}

int main(void)
{
	// setup ports
	PORT_SETUP();
	// setup receive and transmit of USART
  init_USART(UBRR_VALUE);
  init_timer();

	while (1)
	{
		// Receive user input for timer value via USART
      USART_Transmit_String("Enter timer value (0-65535): ");
      
      char buffer[6]; // Buffer to store user input
      uint8_t index = 0; // Index to keep track of characters in the buffer
      char received_char;

      // Receive characters until newline ('\n') is received
      do
      {
          received_char = USART_Receive();
          USART_Transmit(received_char);
          if (received_char != '\r' && index < 6)
          { // Ignore carriage return ('\r')
              buffer[index++] = received_char;
          }
      }
      while (received_char != '\n');
      buffer[index] = '\0'; // Null-terminate the string
      
      // Convert the user input string to an integer
      set_time = atoi(buffer)
      
    while(set_time == 0 && !BUTTON_STATE){}  //wait for user to submit time and push the button to start timer
	
    PORTB |= (1 << LED); // LED on to show timer running
    TCCR1B |= (1 << CS12) | (1 << CS10);  // Set prescaler to 1024 to start the timer
    while(OverFlow_count <= set_time) //waiting for timer to reach alarm
    {
      if(!BUTTON_STATE){break;}  //escaping waiting loop if timer is stopped
    }
    TCCR1B &= ~((1 << CS12) | (1 << CS10)); // Clear prescaler to stop the timer
    PORTB &= ~(1 << LED); // Turn off LED
    if(OverFlow_count == set_time){BeepBuzzer();} //2 sec alarm sound at the end of the timing

    OverFlow_count = 0;  //reset timer
  }
	

	return 0;
}

// Interrupt service for pressing the button and setting the system on and off
ISR(PCINT0_vect)
{
	// Debounce the button input
	_delay_ms(50);

	// If the button is pressed, toggle the power switch state
	if ((PIND & (1 << BUTTON_STATE)) == 0)
	{
		BUTTON_STATE ^= 1;
	}
	// If the button is released, do nothing
	else
	{
		return;
	}
}

// Timer1 overflow ISR
ISR(TIMER1_OVF_vect)
{
    OverFlow_count++; // Increment overflow count
}
