//========================================================================= define library
#include <SPI.h>
#include <nRF24L01.h> //library nrfl01
#include <RF24.h> //library nrf24l01
#include <Wire.h> //library I2C
#include <LiquidCrystal_I2C.h>    //library LCD
#include <Fuzzy.h>

//========================================================================= Pin definitions
#define windPin 2 // Receive the data from sensor
#define CE_PIN    7
#define CSN_PIN   8

//========================================================================= define function
void start();
void setReciever();
void getData();
float fuzzycalc();
void showData();
void windvelocity();
void RPMcalc();
void WindSpeed();
void addcount();

//========================================================================= Constants definitions
const float pi = 3.14159265;  // pi number
int period = 10000;           // Measurement period (miliseconds)
int delaytime = 10000;        // Time between samples (miliseconds)
int an_radio = 90;            // Distance from center windmill to outer cup (mm)
int jml_celah = 18;           // jumlah celah sensor
const int btnPin = 4;         //for button start
int button = 5;               //for button next lcd
const byte thisSlaveAddress[5] = {'R','x','A','A','A'}; //address radio
unsigned long triggerDuration = 10000;
unsigned long getDataMinute = 2;
unsigned long getDataDuration = getDataMinute * 60000 + 20000;

//========================================================================= Variable definitions
unsigned int Sample = 0;      // Sample number
unsigned int counter = 0;     // B/W counter for sensor
unsigned int RPM = 0;         // Revolutions per minute
float speedwind = 0;          // Wind speed (m/s)
float totalWindspeed = 0;     
volatile float avgWind = 0;
float PembacaanTG;            //for recieved wave height
bool newData = false;
int dataTrigger=0; 
String rate="";
float output;                 //result fuzzy rule for persentace rate
int count = 0;                //for lcd next button
unsigned long triggerStartedMillis = 0;
unsigned long getDataStartMillis = 0;

//========================================================================= Create 
RF24 radio(CE_PIN, CSN_PIN);      // Create rdio
LiquidCrystal_I2C lcd(0x27,16,2); // set address I2C dan besar karakter untuk lcd 16×2

//========================================================================= Progress Bar

//========================================================================= define for fuzzy logic
Fuzzy *fuzzy = new Fuzzy();

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

//========================================================================= Setup
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
    lcd.clear();
    lcd.setCursor(5,0);
    lcd.print("Tekan");
    lcd.setCursor(5,1);
    lcd.print("Tombol");
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

//========================================================================= Loop
void loop() 
{
  // get wave height
  if(digitalRead(btnPin)==0) {
    lcd.clear();
    triggerStartedMillis = millis();
    start();
  }
}

//========================================================================= start function
void start(){
    bool rslt;
    rslt = radio.write( &dataTrigger, sizeof(dataTrigger) );
    Serial.print("Data Sent ");
    Serial.print(dataTrigger);
    if (rslt) {
        lcd.clear();
        Serial.println("  Acknowledge received");
        setReciever();
    }
    else {
        unsigned long currTriggerMillis = millis();

        if(currTriggerMillis - triggerStartedMillis < triggerDuration){
          Serial.println("  Tx failed");
          lcd.setCursor(0,0);
          lcd.print("Mengirim trigger");

          int millProgress = ((currTriggerMillis - triggerStartedMillis) / 1000) % 15;
          char progressBar[millProgress];
          for (int i=1; i<=millProgress; i++) {
            progressBar[i] = "."
          }
    
          lcd.setCursor(1,1);
          lcd.print(progressBar);

          lcd.print("Mengirim trigger");
          start(); 
        }
        else {
          lcd.clear();
          lcd.setCursor(4,0);
          lcd.print("Time Out");
          lcd.setCursor(4,1);
          lcd.print("Cek Alat");
          delay(2000);
          setup();
          
        }
    }
}

