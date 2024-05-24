#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <PMS.h>
#include <Wire.h>
#include <dht_nonblocking.h>
#include <LiquidCrystal_I2C.h>
#include "ThingSpeak.h"
LiquidCrystal_I2C lcd(0x27,16,2);

#ifndef STASSID
#define STASSID "Huu Huong"   // thay ten wifi vao day
#define STAPSK  "tumotdenchin" // thay pass wifi vao day
#endif
const char* ssid     = STASSID;
const char* password = STAPSK;

#define TIME_TO_SEND_DATA 20000
#define DHT_SENSOR_TYPE DHT_TYPE_22
static const int  DHT_SENSOR_PIN = 16;
DHT_nonblocking dht_sensor( DHT_SENSOR_PIN, DHT_SENSOR_TYPE );
PMS pms(Serial);
PMS::DATA pmsdata;

uint32_t lastReadDHT = 0;
float temperatureSum = 0;
float humiditySum = 0;
float temperature;
float humidity;
uint16_t dataDHTCount = 0;

unsigned long PMS_pm10sum = 0;
unsigned long PMS_pm25sum = 0;
uint8_t dataPMScount = 0;

void lcdInit();
void getPMSdata();
void getDHTdata();
void sendDataThingSpeak();
unsigned long lastSendData = 0;

// cac em phai lap tai khoan thingspeak roi lay ma kenh va API sau do thay vao dong duoi
unsigned long myChannelNumber = 856085;  // thay 856085 bang ma kenh cua cac em 
const char * myWriteAPIKey = "VQX1V1IQGD2NDSBU";    // thay API cua cac em vao day
WiFiClient  client;
static int j=0,i=0;

const int RED_PIN = 14;    // led đỏ
const int GREEN_PIN = 12; // led xanh lá
const int BLUE_PIN = 13;  // led xanh dương

void setup() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  Serial.begin(9600);  // cong Serial
  Serial.println(); //
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
    j=1-j;
    if(j==0)RGB_red();
    if(j==1)RGB_black();
  }
  RGB_green();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  ThingSpeak.begin(client); 
  lcdInit();
}

void loop() {
  if (millis() - lastSendData > 30000)
    { 
      sendDataThingSpeak();
      lastSendData=millis();
    }
    getPMSdata(); 
    getDHTdata();
  }
void getPMSdata(){
  if (pms.read(pmsdata))
  {
    PMS_pm25sum += pmsdata.PM_AE_UG_2_5;
    PMS_pm10sum += pmsdata.PM_AE_UG_10_0;
    dataPMScount++;

    
    float pm10Float = pmsdata.PM_AE_UG_10_0;
    float pm25Float = pmsdata.PM_AE_UG_2_5;
    char pms25Char[3];
    char pms10Char[3];
    
    pm25Float = 1.33 * pow(pm25Float, 0.85); //cong thuc cua airbeam
    pm10Float = 1.06 * pm10Float;
    
    uint16_t pm10Int = pm10Float + 0.5; //lam tron len ket qua thu duoc
    uint16_t pm25Int = pm25Float + 0.5;
    Serial.println("Pm2.5:"+String(pm25Int)+" Pm10:"+String(pm10Int));
    lcd.setCursor(5, 0); 
    lcd.print("   ");
    lcd.setCursor(5, 0);
    lcd.print(pm25Int);
    canhbao(pm25Int);
  }
}

void getDHTdata()
{
  if ( millis( ) - lastReadDHT > 2500 )
  {
    if ( dht_sensor.measure( &temperature, &humidity ) == true )
    {
      lastReadDHT = millis( );

      if (temperature != 0 && humidity != 0)
      {
        temperatureSum += temperature;
        humiditySum += humidity;
        dataDHTCount++;
        
        lcd.setCursor(3, 1);
        lcd.print(int(temperature));
        lcd.setCursor(12, 1);
        lcd.print(int(humidity));
        Serial.println("Temp: "+String(temperature) + " Humi: " + String(humidity));  
      }
    }
  }
}
void sendDataThingSpeak()
{
  // tinh trung binh
  float temp = 0;
  float hum = 0;
  if (dataDHTCount != 0)
  {
    temp = temperatureSum / dataDHTCount;
    hum = humiditySum / dataDHTCount;
    temperatureSum = 0;
    humiditySum = 0;
    dataDHTCount = 0;
  } 
  
  float PMS_pm25 = 0;
  float PMS_pm10 = 0;
  if (dataPMScount != 0)
  {
    PMS_pm25 = PMS_pm25sum / dataPMScount;
    PMS_pm10 = PMS_pm10sum / dataPMScount;
    PMS_pm25sum = 0;
    PMS_pm10sum = 0;
    dataPMScount = 0;
  }

  //chuan bi data de gui len mang
  uint8_t _temp = temp + 0.5;
  uint8_t _hum = hum + 0.5;
  uint16_t _PMSpm25 = 1.33 * pow(PMS_pm25, 0.85);
  uint16_t _PMSpm10 = 1.06 * PMS_pm10;
      if(i==0){
        ThingSpeak.writeField(myChannelNumber, 1 , _temp, myWriteAPIKey);
        Serial.println("Temp: "+String(_temp));
      }
      if(i==1){
        ThingSpeak.writeField(myChannelNumber, 2 , _hum, myWriteAPIKey);
        Serial.println("Humi: "+String(_hum));
      }
      if(i==2){
        ThingSpeak.writeField(myChannelNumber, 3 , _PMSpm25, myWriteAPIKey);
        Serial.println("Pm2.5: "+String(_PMSpm25));

      }
      if(i==3){
        ThingSpeak.writeField(myChannelNumber, 4 , _PMSpm10, myWriteAPIKey);
        Serial.println("Pm10: "+String(_PMSpm10));

      }
      i++;
      if(i>3) i=0;
}

void lcdInit()
{
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0); 
  lcd.print("PMS:");
  lcd.setCursor(12, 0); 
  lcd.print("g/m3");
  lcd.setCursor(0, 1); 
  lcd.print("T:");
  lcd.setCursor(6, 1);
  lcd.print("*C");
  lcd.setCursor(9, 1); 
  lcd.print("H:");
  lcd.setCursor(15, 1);
  lcd.print("%");
  Serial.println("LCD done");
}
void canhbao(int pm25Int){
  if(pm25Int <= 25){
       RGB_green();
     }
    if(25 < pm25Int && pm25Int<= 60){
       RGB_yellow();
     }
    if(60 <pm25Int&& pm25Int<= 110){
       RGB_red();
     }
    if(110<pm25Int){
      RGB_red();
    }
}
void RGB_red(){
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);
}
void RGB_green(){
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, LOW);
}
void RGB_yellow(){
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, LOW);
}
void RGB_violet(){
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, HIGH);
}
void RGB_trang(){
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, HIGH);
}
void RGB_black(){
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);
}
