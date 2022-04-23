// get libraries
#include <Keypad.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "ESP32_MailClient.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// the pin assignment
int trigPin = 12; // pin Trig of HC-SR04 is output
int echoPin = 14; // pin Echo of HC-SR04 is input
int buzzPin = 6;   // Set the buzz pin as OUTPUT
int motorTopPin = 17; // set pin for pwm for top motor
int motorBottomPin = 18; // set pin for pwm for bottom motor
int SCL_LCD = 9;
int SDA_LCD = 8;
int SCL_ACC = 36; //wire red
int SDA_ACC = 35; //wire brown 
int keypadR1 = 11;
int keypadR2 = 10;
int keypadR3 = 7;
int keypadR4 = 3;
int keypadC1 = 1;
int keypadC2 = 38;
int keypadC3 = 33;

int looper = 0;

// info for keypad set up
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {keypadR1, keypadR2, keypadR3, keypadR4}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {keypadC1, keypadC2, keypadC3}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS ); //Create an object of keypad


// varaible to store info
int currentState;
String codeS;
String currentNumS;
String newssid;
String newPassword;
String newGmail;
String displayTop;
String displayBottom;
char lastInput;
bool isLocked;
bool hasBroken;
bool displayOn;
bool readyMoveSense;

//Server Vars
int ownerID = 1;
String mailboxName = "Arduino";
int mailboxID = 2;
StaticJsonDocument<256> doc;

int status = WL_IDLE_STATUS;
const String serverName = "http://192.168.1.23:8000/";

// I2C stuff
LiquidCrystal_I2C lcd(0x27, 24, 2); //set up for 24 col and 2 row   
Adafruit_MPU6050 mpu;

/**
* email set
* inclueds wifi
*/
//email setup
// REPLACE WITH YOUR NETWORK CREDENTIALS
String ssid = "M042091PhD1200jpGjeG";
String password = "NA792611um04ut79";


// To send Email using Gmail use port 465 (SSL) and SMTP Server smtp.gmail.com
// YOU MUST ENABLE less secure app option https://myaccount.google.com/lesssecureapps?pli=1
#define emailSenderAccount    "senderAPP2022@gmail.com"
#define emailSenderPassword   "12345678a!"
#define smtpServer            "smtp.gmail.com"
#define smtpServerPort        465


// Default Recipient Email Address
String inputMessage = "jamesegranata@gmail.com";
String enableEmailChecked = "checked";
String inputMessage2 = "true";


// HTML web page to handle 3 input fields (email_input, enable_email_input, threshold_input)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Email Notification with Temperature</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <h2>DS18B20 Temperature</h2> 
  <h3>%TEMPERATURE% &deg;C</h3>
  <h2>ESP Email Notification</h2>
  <form action="/get">
    Email Address <input type="email" name="email_input" value="%EMAIL_INPUT%" required><br>
    Enable Email Notification <input type="checkbox" name="enable_email_input" value="true" %ENABLE_EMAIL%><br>
    Temperature Threshold <input type="number" step="0.1" name="threshold_input" value="%THRESHOLD%" required><br>
    <input type="submit" value="Submit">
  </form>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

// port 80
AsyncWebServer server(80);

// Flag variable to keep track if email notification was sent or not
bool emailSent = false;

const char* PARAM_INPUT_1 = "email_input";
const char* PARAM_INPUT_2 = "enable_email_input";

// need for email
String processor(const String& var){
  if(var == "EMAIL_INPUT"){
    return inputMessage;
  }
  else if(var == "ENABLE_EMAIL"){
    return enableEmailChecked;
  }
  return String();
}

// The Email Sending data object contains config and data to send
SMTPData smtpData;


