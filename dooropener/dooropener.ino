/*
 Name:		dooropener.ino
 Created:	01/10/2015 10:40:15
 Author:	ottor
*/

// the setup function runs once when you press reset or power the board
#include <SPI.h>        // RC522 Module uses SPI protocol
#include <MFRC522.h>  // Library for Mifare RC522 Devices
#include <EEPROM.h>  
#include <Servo.h>
int successRead;
boolean match = false;
byte readCard[4];
byte masterCard[4];
#define LED_ON HIGH
#define LED_OFF LOW
#define SS_PIN 10
#define RST_PIN 9
#define loadLED 4
#define okLed 2
#define deniedLed 7
Servo doorServo;
int angle = 90;

MFRC522 mfrc522(SS_PIN, RST_PIN);
void setup() {
	doorServo.attach(3);
	Serial.begin(9600);
	SPI.begin();
	mfrc522.PCD_Init();
	ShowReaderDetails();
	pinMode(loadLED, OUTPUT);
	pinMode(okLed, OUTPUT);
	pinMode(deniedLed, OUTPUT);

	if (EEPROM.read(1) != 143) {
		Serial.println(F("No Master Card Defined"));
		Serial.println(F("Scan A PICC to Define as Master Card"));
		do {
			successRead = getID();            // sets successRead to 1 when we get read from reader otherwise 0
			blinkLed();
		} while (!successRead);                  // Program will not go further while you not get a successful read
		for (int j = 0; j < 4; j++) {        // Loop 4 times
			EEPROM.write(2 + j, readCard[j]);  // Write scanned PICC's UID to EEPROM, start from address 3
		}
		EEPROM.write(1, 143);                  // Write to EEPROM we defined Master Card.
		Serial.println(F("Master Card Defined"));
	}
	Serial.println(F("-------------------"));
	Serial.println(F("Master Card's UID"));
	for (int i = 0; i < 4; i++) {          // Read Master Card's UID from EEPROM
		masterCard[i] = EEPROM.read(2 + i);    // Write it to masterCard
		Serial.print(masterCard[i], HEX);
	}
	blinkLed();
}

// the loop function runs over and over again until power down or reset

void loop() {

	successRead = getID();
	if (successRead)
	{
		Serial.println(F("Read success"));
		if (checkTwo(readCard, masterCard)) {
			Serial.println(F("Mastercard found!"));
			cardOkCycle();
		}
		else
		{
			glowDenied();
		}
	}


}
void openDoor()
{
	doorServo.write(165);
}
void closeDoor()
{
	doorServo.write(90);
}
int getID() {

	// Getting ready for Reading PICCs
	if (!mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
		return 0;
	}
	if (!mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
		return 0;
	}

	blinkLed();
	byte piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
	if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI
		&&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
		&&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
		Serial.println(F("This program only works with MIFARE Classic cards."));
		return 0;
	}
	// There are Mifare PICCs which have 4 byte or 7 byte UID care if you use 7 byte PICC
	// I think we should assume every PICC as they have 4 byte UID
	// Until we support 7 byte PICCs
	Serial.println(F("Scanned PICC's UID:"));
	for (int i = 0; i < 4; i++) {  //
		readCard[i] = mfrc522.uid.uidByte[i];
	}
	dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
	Serial.println("");
	mfrc522.PICC_HaltA(); // Stop reading
						  // Get data ready

	return 1;
}
void ShowReaderDetails() {
	// Get the MFRC522 software version
	byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
	Serial.print(F("MFRC522 Software Version: 0x"));
	Serial.print(v, HEX);
	if (v == 0x91)
		Serial.print(F(" = v1.0"));
	else if (v == 0x92)
		Serial.print(F(" = v2.0"));
	else
		Serial.print(F(" (unknown)"));
	Serial.println("");
	// When 0x00 or 0xFF is returned, communication probably failed
	if ((v == 0x00) || (v == 0xFF)) {
		Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
		while (true); // do not go further
	}

}
void blinkLed()
{
	digitalWrite(loadLED, LED_ON); 	// Make sure green LED is on
	delay(200);
	digitalWrite(loadLED, LED_OFF); 	// Make sure green LED is off
	delay(200);
	digitalWrite(loadLED, LED_ON); 	// Make sure green LED is on
	delay(200);
	digitalWrite(loadLED, LED_OFF); 	// Make sure green LED is off
	delay(200);
	digitalWrite(loadLED, LED_ON); 	// Make sure green LED is on
	delay(200);
	digitalWrite(loadLED, LED_OFF);
}
void cardOkCycle()
{
	digitalWrite(okLed, LED_ON);
	openDoor();
	delay(3000);
	digitalWrite(okLed, LED_OFF);
	closeDoor();
}
void glowDenied()
{
	digitalWrite(deniedLed, LED_ON);
	delay(1000);
	digitalWrite(deniedLed, LED_OFF);
}

boolean checkTwo(byte a[], byte b[]) {
	if (a[0] != NULL) 			// Make sure there is something in the array first
		match = true; 			// Assume they match at first
	for (int k = 0; k < 4; k++) { 	// Loop 4 times
		if (a[k] != b[k]) 		// IF a != b then set match = false, one fails, all fail
			match = false;
	}
	if (match) { 			// Check to see if if match is still true
		return true; 			// Return true
	}
	else {
		return false; 			// Return false
	}
}
boolean isMaster(byte test[]) {
	if (checkTwo(test, masterCard))
		return true;
	else
		return false;
}

void dump_byte_array(byte *buffer, byte bufferSize) {
	for (byte i = 0; i < bufferSize; i++) {
		Serial.print(buffer[i] < 0x10 ? " 0" : " ");
		Serial.print(buffer[i], HEX);
	}
}