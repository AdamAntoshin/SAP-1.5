#define SHIFT_DATA_PIN 2
#define SHIFT_LATCH_PIN 3
#define SHIFT_CLOCK_PIN 4
#define OUTPUT_ENABLE_PIN A0
#define EEPROM_D0 5
#define EEPROM_D7 12
#define WRITE_ENABLE_PIN 13
#define PROG_DELAY 15

byte sevseg[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f};

void setup() {
  Serial.begin(9600);
  pinMode(SHIFT_DATA_PIN, OUTPUT);
  pinMode(SHIFT_LATCH_PIN, OUTPUT);
  pinMode(SHIFT_CLOCK_PIN, OUTPUT);
  pinMode(OUTPUT_ENABLE_PIN, OUTPUT);
  digitalWrite(WRITE_ENABLE_PIN, HIGH);
  pinMode(WRITE_ENABLE_PIN, OUTPUT);

  const int bytes_programmed = 2048;
  print_init_msg(bytes_programmed, PROG_DELAY);
  
  for (int val = 0; val <= 255; val++) {
    //ones place
    writeEEPROM(val, sevseg[val % 10]); 

    //tens place
    if (val < 10) writeEEPROM(val + 256, 0);  
    else writeEEPROM(val + 256, sevseg[(val / 10) % 10]);

    //hundreds place
    if (val < 100) writeEEPROM(val + 512, 0);
    else writeEEPROM(val + 512, sevseg[val/100]);

    //thousands place
    writeEEPROM(val + 768, 0);
    }

  //two's complement
  for (int val = -128; val <= 127; val++) {
    //ones place
    writeEEPROM((byte)val + 1024, sevseg[abs(val) % 10]); 

    //tens place
    if (abs(val) < 10) writeEEPROM((byte)val + 1280, 0);  
    else writeEEPROM((byte)val + 1280, sevseg[(abs(val / 10)) % 10]);

    //hundreds place
    if (abs(val) < 100) writeEEPROM((byte)val + 1536, 0);
    else writeEEPROM((byte)val + 1536, sevseg[abs(val/100)]);

    //thousands place
    if (val < 0) writeEEPROM((byte)val + 1792, 0x40);
    else writeEEPROM((byte)val + 1792, 0);
    }
    

  /*
  for (int i = 0; i < 10; i++)
    writeEEPROM(i, sevseg[i]);
    //writeEEPROM(i, 0x55);
  
  for (int i = 10; i <= 255; i++) 
    writeEEPROM(i, 0xff);
    */
  Serial.println("Programming complete!\n");
  delay(100);

  printContents(2048);
  
}

void loop() {
  // put your main code here, to run repeatedly:

}

void print_init_msg(int num, int del) {
  float est = num * del / 1000.0;
  Serial.print("Estimated programming time: "); Serial.print(est); Serial.println(" seconds\n");
  Serial.println("PROGRAMMING...\n");
  }

void writeEEPROM(int address, byte data) {
  setAddress(address, false);
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

void printContents(int num) {
  Serial.print("Reading first "); Serial.print(num); Serial.println(" bytes...\n");
  for (int i = 0; i <= num - 1; i += 16) {
    byte dataIn[16];
    char buff[100];
    for (int j = 0; j < 16; j++) {
      dataIn[j] = readEEPROM(j + i);
      delay(1);
      }
    sprintf(buff, "%03x:  %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x ",
      i, dataIn[0], dataIn[1], dataIn[2], dataIn[3], dataIn[4], dataIn[5], dataIn[6], dataIn[7], dataIn[8], dataIn[9], 
      dataIn[10], dataIn[11], dataIn[12], dataIn[13], dataIn[14], dataIn[15]);
    Serial.println(buff);
    }
  }
  
byte readEEPROM(int address) {
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin++) pinMode(pin, INPUT);
  
  setAddress(address, true);
  
  byte data = 0;
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin-=1) {
    data = (data << 1) + digitalRead(pin);
    }
  return data;
  }
  
void setAddress(int address, bool output) {
  digitalWrite(OUTPUT_ENABLE_PIN, !output);
  shiftOut(SHIFT_DATA_PIN, SHIFT_CLOCK_PIN, MSBFIRST, (address >> 8));
  shiftOut(SHIFT_DATA_PIN, SHIFT_CLOCK_PIN, MSBFIRST, address);
  digitalWrite(SHIFT_LATCH_PIN, LOW);
  digitalWrite(SHIFT_LATCH_PIN, HIGH);
  digitalWrite(SHIFT_LATCH_PIN, LOW);
  }