void setup() {
// put your setup code here, to run once:

  // initialize variables
  currentState = 0;
  codeS = "5542";
  isLocked = false;
  hasBroken = false;
  displayOn = false;
  readyMoveSense = false;
  currentNumS = "";
  newssid = "";
  newPassword = "";
  newGmail = "";
  displayTop = "Setting up";
  displayBottom = "Please wait";

  // have pins
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT
  pinMode(buzzPin, OUTPUT); // Set the buzz pin as OUTPUT
  pinMode(motorTopPin, OUTPUT); // set pwm pin for motor top as OUTPUT
  pinMode(motorBottomPin, OUTPUT); // set pwm pin for motor bottom as OUTPUT
  
  // initialize the serial port:
  Serial.begin(115200);

  // get display ready
  Wire.begin(SDA_LCD, SCL_LCD); // connect to SDA and SDL pins
  lcd.init(); // initialize LCD                    
  lcd.noBacklight(); // turn on LCD backlight
  delay(10);
  turnOnDisplay();
  updateDisplay();

  // get motor ready there is bug where motor will take 0-2 primes to work
  //unlock then lock 1
  armInTop();
  delay(750); //better if two delay
  delay(750);
  armInBottom();
  delay(750); //better if two delay
  delay(750);
  
  armOutTop();
  delay(750); //better if two delay
  delay(750);
  armOutBottom();
  delay(750); //better if two delay
  delay(750);

  //unlock then lock 2
  armInTop();
  delay(750); //better if two delay
  delay(750);
  armInBottom();
  delay(750); //better if two delay
  delay(750);
  
  armOutTop();
  delay(750); //better if two delay
  delay(750);
  armOutBottom();
  delay(750); //better if two delay
  delay(750);


  // have it start as unlocked not
  armInTop();
  delay(750); //better if two delay
  delay(750);
  armInBottom();

  
  delay(500); //better if two delay
  delay(500);
  setUpEmail();

  displayTop = "M:0";
  displayBottom = "";
  updateDisplay();
  
  // set up wire for move sensor
  Wire.end();
  Wire.begin(35, 36); // sdo35to sda and skc36 to scl
  readyMove();
  
  getMailbox();
  
}

