#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h> // change to #include <WiFi101.h> for MKR1000

#include "arduino_secrets.h"

#define TRIG 9 //TRIG 핀 설정 (초음파 보내는 핀)
#define ECHO 8 //ECHO 핀 설정 (초음파 받는 핀)
#include <Servo.h>
Servo myservo;  //서보모터 사용

#include <ArduinoJson.h>

/////// Enter your sensitive data in arduino_secrets.h
const char ssid[]        = SECRET_SSID;
const char pass[]        = SECRET_PASS;
const char broker[]      = SECRET_BROKER;
const char* certificate  = SECRET_CERTIFICATE;

WiFiClient    wifiClient;            // Used for the TCP socket connection
BearSSLClient sslClient(wifiClient); // Used for SSL/TLS connection, integrates with ECC508
MqttClient    mqttClient(sslClient);

unsigned long lastMillis = 0;
int sensor = 7; //인체감지 센서 7번
int servopin = 6; // 서보모터 6번


int value = 0; // 인체감지 센서 값 value

void setup() {
  
  Serial.begin(115200);
  while (!Serial);


  if (!ECCX08.begin()) {
    Serial.println("No ECCX08 present!");
    while (1);
  }

  ArduinoBearSSL.onGetTime(getTime);

 
  sslClient.setEccSlot(0, certificate);

 
  mqttClient.onMessage(onMessageReceived);
         
  myservo.attach(servopin);

  pinMode(TRIG, OUTPUT);

  pinMode(ECHO, INPUT);
 
  pinMode (sensor, INPUT); 
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!mqttClient.connected()) {

    connectMQTT();
  }

  // poll for new MQTT messages and send keep alives
  mqttClient.poll();


  // publish a message roughly every 5 seconds.
  if (millis() - lastMillis > 5000) {
    lastMillis = millis();
    char payload[512];
   getDeviceStatus(payload);
   sendMessage(payload);
 }

//  value = digitalRead(sensor);    // 인체감지센서인 value
//
//   if(value == HIGH)         
//   {
//    myservo.write(90);                //value값이 HIGH 일때 서보모터 회전
//     delay(15);
//  }

}

unsigned long getTime() {
  // get the current time from the WiFi module  
  return WiFi.getTime();
}

void connectWiFi() {
  Serial.print("Attempting to connect to SSID: ");
  Serial.print(ssid);
  Serial.print(" ");

  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the network");
  Serial.println();
}

void connectMQTT() {
  Serial.print("Attempting to MQTT broker: ");
  Serial.print(broker);
  Serial.println(" ");

  while (!mqttClient.connect(broker, 8883)) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the MQTT broker");
  Serial.println();

  // subscribe to a topic
  mqttClient.subscribe("$aws/things/MyMKRWiFi1010/shadow/update/delta");
}

void getDeviceStatus(char* payload) {

  
  value = digitalRead(sensor);   //인체감지 센서 값 value에 저장
   int s = 0;           // state 값(Open, Close) s에 

    if(value == HIGH)      // 인체를 감지 했을때
    {
         myservo.write(90);    //모터가  90도 돌아가서 열림
         s = 1;
    }
    else if(value == LOW)
    {
      myservo.write(0);
         s= 0;
    }


  digitalWrite(TRIG, LOW);               //초음파

  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);             

  delayMicroseconds(10);

  digitalWrite(TRIG, LOW);

    long duration, Distance;
    duration = pulseIn (ECHO, HIGH);          //  초음파가 물체에 도달하고 돌아오는 시간
    Distance = duration * 17 / 1000;          //  초음파 거리값을 cm으로 환산하는 공식 

     Serial.print(Distance);
     Serial.println(" Cm");
     Serial.println(payload);

    const char* a = (s == 1)? "Open" : "Closed";

 
  sprintf(payload,"{\"state\":{\"reported\":{\"distance\":\"%ld\",\"Currentstate\":\"%s\"}}}",Distance,a);
}
 
void sendMessage(char* payload) {
  char TOPIC_NAME[]= "$aws/things/MyMKRWiFi1010/shadow/update";
  
  Serial.print("Publishing send message:");
  Serial.println(payload);
  mqttClient.beginMessage(TOPIC_NAME);
  mqttClient.print(payload);
  mqttClient.endMessage();
}


void onMessageReceived(int messageSize) {
  // we received a message, print out the topic and contents
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  // store the message received to the buffer
  char buffer[512] ;
  int count=0;
  while (mqttClient.available()) {
     buffer[count++] = (char)mqttClient.read();
  }
  buffer[count]='\0'; // 버퍼의 마지막에 null 캐릭터 삽입
  Serial.println(buffer);
  Serial.println();

  DynamicJsonDocument doc(1024);
  deserializeJson(doc, buffer);
  JsonObject root = doc.as<JsonObject>();
  JsonObject state = root["state"];
  //const char* led = state["LED"];
  //Serial.println(led);
  
  char payload[512];  
 
}
