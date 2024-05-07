// Ring buffer

#define BUFFER_SIZE 32

typedef struct {
    uint8_t buffer[BUFFER_SIZE];
    volatile uint8_t head;
    volatile uint8_t tail;
} RingBuffer;

void initRingBuffer(RingBuffer* buffer) {
    
    buffer->head = 0;
    buffer->tail = 0;
}

uint8_t isBufferEmpty(RingBuffer* buffer) {
    
    return (buffer->head == buffer->tail);
}

uint8_t isBufferFull(RingBuffer* buffer) {
    
    return ((buffer->head + 1) % BUFFER_SIZE == buffer->tail);
}

uint8_t readFromBuffer(RingBuffer* buffer) {
    
    if (isBufferEmpty(buffer))
    {
        // Handle buffer underflow
        return 0; // Or any suitable error code
    } 
    else
    {
        uint8_t data = buffer->buffer[buffer->tail];
        buffer->tail = (buffer->tail + 1) % BUFFER_SIZE;
        return data;
    }
}

void writeToBuffer(RingBuffer* buffer, uint8_t data) {
    
    if (isBufferFull(buffer))
    {
        initRingBuffer;   //overwrite
    }

    buffer->buffer[buffer->head] = data;
    buffer->head = (buffer->head + 1) % BUFFER_SIZE;

}

main()
{
    // Initialize the ring buffer
    RingBuffer buffer;
    initRingBuffer(&buffer);
}