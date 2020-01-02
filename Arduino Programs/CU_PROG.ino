/*
Initialize and program microcode into the EEPROMs in the control unit.
*/

//Program most significant byte of control words or least significant byte
#define MSBYTE true

//Define I/O and programming properties
#define SERIAL_BAUD_RATE 115200
#define SHIFT_DATA_PIN 2
#define SHIFT_LATCH_PIN 3
#define SHIFT_CLOCK_PIN 4
#define OUTPUT_ENABLE_PIN A0
#define EEPROM_D0 5
#define EEPROM_D7 12
#define WRITE_ENABLE_PIN 13
#define PROG_DELAY 20
#define EEPROM_MEM 8192

//Define uC properties
#define STEPS_NUM 8
#define INSTRUCTIONS_NUM 15
#define F_STATES_NUM 4
#define F_C0Z0 0
#define F_C0Z1 1
#define F_C1Z0 2
#define F_C1Z1 3
#define JZ 0b00001100
#define JC 0b00001101

//Define control signals for uC
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

//uC template (without conditional jumps) stored in flash to save memory
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

//Final uC array
uint16_t ucode[F_STATES_NUM][INSTRUCTIONS_NUM][STEPS_NUM];

//Initialize I/O (serial comm, pin modes...)
void init_IO() {
  Serial.begin(SERIAL_BAUD_RATE);
  pinMode(SHIFT_DATA_PIN, OUTPUT);
  pinMode(SHIFT_LATCH_PIN, OUTPUT);
  pinMode(SHIFT_CLOCK_PIN, OUTPUT);
  pinMode(OUTPUT_ENABLE_PIN, OUTPUT);
  digitalWrite(WRITE_ENABLE_PIN, HIGH);
  pinMode(WRITE_ENABLE_PIN, OUTPUT);
  }

//Calculate EEPROM programming time and print message
void print_init_msg(uint16_t num, uint16_t del) {
  float est = del / 1000.0;
  est *= num;
  
  Serial.print("Estimated programming time: "); Serial.print(est); Serial.println(" seconds\n");
  
  if (MSBYTE == true) Serial.println("PROGRAMMING MOST SIGNIFICANT BYTE OF CONTROL WORDS...");
  else Serial.println("PROGRAMMING LEAST SIGNIFICANT BYTE OF CONTROL WORDS...");
  }

//calculate EEPROM erasing time and print message
void print_clear_msg(uint16_t del) {
  float est = del / 1000.0;
  est *= EEPROM_MEM;
  
  Serial.print("Estimated erasing time: "); Serial.print(est); Serial.println(" seconds\n");
  Serial.println("ERASING...\n");
  }

//Take uC template, modify for different flag states and put in final array
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

//Set EEPROM address to point to and output enable pin state (for either programming or reading)
void set_address(uint16_t address, bool output) {
  digitalWrite(OUTPUT_ENABLE_PIN, !output);
  
  shiftOut(SHIFT_DATA_PIN, SHIFT_CLOCK_PIN, MSBFIRST, (address >> 8));
  shiftOut(SHIFT_DATA_PIN, SHIFT_CLOCK_PIN, MSBFIRST, address);
  
  digitalWrite(SHIFT_LATCH_PIN, LOW);
  digitalWrite(SHIFT_LATCH_PIN, HIGH);
  digitalWrite(SHIFT_LATCH_PIN, LOW);
  }

//Write a byte of data into EEPROM
void write_EEPROM(uint16_t address, uint8_t data) {
  Serial.print(address, HEX);
  Serial.print("  ");
  Serial.println(data, HEX);
  
  set_address(address, false);
  
  for (uint8_t pin = EEPROM_D0; pin <= EEPROM_D7; pin++) {
    digitalWrite(pin, data & 0x01);
    pinMode(pin, OUTPUT);
    data = data >> 1;
    }
    
  digitalWrite(WRITE_ENABLE_PIN, LOW);
  delayMicroseconds(1);
  digitalWrite(WRITE_ENABLE_PIN, HIGH);
  delay(PROG_DELAY);
  }

//Read a byte of data from EEPROM
uint8_t read_EEPROM(uint16_t address) {
  for (uint8_t pin = EEPROM_D0; pin <= EEPROM_D7; pin++) pinMode(pin, INPUT);
  
  set_address(address, true);
  
  uint8_t data = 0;
  
  for (uint8_t pin = EEPROM_D7; pin >= EEPROM_D0; pin-=1) 
    data = (data << 1) + digitalRead(pin);
    
  return data;
  }

//Erase EEPROM contents
void clear_EEPROM() {
  print_clear_msg(PROG_DELAY);
  
  for (uint16_t address = 0; address < EEPROM_MEM; address++) {
    write_EEPROM(address, 0);
    Serial.println(address);
    }
  }

//Print first [num] bytes stored in EEPROM
void print_contents(uint16_t num) {
  Serial.print("Reading first "); Serial.print(num); Serial.println(" bytes...\n");
  /*
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
    */
  for (uint16_t i = 0; i < num; i++) {
    uint8_t dataIn = read_EEPROM(i);
    Serial.print(i, HEX);
    Serial.print("  ");
    Serial.println(dataIn, HEX);
    }
  }

//Write uC into EEPROM
void write_ucode() {
  print_init_msg(F_STATES_NUM * INSTRUCTIONS_NUM * STEPS_NUM, PROG_DELAY);
  
  for (uint16_t flag = 0; flag < F_STATES_NUM; flag++) {
    for (uint16_t instruction = 0; instruction < INSTRUCTIONS_NUM; instruction++) {
      for (uint16_t ustep = 0; ustep < STEPS_NUM; ustep++) {
        
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

  //clear_EEPROM(); 
  //write_ucode();

  print_contents(EEPROM_MEM);
  //Serial.println(read_EEPROM(98), HEX);
}

void loop() {
  // put your main code here, to run repeatedly:

}
