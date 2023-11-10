//========================================================================= define library
#include <MPU6050_tockn.h>
#include <Wire.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <math.h>
#include <vector>

//========================================================================= Pin definitions
#define BUTTON_PIN 4
#define CE_PIN 26
#define CSN_PIN 27
#define LED_BUILTIN 2

//========================================================================= define function
void getData();
void getH();
float getHs();
void sends();

//========================================================================= Constants definitions
const byte slaveAddress[5] = {'R', 'x', 'A', 'A', 'A'};
unsigned long sendDuration = 20000;


//========================================================================= Variable definitions
long timer = 0;
float PembacaanTG = 0;
int dataTrigger = 1;
float wx0; float wy0; float wz0;
float wx1; float wy1; float wz1;
float xn;
float ax; float ay; float az;
float a;
float h;
int minimalMinute = 1;
unsigned long prevMinutes;
unsigned long minimalSecond = minimalMinute * 60;
unsigned long pointer_Hs = 0;
std::vector<float> Hs;
// waktu penghitungan Hs, untuk sekarang 1 menit.
unsigned long currentMillis;
unsigned long prevMillis;
unsigned long txIntervalMillis = 1000; // send once per second
unsigned long sendStartMillis = 0;

//========================================================================= Create 
MPU6050 mpu6050(Wire); //Create mpu6050
RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

//========================================================================= Setup
void setup()
{
    Serial.begin(9600);
    Serial.println("SimpleTx Starting");
    Wire.begin();
    mpu6050.begin();
    mpu6050.calcGyroOffsets(true);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    // Serial.println("SimpleTx Starting");

    radio.begin();
    radio.setDataRate(RF24_250KBPS);
    // radio.setRetries(3,5); // delay, count
    radio.openReadingPipe(1, slaveAddress);
    radio.startListening();
    // radio.openWritingPipe(slaveAddress);
    Serial.println(dataTrigger);
}

//========================================================================= Loop
void loop()
{
    mpu6050.update();
    getData();
    currentMillis = millis();
    if (currentMillis - prevMillis >= txIntervalMillis && dataTrigger == 0)
    { /// ketika ada dapat nilai 0 dari user, maka device laut jadi transmitter
        radio.stopListening();
        radio.setRetries(3, 5); // delay, count
        radio.openWritingPipe(slaveAddress);
        // Untuk menghitung tinggi gelombang saat ini
        getH();
        // Pointer untuk perhitungan Significant Wave
        pointer_Hs += 1;

        Serial.print("angleX : ");
        Serial.print(mpu6050.getAngleX());
        Serial.print("\tangleY : ");
        Serial.print(mpu6050.getAngleY());
        Serial.print("\tangleZ : ");
        Serial.println(mpu6050.getAngleZ());
        Serial.print("\accX : ");
        Serial.print(mpu6050.getAccX());
        Serial.print("\taccY : ");
        Serial.print(mpu6050.getAccY());
        Serial.print("\taccZ : ");
        Serial.println(mpu6050.getAccZ());

        Serial.println("");
        prevMillis = millis();
        timer = millis();
        radio.openReadingPipe(1, slaveAddress);
        radio.startListening();
    }
    // Jika pointer HS sudah menunjukkan menit ke
    if (pointer_Hs == minimalSecond)
    {
        // mengirim Hs
        sendStartMillis = millis();
        sends();
        pointer_Hs = 0;
        dataTrigger = 1;
    }
    
    Serial.println(dataTrigger);
    delay(500);
}

//========================================================================= getData trigger function
void getData()
{
    if (radio.available())
    {
        radio.read(&dataTrigger, sizeof(dataTrigger));
    }
}

//========================================================================= get waveheight function
void getH()
{
    Serial.println("wX0\twY0\twZ0");
    wx0 = mpu6050.getAngleX();
    wy0 = mpu6050.getAngleY();
    wz0 = mpu6050.getAngleZ();
    Serial.print(wx0);
    Serial.print("\t");
    Serial.print(wy0);
    Serial.print("\t");
    Serial.print(wz0);
    Serial.println("");

    delay(1000);
    Serial.println("wX1\twY1\twZ1");
    mpu6050.update();
    wx1 = mpu6050.getAngleX();
    wy1 = mpu6050.getAngleY();
    wz1 = mpu6050.getAngleZ();
    Serial.print(wx1);
    Serial.print("\t");
    Serial.print(wy1);
    Serial.print("\t");
    Serial.print(wz1);
    Serial.println("");

    ax = mpu6050.getAccX();
    ay = mpu6050.getAccY();
    az = mpu6050.getAccZ();

    a = sqrt((pow(ax / 9.806, 2) + pow(ay / 9.806, 2) + pow(az / 9.806, 2)));
    xn = sqrt((pow((wx1 - wx0) * 0.0175, 2) + pow((wy1 - wy0) * 0.0175, 2) + pow((wz1 - wz0) * 0.0175, 2)));
    h = a / pow(xn, 2);

    if (h>100){
      h = 0;
    }

    Hs.push_back(h);
}

//========================================================================= get significant wave height function
float getHs()
{
    // urutkan hs untuk mendapat nilai terbesar
    sort(Hs.begin(), Hs.end(), std::greater<float>());

    float Hs_akhir = 0;
    float totalH = minimalSecond / 3;

    for (int i = 0; i < totalH; i++)
    {
        Hs_akhir += Hs[i];
    }

    Hs_akhir /= totalH;

    return Hs_akhir;
}

//========================================================================= send significant wave height function
void sends()
{
    bool rslt;
    radio.stopListening();
    radio.openWritingPipe(slaveAddress);
    PembacaanTG = getHs();

    rslt = radio.write(&PembacaanTG, sizeof(PembacaanTG));
    // Always use sizeof() as it gives the size as the number of bytes.
    // For example if dataToSend was an int sizeof() would correctly return 2
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);

    Serial.println("Data Sent");
    Serial.print("Tinggi Hs\t: ");
    Serial.println(PembacaanTG);
    
    if (rslt)
    {
        Serial.println(" Acknowledge received");
        radio.openReadingPipe(1, slaveAddress);
        radio.startListening();
        Serial.println("R");
    }
    else
    {
        unsigned long currSendMillis = millis();

        if(currSendMillis - sendStartMillis < sendDuration){
          Serial.println(" Tx failed");
          sends();
        }
        else {
          radio.openReadingPipe(1, slaveAddress);
          radio.startListening();
        }
      
    }
}
