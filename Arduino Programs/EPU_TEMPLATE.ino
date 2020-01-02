#include <SPI.h>

#define SERIAL_BAUD_RATE 9600

#define HLT_PIN 2
#define IWR_PIN 3
#define RST_PIN 4
#define SS_OUTPUT_REG 9
#define SS_INPUT_REG 10

void init_IO() {
  Serial.begin(SERIAL_BAUD_RATE);
  SPI.begin();
  
  pinMode(HLT_PIN, OUTPUT);
  pinMode(IWR_PIN, OUTPUT);
  pinMode(RST_PIN, OUTPUT);
  pinMode(SS_OUTPUT_REG, OUTPUT);
  pinMode(SS_INPUT_REG, OUTPUT);

  digitalWrite(HLT_PIN, LOW);
  digitalWrite(IWR_PIN, LOW);
  digitalWrite(RST_PIN, LOW);
  digitalWrite(SS_OUTPUT_REG, HIGH);
  digitalWrite(SS_INPUT_REG, HIGH);
  }

void halt_epu() {
  digitalWrite(HLT_PIN, HIGH);
  }
void unhalt_epu() {
  digitalWrite(HLT_PIN, LOW);
  }

void reset_epu() {
  digitalWrite(RST_PIN, HIGH);
  delay(1);
  digitalWrite(RST_PIN, LOW);
  }

void send_input_epu(uint8_t input) {
  digitalWrite(SS_INPUT_REG, LOW);
  SPI.transfer(input);
  digitalWrite(SS_INPUT_REG, HIGH);
  }

uint8_t return_output_epu() {
  uint8_t output;

  digitalWrite(SS_OUTPUT_REG, LOW);
  output = SPI.transfer(0);
  digitalWrite(SS_OUTPUT_REG, HIGH);

  return output;
  }
  
uint8_t return_output_change_epu(uint8_t prev_output) {
  uint8_t output = prev_output;

  while (output == prev_output) {
    digitalWrite(SS_OUTPUT_REG, LOW);
    output = SPI.transfer(0);
    digitalWrite(SS_OUTPUT_REG, HIGH);
    }
    
  return output;
  }

void serial_clear() {
  while (Serial.available() > 0) Serial.read();
  }

uint8_t serial_get_uint8() {
  uint8_t serial_uint8;
  
  while (Serial.available() == 0);
  serial_uint8 = Serial.parseInt();
  serial_clear();

  return serial_uint8;
  }

void setup() {
  init_IO();
}

void loop() {
  // put your main code here, to run repeatedly:

}