void loop() {
// put your main code here, to run repeatedly:

  //get input
  int inputNum = getInput();
  if(inputNum == -1){
    inputNum = getInput();    
  }
  // unlocking box
  looper = (looper + 1) % 100;
  if(looper == 0){
      bool temp = isLocked;
      getMailbox();
      if(temp && !isLocked){
        unlockBox();
      }
      else if(!temp && isLocked){
        lockBox();
      }    
  }

  //to help with eaten imput
  if(inputNum == -1){
    inputNum = getInput();
  }

  // do action if a key is pressed
  if(inputNum != -1){

    //state 0 is menu state
    if(currentState == 0){ 
      //state 1 = lock box
      // will go back to state 0 afterwards
      if(inputNum == 1){ 
        if(! isLocked){ 
          lockBox(); 
        }        
      delay(500);
      displayTop = "M:0";
      displayBottom = "";
      updateDisplay();
      }
      if(inputNum == 2){ 
        currentState = 2;
        Serial.println("mode change to 2");
      displayTop = "M:2";
      displayBottom = "CI:";
      updateDisplay();
      }
      if(inputNum == 3){ 
        currentState = 3;
        Serial.println("mode change to 3");
    displayTop = "M:3-1";
    displayBottom = "CI:";
    updateDisplay();
      }
    if(inputNum == 4){ 
        currentState = 4;
        Serial.println("mode change to 4");
    displayTop = "M:4-1";
    displayBottom = "CI:";
    updateDisplay();
      }
      if(inputNum == 5){ 
        currentState = 5;
        Serial.println("mode change to 5");
    displayTop = "M:5-1";
    displayBottom = "CI:";
    updateDisplay();
      }
    if(inputNum == 6){ 
        currentState = 6;
        Serial.println("mode change to 6");
        displayTop = "M:6-1";
        displayBottom = "CI:";
        updateDisplay();
      }
      if(inputNum == 7){ 
        setUpEmail();
        Serial.println("trying to connect");
      displayTop = "M:0";
      displayBottom = "";
      updateDisplay();
      }
      if(inputNum == 8){ 
        if(displayOn){
          turnOffDisplay();
          Serial.println("dis off");
        }
        else{
          turnOnDisplay();
          Serial.println("dis on");
        }

      }
    } 
  //state 11 = broken into
  else if(currentState == 11){
      //add number
      if(inputNum >= 0){
        currentNumS = currentNumS + inputNum;  
        Serial.print("Current Number is ");
        Serial.println(currentNumS);
        displayBottom = "CI:" + currentNumS;
        updateDisplay();
      }
      // not doing else if(inputNum == -2){}
      // check if right
      else if(inputNum == -3){
          if(currentNumS.equals(codeS)){ 
            Serial.println("code is correct!");
            if(isLocked){
              unlockBox();
              currentState = 0;
              displayTop = "M:0";
              displayBottom = "";
              updateDisplay();        
            } 
          }
          else {  
      Serial.println("code is incorrect"); 
      displayBottom = "CI:";
            updateDisplay();  
      }
          currentNumS = "";   
      }    
    }
    //state 2 = input code to unlock
    else if(currentState == 2){
      //add number
      if(inputNum >= 0){
        currentNumS = currentNumS + inputNum;  
        Serial.print("Current Number is ");
        Serial.println(currentNumS);
    displayBottom = "CI:" + currentNumS;
    updateDisplay();
      }
      else if(inputNum == -2){
      currentNumS = "";  
      currentState = 0;  
      Serial.println("mode change to 0");
      displayTop = "M:0";
      displayBottom = ""; 
        updateDisplay();    
      }
      // check if right
      else if(inputNum == -3){
          if(currentNumS.equals(codeS)){ 
            Serial.println("code is correct!");
            if(isLocked){
              unlockBox();  
              Serial.println("is unlocked");     
            } 
          displayTop = "M:0";
          displayBottom = "";
          updateDisplay(); 
          }
          else {  Serial.println("code is incorrect"); }
          currentNumS = "";  
          currentState = 0;
          Serial.println("mode change to 0");
      displayTop = "M:0";
      displayBottom = "";
          updateDisplay();      
      }    
    }
    //state 3  = change code part 1
    //make sure authized person is doing it by checking code
    else if(currentState == 3){
      //add number
      if(inputNum >= 0){
        currentNumS = currentNumS + inputNum;  
        Serial.print("Current Number is ");
        Serial.println(currentNumS);
    displayBottom = "CI:" + currentNumS;
    updateDisplay();
      }
      else if(inputNum == -2){ 
        currentNumS = "";
        currentState = 0;  
        Serial.println("mode change to 0");  
    displayTop = "M:0";
    displayBottom = "";
    updateDisplay();
      }
      // check if right
      else if(inputNum == -3){
          if(currentNumS.equals(codeS)){ 
            Serial.println("code is correct. Procced to change.");
            currentState = 33;
      displayTop = "M:3-2";
      displayBottom = "CI:";
      updateDisplay();
          }
          else {  
            Serial.println("code is incorrect"); 
            currentState = 0;
            Serial.println("mode change to 0"); 
      displayTop = "M:0";
        displayBottom = "";
        updateDisplay();
          }
          currentNumS = "";       
      }    
    }
    //state 3  = change code part 2
    //is gotten to by entering correct code in state 3
    //imput new code
    else if(currentState == 33){
      //add number
      if(inputNum >= 0){
         
        currentNumS = currentNumS + inputNum;  
        Serial.print("Current Number is ");
        Serial.println(currentNumS);
    displayTop = "M:3-2";
    displayBottom = "CI:" + currentNumS;
    updateDisplay();
      }
      else if(inputNum == -2){
        currentNumS = ""; 
        currentState = 0;
        Serial.println("mode change to 0");
    displayTop = "M:0";
    displayBottom = "";
    updateDisplay();    
      }
      // check if right
      else if(inputNum == -3){
          codeS = currentNumS;
          updateMailbox();
          makeUpdate();
          Serial.print("New code is ");
          Serial.println(codeS);
          currentNumS = "";  
          currentState = 0;  
          Serial.println("mode change to 0"); 
      displayTop = "M:0";
      displayBottom = "";
      updateDisplay();        
      }    
    }
  //state 4  = change ssid part 1
  //make sure authized person is doing it by checking code
    else if(currentState == 4){
      //add number
      if(inputNum >= 0){
        currentNumS = currentNumS + inputNum;  
        Serial.print("Current Number is ");
        Serial.println(currentNumS);
        displayBottom = "CI:" + currentNumS;
        updateDisplay();
      }
      else if(inputNum == -2){ 
        currentNumS = "";
        currentState = 0;  
        Serial.println("mode change to 0"); 
        displayTop = "M:0";
        displayBottom = "";
        updateDisplay();   
      }
      // check if right
      else if(inputNum == -3){
          if(currentNumS.equals(codeS)){ 
            Serial.println("code is correct. Procced to change.");
            currentState = 44;
      displayTop = "M:4-2  CI:";
      displayBottom = "SI:";
      updateDisplay();
          }
          else {  
            Serial.println("code is incorrect"); 
            currentState = 0;
            Serial.println("mode change to 0"); 
      displayTop = "M:0";
      displayBottom = "";
      updateDisplay();
          }
          currentNumS = "";       
      }    
    }
    //state 4  = change ssid part 2
    //is gotten to by entering correct code in state 4
    //imput new ssid
    else if(currentState == 44){
      //add number
      if(inputNum >= 0){
         
        currentNumS = currentNumS + inputNum;  
        Serial.print("Current Number is ");
    Serial.println(currentNumS);
    if(currentNumS.length() == 3){
      newssid = newssid + static_cast<char>(currentNumS.toInt());
      currentNumS = "";
    }
    Serial.print("Current string is:   ");
        Serial.println(newssid);
    displayTop = "M:4-2  CI:" + currentNumS;
    displayBottom = "SI:" + newssid;
    updateDisplay();
      }
      else if(inputNum == -2){
        currentNumS = ""; 
    newssid = "";
        currentState = 0;
        Serial.println("mode change to 0");  
    displayTop = "M:0";
    displayBottom = "";
    updateDisplay();    
      }
      // check if right
      else if(inputNum == -3){
          //ssid = const_cast<char*>(newssid.c_str());
          ssid = newssid;
          Serial.print("New ssid is ");
          Serial.println(ssid);
          currentNumS = "";  
      newssid = "";
          currentState = 0;  
          Serial.println("mode change to 0");
      displayTop = "M:0";
          displayBottom = "";
          updateDisplay();      
      }    
    }
  //state 5  = change password part 1
  //make sure authized person is doing it by checking code
    else if(currentState == 5){
      //add number
      if(inputNum >= 0){
        currentNumS = currentNumS + inputNum;  
        Serial.print("Current Number is ");
        Serial.println(currentNumS);
    displayBottom = "CI:" + currentNumS;
    updateDisplay();
      }
      else if(inputNum == -2){ 
        currentNumS = "";
        currentState = 0;  
        Serial.println("mode change to 0");  
    displayTop = "M:0";
        displayBottom = "";
        updateDisplay();
      }
      // check if right
      else if(inputNum == -3){
          if(currentNumS.equals(codeS)){ 
            Serial.println("code is correct. Procced to change.");
            currentState = 55;
      displayTop = "M:5-2  CI:";
      displayBottom = "SI:";
      updateDisplay();
          }
          else {  
            Serial.println("code is incorrect"); 
            currentState = 0;
            Serial.println("mode change to 0");
        displayTop = "M:0";
            displayBottom = "";
            updateDisplay();      
          }
          currentNumS = "";       
      }    
    }
  //state 5  = change pasword part 2
    //is gotten to by entering correct code in state 4
    //imput new password
    else if(currentState == 55){
      //add number
      if(inputNum >= 0){
         
        currentNumS = currentNumS + inputNum;  
        Serial.print("Current Number is ");
    Serial.println(currentNumS);
    if(currentNumS.length() == 3){
      newPassword = newPassword + static_cast<char>(currentNumS.toInt());
      currentNumS = "";
    }
    Serial.print("Current string is:   ");
    Serial.println(newPassword);
    displayTop = "M:5-2  CI:" + currentNumS;
    displayBottom = "SI:" + newPassword;
    updateDisplay();
      }
      else if(inputNum == -2){
        currentNumS = ""; 
    newPassword = "";
        currentState = 0;
        Serial.println("mode change to 0");
    displayTop = "M:0";
    displayBottom = "";
    updateDisplay();    
      }
      // check if right
      else if(inputNum == -3){
          password = newPassword;
          Serial.print("New password is ");
          Serial.println(password);
          currentNumS = "";  
      newPassword = "";
          currentState = 0;  
          Serial.println("mode change to 0");  
      displayTop = "M:0";
      displayBottom = "";
          updateDisplay();
      }    
    }
  //state 6  = change gnmail part 1
  //make sure authized person is doing it by checking code
    else if(currentState == 6){
      //add number
      if(inputNum >= 0){
        currentNumS = currentNumS + inputNum;  
        Serial.print("Current Number is ");
        Serial.println(currentNumS);
        displayBottom = "CI:" + currentNumS;
        updateDisplay();
      }
      else if(inputNum == -2){ 
        currentNumS = "";
        currentState = 0;  
        Serial.println("mode change to 0");
        displayTop = "M:0";
        displayBottom = "";
        updateDisplay();    
      }
      // check if right
      else if(inputNum == -3){
          if(currentNumS.equals(codeS)){ 
            Serial.println("code is correct. Procced to change.");
            currentState = 66;
            displayTop = "M:6-2  CI:" + currentNumS;
            displayBottom = "SI:" + newGmail;
            updateDisplay();
          }
          else {  
            Serial.println("code is incorrect"); 
            currentState = 0;
            Serial.println("mode change to 0"); 
            displayTop = "M:0";
            displayBottom = "";
            updateDisplay();
          }
          currentNumS = "";       
      }    
    }
  //state 6 = change gmail part 2
    //is gotten to by entering correct code in state 4
    //imput new gmail
    else if(currentState == 66){
      //add number
      if(inputNum >= 0){
        currentNumS = currentNumS + inputNum;  
        Serial.print("Current Number is ");
    Serial.println(currentNumS);
    if(currentNumS.length() == 3){
      newGmail = newGmail + static_cast<char>(currentNumS.toInt());
      currentNumS = "";
    }
    Serial.print("Current string is:   ");
        Serial.println(newGmail);
        displayTop = "M:6-2  CI:" + currentNumS;
        displayBottom = "SI:" + newGmail;
        updateDisplay();
      }
      else if(inputNum == -2){
        currentNumS = ""; 
    newGmail = "";
        currentState = 0;
        Serial.println("mode change to 0");
    displayTop = "M:0";
    displayBottom = "";
    updateDisplay();    
      }
      // check if right
      else if(inputNum == -3){
          inputMessage = newGmail + "@gmail.com";
          Serial.print("New gmail is ");
          Serial.println(newGmail);
          currentNumS = "";  
      newGmail = "";
          currentState = 0;  
          Serial.println("mode change to 0");
      displayTop = "M:0";
      displayBottom = "";
      updateDisplay();      
      }    
    }
  }
  //check break
  else if(isLocked && !hasBroken){
    // check move
    if(readyMoveSense){

       if(checkMovement()){
        hasBroken = true;
        currentState = 11;
        updateMailbox();
        makeUpdate();
        Serial.println("mode to 11");       
         if(! isLocked){ 
          lockBox(); 
        }
        sendIsBroke();   
        displayTop = "M:Alarm";
        displayBottom = "CI:";
        updateDisplay();
      
    }
   }
    // check door
    if(isLocked && !hasBroken ){
      int temp = getDistanceDoor();
      Serial.println(temp);
      delay(150);
      if(temp > 40){     
        hasBroken = true;
        currentState = 11;
        Serial.println("mode to 11");
        sendIsBroke();
        displayTop = "M:Alarm";
        displayBottom = "CI:" + currentNumS;
        updateDisplay();
      }
    }
  }
  // buzz if broken
  if(hasBroken){ buzz();}
}

