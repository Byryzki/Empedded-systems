//  Default sleep set
#include <avr/interrupt.h> // interrupt definitions
#include <avr/sleep.h> // sleep-macro definitions
…
set_sleep_mode(<mode>); // set the sleep-mode you want
cli(); // disable interrupts
if (some_condition)
{
sleep_enable(); // set SE-bit
sei(); // enable interrupts
sleep_cpu(); // SLEEP-instruction
//  entry-point after wake-up
sleep_disable(); // reset SE-bit
}
sei(); // enable interrupts

//  Wake on PC interrupt
#include <avr/interrupt.h> // interrupt definitions
#include <avr/sleep.h> // sleep-macro definitions

DDRB &= ~(1<<PB0); // port B pin 0 as input
PORTB |= (1<<PB0); // and enable pull-up
PCICR |= (1<<PCIE0); // enable PCINT
PCMSK0 |= (1<<PCINT0); // on B port pin 0
//...
ISR (PCINT0_vect) { } // wake-up from PCINT0 (start-button port B pin 0)
//...
set_sleep_mode(SLEEP_MODE_PWR_DOWN); // set the power-down sleep-mode
cli(); // disable interrupts
if (~(PINC & (1<<PC4)) // if the stop-button is pressed (port C pin 4 is 0)
{
    sleep_enable(); // set SE-bit
    sei(); // enable interrupts
    sleep_cpu(); // SLEEP-instruction
    // entry-point after wake-up
    sleep_disable(); // reset SE-bit
}
sei(); // enable interrupts