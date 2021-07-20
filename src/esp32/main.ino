#include <ESP32Servo.h>
#include <WiFi.h>

int statusLedPin = 4;
int codeEnterPin = 3;
int enterKeyPin = 2;
int servoMotorPin = 5;
int lockRotationAngle = 180;
TaskHandle_t motorControllerTaskHandle = NULL;

long long inputCode = 0;
long long passwordCode = 0;
int lockerLockStatus = 0;
int inputCodeStatus = 0;
int enterKeyStatus = 0;
int inputCodeTimer = 0;
int passwordLength = 0;

int motorControlAction = -1;

// To be set
const char* ssid = "";
const char* password = "";
const String uid = "";
const char* host = "";
const int port = 0;

Servo ServoMotor;

void unlockLocker();
void lockLocker();
void resetLocker();
bool isLocked();
bool isReset();
void checkForInput();
void checkForFlush();
void noteInput();
void flushPassword();
void displayIncorrectStatus();
void storePermanent(int);
void retrievePermanent(int);
void sendMotorNotification(int);

// Tasks
void pinsChecker(void *);
void wifiLogger(void *);
void motorController(void *);

void setup() {
  pinMode(statusLedPin, OUTPUT);
  pinMode(codeEnterPin, INPUT);
  pinMode(enterKeyPin, INPUT);
  ServoMotor.attach(servoMotorPin);
  unlockLocker();
  Serial.begin(9600);

  WiFi.begin(ssid, password);
  int i = 0;
  while ( WiFi.status() != WL_CONNECTED ) {
    delay(500);
    Serial.print(".");
    i++;
    if ( i % 10 == 0 ) {
      WiFi.begin(ssid, password);
    }
  }

  delay(10);
  xTaskCreatePinnedToCore(pinsChecker, "Pins Checker", 1000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(wifiLogger, "WiFi Logger", 1000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(motorController, "Motor Controller", 1000, NULL, 2, &motorControllerTaskHandle, 1);
}

void loop() {
}

void pinsChecker(void *_) {
  for (;;) {
    checkForInput();
    checkForFlush();
  }
}

void wifiLogger(void *_) {
  for (;;) {
    WiFiClient client;
    if ( !client.connect(host, port) ) {
      Serial.println("Failed");
    } else {
      client.print(String("STA:") + uid + String(":") + ( isLocked() ? String("1") : String("0") ) + ( isReset() ? String("1") : String("0") ) + String("\n"));
      while ( client.available() == 0 ) {
        delay(500);
      }
      
      String returnCode = client.readStringUntil('\n');
      if ( returnCode != "2" ) {
        sendMotorNotification(2);
      }
    }
  
    client.stop();
    delay(1000);
  }
}

void sendMotorNotification(int motorStatus) {
  if ( motorControlAction == -1 ) {
    motorControlAction = motorStatus;
    xTaskNotifyGive( motorControllerTaskHandle );
  } else if ( motorControlAction != motorStatus ) {
    motorControlAction = motorStatus;
  }
}

void motorController(void *_) {
  for (;;) {
    while ( ulTaskNotifyTake( pdTRUE, pdMS_TO_TICKS(1000) ) != 0 );
    int motorAction = motorControlAction;
    motorControlAction = -1;
    switch ( motorAction ) {
      case 0:
        unlockLocker();
        break;
      case 1:
        lockLocker();
        break;
      case 2:
        resetLocker();
        break;
    }
  }
}

void displayIncorrectStatus() {
  digitalWrite(statusLedPin, HIGH);
  delay(300);
  digitalWrite(statusLedPin, LOW);
  delay(300);
  digitalWrite(statusLedPin, HIGH);
  delay(300);
  digitalWrite(statusLedPin, LOW);
}

void checkForFlush() {
  if ( digitalRead(enterKeyPin) == HIGH ) {
    if ( enterKeyStatus == 0 ) {
      enterKeyStatus = 1;
    }
  } else {
    if ( enterKeyStatus != 0 ) {
      enterKeyStatus = 0;
      flushPassword();
    }
  }
}

void flushPassword() {
  if ( isLocked() ) {
    if ( passwordCode == inputCode ) {
      sendMotorNotification(0);
    } else {
      displayIncorrectStatus();
    }
  } else {
    if ( isReset() && inputCode == 0 ) {
      displayIncorrectStatus();
    } else {
      if ( inputCode != 0 ) {
        passwordCode = inputCode; 
      }
      sendMotorNotification(1);
    }
  }
  inputCode = 0;
  passwordLength = 0;
}

void checkForInput() {
  if ( digitalRead(codeEnterPin) == HIGH ) {
    if ( inputCodeStatus == 0 ) {
      inputCodeTimer = millis();
      inputCodeStatus = 1;
    }
  } else {
    if ( inputCodeStatus != 0 ) {
      inputCodeTimer = millis() - inputCodeTimer;
      noteInput();
      inputCodeStatus = 0;
      inputCodeTimer = 0;
    }
  }
}

void noteInput() {
  if ( inputCodeTimer <= 0 ) {
    return;
  }
  passwordLength++;
  inputCode |= 1 << (2*passwordLength - 1);
  if ( inputCodeTimer > 2000 ) {
    inputCode |= 1 << (2*passwordLength - 2);
  }
}

bool isLocked() {
  if ( lockerLockStatus == 0 ) {
    return false;
  } else {
    return true;
  }
}

bool isReset() {
  if ( passwordCode == 0 ) {
    return true;
  } else {
    return false;
  }
}

void unlockLocker() {
  ServoMotor.write(lockRotationAngle);
  lockerLockStatus = 0;
}

void lockLocker() {
  ServoMotor.write(0);
  lockerLockStatus = 1;
}

void resetLocker() {
  passwordCode = 0;
  unlockLocker();
}