/*
* helper methods
*/
// method to get input as number
int getInput(){
  int inputNum = -1;

  char key = keypad.getKey();// Read the key

  if (key){
    Serial.print("Key Pressed : ");
    Serial.println(key);

    //char newInput = ' ';
    // convert to numbers
    if(key == '0'){      inputNum = 0;  }
    if(key == '1'){      inputNum = 1;  }
    if(key == '2'){      inputNum = 2;  }
    if(key == '3'){      inputNum = 3;  }
    if(key == '4'){      inputNum = 4;  }
    if(key == '5'){      inputNum = 5;  }
    if(key == '6'){      inputNum = 6;  }
    if(key == '7'){      inputNum = 7;  }
    if(key == '8'){      inputNum = 8;  }
    if(key == '9'){      inputNum = 9;  }
    if(key == '*'){      inputNum = -2; }
    if(key == '#'){      inputNum = -3; }
  }
  return inputNum;
}
        
void lockBox(){
  armOutTop();
    delay(45);
    armOutBottom();
    isLocked = true;
  Serial.println("is locked");      
    delay(1500);  
    sendIsLock(); 
    updateMailbox();
    makeUpdate();
}

void unlockBox(){
  armInTop();
    delay(45);
    armInBottom();
    isLocked = false;  
  hasBroken = false;
    delay(1500);
  updateMailbox();
  makeUpdate();
}

