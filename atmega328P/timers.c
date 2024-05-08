void timer_init() {
	TCCR1A |= (1 << COM1A1) | (1 << WGM10); //Enabling timer in fast PWM mode. COM1A1 tells the microcontroller to set the output of the OCR1A pin low when the timer's counter reaches a compare value.
	TCCR1B |= (1 << CS11) | (1 << CS10) | (1 << WGM12); // prescalar 64

  // Set compare value for desired frequency (adjustable based on potentiometer value)
  OCR1A = 31250; // Initial frequency (16MHz / (64 * 31250) = 50 Hz)
}