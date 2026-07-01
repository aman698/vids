#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// ✅ Addresses
const uint64_t RX_ADDRESS = 0xE6E6E6E6E6E6;
const uint64_t TX_ADDRESS = 0xB3B4B5B6B7B8;

RF24 radio(9, 10);

#define RELAY1_PIN 5
#define RELAY2_PIN 6

int32_t dataBuffer = 0;

// ✅ UART buffer
char rxBuf[20];
byte index = 0;

// ✅ Blink
bool blinkActive = false;
bool relayState = false;
unsigned long blinkStart = 0;
unsigned long lastBlink = 0;

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);

  digitalWrite(RELAY1_PIN, LOW);
  digitalWrite(RELAY2_PIN, LOW);

  radio.begin();
  radio.setChannel(108);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.setRetries(5, 15);

  radio.openReadingPipe(0, RX_ADDRESS);
  radio.startListening();

  Serial.println("SMART NODE READY");
}

// ================= LOOP =================
void loop() {

  // ✅ RF RECEIVE
  if (radio.available()) {

    radio.read(&dataBuffer, sizeof(dataBuffer));

    if (dataBuffer >= 1 && dataBuffer <= 3) {

      Serial.print("RF CMD: ");
      Serial.print(dataBuffer);
      Serial.print(" -> ");

      handleCommand(dataBuffer);
      forwardData(dataBuffer);
    }
    else {
      Serial.println("RF Noise Ignored");
    }
  }

  // ✅ UART COMMAND
  readSerialCommand();

  // ✅ Blink Handling
  blinkHandler();
}

// ================= UART =================
void readSerialCommand()
{
  while (Serial.available()) {

    char c = Serial.read();

    if (index < sizeof(rxBuf) - 1) {
      rxBuf[index++] = c;
      rxBuf[index] = '\0';
    }

    if (strstr(rxBuf, "|C|6|")) {
      Serial.print("UART CMD: 1 -> ");
      handleCommand(1);
      forwardData(1);
      clearBuf();
    }

    else if (strstr(rxBuf, "|C|5|")) {
      Serial.print("UART CMD: 2 -> ");
      handleCommand(2);
      forwardData(2);
      clearBuf();
    }

    else if (strstr(rxBuf, "|C|4|")) {
      Serial.print("UART CMD: 3 -> ");
      handleCommand(3);
      forwardData(3);
      clearBuf();
    }

    if (index >= sizeof(rxBuf) - 1) clearBuf();
  }
}

void clearBuf() {
  index = 0;
  rxBuf[0] = '\0';
}

// ================= FORWARD =================
void forwardData(int32_t cmd) {

  radio.stopListening();

  radio.openWritingPipe(TX_ADDRESS);

  bool ok = radio.write(&cmd, sizeof(cmd));

  Serial.print(" | Forward: ");
  Serial.println(ok ? "OK" : "FAIL");

  radio.startListening();
}

// ================= COMMAND =================
void handleCommand(int cmd) {

  if (cmd == 1) {
    blinkActive = false;

    digitalWrite(RELAY1_PIN, HIGH);
    digitalWrite(RELAY2_PIN, HIGH);

    Serial.println("Relay ON");
  }

  else if (cmd == 2) {
    blinkActive = false;

    digitalWrite(RELAY1_PIN, LOW);
    digitalWrite(RELAY2_PIN, LOW);

    Serial.println("Relay OFF");
  }

  else if (cmd == 3) {
    blinkActive = true;

    blinkStart = millis();
    lastBlink = millis();
    relayState = true;

    digitalWrite(RELAY1_PIN, HIGH);
    digitalWrite(RELAY2_PIN, HIGH);

    Serial.println("Blink START (12 sec)");
  }
}

// ================= BLINK =================
void blinkHandler() {

  if (blinkActive) {

    if (millis() - lastBlink >= 1000) {

      lastBlink = millis();

      relayState = !relayState;

      digitalWrite(RELAY1_PIN, relayState);
      digitalWrite(RELAY2_PIN, relayState);

      Serial.print("Blink: ");
      Serial.println(relayState ? "ON" : "OFF");
    }

    if (millis() - blinkStart >= 12000) {

      blinkActive = false;

      digitalWrite(RELAY1_PIN, LOW);
      digitalWrite(RELAY2_PIN, LOW);

      Serial.println("Blink COMPLETE -> Relay OFF");
    }
  }
}