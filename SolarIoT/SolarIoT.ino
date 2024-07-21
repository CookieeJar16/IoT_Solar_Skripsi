#include<WiFi.h>
#include<Firebase_ESP_Client.h>
#include<ESP32Servo.h>
#include <Adafruit_ADS1X15.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Home"
#define WIFI_PASSWORD "qwerty16"

// Insert Firebase project API Key
#define API_KEY "AIzaSyBFc9pGGFYor3a1eDdBk419LtjtAjqMRzk"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://solariot-e438b-default-rtdb.firebaseio.com/" 

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
int ledpin = 2;

//Sensor Tegangan
int sensor_v1 = 35;
int sensor_v2 = 34;
float Vmodul1 = 0.0;
float Vmodul2 = 0.0;
float hasil1 = 0.0;
float hasil2 = 0.0;
float R1 = 30000.0;
float R2 = 7500.0;
int value1 = 0;
int value2 = 0;
float power = 0.0;

//Battery
int bat_persen;

//Sensor Arus
const int sensor_v3 = 36;      // pin where the OUT pin from sensor is connected on Arduino
int mVperAmp = 185;           // this the 5A version of the ACS712 -use 100 for 20A Module and 66 for 30A Module
int Watt = 0;
double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;

//Tracking
Adafruit_ADS1115 ads;
Servo myservo1, myservo2;
int max1=0, max2=0, max3=0;
int ser1 = 80, ser2=0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  myservo1.attach(33);
  myservo2.attach(32);

  myservo1.write(ser1);
  myservo2.write(100);
  
  pinMode(sensor_v1, INPUT);
  pinMode(sensor_v2, INPUT);
  pinMode(sensor_v3, INPUT);
  pinMode(ledpin, OUTPUT);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  digitalWrite(ledpin, HIGH);
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if(Firebase.signUp(&config, &auth, "", "")){
    Serial.println("OK"); 
    signupOK = true;
}else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
if (!ads.begin())
  {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }
}

void loop() {

  //Tracking
  int16_t rRDL1, rRDL2, rRDL3, rRDL4;
  rRDL1 = ads.readADC_SingleEnded(0);
  rRDL2 = ads.readADC_SingleEnded(1);
  rRDL3 = ads.readADC_SingleEnded(2);
  rRDL4 = ads.readADC_SingleEnded(3);

  max1 = max(rRDL1, rRDL2);
  max2 = max(rRDL3, rRDL4);
  max3 = max(max1, max2);

  if(rRDL1<max3 && rRDL2<max3)
  {
    if(ser1<140)
      ser1+=1;
    myservo1.write(ser1);
  }
  if(rRDL3<max3 && rRDL4<max3)
  {
    if(ser1>0)
      ser1-=1;
    myservo1.write(ser1);
  }

  if(rRDL2<max3 && rRDL3<max3)
  {
    if(ser2<180)
      ser2+=1;
    myservo2.write(ser2);
  }
  if(rRDL1<max3 && rRDL4<max3)
  {
    if(ser2>0)
      ser2-=1;
    myservo2.write(ser2);
  }
  myservo2.write(ser1);
  myservo2.write(ser2);
  
  // put your main code here, to run repeatedly:
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 3000 || sendDataPrevMillis == 0)){
      sendDataPrevMillis = millis();
  
  //Sensor Tegangan
  value1 = analogRead(sensor_v1);
  Vmodul1 = (value1 * 3.3)/4095.0;
  hasil1 = Vmodul1 / (R2/(R1+R2));

  value2 = analogRead(sensor_v2);
  Vmodul2 = (value2 * 3.3)/4095.0;
  hasil2 = Vmodul2 / (R2/(R1+R2));

  //Sensor Arus
  Voltage = getVPP();
  VRMS = (Voltage/2) *0.707;   //root 2 is 0.707
  AmpsRMS = ((VRMS * 1000)/mVperAmp)/0.6;

  //Power Value
  power = hasil2 * AmpsRMS;

  //Batt Percent
  bat_persen = mapfloat(hasil1, 10.6, 14.4, 0, 100);

  if (bat_persen >= 100){
    bat_persen = 100;
  }
  if (bat_persen <= 0){
    bat_persen = 0;
  }

  //Firebase
  if(Firebase.RTDB.setFloat(&fbdo, "Sensor/V0", hasil1)){
    Serial.println(); 
    Serial.print(hasil1);
    Serial.print(" - Passed saved to: " +fbdo.dataPath());
    Serial.print(" (" + fbdo.dataType() + ")");
  }else{
    Serial.println("Failed: " + fbdo.errorReason());
  }
  if(Firebase.RTDB.setFloat(&fbdo, "Sensor/V1", hasil2)){
    Serial.println(); 
    Serial.print(hasil2);
    Serial.print(" - Passed saved to: " +fbdo.dataPath());
    Serial.print(" (" + fbdo.dataType() + ")");
  }else{
    Serial.println("Failed: " + fbdo.errorReason());
  }
  if(Firebase.RTDB.setFloat(&fbdo, "Sensor/V2", AmpsRMS)){
    Serial.println(); 
    Serial.print(AmpsRMS);
    Serial.print(" - Passed saved to: " +fbdo.dataPath());
    Serial.print(" (" + fbdo.dataType() + ")");
  }else{
    Serial.println("Failed: " + fbdo.errorReason());
  }
  if(Firebase.RTDB.setFloat(&fbdo, "Sensor/V3", power)){
    Serial.println(); 
    Serial.print(power);
    Serial.print(" - Passed saved to: " +fbdo.dataPath());
    Serial.print(" (" + fbdo.dataType() + ")");
  }else{
    Serial.println("Failed: " + fbdo.errorReason());
  }
  if(Firebase.RTDB.setInt(&fbdo, "Sensor/bat", bat_persen)){
    Serial.println(); 
    Serial.print(bat_persen);
    Serial.print(" - Passed saved to: " +fbdo.dataPath());
    Serial.print(" (" + fbdo.dataType() + ")");
  }else{
    Serial.println("Failed: " + fbdo.errorReason());
  }
 }
}
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float getVPP()
{
  float result;
  int readValue;                // value read from the sensor
  int maxValue = 0;             // store max value here
  int minValue = 4096;          // store min value here ESP32 ADC resolution
  
   uint32_t start_time = millis();
   while((millis()-start_time) < 1000) //sample for 1 Sec
   {
       readValue = analogRead(sensor_v3);
       // see if you have a new maxValue
       if (readValue > maxValue) 
       {
           /*record the maximum sensor value*/
           maxValue = readValue;
       }
       if (readValue < minValue) 
       {
           /*record the minimum sensor value*/
           minValue = readValue;
       }
   }
   
   // Subtract min from max
   result = ((maxValue - minValue) * 3.3)/4096.0; //ESP32 ADC resolution 4096
      
   return result;
 }
