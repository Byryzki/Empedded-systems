// reset a bit
PORTA |= (1<<PA2);

// reset a bit
PORTA &= ~(1<<PA2);

// bit flip
PORTA ^= (1<<PA2);