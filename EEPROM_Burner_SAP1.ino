#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4
#define WRITE_EN 13
#define EEPROM_D0 5
#define EEPROM_D7 12
#define ERASE_VALUE 0x00

void eraseEEPROM() {
  Serial.print("Erasing EEPROM ");
  for (int address = 0; address <= 2047; ++address) {
    writeEEPROM(address, ERASE_VALUE);
    printStatus(address, 64);
  }

  Serial.println(" done");
  Serial.println("EEPROM erased.");
}

void printStatus(int address, int value) {
  if (address % value == 0)
    Serial.print(".");
}

void printData() {
  Serial.println("EEPROM Contents");

  for (int baseAddress = 0; baseAddress <= 255; baseAddress += 16) {

    byte data[16];

    for (int offset = 0; offset <= 15; ++offset)
      data[offset] = readEEPROM(baseAddress + offset);

    char buffer[80];
    sprintf(buffer, "0x%011x: % 02x % 02x % 02x % 02x % 02x % 02x % 02x % 02x     % 02x % 02x % 02x % 02x % 02x % 02x % 02x % 02x",
            baseAddress, data[0],  data[1], data[2], data[3], data[4], data[5], data[6],  data[7], data[8],
            data[9], data[10], data[11], data[12], data[13], data[14], data[15]);

    Serial.println(buffer);
  }
}

void setAddress(int address, bool outputEnable) {
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 8) | (outputEnable ? 0x00 : 0x80));
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address);

  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
}

byte readEEPROM(int address) {

  for (int pin = EEPROM_D0; pin <= EEPROM_D7; ++pin)
    pinMode(pin, INPUT);

  setAddress(address, true);

  // Build and create a byte of data.  Iterate I/O pins and shift data into byte.
  byte data = 0;

  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin--)
    data = (data << 1) + digitalRead(pin);

  return data;
}

void writeEEPROM(int address, byte data) {
  setAddress(address, false);

  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin++)
    pinMode(pin, OUTPUT);

  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin++) {
    digitalWrite(pin, data & 1);
    data = data >> 1;
  }

  digitalWrite(WRITE_EN, LOW);
  delayMicroseconds(1);
  digitalWrite(WRITE_EN, HIGH);
  delay(10);
}

void setup() {
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);

  digitalWrite(WRITE_EN, HIGH);
  pinMode(WRITE_EN, OUTPUT);

  Serial.begin(57600);
  while (!Serial);
  Serial.println();

  eraseEEPROM();

  Serial.println("");
  Serial.print("Programming EEPROM...  ");

  // 4-bit hex decoder for common anode 7-segment display
  byte data[] = { 0x81, 0xcf, 0x92, 0x86, 0xcc, 0xa4, 0xa0, 0x8f, 0x80 };

  // Common Cathode
  // byte data[] = { 0x7e, 0x30, 0x6d, 0x79, 0x33, 0x5b, 0x5f, 0x70, 0x7f, 0x7b };

  for (int x = 0; x <= 255; ++x) {
    writeEEPROM(x, (data[x % 10])); // Ones places.
    writeEEPROM((x + 256), (data[(x / 10) % 10])); // 10's place.
    writeEEPROM((x + 512), (data[(x / 100) % 10])); // 100's place.
    writeEEPROM((x + 768), 0); // Signed place (4th/MSB 7 segment display).  Fill with zeros for unsigned ints.
  }

  for (int x = -128; x <= 127; ++x) {
    writeEEPROM((byte)x + 1024, data[abs(x) % 10]);
    writeEEPROM((byte)x + 1280, data[abs(x / 10) % 10]);
    writeEEPROM((byte)x + 1536, data[abs(x / 100) % 10]);

    if (x < 0)
      writeEEPROM((byte)x + 1792, 0x01);
    else
      writeEEPROM((byte)x + 1792, 0);
  }

  Serial.println(" done");
  Serial.println("Upload complete. ");

  printData();
}

void loop() {
}
