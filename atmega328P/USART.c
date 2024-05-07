//  connecting USART
USART_Init(MYUBRR);
for(int i=0;i<20;i++) USART_Transmit('a'+i);
for(int i=0;i<20;i++) USART_Transmit(USART_Receive());

//own implementation
void serial_init()
{
	UBRR0 = 103;	//set bitrate
	UCSR0B = (1 << TXEN0);	//Transmission enabled
	UCSR0C = (1 << UCSZ01)|(1 << UCSZ00);	//async mode, 8N1 frame format
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