void armOutTop(){
 delayMicroseconds(15);
 digitalWrite(motorTopPin, HIGH);
 delayMicroseconds(1700);
 digitalWrite(motorTopPin, LOW);
 delayMicroseconds(18300);

 
 digitalWrite(motorTopPin, HIGH);
 delayMicroseconds(1700);
 digitalWrite(motorTopPin, LOW);
 delayMicroseconds(18300);
}

void armInTop(){
  delayMicroseconds(15);
 digitalWrite(motorTopPin, HIGH);
 delayMicroseconds(1000);
 digitalWrite(motorTopPin, LOW);
 delayMicroseconds(19000);

 digitalWrite(motorTopPin, HIGH);
 delayMicroseconds(1000);
 digitalWrite(motorTopPin, LOW);
 delayMicroseconds(19000);
}

void armOutBottom(){
  delayMicroseconds(15);
 digitalWrite(motorBottomPin, HIGH);
 delayMicroseconds(1700);
 digitalWrite(motorBottomPin, LOW);
 delayMicroseconds(18300);

 
 digitalWrite(motorBottomPin, HIGH);
 delayMicroseconds(1700);
 digitalWrite(motorBottomPin, LOW);
 delayMicroseconds(18300);
}

void armInBottom(){
  delayMicroseconds(15);
 digitalWrite(motorBottomPin, HIGH);
 delayMicroseconds(1000);
 digitalWrite(motorBottomPin, LOW);
 delayMicroseconds(19000);

 digitalWrite(motorBottomPin, HIGH);
 delayMicroseconds(1000);
 digitalWrite(motorBottomPin, LOW);
 delayMicroseconds(19000);
}


