#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

StaticJsonDocument<256> doc;

const char* ssid = "SSID";
const char* pass = "PASSWORD";

String keyCode = "7890";
bool opened = false;
bool moved = true;
bool unlocked = true;
int owner = 1;
String mailboxName = "Arduino";
int id = 2;

int status = WL_IDLE_STATUS;
const String serverName = "http://IP:8000/";

//Get mailbox object
void getMailbox(){
  WiFiClient client;
  HTTPClient http;
  
  String ext = serverName+"api/userBox/"+String(id)+"/?format=json";
  Serial.println("Getting Mailbox");
  http.begin(client,ext);
  http.addHeader("Connection","close");
  int httpCode = http.GET();

  delay(1000);
  if(httpCode > 0){

    Serial.println("\nStatuscode: " + String(httpCode));
    DeserializationError error = deserializeJson(doc, http.getString());

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }
    
    id = doc["id"];
    unlocked = doc["unlocked"];
    const char* temp1 = doc["keyCode"];
    keyCode = String(temp1);
    const char* temp2= doc["name"];
    mailboxName = String(temp2);
    owner = doc["owner"];
    Serial.println(mailboxName);
  }

  http.end();
}

//Updates list
void makeUpdate(){
  WiFiClient client;
  HTTPClient http;
  
  String ext = serverName+"api/updateBox";
  Serial.println("Sending Update");
  http.begin(client,ext);
  http.addHeader("Content-type","application/json");
  http.addHeader("Connection","close");
  String Payload = "{\"opened\":"+String(opened)+
                   ",\"moved\":"+String(moved)+
                   ",\"mailbox\":"+String(id)+"}";
  Serial.println(Payload);
  Serial.println(Payload.length());
  int httpCode = http.POST(Payload);

  delay(1000);
  if(httpCode > 0){

    Serial.println("\nStatuscode: " + String(httpCode));
    Serial.println(http.getString());
  }

  http.end();
}

void updateMailbox(){
  WiFiClient client;
  HTTPClient http;
  
  String ext = serverName+"api/userUpdateBox/"+String(id)+"/";
  Serial.println("Updating Mailbox");
  http.begin(client,ext);
  http.addHeader("Content-type","application/json");
  http.addHeader("Connection","close");
  String Payload = "{\"unlocked\":"+String(unlocked)+
                   ",\"keyCode\":"+keyCode+
                   ",\"name\":\""+mailboxName+
                   "\",\"owner\":\""+String(owner)+"\"}";
  Serial.println(Payload);
  Serial.println(Payload.length());
  int httpCode = http.POST(Payload);

  delay(1000);
  if(httpCode > 0){

    Serial.println("\nStatuscode: " + String(httpCode));
    Serial.println(http.getString());
  }

  http.end();
}

void setup() {

  //Initialize serial and wait for port to open:

  Serial.begin(9600);

  while (!Serial) {

    ; 

  }



  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {

    Serial.print("Attempting to connect to SSID: ");
    
    Serial.println(ssid);
    
    delay(1000);
  }

  Serial.println("Connected to wifi");

  Serial.println(WiFi.localIP());

  Serial.println("\nStarting connection to server...");

  delay(10000);

  makeUpdate();

  delay(10000);

  getMailbox();

  Serial.println("");

  delay(10000);

  updateMailbox();
}

void loop() {

  delay(1000);
}