//========================================================================= set device as reciever function
void setReciever() {
    radio.openReadingPipe(1, thisSlaveAddress);
    radio.startListening();
    Serial.println("R");
    getDataStartMillis = millis();
    getData();
    showData();
}
//========================================================================= get data function
void getData() {
  int xyz=0;
  while (xyz==0){
    if ( radio.available() ) {
        radio.read( &PembacaanTG, sizeof(PembacaanTG) );
        newData = true;
        lcd.clear();
        avgWind=totalWindspeed/Sample;
        xyz=1;
        Sample=0;
    }
    else
    {
      lcd.setCursor(5,0);
      lcd.print("Mohon Tunggu");
      
      unsigned long currGetDataMillis = millis();

      int millProgress = ((currGetDataMillis - getDataStartMillis) / 1000) % 15;
      char progressBar[millProgress];
      for (int i=1; i<=millProgress; i++) {
        progressBar[i] = "."
      }

      lcd.setCursor(4,1);
      lcd.print(progressBar);

      if(currGetDataMillis - getDataStartMillis < getDataDuration){
          Sample++;
          windvelocity();
          RPMcalc();
          WindSpeed();
          Serial.println(speedwind);
          Serial.println(Sample);
          totalWindspeed+=speedwind;
      }
      else {
          lcd.clear();
          lcd.setCursor(4,0);
          lcd.print("Time Out");
          lcd.setCursor(4,1);
          lcd.print("Cek Alat");
          delay(2000);
          xyz=1;
          setup();
          //return;
      }
    } 
  }   
}

float fuzzycalc(){
  // fuzzy logic
  float in_wave = PembacaanTG;
  float in_wind = avgWind; // nanti dari anemo
      
  fuzzy->setInput(1, in_wave);
  fuzzy->setInput(2, in_wind);
  fuzzy->fuzzify();
        
  float output = fuzzy->defuzzify(1);
  Serial.println(output);
      
  if(output < 0.5){
    rate=" Aman ";
    }
  else {
    rate="Bahaya";
    }
  return output;
}
        
//========================================================================= show data function
void showData() {
  while(digitalRead(btnPin)==1){
    if (newData == true) {
      Serial.print("Data received ");
      Serial.println(PembacaanTG);
      float fuz = fuzzycalc()* 100;
      
      if(digitalRead(button)==0){
        delay(100);
        count++;
        lcd.clear();
      }
      
      if(count%4 == 0){
        lcd.setCursor(5,0);
        lcd.print(rate);
        Serial.println(rate);
        lcd.setCursor(4,1);
        lcd.print("Berlayar");
        delay(100);
      }
      
      else if(count%4 == 1){
        lcd.setCursor(3,0);
        lcd.print("Kec. Angin");
        lcd.setCursor(5,1);
        lcd.print(speedwind);
        lcd.print(" m/s"); 
        delay(100);
      }
      
      else if(count%4 == 2){
        lcd.setCursor(0,0);
        lcd.print("Tinggi Gelombang");
        lcd.setCursor(6,1);
        lcd.print(PembacaanTG);
        lcd.print(" m");   
        delay(100);
      }
      
      else if(count%4 == 3){
        lcd.setCursor(1,0);
        lcd.print("Tingkat Bahaya");
        lcd.setCursor(6,1);
        lcd.print(fuz);
        Serial.println(fuz);
        lcd.print("%");   
        delay(100);
      }
    }
  }
  newData = false;
  radio.stopListening();
  radio.openWritingPipe(thisSlaveAddress);
  count=0;
  lcd.clear();
  Serial.println("T");
}

// Measure wind speed
void windvelocity()
{
  speedwind = 0;
  counter = 0;
  attachInterrupt(0, addcount, CHANGE);
  unsigned long millis();
  long startTime = millis();
  while(millis() < startTime + period) {}
  
  detachInterrupt(1);
}

void RPMcalc()
{
  RPM=((counter/jml_celah)*60)/(period/1000); // Calculate revolutions per minute (RPM)
}

void WindSpeed()
{
  speedwind = ((2 * pi * an_radio * RPM)/60) / 1000; // Calculate wind speed on m/s
}

void addcount()
{
  counter++;
}
