#define BUTTON_PIN 4
#include <MPU6050_tockn.h>
#include <Wire.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


#define CE_PIN   26
#define CSN_PIN 27

const byte slaveAddress[5] = {'R','x','A','A','A'};
MPU6050 mpu6050(Wire);
long timer = 0;
RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

float PembacaanTG=0;
int dataTrigger=1;
char txNum = '0';


unsigned long currentMillis;
unsigned long prevMillis;
unsigned long txIntervalMillis = 1000; // send once per second


void setup() {
  Serial.begin(9600);
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  //Serial.println("SimpleTx Starting");

  radio.begin();
  radio.setDataRate( RF24_250KBPS );
  //radio.setRetries(3,5); // delay, count
  radio.openReadingPipe(1, slaveAddress);
  radio.startListening();
  //radio.openWritingPipe(slaveAddress);
}

//====================

void loop() {
  mpu6050.update();
  getData();
  currentMillis = millis();
  if (currentMillis - prevMillis >= txIntervalMillis && dataTrigger==0) {  ///ketika ada dapat nilai 0 dari user, maka device laut jadi transmitter
    radio.stopListening();
    Serial.println("SimpleTx Starting");
    radio.setRetries(3,5); // delay, count
    radio.openWritingPipe(slaveAddress);
    send();
    
    Serial.print("angleX : ");Serial.print(mpu6050.getAngleX());
    Serial.print("\tangleY : ");Serial.print(mpu6050.getAngleY());
    Serial.print("\tangleZ : ");Serial.println(mpu6050.getAngleZ());
    Serial.print("\accX : ");Serial.print(mpu6050.getAccX());
    Serial.print("\taccY : ");Serial.print(mpu6050.getAccY());
    Serial.print("\taccZ : ");Serial.println(mpu6050.getAccZ());

    Serial.println("");
    prevMillis = millis();
    timer = millis();
    radio.openReadingPipe(1, slaveAddress);
    radio.startListening();
  }
  dataTrigger=1;
  Serial.println(dataTrigger);
  delay(500);
}

//====================

void send() {
    bool rslt;
    PembacaanTG = mpu6050.getAngleX();
    rslt = radio.write( &PembacaanTG, sizeof(PembacaanTG) );
        // Always use sizeof() as it gives the size as the number of bytes.
        // For example if dataToSend was an int sizeof() would correctly return 2

    Serial.print("Data Sent ");
    Serial.print(PembacaanTG);
    if (rslt) {
        Serial.println("  Acknowledge received");
    }
    else {
        Serial.println("  Tx failed");
    }
}

//================

void updateMessage() {
    PembacaanTG = mpu6050.getAngleX();
}


void getData() {
    if ( radio.available() ) {
        radio.read( &dataTrigger, sizeof(dataTrigger) );
        
    }
}

