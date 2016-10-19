#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
#include <ESP8266WiFi.h>
#include "Pushover.h"

MPU6050 accelgyro;
int state = 0 ;
int16_t ax, ay, az, gx, gy, gz;
float acx, acy, acz;
float cx = 0, cy = 0 , cz = 0;//calibration
unsigned long timer, preTime , timeOut;
int tmp[2];
int m;
float Raw_AM;
int i;
int check = 1;
const int buttonPin = 16;     // D0
const int ledPin =  12;      // D7
const int vibration = 2 ;
const int ledwifi = 13 ;
int buttonState = 0; 

const char* ssid     = "bangorn";
const char* password = "1111100000";
void setup() {
  #ifdef ESP8266
  Wire.begin(5,4);
  #endif
  Serial.begin(9600);
  accelgyro.initialize();
  accelgyro.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);
  //accelgyro.setXAccelOffset(0);
  pinMode(vibration, OUTPUT);
  pinMode(ledwifi, OUTPUT);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(ledwifi,LOW);
    delay(500);
    Serial.print(".");
  }
  digitalWrite(ledwifi,HIGH);
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if(WiFi.status() != WL_CONNECTED){
    WiFi.begin(ssid, password);
    digitalWrite(ledwifi,LOW);
   while (WiFi.status() != WL_CONNECTED) {
    
    delay(500);
    Serial.print(".");
  }
  digitalWrite(ledwifi,HIGH);
  }
  timer = millis();
  
  if(state == 0){
    
    buttonState = digitalRead(buttonPin);
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    acx = (ax+cx);//(16384 MPU6050_ACCEL_FS_2)
    acy = (ay+cy);//(2048 MPU6050_ACCEL_FS_16)
    acz = (az+cz);
    tmp[0] = Raw_AM;
    Raw_AM = pow(pow(acx,2)+pow(acy,2)+pow(acz,2),0.5);
    Serial.println(Raw_AM);
    if(timer<2000){
      tmp[1] = Raw_AM - tmp[0];
    }else{
    m = Raw_AM - tmp[0];
    }
//  Serial.println(m);
    
    if(m > 10000 || m < -10000 && check == 0){
      
      Serial.println("FALL DETECTED");
      check = 1;
      state = 1;
      timeOut = timer;
    }else{
      //Serial.println("no problem");
      check = 0;
      state = 0;
     
    }
    
    if (buttonState == HIGH) {
            unsigned long timerAck = ((timer - preTime) / 1000);
            if( timerAck >= 0.0) {
              Serial.println("I got some problem at state 0");
              delay(50);
              digitalWrite(ledPin, HIGH);
              state = 3; //
              
            }
        }else{
            preTime = timer;
         }

         
  }else if (state == 1){
      Serial.println((timer - timeOut) /1000);
      buttonState = digitalRead(buttonPin);
      
      if(((timer - timeOut) /1000)<4){
        digitalWrite(ledPin, LOW);
        digitalWrite(vibration , LOW);
        delay(500);
         digitalWrite(ledPin, HIGH);
        digitalWrite(vibration , HIGH);
        delay(500);
        
        if (buttonState == HIGH) {
            unsigned long timerAck = ((timer - preTime) / 1000);
            if( timerAck >= 1) {
              Serial.println("no problem ");
              digitalWrite(ledPin, LOW);
              digitalWrite(vibration , LOW);
              delay(50);
              state = 5;
            }
        }else{
            preTime = timer;
         }
        
      }else {
        Serial.println("notify()");
        send_notify();
        state = 2;
      }
      
  }else if(state == 2){
    //buzzer , vibration on 
        
        Serial.println("state2");
        digitalWrite(ledPin, HIGH);
        digitalWrite(vibration , HIGH);
        delay(50);
        buttonState = digitalRead(buttonPin);
        digitalWrite(ledPin, LOW);
        digitalWrite(vibration , LOW);
        delay(50);
         
        if (buttonState == HIGH ) {
          
          unsigned long timerAck = ((timer - preTime) / 1000);
          if( timerAck >= 1) {
            state = 4;
          }
          
        }else{
          preTime = timer;
        }
        
  }else if(state == 3){
    buttonState = digitalRead(buttonPin);
    if(buttonState == HIGH){
     
    }else {
      send_notify();
      delay(50);
      state = 2;
    }
  }else if(state == 4){
    buttonState = digitalRead(buttonPin);
    if(buttonState == HIGH){
     
    }else {
      delay(50);
      state = 0;
    }
  }else if(state == 5){
    buttonState = digitalRead(buttonPin);
    if(buttonState == HIGH){
     
    }else {
      delay(50);
      state = 0;
    }
  }
  
 }
 void send_notify(){
    Pushover po = Pushover("atxx2at12gtx95rs4d54qn5unnw8xi","uja9ap1p4ndz2et5b62114d93trhc2");
  //po.setDevice("chrome");
  po.setMessage("JADE NaJa Test ");
  po.setSound("siren");
  po.setPriority(2);
  po.setRetry(30);
  po.setExpire(3600);
  Serial.println(po.send()); //should return 1 on success
}