int getDistanceDoor(){
   // Clears the trigPin condition
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  long duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  int distanceDoor = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  return distanceDoor;
}

void buzz(){
  for(int i = 0; i < 10; i++){
    digitalWrite(buzzPin, HIGH);
    delayMicroseconds(1500);
    digitalWrite(buzzPin, LOW);
    delayMicroseconds(1500);
  }
}

void sendIsLock(){
      displayTop = "Is sending";
      displayBottom = "please wait";
      updateDisplay();
    // got tempuature reading
    if(readyMoveSense){
    delay(50);
    String theTemp = String( (int)getTemp() );  
    String message = "Is locked. Resting at " + theTemp + " degrees F.";
    if(sendEmailNotification(message, "APP Locked")) {
      emailSent = true;
    }
    else {
      Serial.println("Email failed to send");
    }
    }
    //did not get temputare reading
    else{
    if(sendEmailNotification("Is locked.", "APP Locked")) {
      emailSent = true;
    }
    else {
      Serial.println("Email failed to send");
    }  
    } 
}

void sendIsBroke(){
      displayTop = "Is sending";
      displayBottom = "please wait";
      updateDisplay();
      if(sendEmailNotification("Door has been broken.", "APP Broken")) {
        emailSent = true;
      }
      else {
        Serial.println("Email failed to send");
      }
}

