#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define BTN_ON    3
#define BTN_OFF   4
#define BTN_TIMER 5

RF24 radio(9, 10);

// ✅ Address
const uint64_t address = 0xE6E6E6E6E6E6;

int32_t sendData = 0;

// ✅ UART BUFFER
char rxBuf[20];
byte index = 0;

void setup() {
  Serial.begin(115200);

  pinMode(BTN_ON, INPUT_PULLUP);
  pinMode(BTN_OFF, INPUT_PULLUP);
  pinMode(BTN_TIMER, INPUT_PULLUP);

  radio.begin();
  radio.setChannel(108);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.setRetries(5, 15);

  radio.openWritingPipe(address);
  radio.stopListening();

  Serial.println("MASTER READY");
}

void loop() {

  // ✅ BUTTON CONTROL
  if (digitalRead(BTN_ON) == LOW) {
    sendData = 1;
    sendCommand();
    delay(300);
  }

  if (digitalRead(BTN_OFF) == LOW) {
    sendData = 2;
    sendCommand();
    delay(300);
  }

  if (digitalRead(BTN_TIMER) == LOW) {
    sendData = 3;
    sendCommand();
    delay(300);
  }

  // ✅ UART CONTROL
  readSerialCommand();
}

// ================= UART PARSER =================
void readSerialCommand()
{
  while (Serial.available()) {

    char c = Serial.read();

    // Store char
    if (index < sizeof(rxBuf) - 1) {
      rxBuf[index++] = c;
      rxBuf[index] = '\0';
    }

    // ✅ COMMAND DETECTION
    if (strstr(rxBuf, "|C|6|")) {        // ON
      sendData = 1;
      sendCommand();
      clearBuffer();
    }

    else if (strstr(rxBuf, "|C|5|")) {   // OFF
      sendData = 2;
      sendCommand();
      clearBuffer();
    }

    else if (strstr(rxBuf, "|C|4|")) {   // TIMER (ignore extra like 12)
      sendData = 3;
      sendCommand();
      clearBuffer();
    }

    // ✅ Prevent overflow
    if (index >= sizeof(rxBuf) - 1) {
      clearBuffer();
    }
  }
}

void clearBuffer() {
  index = 0;
  rxBuf[0] = '\0';
}

// ================= SEND =================
void sendCommand() {

  bool ok = radio.write(&sendData, sizeof(sendData));

  Serial.print("Send: ");
  Serial.print(sendData);
  Serial.println(ok ? " OK" : " FAIL");
}