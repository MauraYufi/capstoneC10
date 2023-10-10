// define library
#include <Fuzzy.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h> //library I2C
#include <LiquidCrystal_I2C.h>    //library LCD

void sendTrigger();

Fuzzy *fuzzy = new Fuzzy();
//=========== define for transmitter reciever radio
#define CE_PIN    7
#define CSN_PIN   8

const byte thisSlaveAddress[5] = {'R','x','A','A','A'};
const int btnPin = 4;

RF24 radio(CE_PIN, CSN_PIN);

float PembacaanTG; // this must match dataToSend in the TX
bool newData = false;
int dataTrigger=0;
char rate[6]="";
float output;
//===========

//=========== define for fuzzy logic


// Fuzzy input gelombang
FuzzySet *low = new FuzzySet(0, 0, 0.5 , 2.5);
FuzzySet *medium = new FuzzySet(0.5, 2.5, 2.5, 4);
FuzzySet *high = new FuzzySet(2.5, 4, 14, 14);
  
// Fuzzy input angin
FuzzySet *slow = new FuzzySet(0, 0, 12, 20);
FuzzySet *moderate = new FuzzySet(12, 20, 28, 38);
FuzzySet *strong = new FuzzySet(28, 38, 64, 64);
  
  // Fuzzy output angin
FuzzySet *safe = new FuzzySet(0, 0, 0, 0.5);
FuzzySet *normal = new FuzzySet(0, 0.5, 0.5, 1);
FuzzySet *danger = new FuzzySet(0.5, 1, 1, 1);
//===========

LiquidCrystal_I2C lcd(0x27,16,2); // set address I2C dan besar karakter untuk lcd 16×2
int button = 5;
int count = 0;

//===========

void setup() {

    Serial.begin(9600);

    Serial.println("SimpleRx Starting");
    radio.begin();
    radio.setDataRate( RF24_250KBPS );

    //set as transmitter
    radio.openWritingPipe(thisSlaveAddress);

    // pin start sistem
    pinMode(btnPin,INPUT_PULLUP);
    
    // pin next lcd
    lcd.init(); // inisiasi LCD
    pinMode(button,INPUT_PULLUP);
    lcd.backlight();

    //fuzzyinput
    FuzzyInput *wave = new FuzzyInput(1);
    wave->addFuzzySet(low);
    wave->addFuzzySet(medium);
    wave->addFuzzySet(high);
    fuzzy->addFuzzyInput(wave);
  
    //fuzzyinput
    FuzzyInput *wind = new FuzzyInput(2);
    wind->addFuzzySet(slow);
    wind->addFuzzySet(moderate);
    wind->addFuzzySet(strong);
    fuzzy->addFuzzyInput(wind);
  
    //fuzzyotput
    FuzzyOutput *rate = new FuzzyOutput(1);
    rate->addFuzzySet(safe);
    rate->addFuzzySet(normal);
    rate->addFuzzySet(danger);
    fuzzy->addFuzzyOutput(rate);
  
    //1
    FuzzyRuleAntecedent *low_slow = new FuzzyRuleAntecedent();
    low_slow->joinWithOR(low,slow);
  
    FuzzyRuleConsequent *rate_safe = new FuzzyRuleConsequent();
    rate_safe->addOutput(safe);
  
    FuzzyRule *fuzzyRule01 = new FuzzyRule(1, low_slow, rate_safe);
    fuzzy->addFuzzyRule(fuzzyRule01);
  
    //2
    FuzzyRuleAntecedent *medium_moderate = new FuzzyRuleAntecedent();
    medium_moderate->joinWithAND(medium,moderate);
  
    FuzzyRuleConsequent *rate_normal = new FuzzyRuleConsequent();
    rate_normal->addOutput(normal);
  
    FuzzyRule *fuzzyRule02 = new FuzzyRule(2, medium_moderate, rate_normal);
    fuzzy->addFuzzyRule(fuzzyRule02);
  
    //3
    FuzzyRuleAntecedent *high_strong = new FuzzyRuleAntecedent();
    high_strong->joinWithOR(high,strong);
  
    FuzzyRuleConsequent *rate_danger = new FuzzyRuleConsequent();
    rate_danger->addOutput(danger);
  
    FuzzyRule *fuzzyRule03 = new FuzzyRule(3, high_strong, rate_danger);
    fuzzy->addFuzzyRule(fuzzyRule03);
}

//=============

void loop() 
{
  // get wave height
  if(digitalRead(btnPin)==0) {
    sendTrigger();
    
    //set kembali menjadi transmitter
    radio.openWritingPipe(thisSlaveAddress);
  }
  // fuzzi logic
  float in_wave = PembacaanTG;
  float in_wind = 26.3; // nanti dari anemo

  fuzzy->setInput(1, in_wave);
  fuzzy->setInput(2, in_wind);
  fuzzy->fuzzify();
  
  float opt = fuzzy->defuzzify(1);

  float output = opt * 100;

  if(output < 50){
    char rate[6]="Aman";
  }
  else {
    char rate[6]="Bahaya";
  }

  
}

//==============


void getData() {
  int xyz=0;
  while (xyz==0){
    if ( radio.available() ) {
        radio.read( &PembacaanTG, sizeof(PembacaanTG) );
        newData = true;
        xyz=1;
        lcd.clear();
    }
    else
    {
      lcd.setCursor(5,0);
      lcd.print("Mohon");
      lcd.setCursor(4,1);
      lcd.print("Tunggu");
      //delay(100);
    } 
  }
    
}

void showData() {
  while(digitalRead(btnPin)==1){
    if (newData == true) {
        Serial.print("Data received ");
        Serial.println(PembacaanTG);
        if(digitalRead(button)==0){
          delay(100);
          count++;
          lcd.clear();
          Serial.print(digitalRead(button));
        }
      
        if(count%4 == 0){
          lcd.setCursor(6,0);
          lcd.print(rate);
          lcd.setCursor(4,1);
          lcd.print("Berlayar");
          delay(100);
          Serial.print(count);
        }
      
        else if(count%4 == 1){
          lcd.setCursor(3,0);
          lcd.print("Kec. Angin");
          lcd.setCursor(5,1);
          lcd.print("26.3 km/h");
          delay(100);
          Serial.print(count);
        }
      
        else if(count%4 == 2){
          lcd.setCursor(0,0);
          lcd.print("Tinggi Gelombang");
          lcd.setCursor(6,1);
          lcd.print(PembacaanTG);  
          delay(100);
          Serial.print(count);
        }
      
        else if(count%4 == 3){
          lcd.setCursor(1,0);
          lcd.print("Tingkat Bahaya");
          lcd.setCursor(6,1);
          lcd.print(output);  
          delay(100);
          Serial.print(count);
        }
    }
  }
  newData = false;
}

void sendTrigger(){
    bool rslt;
    rslt = radio.write( &dataTrigger, sizeof(dataTrigger) );
    Serial.print("Data Sent ");
    Serial.print(dataTrigger);
    if (rslt) {
        Serial.println("  Acknowledge received");
        setReciever();
    }
    else {
        Serial.println("  Tx failed");
    }
}

void setReciever() {
    radio.openReadingPipe(1, thisSlaveAddress);
    radio.startListening();
    getData();
    showData();
    //delay(3000);
    radio.stopListening();
}