bool sendEmailNotification(String emailMessage, String emailSubject){
  
  // Set the SMTP Server Email host, port, account and password
  
  smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);


  // For library version 1.2.0 and later which STARTTLS protocol was supported,the STARTTLS will be 
  // enabled automatically when port 587 was used, or enable it manually using setSTARTTLS function.
  //smtpData.setSTARTTLS(true);

  // Set the sender name and Email
  smtpData.setSender("ESP32", emailSenderAccount);

  // Set Email priority or importance High, Normal, Low or 1 to 5 (1 is highest)
  smtpData.setPriority("High");

  // Set the subject
  smtpData.setSubject(emailSubject);

  // Set the message with HTML format
  smtpData.setMessage(emailMessage, true);

  // Add recipients
  smtpData.addRecipient(inputMessage);

  smtpData.setSendCallback(sendCallback);

  // Start sending Email, can be set callback function to track the status
  if (!MailClient.sendMail(smtpData)) {
    Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
    return false;
  }
  // Clear all data from Email object to free memory
  smtpData.empty();
  //Serial.end();
  // Serial.begin(115200);
  return true;
}

// Callback function to get the Email sending status
void sendCallback(SendStatus msg) {
  // Print the current status
  Serial.println(msg.info());

  // Do something when complete
  if (msg.success()) {
    Serial.println("----------------");
  }
}

void setUpEmail(){
  // start email setup
  WiFi.mode(WIFI_STA);
  char* c1 = const_cast<char*>(ssid.c_str());
  char* c2 = const_cast<char*>(password.c_str());
  WiFi.begin(c1, c2);

  if (WiFi.waitForConnectResult(30000) != WL_CONNECTED){
    Serial.println("WiFi Failed!");
      displayTop = "WiFi Failed";
      displayBottom = "Please wait";
      updateDisplay();
      delay(1500);
    return;
  }
  Serial.println();
  Serial.print("ESP IP Address: http://");
  Serial.println(WiFi.localIP());
  
  // Send web page to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Receive an HTTP GET request at <ESP_IP>/get?email_input=<inputMessage>&enable_email_input=<inputMessage2>&threshold_input=<inputMessage3>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    // GET email_input value on <ESP_IP>/get?email_input=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      // GET enable_email_input value on <ESP_IP>/get?enable_email_input=<inputMessage2>
      if (request->hasParam(PARAM_INPUT_2)) {
        inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
        enableEmailChecked = "checked";
      }
      else {
        inputMessage2 = "false";
        enableEmailChecked = "";
      }
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    Serial.println(inputMessage2);
    request->send(200, "text/html", "HTTP GET request sent to your ESP.<br><a href=\"/\">Return to Home Page</a>");
  });
  server.onNotFound(notFound);
  server.begin();

        displayTop = "WiFi Good";
      displayBottom = "Please wait";
      updateDisplay();
      delay(1500);
}

