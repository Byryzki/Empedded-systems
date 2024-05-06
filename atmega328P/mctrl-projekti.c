/*
 * Laine_Pyry_project.c
 *
 * Created: 12.4.2024 14.04.31
 * Author : smpyla
 */ 

#define F_CPU           16000000
#define USART_BAUDRATE  9600
#define UBRR_VALUE      ((F_CPU/16/USART_BAUDRATE)-1)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>

//Global variables
volatile uint8_t MAX_FREQ = 1000;
volatile uint8_t ADC_VALUE = 0;
volatile uint16_t vol = 0;

void io_pins_init() 
{
	  // Input pins activation
	  DDRD &= ~(1 << PIND2);  //on/off-button
	  DDRD &= ~(1 << PIND0);  //virtual terminal TX
	  DDRC &= ~(1 << PINC2);  //Potentiometer

	  //Output pins
	  DDRB |= (1 << PINB1);   //LED
	  DDRB |= (1 << PINB2);   //Sounder
	  DDRB |= (1 << PINB3);   //SPI connector 4
	  DDRB |= (1 << PINB4);   //SPI connector 1
	  DDRB |= (1 << PINB5);   //SPI connector 3
	  DDRB |= (1 << PINB6);   //XTAL 1
	  DDRB |= (1 << PINB7);   //XTAL 2
	  DDRD |= (1 << PIND0);   //Virtual terminal RX
	  DDRD |= (1 << PIND1);   //Virtual terminal TX
}

void PowerOn()
// If the on/off switch is on, the system operates normally. 
{
	  PORTB |= (1 << PINB1);   //LED
	  PORTB |= (1 << PINB2);   //Sounder
	  PORTB |= (1 << PINB3);   //SPI connector 4
	  PORTB |= (1 << PINB4);   //SPI connector 1
	  PORTB |= (1 << PINB5);   //SPI connector 3
	  PORTB |= (1 << PINB6);   //XTAL 1
	  PORTB |= (1 << PINB7);   //XTAL 2
	  PORTC |= (1 << PINC2);   //Potentiometer
	  PORTD |= (1 << PIND0);   //Virtual terminal RX
	  PORTD |= (1 << PIND1);   //Virtual terminal TX
	  PORTC |= (1 << PINC2);   //Potentiometer on
}

void LowPowerMode()
//If it is off, the system is still powered but 
//stops operating and waits until the switch is again in on-position. Waiting should be done in a "low power mode".
{
	PORTB &= ~(1 << PINB1);   //LED
	PORTB &= ~(1 << PINB2);   //Sounder
	PORTB &= ~(1 << PINB3);   //SPI connector 4
	PORTB &= ~(1 << PINB4);   //SPI connector 1
	PORTB &= ~(1 << PINB5);   //SPI connector 3
	PORTB &= ~(1 << PINB6);   //XTAL 1
	PORTB &= ~(1 << PINB7);   //XTAL 2
	PORTC &= ~(1 << PINC2);   //Potentiometer
	PORTD &= ~(1 << PIND0);   //Virtual terminal RX
	PORTD &= ~(1 << PIND1);   //Virtual terminal TX
	PORTC &= ~(1 << PINC2);   //Potentiometer on
}

/*
Optional implementation on the sleepmode

void SleepMode()
//If it is off, the system is still powered but
//stops operating and waits until the switch is again in on-position. Waiting should be done in a low power mode.
{
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	cli();
	sleep_enable();
	sei();
	sleep_cpu();
	sleep_disable();
}
*/
void timer_init() {
	TCCR1A |= (1 << COM1A1) | (1 << WGM10); //Enabling timer in fast PWM mode. COM1A1 tells the microcontroller to set the output of the OCR1A pin low when the timer's counter reaches a compare value.
	TCCR1B |= (1 << CS11) | (1 << CS10) | (1 << WGM12); // prescalar 64

  // Set compare value for desired frequency (adjustable based on potentiometer value)
  OCR1A = 31250; // Initial frequency (16MHz / (64 * 31250) = 50 Hz)
}

