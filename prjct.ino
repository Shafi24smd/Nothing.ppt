#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include"DHT.h"

DHT rak(D1, DHT11); //dht11 is a type of sensor:
const int ldrPin = A0;
int sensor = D7;
String command;
String data = "";

void callback(char* topic, byte* payload, unsigned int payloadLength);

// CHANGE TO YOUR WIFI CREDENTIALS
const char* ssid = "Shafi";
const char* password = "shafi@24";

// CHANGE TO YOUR DEVICE CREDENTIALS AS PER IN IBM BLUMIX
#define ORG "s3aon4"
#define DEVICE_TYPE "Iot"
#define DEVICE_ID "id"
#define TOKEN "Shafismd" //  Authentication Token OF THE DEVICE

//-------- Customise the above values --------
const char publishTopic[] = "iot-2/evt/Data/fmt/json";
char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char topic[] = "iot-2/cmd/home/fmt/String";// cmd  REPRESENT command type AND COMMAND IS TEST OF FORMAT STRING
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;


WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);

int publishInterval = 5000; // 30 seconds
long lastPublishMillis;
void publishData();
int temp, hum, ldrStatus;
int state;
String sid;
void setup() {

  rak.begin();
  pinMode(D0, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(sensor, INPUT);
  pinMode(ldrPin, INPUT);
  Serial.begin(9600);
  wifiConnect();
  mqttConnect();

}

void loop() {

  hum = rak.readHumidity();
  temp = rak.readTemperature();
  state = digitalRead(sensor);
  Serial.println("***************");
  Serial.println(state);
  delay(3000);
  if (state == 0)
  {
    sid = "NO One";
  }
  else
  {
    sid = "Entered";
  }
  int ldrStatus = analogRead(ldrPin);

  if (millis() - lastPublishMillis > publishInterval)
  {
    publishData();
    lastPublishMillis = millis();
  }

  if (!client.loop()) {
    mqttConnect();
  }
  
}


void wifiConnect() {
  Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("WiFi connected, IP address: ");
  Serial.println(WiFi.localIP());
}

void mqttConnect() {
  if (!client.connected())
  {
    Serial.print("Reconnecting MQTT client to "); Serial.println(server);
    while (!client.connect(clientId, authMethod, token)) {
      Serial.print(".");
      delay(500);
    }

    initManagedDevice();
    Serial.println();
  }
}

void initManagedDevice() {
  if (client.subscribe(topic)) {

    Serial.println("subscribe to cmd OK");
  } else {
    Serial.println("subscribe to cmd FAILED");
  }
}

void callback(char* topic, byte* payload, unsigned int payloadLength) {

  Serial.print("callback invoked for topic: ");
  Serial.println(topic);

  for (int i = 0; i < payloadLength; i++) {

    command += (char)payload[i];
  }

  Serial.print("data: " + command);
  control_func();
  command = "";
}
void control_func()
{
  if (command == "FANOFF")
  {
    digitalWrite(D2, LOW);
    Serial.println(".......Fans are off..........");

  }
  else if (command == "FANON")
  {
    digitalWrite(D2, HIGH);
    Serial.println(".......Fan are on..........");

  }
  if (command == "LIGHTOFF")
  {
    digitalWrite(D0, LOW);
    Serial.println(".......lights are off..........");

  }
  else if (command == "LIGHTON")
  {
    digitalWrite(D0, HIGH);
    Serial.println(".......lights are on..........");

  }
  else if (command == "AWAY")
  {
    digitalWrite(D2, LOW);
    digitalWrite(D0, LOW);
    Serial.println(".......All Devices are off..........");
  }
  else if (command == "ECO")
  {
    if (hum >= 50 && temp >= 33)
  {
    digitalWrite(D0, HIGH);
    Serial.println("Fan is on");
  }
  else
  {
    digitalWrite(D0, LOW);
    Serial.println("Fan is off");
  }
  Serial.print(" ldrStatus is " );
  Serial.print(ldrStatus);
  if (ldrStatus >= 1024)
  {
    digitalWrite(D2, HIGH);
    Serial.println("Light is on");
  }
  else
  {
    digitalWrite(D2, LOW);
    Serial.println("Light is off");
  }
  if (state == 1) {
    Serial.println("Motion detected!");
    digitalWrite(D2, HIGH);
    Serial.println("Automatically Light is on");
  }
  else
  {
    digitalWrite(D2, LOW);
    Serial.println("There is no one in Home ");
    Serial.println("Motion absent!");
  }

  delay(3000);
    }
}
void publishData()
{
  String payload = "{\"d\":{\"temperature\":";
  payload += temp;
  payload += ",""\"humidity\":";
  payload += hum;
  payload += ",""\"State\":";
  payload += "\"" + sid + "\"";
  payload += "}}";


  Serial.print("\n");
  Serial.print("Sending payload: "); Serial.println(payload);

  if (client.publish(publishTopic, (char*) payload.c_str())) {
    Serial.println("Publish OK");
    Serial.println("**************************************");
  } else {
    Serial.println("Publish FAILED");
  }
  delay(3000);
}
