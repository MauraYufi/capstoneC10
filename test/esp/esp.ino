#define BUTTON_PIN 4
#include <MPU6050_tockn.h>
#include <Wire.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <math.h>

#define CE_PIN   26
#define CSN_PIN 27

const byte slaveAddress[5] = {'R','x','A','A','A'};
MPU6050 mpu6050(Wire);
long timer = 0;
RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

float PembacaanTG=0;
int dataTrigger=1;
char txNum = '0';
float wx0;
float wy0;
float wz0;
float wx1;
float wy1;
float wz1;
float xn;
float ax;
float ay;
float az;
float a;
float h;


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
    wx0 = mpu6050.getAngleX();
    wy0 = mpu6050.getAngleY();
    wz0 = mpu6050.getAngleZ();
    Serial.println(wx0);
    Serial.println(wy0);
    Serial.println(wz0);
    
    delay(1000);
    mpu6050.update();
    wx1 = mpu6050.getAngleX();
    wy1 = mpu6050.getAngleY();
    wz1 = mpu6050.getAngleZ();
    Serial.println(wx1);
    Serial.println(wy1);
    Serial.println(wz1);
    ax = mpu6050.getAccX();
    ay = mpu6050.getAccY();
    az = mpu6050.getAccZ();
    a = sqrt((pow(ax/9.806,2)+pow(ay/9.806,2)+pow(az/9.806,2)));
    xn = sqrt((pow((wx1-wx0)*0.0175,2)+pow((wy1-wy0)*0.0175,2)+pow((wz1-wz0)*0.0175,2)));
    h = a/pow(xn,2);
    PembacaanTG = h;
    rslt = radio.write( &PembacaanTG, sizeof(PembacaanTG) );
        // Always use sizeof() as it gives the size as the number of bytes.
        // For example if dataToSend was an int sizeof() would correctly return 2

    Serial.println("Data Sent ");
    Serial.println(PembacaanTG);
    Serial.println(a);
    Serial.println(xn);
    // Serial.println(h);
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

