/*
 * 8-bit computer project (SAP-1.5)
 * -------------------------------------
 * 
 * Program microcode on EEPROM chips for control unit.
 *  
 * -Adam Antoshin 2020
*/

#include <SPI.h>

//Program either most or least significant byte of control words
#define MSBYTE false

//wipe EEPROM memory before programming
#define CLEAR_BEFORE_PROG true

//IO parameters
#define SERIAL_BAUD_RATE 115200
#define OUTPUT_ENABLE_PIN A0
#define EEPROM_D0 2
#define EEPROM_D1 3
#define EEPROM_D2 4
#define EEPROM_D3 5
#define EEPROM_D4 6
#define EEPROM_D5 7
#define EEPROM_D6 8
#define EEPROM_D7 9
#define SS_595 10
#define WRITE_ENABLE_PIN A1

//EEPROM parameters
#define PROG_DELAY 20
#define EEPROM_MEM 8192

//ucode parameters
#define STEPS_NUM 8
#define INSTRUCTIONS_NUM 15
#define F_STATES_NUM 4
#define F_C0Z0 0
#define F_C0Z1 1
#define F_C1Z0 2
#define F_C1Z1 3
#define JZ 0b00001100
#define JC 0b00001101

//control word definition
#define HLT 0b1000000000000000
#define AI  0b0100000000000000
#define AO  0b0010000000000000
#define BI  0b0001000000000000
#define SU  0b0000100000000000
#define EO  0b0000010000000000
#define MI  0b0000001000000000
#define II  0b0000000100000000
#define RI  0b0000000010000000
#define RO  0b0000000001000000
#define CE  0b0000000000100000
#define CO  0b0000000000010000
#define NO  0b0000000000001000
#define OI  0b0000000000000100
#define J   0b0000000000000010
#define FI  0b0000000000000001

//ucode template (no conditional jumps)
const PROGMEM uint16_t UCODE_TEMPLATE[INSTRUCTIONS_NUM][STEPS_NUM] = 
{
    {MI|CO,  II|RO|CE,   CE,        0,         0,              0,              0,              0},  //00000000 - NOP
    {MI|CO,  II|RO|CE,   AO|CE|OI,  0,         0,              0,              0,              0},  //00000001 - OUT
    {MI|CO,  II|RO|CE,   MI|CO,     MI|RO,     BI|RO,          AI|EO|CE|FI,    0,              0},  //00000010 - ADD
    {MI|CO,  II|RO|CE,   MI|CO,     MI|RO,     BI|RO,          AI|SU|EO|CE|FI, 0,              0},  //00000011 - SUB
    {MI|CO,  II|RO|CE,   MI|CO,     MI|RO,     AI|RO|CE,       0,              0,              0},  //00000100 - LDA
    {MI|CO,  II|RO|CE,   AI|CE|NO,  0,         0,              0,              0,              0},  //00000101 - LDN
    {MI|CO,  II|RO|CE,   MI|CO,     AI|RO|CE,  0,              0,              0,              0},  //00000110 - LDI
    {MI|CO,  II|RO|CE,   MI|CO,     BI|RO,     AI|EO|CE|FI,    0,              0,              0},  //00000111 - ADI
    {MI|CO,  II|RO|CE,   MI|CO,     BI|RO,     AI|SU|EO|CE|FI, 0,              0,              0},  //00001000 - SUI
    {MI|CO,  II|RO|CE,   MI|CO,     MI|RO,     AO|RI|CE,       0,              0,              0},  //00001001 - STA
    {MI|CO,  II|RO|CE,   MI|CO,     MI|RO,     RI|CE|NO,       0,              0,              0},  //00001010 - STN
    {MI|CO,  II|RO|CE,   MI|CO,     RO|J,      0,              0,              0,              0},  //00001011 - JMP
    {MI|CO,  II|RO|CE,   0,         0,         0,              0,              0,              0},  //00001100 - JZ
    {MI|CO,  II|RO|CE,   0,         0,         0,              0,              0,              0},  //00001101 - JC
    {MI|CO,  II|RO|CE,   HLT,       0,         0,              0,              0,              0}   //00001110 - HLT
};

//final ucode array
uint16_t ucode[F_STATES_NUM][INSTRUCTIONS_NUM][STEPS_NUM];

//EEPROM I/O pins
byte eeprom_io_pins[8] = {EEPROM_D0, EEPROM_D1, EEPROM_D2, EEPROM_D3, EEPROM_D4, EEPROM_D5, EEPROM_D6, EEPROM_D7}; 

//initialize Arduino IO
void init_IO() {
  Serial.begin(SERIAL_BAUD_RATE);
  digitalWrite(SS_595, HIGH);
  pinMode(SS_595, OUTPUT);
  pinMode(OUTPUT_ENABLE_PIN, OUTPUT);
  digitalWrite(WRITE_ENABLE_PIN, HIGH);
  pinMode(WRITE_ENABLE_PIN, OUTPUT);
  }

//estimate programming time
float est_prog_time(int num, int del) {
  float est = del / 1000.0;
  est *= num;

  return est;
  }
  
//print initializing message
void print_init_msg(float p_time) {
  Serial.print("Estimated programming time: "); Serial.print(p_time); Serial.println(" seconds\n");
  
  if (MSBYTE == true) Serial.println("PROGRAMMING MOST SIGNIFICANT BYTE OF CONTROL WORDS...");
  else Serial.println("PROGRAMMING LEAST SIGNIFICANT BYTE OF CONTROL WORDS...");
  }

