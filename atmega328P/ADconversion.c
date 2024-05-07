//  with polling

void initADC(void)
{
// init AD channel 0
// disable channel 0 input buffer
DIDR0 |= 0x01;
// write 1 to corresponding output pin
PORTC |= 0x01;
// choose channel 0, ADMUX[MUX3:MUX0] -> 0b0000
// default is OK
// choose internal Vcc reference,
// ADMUX[REFS1:REFS0] -> 0b01
ADMUX |= (1<<REFS0);
// use only high byte ADCSRA[ADLAR] -> 1
ADMUX |= (1<<ADLAR);
// clock prescaler 128, ADCSRA [ADPS 2:0] -> 0x111
ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
// enable AD-converter in Power Reduction Register,
// PRR[PRADC] -> 0
PRR &= ~(1<<PRADC);
// enable AD-converter in ADCSRA
ADCSRA |= (1<<ADEN);
}

uint8_t readADC8(void)
{
// start conversion ADCSRA[ADSC] -> 1
ADCSRA |= (1<<ADSC);
// poll ADCSRA[ADSC] -> 1
while (ADCSRA & (1<<ADSC)) ;
return ADCH;
}

// with interrupts

void initADC(void)
{
    TCCR1B = (1 << WGM12); // timer/counter to waveform generation mode (clear timer on compare match mode)
    TCCR1B |= ((1 << CS12)|(1 << CS10))   //prescaling to 1024

    OCR1A = 1000;   // set compare intervals
    OCR1B = 1000;

    TIFR1 |= (1 << OCF1B)   //clear possible waiting interrupt
    TIMSK1 = (1 << OCIE1B); //enable interrupt

    ADMUX = (1 << REFS0);   //AREF = AVCC

    ADCSRB = ((1 << ADTS2)|(1 << ADTS0));   //ADC trigger source is timer1 compare match B

    //ADEN: enable conversion, ADSC: would start the conversion, ADATE: enable auto trigger, ADIF: possible waiting interrupt, ADIE: ADC interrupt enabled, ADPs: ADC clock divider 
    ADCSRA = ((1 << ADEN)|(1 << ADATE)|(1 << ADIF)|(1 << ADIE)|(1 << ADPS2)|(1 << ADPS1)|(1 << ADPS0));

    DIDR0 = (1 << ADC0D);   //power reduction by disabling digital input on utilized ADC pins
}

main(void)
{
    initADC();
    sei();  //enable global interrupts
}

ISR(ADC_vect)
{
    //read from 8bit ADCH 16bit ADC
    conversion_result = ADC;
    print = 1;
}