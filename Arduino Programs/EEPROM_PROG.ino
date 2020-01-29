/*
 * 8-bit computer project (SAP-1.5)
 * -------------------------------------
 * 
 * Program microcode on EEPROM chips for display module.
 * 
 * TODO: incorporate SPI for writing purposes
 * 
 * -Adam Antoshin 2020
*/

//wipe EEPROM memory before programming
#define CLEAR_BEFORE_PROG true

//IO parameters
#define SERIAL_BAUD_RATE 115200
#define SHIFT_DATA_PIN 2
#define SHIFT_LATCH_PIN 3
#define SHIFT_CLOCK_PIN 4
#define OUTPUT_ENABLE_PIN A0
#define EEPROM_D0 5
#define EEPROM_D7 12
#define WRITE_ENABLE_PIN 13

//programming parameters
#define PROG_DELAY 15
#define EEPROM_MEM 8192

//7 segment hex table (CC)
byte sevseg[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f};

//initialize Arduino IO
void init_IO() {
  Serial.begin(SERIAL_BAUD_RATE);
  pinMode(SHIFT_DATA_PIN, OUTPUT);
  pinMode(SHIFT_LATCH_PIN, OUTPUT);
  pinMode(SHIFT_CLOCK_PIN, OUTPUT);
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
  Serial.println("PROGRAMMING...\n");
  }

//print erasing message
void print_clear_msg(float c_time) {
  Serial.print("Estimated erasing time: "); Serial.print(c_time); Serial.println(" seconds\n");
  Serial.println("ERASING...\n");
  }

//output address with 595 shift registers and either output or input contents
void set_address(int address, bool output) {
  digitalWrite(OUTPUT_ENABLE_PIN, !output);
  shiftOut(SHIFT_DATA_PIN, SHIFT_CLOCK_PIN, MSBFIRST, (address >> 8));
  shiftOut(SHIFT_DATA_PIN, SHIFT_CLOCK_PIN, MSBFIRST, address);
  digitalWrite(SHIFT_LATCH_PIN, LOW);
  digitalWrite(SHIFT_LATCH_PIN, HIGH);
  digitalWrite(SHIFT_LATCH_PIN, LOW);
  }

//write byte to EEPROM memory
void write_EEPROM(int address, byte data) {
  set_address(address, false);
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin++) {
    digitalWrite(pin, data & 0x01);
    pinMode(pin, OUTPUT);
    data = data >> 1;
    }
  digitalWrite(WRITE_ENABLE_PIN, LOW);
  delayMicroseconds(1);
  digitalWrite(WRITE_ENABLE_PIN, HIGH);
  delay(PROG_DELAY);
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

//read byte from EEPROM memory
byte read_EEPROM(int address) {
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin++) pinMode(pin, INPUT);
  
  set_address(address, true);
  
  byte data = 0;
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin-=1) {
    data = (data << 1) + digitalRead(pin);
    }
  return data;
  }

//print EEPROM contents
void print_contents(int num) {
  Serial.print("Reading first "); Serial.print(num); Serial.println(" bytes...\n");
  for (int i = 0; i <= num - 1; i += 16) {
    byte dataIn[16];
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
  }

//program 7seg values into EEPROM
void write_7seg_code() {
  const int bytes_programmed = 2048;

  float prog_time = est_prog_time(bytes_programmed, PROG_DELAY);
  print_init_msg(prog_time);
  
  for (int val = 0; val <= 255; val++) {
    //ones place
    write_EEPROM(val, sevseg[val % 10]); 

    //tens place
    if (val < 10) write_EEPROM(val + 256, 0);  
    else write_EEPROM(val + 256, sevseg[(val / 10) % 10]);

    //hundreds place
    if (val < 100) write_EEPROM(val + 512, 0);
    else write_EEPROM(val + 512, sevseg[val/100]);

    //thousands place
    write_EEPROM(val + 768, 0);
    }

  //two's complement
  for (int val = -128; val <= 127; val++) {
    //ones place
    write_EEPROM((byte)val + 1024, sevseg[abs(val) % 10]); 

    //tens place
    if (abs(val) < 10) write_EEPROM((byte)val + 1280, 0);  
    else write_EEPROM((byte)val + 1280, sevseg[(abs(val / 10)) % 10]);

    //hundreds place
    if (abs(val) < 100) write_EEPROM((byte)val + 1536, 0);
    else write_EEPROM((byte)val + 1536, sevseg[abs(val/100)]);

    //thousands place
    if (val < 0) write_EEPROM((byte)val + 1792, 0x40);
    else write_EEPROM((byte)val + 1792, 0);
    }
   
  Serial.println("Programming complete!\n");
  }
  
void setup() {
  init_IO();

  if (CLEAR_BEFORE_PROG) clear_EEPROM();
  
  write_7seg_code();
  
  print_contents(2048); 
}

void loop() {
}