void ADC_init() {
    ADMUX |= (1 << REFS0);  // Set ADC reference voltage to "AVCC with external capacitor at AREF pin"
    ADMUX |= (1 << ADLAR);  // Set ADC left adjust result to use ADCH register
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Set ADC prescaler to 128 (for 16MHz clock, this results in ADC clock of 125kHz)
    ADCSRA |= (1 << ADEN);  // Enable ADC
    ADCSRA |= (1 << ADSC);  // Start ADC conversion
}

void serial_init()
{
	UBRR0 = 103;	//set bitrate
	UCSR0B = (1 << TXEN0);	//Transmission enabled
	UCSR0C = (1 << UCSZ01)|(1 << UCSZ00);	//async mode, 8N1 frame format
}

void sounder()
//The value of the potentiometer to control the frequency of the SOUNDER is read by the internal AD-converter. Scale the frequency inside the range 50…1000Hz.
{
  vol = read_ADC(2);  // Read potentiometer value
  uint16_t frequency = 50 + (vol / 1023.0) * 450; // Map potentiometer value to frequency range 50 Hz to 500 Hz
  OCR1B = 16000000 / (64 * frequency);  // Set new frequency for sounder
}

uint16_t read_ADC(uint8_t channel) {
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);  // Clear previous channel selection and set new channel
    ADCSRA |= (1 << ADSC);  // Start ADC conversion
    while (ADCSRA & (1 << ADSC)); // Wait for conversion to complete
    return ADC; // Read ADC result
}

uint16_t ADC_convert (void)
{
	uint16_t val = 0;
	ADCSRA |= (1<<ADSC);  // start conversion
	while ((ADCSRA & (1<<ADSC))); // wait for the conversion to finish
	val |= ADCL;
	val |= (ADCH << 8);
	return ((uint32_t)val*1000)/1023; //sounder frequency
}

void serial_transmit( uint32_t data)
// Transmitting to a virtual terminal
{
	
	while (!( UCSR0A & (1<<UDRE0)));  // Wait for empty transmit buffer
	UDR0 = data;  //Puts data into buffer, sends the data
}

void VirtualTerminal_on()
// The frequency can also be tuned by sending a ‘+’ or a ‘-‘ characters from the virtual terminal.
// E.g., ‘+’ means adding 10 Hz to the frequency. Also, as a non- mandatory extra,
// you can send the frequency from the terminal as a numerical value (like 440).
//ei vielä toteutusta tässä
{
  uint16_t FREQ = ADC_convert();  //Acquires value from ADC2
  serial_transmit(FREQ);  // Send given value to USART
  UDR0='\r';  //  New row in virtual terminal
	UDR0='\n';
  _delay_ms(200);  //wait 2 seconds
}

void LED_blink()
// LED blinks smoothly when the sound is on. The brightness of the LED is controlled with PWM generated with the AVR timer/counter.
{
	//timer_init() initiated timer with PWM generated already

  uint8_t pwm = 0x00;
	bool up = true;

  for(;;) //Bad implementation but makes the LED work with what we got
  {
    if(PIND & (1 << PIND2)) //When power button toggled off
    {
      OCR1A = 0x00; //LED flickering off
      return 0;
    }

    OCR1A = pwm;  //Transfer PWM signal to the LED
    pwm += up ? 1 : -1; //brightness oscillates back and forth
    if(pwm == 0xff)
    {
      up = false;
    }
    else if(pwm == 0x00)
    {
      up = true;
    }
    _delay_ms(0.1);
  }
}

int main()
{
  io_pins_init();
  ADC_init();
  serial_init();
  timer_init();
  sei();  // Enable global interrupts

  while(1) // main operation loop
    {
      if(!(PIND & (1 << PIND2)))  //ON/OFF-button is pressed (rising edge)
      {
        PowerOn();
        sounder();
        if(PINB & (1 << PINB2)){LED_blink();} //LED follows the sound of the sounder
        VirtualTerminal_on();
      }
      else
      {
        LowPowerMode(); //power button up, low mode on
      }
    }

  return 0;
}

ISR(ADC_vect) //change sounder frequency when potentiometer changes
{
    uint16_t vol = ADC_convert(); // Read ADC value
    OCR1B = vol >> 2; // Scale 10-bit ADC value to 8-bit (0-255) and update to sounder pin
}