void updateDisplay(){
  if(displayOn){
  // set up wire
  Wire.end();
  Wire.begin(SDA_LCD, SCL_LCD); 
    lcd.init();  
    // clear
    lcd.clear(); 
    delay(5);
    //top row
    lcd.setCursor(0, 0);
    lcd.print(displayTop);
    delay(5);
    //bottom row
    lcd.setCursor(0, 1);
    lcd.print(displayBottom);
    delay(5);
  // wire to move sensor
  Wire.end();
  readyMoveSense = false;
  Wire.begin(SDA_ACC, SCL_ACC);
    readyMove();  
  }
}

void turnOnDisplay(){
  // get display ready
  Wire.end();
  Wire.begin(SDA_LCD, SCL_LCD); 
  lcd.init();
  // turn on  
  lcd.backlight();
  displayOn = true;
  updateDisplay();
  // wire to move sensor
  Wire.end();
  Wire.begin(SDA_ACC, SCL_ACC);
  readyMoveSense = false;
  readyMove();  
}

void turnOffDisplay(){
  // get display ready    
  Wire.end();
  Wire.begin(SDA_LCD, SCL_LCD); 
  lcd.init();
  // turn off    
  lcd.noBacklight();
  displayOn = false;
  // wire to move sensor
  Wire.end();
  Wire.begin(SDA_ACC, SCL_ACC);
  readyMoveSense = false;
  readyMove();  
}

void readyMove(){
  if (mpu.begin(MPU6050_I2CADDR_DEFAULT)) {
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG); // not currently using
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
    readyMoveSense = true;
  }
  else{
    Serial.println("fail to connect to acc meter");  
  }
}

bool checkMovement(){
  sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
  float moveX = a.acceleration.x;
  float moveY = a.acceleration.y;
  float moveZ = a.acceleration.z;
  /* // for debugging print but make it miss keys sometimes
  Serial.print("Acceleration X: ");
    Serial.println(moveX);
    Serial.print("Acceleration y: ");
    Serial.println(moveY);
    Serial.print("Acceleration z: ");
    Serial.println(moveZ);
    Serial.print("The Temp: ");
    Serial.println(getTemp());
    */
   if(moveX > 3 || moveX < -3){
          return true;
    }
  if(moveY > 3 || moveY < -3){
          return true;
    }
  if(moveZ > 13 || moveZ < -13){
          return true;
    }
  return false; 
}

double getTemp(){
  sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
  return (temp.temperature*1.8)+32; 
}

//Get mailbox object
void getMailbox(){
  WiFiClient client;
  HTTPClient http;
  
  String ext = serverName+"api/userBox/"+String(mailboxID)+"/?format=json";
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
    
    mailboxID = doc["id"];
    isLocked = doc["unlocked"];
    const char* temp1 = doc["keyCode"];
    codeS = String(temp1);
    const char* temp2= doc["name"];
    mailboxName = String(temp2);
    ownerID = doc["owner"];
    Serial.println(mailboxName);
  }

  http.end();
}

//Send update to update list
void makeUpdate(){
  WiFiClient client;
  HTTPClient http;
  
  String ext = serverName+"api/updateBox";
  Serial.println("Sending Update");
  http.begin(client,ext);
  http.addHeader("Content-type","application/json");
  http.addHeader("Connection","close");
  String Payload = "{\"opened\":"+String(isLocked)+
                   ",\"moved\":"+String(hasBroken)+
                   ",\"mailbox\":"+String(mailboxID)+"}";
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

//Updates the mailbox state
void updateMailbox(){
  WiFiClient client;
  HTTPClient http;
  
  String ext = serverName+"api/userUpdateBox/"+String(mailboxID)+"/";
  Serial.println("Updating Mailbox");
  http.begin(client,ext);
  http.addHeader("Content-type","application/json");
  http.addHeader("Connection","close");
  String Payload = "{\"unlocked\":"+String(isLocked)+
                   ",\"keyCode\":"+codeS+
                   ",\"name\":\""+mailboxName+
                   "\",\"owner\":\""+String(ownerID)+"\"}";
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
