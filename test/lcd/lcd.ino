#include <Wire.h> //library I2C

#include <LiquidCrystal_I2C.h>    //library LCD

LiquidCrystal_I2C lcd(0x27,16,2); // set address I2C dan besar karakter untuk lcd 16×2
int button = 4;
int count = 0;

void setup()
{
  Serial.begin(9600);
  lcd.init(); // inisiasi LCD
  pinMode(button,INPUT_PULLUP);
  lcd.backlight();
}

void loop()
{
  if(digitalRead(button)==0){
    delay(100);
    count++;
    lcd.clear();
    Serial.print(digitalRead(button));
  }

  if(count%5 == 1){
    lcd.setCursor(6,0);
    lcd.print("Aman");
    lcd.setCursor(4,1);
    lcd.print("Berlayar");
    delay(100);
    Serial.print(count);
  }

  else if(count%5 == 2){
    lcd.setCursor(3,0);
    lcd.print("Kec. Angin");
    lcd.setCursor(5,1);
    lcd.print("xx km/h");
    delay(100);
    Serial.print(count);
  }

  else if(count%5 == 3){
    lcd.setCursor(0,0);
    lcd.print("Tinggi Gelombang");
    lcd.setCursor(6,1);
    lcd.print("yy m");  
    delay(100);
    Serial.print(count);
  }

  else if(count%5 == 4){
    lcd.setCursor(1,0);
    lcd.print("Tingkat Bahaya");
    lcd.setCursor(6,1);
    lcd.print("zz %");  
    delay(100);
    Serial.print(count);
    }
}
