#define RX_PIN 15
#define BAUD_RATE 9600

int val = 0;
int old_val = 0;
char message_buf[50] = {0};
bool receiving_byte = false;
int baud_rate = 9600; // The baud rate of the transmitting device
unsigned long wait_time_us = 1000000/BAUD_RATE;
int buff_idx = 0;

volatile char received_byte = 0;
uint8_t bits_received = 0;
char temp_byte = 0;

enum state {
  RECEIVING,
  NOT_RECEIVING
};

volatile state STATE = NOT_RECEIVING;
volatile unsigned long next_bit_time = 0;

void uart_receive_bit() {
  next_bit_time = micros() + wait_time_us;
  val = digitalRead(RX_PIN);
  received_byte |= (val << bits_received);
  bits_received++;
}

void set_receive_flag() {
  noInterrupts();
  if (STATE != RECEIVING) {
    STATE = RECEIVING;  
    next_bit_time = micros() + (wait_time_us/2) + wait_time_us;
  }
}

inline bool wait_over() {
  return (micros() > next_bit_time); 
}

inline bool received_full_byte() {
  if (bits_received == 7) {
    bits_received = 0;
    return true;
  } else return false;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(RX_PIN, INPUT);
  attachInterrupt(RX_PIN, set_receive_flag, FALLING);
  delay(1000);
  Serial.println("hello");
}

void loop() {
  switch (STATE) {
    case NOT_RECEIVING:
      // do whatever
      received_byte = 0;
    break;

    case RECEIVING:
      if (wait_over()) {
        uart_receive_bit();
        
        if (received_full_byte()) {
          delayMicroseconds(wait_time_us/2);
          message_buf[buff_idx++] = received_byte;
          temp_byte = received_byte;
          received_byte = 0;
          
          if (temp_byte == '\r') {
            message_buf[buff_idx] = '\0';
            buff_idx = 0;
            Serial.println("Received string:");
            Serial.println(message_buf);
            STATE = NOT_RECEIVING;
          }

          // handle potential overflow
          if (buff_idx >= sizeof(message_buf) -1) {
            message_buf[buff_idx] = '\0';
            buff_idx = 0;
            Serial.println("Overflow:");
            Serial.println(message_buf);
            STATE = NOT_RECEIVING;
          }
          interrupts();
          STATE = NOT_RECEIVING;
        }
      }
    break;
  }
}
