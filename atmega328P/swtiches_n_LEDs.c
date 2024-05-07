// most simple LED button toggle
#include <avr/interrupt.h>
#include <avr/sleep.h>
int main()
{
    short int a1;
    DDRB = DDRB & 0xFE; // pin 0 of port B as input, 0xFE == 0b11111110
    PORTB = 0x01; // pin 0 pull-up
    DDRC = DDRC | 0x01; // pin 0 of port C as output
    while (1)
    {
        a1 = PINB; // read push button
        PORTC = a1; // write output for LED
    }
    return 0;
}

//  Read 10 times
#define LIMIT 10 // define a name to a constant
int status; // momentary value of the switch
int count; // variable for counting states

DDRB = 0x00; // set all pins inputs
// read the switch ten times to check that the signal is stable
stable = 1; // we initially think signal is stable
status = PINB; // read the initial state
for(count=0; count<LIMIT; count++) 
{ // repeat LIMIT times
    if (status != PINB) // if the new value is different
        stable = 0; // the signal is not stable
}

//  waiting until switch is closed
#define LIMIT 10 // define a name to a constant
int status; // momentary value of the switch
int count; // variable for counting states
…
DDRB = 0x00; // set all pins inputs
// wait until the signal has setled to 0, i.e. the switch is closed
count = 0; // reset count
while (count < LIMIT) {
status = PINB; // read the switch
if (status = 0) count = count + 1; // increase count
if (status = 1) count = 0; // reset the counter if
// another wrong state found
}