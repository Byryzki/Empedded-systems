#include <avr/io.h>
#include <avr/interrupt.h>

ISR(INT0_vect) {
    // Read pins 0-3 of PORTB
    uint8_t input = PINB & 0x0F;

    // Output the same value to pins 4-7 of PORTB
    PORTB = (PORTB & 0xF0) | input;
}

void initInterrupt() {
    // Enable external interrupt INT0
    EIMSK |= (1 << INT0);

    // Trigger INT0 on falling edge
    EICRA |= (1 << ISC01);
    EICRA &= ~(1 << ISC00);

    // Enable global interrupts
    sei();
}

int main() {
    // Set pins 0-3 of PORTB as input
    DDRB &= 0xF0;

    // Set pins 4-7 of PORTB as output
    DDRB |= 0xF0;

    // Enable pull-up resistors for pins 0-3 of PORTB
    PORTB |= 0x0F;

    initInterrupt();

    while (1) {
        // Your main program loop
    }

    return 0;
}
