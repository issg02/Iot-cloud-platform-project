#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h> // change to #include <WiFi101.h> for MKR1000

#include "arduino_secrets.h"

#define TRIG 9 //TRIG 핀 설정 (초음파 보내는 핀)
#define ECHO 8 //ECHO 핀 설정 (초음파 받는 핀)
#include <Servo.h> // 서보모터 라이브러리 포함
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
         
  myservo.attach(servopin);           //서보모터 활성화

  pinMode(TRIG, OUTPUT);              //초음파 센서 TRIG OUTPUT

  pinMode(ECHO, INPUT);               // 초음파 센서 ECHO INPUT
 
  pinMode (sensor, INPUT);            // 인체 감지 센서 sensor INPUT 
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
   int s = 0;           

    if(value == HIGH)      // 인체를 감지 했을 때 (value 가 HIGH 일 때) 
    {
      myservo.write(90);    //모터가  90도 돌아가서 열리게 된다.
      s = 1;                
    }
    else if(value == LOW)
    {
      myservo.write(0);        //인체를 감지하지 않았을 때 (value 가 LOW 일 때) 
      s= 0;                 // 모터가 0도로 돌아간다.
    
    }

    digitalWrite(TRIG, LOW);                  //
    delayMicroseconds(2);
    digitalWrite(TRIG, HIGH);                 //
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);                  //초음파 송수신

    long duration, Distance;
    duration = pulseIn (ECHO, HIGH);          //  초음파가 물체에 도달하고 돌아오는 시간
    Distance = duration * 17 / 1000;          //  초음파 거리값을 cm으로 환산하는 공식 
  
    Serial.println(payload);                  // payload값 시리얼 모니터에서 확인

    const char* a = (s == 1)? "Open" : "Closed";    // s의 값이 1이면 a에 "Open" 문자열을 0이면 "Closed" 문자열 삽입
 
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
  const char* a = state["Currentstate"];

  
  char payload[512];  
   if (strcmp(a,"Open")==0) {
    sprintf(payload,"{\"state\":{\"reported\":{\"LED\":\"%s\"}}}","Open");
    delay(5000);
  } else if (strcmp(a,"Closed")==0) {
    myservo.write(0);
    sprintf(payload,"{\"state\":{\"reported\":{\"LED\":\"%s\"}}}","Closed");
    delay(5000);
  }
  
}