//print erasing message
void print_clear_msg(float c_time) {
  Serial.print("Estimated erasing time: "); Serial.print(c_time); Serial.println(" seconds\n");
  Serial.println("ERASING...\n");
  }

//merge conditional control words with ucode template to build final ucode
void init_ucode() {
  memcpy_P(ucode[F_C0Z0], UCODE_TEMPLATE, sizeof(UCODE_TEMPLATE));
  ucode[F_C0Z0][JZ][2] = CE;
  ucode[F_C0Z0][JC][2] = CE;
  
  memcpy_P(ucode[F_C0Z1], UCODE_TEMPLATE, sizeof(UCODE_TEMPLATE));
  ucode[F_C0Z1][JZ][2] = MI|CO;
  ucode[F_C0Z1][JZ][3] = RO|J;
  ucode[F_C0Z1][JC][2] = CE;
  
  memcpy_P(ucode[F_C1Z0], UCODE_TEMPLATE, sizeof(UCODE_TEMPLATE));
  ucode[F_C1Z0][JZ][2] = CE;
  ucode[F_C1Z0][JC][2] = MI|CO;
  ucode[F_C1Z0][JC][3] = RO|J;
  
  memcpy_P(ucode[F_C1Z1], UCODE_TEMPLATE, sizeof(UCODE_TEMPLATE));
  ucode[F_C1Z1][JZ][2] = MI|CO;
  ucode[F_C1Z1][JZ][3] = RO|J;
  ucode[F_C1Z1][JC][2] = MI|CO;
  ucode[F_C1Z1][JC][3] = RO|J;
  }

//output address with 595 shift registers and either output or input contents
void set_address(int address, bool output) {
  digitalWrite(OUTPUT_ENABLE_PIN, !output);
  digitalWrite(SS_595, LOW);
  SPI.transfer(address >> 8);
  SPI.transfer(address);
  //shiftOut(SHIFT_DATA_PIN, SHIFT_CLOCK_PIN, MSBFIRST, (address >> 8));
  //shiftOut(SHIFT_DATA_PIN, SHIFT_CLOCK_PIN, MSBFIRST, address);
  digitalWrite(SS_595, HIGH);
  }

//write byte to EEPROM memory
void write_EEPROM(uint16_t address, byte data) {
  Serial.print(address, HEX);
  Serial.print("  ");
  Serial.println(data, HEX);
  set_address(address, false);
  for (int i = 0; i < 8; i++) {
    digitalWrite(eeprom_io_pins[i], data & 0x01);
    pinMode(eeprom_io_pins[i], OUTPUT);
    data = data >> 1;
    }
  digitalWrite(WRITE_ENABLE_PIN, LOW);
  delayMicroseconds(1);
  digitalWrite(WRITE_ENABLE_PIN, HIGH);
  delay(PROG_DELAY);
  }

//read byte from EEPROM memory
uint8_t read_EEPROM(int address) {
  for (int i = 0; i < 8; i++) pinMode(eeprom_io_pins[i], INPUT);
  
  set_address(address, true);
  
  uint8_t data = 0;
  for (int i = 7; i >= 0; i--) {
    data = (data << 1) + digitalRead(eeprom_io_pins[i]);
    }
  return data;
  }

//wipe EEPROM memory
void clear_EEPROM() {
  float clear_time = est_prog_time(EEPROM_MEM, PROG_DELAY);
  print_clear_msg(clear_time);
  for (int address = 0; address < EEPROM_MEM; address++) {
    write_EEPROM(address, 0);
    Serial.println(address);
    }
  }

//print EEPROM contents
void print_contents(int num) {
  Serial.print("Reading first "); Serial.print(num); Serial.println(" bytes...\n");
  
  for (int i = 0; i <= num - 1; i += 16) {
    uint8_t dataIn[16];
    char buff[100];
    for (int j = 0; j < 16; j++) {
      dataIn[j] = read_EEPROM(j + i);
      delay(1);
      }
    sprintf(buff, "%03x:  %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x ",
      i, dataIn[0], dataIn[1], dataIn[2], dataIn[3], dataIn[4], dataIn[5], dataIn[6], dataIn[7], dataIn[8], dataIn[9], 
      dataIn[10], dataIn[11], dataIn[12], dataIn[13], dataIn[14], dataIn[15]);
    Serial.println(buff);
    }

  /*
  for (int i = 0; i < num; i++) {
    uint8_t dataIn = read_EEPROM(i);
    Serial.print(i, HEX);
    Serial.print("  ");
    Serial.println(dataIn, HEX);
    }
    */
  }

//program ucode into EEPROM chip
void write_ucode() {
  float prog_time = est_prog_time(F_STATES_NUM * INSTRUCTIONS_NUM * STEPS_NUM, PROG_DELAY);
  print_init_msg(prog_time);
  for (int flag = 0; flag < F_STATES_NUM; flag++) {
    for (int instruction = 0; instruction < INSTRUCTIONS_NUM; instruction++) {
      for (int ustep = 0; ustep < STEPS_NUM; ustep++) {
        uint16_t address = (flag << 11) | (instruction << 3) | ustep;
        if (MSBYTE == true) write_EEPROM(address, (ucode[flag][instruction][ustep] >> 8));
        else write_EEPROM(address, ucode[flag][instruction][ustep]);
        }
      }
    }
  }

void setup() {
  init_IO();
  init_ucode();

  if (CLEAR_BEFORE_PROG) clear_EEPROM();
   
  write_ucode();

  print_contents(EEPROM_MEM);
}

void loop() {
}
