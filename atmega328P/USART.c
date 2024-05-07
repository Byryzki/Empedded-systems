//  connecting USART
USART_Init(MYUBRR);
for(int i=0;i<20;i++) USART_Transmit('a'+i);
for(int i=0;i<20;i++) USART_Transmit(USART_Receive());