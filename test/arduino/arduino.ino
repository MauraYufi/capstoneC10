// SimpleRx - the slave or the receiver

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN   7
#define CSN_PIN 8

const byte thisSlaveAddress[5] = {'R','x','A','A','A'};
const int btnPin = 4;

RF24 radio(CE_PIN, CSN_PIN);

float PembacaanTG; // this must match dataToSend in the TX
bool newData = false;
int dataTrigger=0;

//===========

void setup() {

    Serial.begin(9600);

    Serial.println("SimpleRx Starting");
    radio.begin();
    radio.setDataRate( RF24_250KBPS );
    //radio.openReadingPipe(1, thisSlaveAddress);
    radio.openWritingPipe(thisSlaveAddress);
    //radio.startListening();
    pinMode(btnPin,INPUT_PULLUP);
}

//=============

void loop() {
 // Serial.print(digitalRead(btnPin));
  if(digitalRead(btnPin)==0) {
    bool rslt;
    rslt = radio.write( &dataTrigger, sizeof(dataTrigger) );
        // Always use sizeof() as it gives the size as the number of bytes.
        // For example if dataToSend was an int sizeof() would correctly return 2

    Serial.print("Data Sent ");
    Serial.print(dataTrigger);
    if (rslt) {
        Serial.println("  Acknowledge received");
        radio.openReadingPipe(1, thisSlaveAddress);
        radio.startListening();
        getData();
        showData();
        delay(3000);
        radio.stopListening();
    }
    else {
        Serial.println("  Tx failed");
    }
    radio.openWritingPipe(thisSlaveAddress);
  }
}

//==============


void getData() {
    if ( radio.available() ) {
        radio.read( &PembacaanTG, sizeof(PembacaanTG) );
        newData = true;
    }
}

void showData() {
    if (newData == true) {
        Serial.print("Data received ");
        Serial.println(PembacaanTG);
        newData = false;
    }
}
