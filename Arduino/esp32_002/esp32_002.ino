#include <Arduino.h>
//ESP32
#include <WiFi.h>
#define RF_RECEIVER 13
#define AUTOMATIC_MODE 4
#define RELAY_PIN_2 14


#include <Espalexa.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define SERIAL_BAUDRATE 115200

//Wifi Configuration
String WIFI_SSID = "SSID";
String WIFI_PASS = "PASSWORD";

//Servos Configuration
Adafruit_PWMServoDriver servos_1 = Adafruit_PWMServoDriver(0x41);
Adafruit_PWMServoDriver servos_2 = Adafruit_PWMServoDriver(0x40);

unsigned int pos0 = 172;    // ancho de pulso en cuentas para posición 0°
unsigned int pos180 = 565;  // ancho de pulso en cuentas para la posicin 180°

#define COXIS_1 0
#define HEAD_1 2
#define ARM_R_1 8
#define HAND_R_1 6
#define ARM_L_1 4
#define HAND_L_1 10

#define COXIS_2 0
#define HEAD_2 2
#define ARM_R_2 4
#define HAND_R_2 6
#define ARM_L_2 8
#define HAND_L_2 10

//Dispositivos Alexa
#define CALS "CALAVERAS"
#define CAL_1 "CALAVERA 1"
#define CAL_2 "CALAVERA 2"
#define CONNECTION_TIMEOUT 50

//Rutinas
bool Rut_1 = false;
bool Rut_2 = false;
bool Rut_3 = false;
bool Rut_4 = false;
bool Rut_5 = false;
bool Rut_6 = false;
bool Rut_7 = false;
bool Rut_8 = false;
bool Rut_9 = false;
bool Rut_10 = false;

//Global Variables
int step = 0;
bool masterEnable = true;
bool enable = false;
bool isPWNActive = false;
int timerInit = 0;
int timerElapsed = 1000;
int currentRutine = 0;

//Callbacks Alexa
void calaveras(uint8_t brightness);
Espalexa espalexa;
EspalexaDevice* deviceAlexa;

void calaveras(uint8_t brightness) {
  //brightness parameter contains the new device state (0:off,255:on,1-254:dimmed)
  Serial.println(CALS);
  //Serial.println(brightness);
  int rutine = map(brightness, 0, 255, 0, 10);
  currentRutine = rutine;
  Serial.println(rutine);
  enable = rutine > 0;
  step = 0;

  Rut_1 = false;
  Rut_2 = false;
  Rut_3 = false;
  Rut_4 = false;
  Rut_5 = false;
  Rut_6 = false;
  Rut_7 = false;
  Rut_8 = false;
  Rut_9 = false;
  Rut_10 = false;

  switch (rutine) {
    case 1:
      Rut_1 = true;
      break;
    case 2:
      Rut_2 = true;
      break;
    case 3:
      Rut_3 = true;
      break;
    case 4:
      Rut_4 = true;
      break;
    case 5:
      Rut_5 = true;
      break;
    case 6:
      Rut_6 = true;
      break;
    case 7:
      Rut_7 = true;
      break;
    case 8:
      Rut_8 = true;
      break;
    case 9:
      Rut_9 = true;
      break;
    case 10:
      Rut_10 = true;
      break;
  }
}


void setup() {
  pinMode(AUTOMATIC_MODE, INPUT);
  // Init serial port and clean garbage
  Serial.begin(SERIAL_BAUDRATE);
  Serial.println();
  delay(1000);

  if (true) {
    // Wi-Fi connection
    wifiSetup();
    delay(1000);

    //Alexa Configuration
    deviceAlexa = new EspalexaDevice(CALS, calaveras);
    espalexa.addDevice(deviceAlexa);
    espalexa.begin();
  }

  //Servos Connection
  servos_1.begin();
  servos_1.setPWMFreq(50);  //Frecuecia PWM de 50Hz o T=16,66ms

  servos_2.begin();
  servos_2.setPWMFreq(50);
  ResetServos();
}

void loop() {
  if (digitalRead(AUTOMATIC_MODE)) {
    Rut_9 = true;
    enable = true;
  }
  espalexa.loop();
  if (enable) {
    if (timerInit == 0) {
      timerInit = millis();
    }
    if (millis() - timerInit >= timerElapsed) {

      CallRutine(step);
      step++;
      timerInit = millis();
    }
  } else if (step == 0) {
    ResetServos();
  }
  if (Serial.available() > 0) {
    // read the incoming byte:
    String incomingByte = "";
    incomingByte = Serial.readString();

    // say what you got:
    Serial.print("I received: ");
    Serial.println(incomingByte);
    if (incomingByte == "1") {
      Serial.println("Rutine 1");
      Rut_1 = true;
      step = 0;
      enable = true;
      //Saludo_2();
      //ResetServos();
    } else if (incomingByte == "8") {
      Serial.println("Rutine 8");
      Rut_8 = true;
      step = 0;
      enable = true;
      //Saludo_2();
      //ResetServos();
    } else if (incomingByte == "0") {
      ResetServos();
      Rut_1 = false;
      Rut_2 = false;
      Rut_3 = false;
      Rut_4 = false;
      Rut_5 = false;
      Rut_6 = false;
      Rut_7 = false;
      Rut_8 = false;
      Rut_9 = false;
      Rut_10 = false;
      step = 0;
      enable = false;
    }
  }
}


// Wi-Fi Connection
void wifiSetup() {
  //WiFi.disconnect();
  // Set WIFI module to STA mode
  WiFi.mode(WIFI_STA);
  WiFi.enableSTA(true);

  // Connect
  Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
  Serial.printf("@  %s ", WIFI_PASS);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int timeout_counter = 0;
  int status = WL_IDLE_STATUS;
  // Wait
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    status = WiFi.status();
    Serial.println(get_wifi_status(status));
    delay(250);
    timeout_counter++;
    if (timeout_counter >= CONNECTION_TIMEOUT * 5) {
      ESP.restart();
    }
  }
  Serial.println();

  // Connected!
  Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
}

String get_wifi_status(int status) {
  switch (status) {
    case WL_IDLE_STATUS:
      return "WL_IDLE_STATUS";
    case WL_SCAN_COMPLETED:
      return "WL_SCAN_COMPLETED";
    case WL_NO_SSID_AVAIL:
      return "WL_NO_SSID_AVAIL";
    case WL_CONNECT_FAILED:
      return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST:
      return "WL_CONNECTION_LOST";
    case WL_CONNECTED:
      return "WL_CONNECTED";
    case WL_DISCONNECTED:
      return "WL_DISCONNECTED";
  }
}

void CallRutine(int step) {
  if (Rut_1) {
    Rutine1(step);
  } else if (Rut_2) {
    Rutine2(step);
  } else if (Rut_3) {
    Rutine3(step);
  } else if (Rut_4) {
    Rutine4(step);
  } else if (Rut_5) {
    Rutine5(step);
  } else if (Rut_6) {
    Rutine6(step);
  } else if (Rut_7) {
    Rutine7(step);
  } else if (Rut_8) {
    Rutine8(step);
  } else if (Rut_9) {
    Rutine9(step);
  } else if (Rut_10) {
    Rutine10(step);
  }
}

//1. Thriller
void Rutine1(int stepInit) {
  Serial.print("Rutina Actual 1 / Step: ");
  Serial.println(stepInit);
  timerElapsed = 600;
  int size = 20;
  int duration = 60;
  int delay = 4;
  int step = stepInit - delay;

  if (step >= duration | step < 0)
    return;

  //Posiciones
  int xCOXIS_1[] = { 85, -1, -1, 80, 90, 80, 90, 80, 90, 80, 90, 80, 90, 80, 90, 80, -1, 85, -1, -1 };    // 60 a 120. Reposo: 85
  int xHEAD_1[] = { 110, -1, 90, -1, 110, -1, 90, -1, 110, -1, 90, -1, -1, -1, -1, -1, -1, -1, -1, 90 };  // 75 a 120. Reposo: 90
  int xARM_R_1[] = { 30, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };    // 30 a 135. Reposo: 135
  int xHAND_R_1[] = { 0, -1, 50, -1, 0, -1, 50, -1, 0, -1, 50, -1, 0, -1, -1, -1, -1, -1, -1, -1 };       // 00 a 80. Reposo: 75
  int xARM_L_1[] = { 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 10 };    // 10 a 130. Reposo: 10
  int xHAND_L_1[] = { 55, -1, 80, -1, 55, -1, 80, -1, 55, -1, 80, -1, 55, -1, -1, -1, -1, -1, -1, -1 };   // 00 a 80. Reposo: 00

  int xCOXIS_2[] = { 90, -1, -1, 85, 90, 85, 90, 85, 90, 85, 90, 85, 90, 85, 90, 85, -1, 90, -1, -1 };          //85 a 95. Reposo:90
  int xHEAD_2[] = { 90, -1, 75, -1, 90, -1, 75, -1, 90, -1, 75, -1, -1, -1, -1, -1, -1, -1, -1, 75 };           //60 a 90. Reposo: 75
  int xARM_R_2[] = { 170, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };         //90 a 180. Reposo: 180
  int xHAND_R_2[] = { 0, -1, 50, -1, 0, -1, 50, -1, 0, -1, 50, -1, 0, -1, -1, -1, -1, -1, -1, -1 };             // 0 a 80. Reposo:80
  int xARM_L_2[] = { 90, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };          // 0 a 90. Reposo: 0
  int xHAND_L_2[] = { 130, -1, 150, -1, 130, -1, 150, -1, 130, -1, 150, -1, 130, -1, -1, -1, -1, -1, -1, -1 };  //70 a 150 Reposo: 70

  //COXIS_1
  if (step % size < (sizeof(xCOXIS_1) / sizeof(xCOXIS_1[0]))) {
    Serial.print("xCOXIS_1: ");
    Serial.println(xCOXIS_1[step % size]);
    setServo1(COXIS_1, xCOXIS_1[step % size]);
  }
  //HEAD_1
  if (step % size < (sizeof(xHEAD_1) / sizeof(xHEAD_1[0]))) {
    Serial.print("xHEAD_1: ");
    Serial.println(xHEAD_1[step % size]);
    setServo1(HEAD_1, xHEAD_1[step % size]);
  }
  //ARM_R_1
  if (step % size < (sizeof(xARM_R_1) / sizeof(xARM_R_1[0]))) {
    Serial.print("xARM_R_1: ");
    Serial.println(xARM_R_1[step % size]);
    setServo1(ARM_R_1, xARM_R_1[step % size]);
  }
  //HAND_R_1
  if (step % size < (sizeof(xHAND_R_1) / sizeof(xHAND_R_1[0]))) {
    Serial.print("xHAND_R_1: ");
    Serial.println(xHAND_R_1[step % size]);
    setServo1(HAND_R_1, xHAND_R_1[step % size]);
  }
  //ARM_L_1
  if (step % size < (sizeof(xARM_L_1) / sizeof(xARM_L_1[0]))) {
    Serial.print("xARM_L_1: ");
    Serial.println(xARM_L_1[step % size]);
    setServo1(ARM_L_1, xARM_L_1[step % size]);
  }
  //HAND_L_1
  if (step % size < (sizeof(xHAND_L_1) / sizeof(xHAND_L_1[0]))) {
    Serial.print("xHAND_L_1: ");
    Serial.println(xHAND_L_1[step % size]);
    setServo1(HAND_L_1, xHAND_L_1[step % size]);
  }

  //COXIS_2
  if (step % size < (sizeof(xCOXIS_2) / sizeof(xCOXIS_2[0]))) {
    Serial.print("xCOXIS_2: ");
    Serial.println(xCOXIS_2[step % size]);
    setServo2(COXIS_2, xCOXIS_2[step % size]);
  }
  //HEAD_2
  if (step % size < (sizeof(xHEAD_2) / sizeof(xHEAD_2[0]))) {
    Serial.print("xHEAD_2: ");
    Serial.println(xHEAD_2[step % size]);
    setServo2(HEAD_2, xHEAD_2[step % size]);
  }
  //ARM_R_2
  if (step % size < (sizeof(xARM_R_2) / sizeof(xARM_R_2[0]))) {
    Serial.print("xARM_R_2: ");
    Serial.println(xARM_R_2[step % size]);
    setServo2(ARM_R_2, xARM_R_2[step % size]);
  }
  //HAND_R_2
  if (step % size < (sizeof(xHAND_R_2) / sizeof(xHAND_R_2[0]))) {
    Serial.print("xHAND_R_2: ");
    Serial.println(xHAND_R_2[step % size]);
    setServo2(HAND_R_2, xHAND_R_2[step % size]);
  }
  //ARM_L_2
  if (step % size < (sizeof(xARM_L_2) / sizeof(xARM_L_2[0]))) {
    Serial.print("xARM_L_2: ");
    Serial.println(xARM_L_2[step % size]);
    setServo2(ARM_L_2, xARM_L_2[step % size]);
  }
  //HAND_L_2
  if (step % size < (sizeof(xHAND_L_2) / sizeof(xHAND_L_2[0]))) {
    Serial.print("xHAND_L_2: ");
    Serial.println(xHAND_L_2[step % size]);
    setServo2(HAND_L_2, xHAND_L_2[step % size]);
  }
}

//2. Sweet Child O'Mine
void Rutine2(int stepInit) {
  Serial.print("Rutina Actual 2 / Step: ");
  Serial.println(stepInit);
  timerElapsed = 600;
  int size = 20;
  int duration = 60;
  int delay = 4;
  int step = stepInit - delay;

  if (step >= duration | step < 0)
    return;
  //Posiciones
  int xCOXIS_1[] = { 85, 90, 95, 100, 95, 90, 85, 80, 75, 70, 75, 80, 85, 90, 95, 100, 95, 90, 80, 85 };    //60 a 120. Reposo: 85
  int xHEAD_1[] = { 90, 95, 100, 95, 90, 85, 80, 75, 70, 75, 80, 85, 90, 95, 100, 95, 90, 80, 85, 90 };     //75 a 120. Reposo: 90
  int xARM_R_1[] = { 100, -1, -1, 75, -1, -1, 100, -1, -1, 75, -1, -1, 100, -1, -1, 75, -1, -1, 135, -1 };  // 30 a 135. Reposo: 135.
  int xHAND_R_1[] = { -1, 10, 50, -1, 10, 50, -1, 10, 50, -1, 10, 50, -1, 10, 50, -1, 75 - 1, -1, -1 };     // 00 a 80. Reposo: 75
  int xARM_L_1[] = { 100, -1, 60, -1, -1, 100, -1, -1, 60, -1, -1, 100, -1, -1, 60, -1, -1, 50, -1, -1 };   // 10 a 130. Reposo: 10
  int xHAND_L_1[] = { 80, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };     // 00 a 80. Reposo: 00

  int xCOXIS_2[] = { 89, 88, 87, 86, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 94, 93, 92, 91, 90 };   //85 a 95. Reposo:90
  int xHEAD_2[] = { 75, 80, 85, 90, 95, 90, 85, 80, 75, 70, 65, 60, 65, 70, 75, 80, 85, 90, 85, 75 };    //60 a 90. Reposo: 75
  int xARM_R_2[] = { 180, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };  //90 a 180. Reposo: 180
  int xHAND_R_2[] = { 80, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };  // 0 a 80. Reposo:80
  int xARM_L_2[] = { 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };    // 0 a 90. Reposo: 0
  int xHAND_L_2[] = { 70, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };  //70 a 150 Reposo: 70

  //COXIS_1
  if (step % size < (sizeof(xCOXIS_1) / sizeof(xCOXIS_1[0]))) {
    Serial.print("xCOXIS_1: ");
    Serial.println(xCOXIS_1[step % size]);
    setServo1(COXIS_1, xCOXIS_1[step % size]);
  }
  //HEAD_1
  if (step % size < (sizeof(xHEAD_1) / sizeof(xHEAD_1[0]))) {
    Serial.print("xHEAD_1: ");
    Serial.println(xHEAD_1[step % size]);
    setServo1(HEAD_1, xHEAD_1[step % size]);
  }
  //ARM_R_1
  if (step % size < (sizeof(xARM_R_1) / sizeof(xARM_R_1[0]))) {
    Serial.print("xARM_R_1: ");
    Serial.println(xARM_R_1[step % size]);
    setServo1(ARM_R_1, xARM_R_1[step % size]);
  }
  //HAND_R_1
  if (step % size < (sizeof(xHAND_R_1) / sizeof(xHAND_R_1[0]))) {
    Serial.print("xHAND_R_1: ");
    Serial.println(xHAND_R_1[step % size]);
    setServo1(HAND_R_1, xHAND_R_1[step % size]);
  }
  //ARM_L_1
  if (step % size < (sizeof(xARM_L_1) / sizeof(xARM_L_1[0]))) {
    Serial.print("xARM_L_1: ");
    Serial.println(xARM_L_1[step % size]);
    setServo1(ARM_L_1, xARM_L_1[step % size]);
  }
  //HAND_L_1
  if (step % size < (sizeof(xHAND_L_1) / sizeof(xHAND_L_1[0]))) {
    Serial.print("xHAND_L_1: ");
    Serial.println(xHAND_L_1[step % size]);
    setServo1(HAND_L_1, xHAND_L_1[step % size]);
  }

  //COXIS_2
  if (step % size < (sizeof(xCOXIS_2) / sizeof(xCOXIS_2[0]))) {
    Serial.print("xCOXIS_2: ");
    Serial.println(xCOXIS_2[step % size]);
    setServo2(COXIS_2, xCOXIS_2[step % size]);
  }
  //HEAD_2
  if (step % size < (sizeof(xHEAD_2) / sizeof(xHEAD_2[0]))) {
    Serial.print("xHEAD_2: ");
    Serial.println(xHEAD_2[step % size]);
    setServo2(HEAD_2, xHEAD_2[step % size]);
  }
  //ARM_R_2
  if (step % size < (sizeof(xARM_R_2) / sizeof(xARM_R_2[0]))) {
    Serial.print("xARM_R_2: ");
    Serial.println(xARM_R_2[step % size]);
    setServo2(ARM_R_2, xARM_R_2[step % size]);
  }
  //HAND_R_2
  if (step % size < (sizeof(xHAND_R_2) / sizeof(xHAND_R_2[0]))) {
    Serial.print("xHAND_R_2: ");
    Serial.println(xHAND_R_2[step % size]);
    setServo2(HAND_R_2, xHAND_R_2[step % size]);
  }
  //ARM_L_2
  if (step % size < (sizeof(xARM_L_2) / sizeof(xARM_L_2[0]))) {
    Serial.print("xARM_L_2: ");
    Serial.println(xARM_L_2[step % size]);
    setServo2(ARM_L_2, xARM_L_2[step % size]);
  }
  //HAND_L_2
  if (step % size < (sizeof(xHAND_L_2) / sizeof(xHAND_L_2[0]))) {
    Serial.print("xHAND_L_2: ");
    Serial.println(xHAND_L_2[step % size]);
    setServo2(HAND_L_2, xHAND_L_2[step % size]);
  }
}

//3.Mambo No 8
void Rutine3(int stepInit) {
  Serial.print("Rutina Actual 3 / Step: ");
  Serial.println(stepInit);
  timerElapsed = 600;
  int size = 20;
  int duration = 60;
  int delay = 7;
  int step = stepInit - delay;

  if (step >= duration | step < 0)
    return;
  //Posiciones
  int xCOXIS_1[] = { 85, -1, -1, -1, -1, -1, -1, -1, -1, 80, 95, 85, 95, 85, 95, 85, 95, -1, 85, -1 };    //60 a 120. Reposo: 85
  int xHEAD_1[] = { 90, -1, -1, -1, -1, -1, -1, -1, -1, -1, 80, -1, 105, -1, 80, -1, 105, -1, 90, -1 };   //75 a 120. Reposo: 90
  int xARM_R_1[] = { 100, -1, -1, -1, -1, -1, -1, -1, -1, 60, -1, -1, -1, -1, -1, -1, -1, -1, 135, -1 };  // 30 a 135. Reposo: 135.
  int xHAND_R_1[] = { 75, -1, -1, -1, 50, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 75, -1 };   // 00 a 80. Reposo: 75
  int xARM_L_1[] = { 10, -1, 40, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 10, -1 };    // 10 a 130. Reposo: 10
  int xHAND_L_1[] = { 0, -1, -1, -1, -1, -1, 30, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, -1 };     // 00 a 80. Reposo: 00

  int xCOXIS_2[] = { 90, -1, -1, -1, -1, -1, -1, -1, -1, 95, 85, 95, 85, 95, 85, 95, 85, -1, 90, -1 };      //85 a 95. Reposo:90
  int xHEAD_2[] = { 75, -1, -1, -1, -1, -1, -1, -1, -1, -1, 90, -1, 60, -1, 90, -1, 60, -1, 75, -1 };       //60 a 90. Reposo: 75
  int xARM_R_2[] = { 180, 150, -1, -1, -1, -1, -1, -1, -1, 110, -1, -1, -1, -1, -1, -1, -1, -1, 180, -1 };  //90 a 180. Reposo: 180
  int xHAND_R_2[] = { 80, -1, -1, -1, -1, 55, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 80, -1 };     // 0 a 80. Reposo:80
  int xARM_L_2[] = { 0, -1, -1, 30, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, -1 };        // 0 a 90. Reposo: 0
  int xHAND_L_2[] = { 70, -1, -1, -1, -1, -1, -1, 100, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 70, -1 };    //70 a 150 Reposo: 70

  //COXIS_1
  if (step % size < (sizeof(xCOXIS_1) / sizeof(xCOXIS_1[0]))) {
    Serial.print("xCOXIS_1: ");
    Serial.println(xCOXIS_1[step % size]);
    setServo1(COXIS_1, xCOXIS_1[step % size]);
  }
  //HEAD_1
  if (step % size < (sizeof(xHEAD_1) / sizeof(xHEAD_1[0]))) {
    Serial.print("xHEAD_1: ");
    Serial.println(xHEAD_1[step % size]);
    setServo1(HEAD_1, xHEAD_1[step % size]);
  }
  //ARM_R_1
  if (step % size < (sizeof(xARM_R_1) / sizeof(xARM_R_1[0]))) {
    Serial.print("xARM_R_1: ");
    Serial.println(xARM_R_1[step % size]);
    setServo1(ARM_R_1, xARM_R_1[step % size]);
  }
  //HAND_R_1
  if (step % size < (sizeof(xHAND_R_1) / sizeof(xHAND_R_1[0]))) {
    Serial.print("xHAND_R_1: ");
    Serial.println(xHAND_R_1[step % size]);
    setServo1(HAND_R_1, xHAND_R_1[step % size]);
  }
  //ARM_L_1
  if (step % size < (sizeof(xARM_L_1) / sizeof(xARM_L_1[0]))) {
    Serial.print("xARM_L_1: ");
    Serial.println(xARM_L_1[step % size]);
    setServo1(ARM_L_1, xARM_L_1[step % size]);
  }
  //HAND_L_1
  if (step % size < (sizeof(xHAND_L_1) / sizeof(xHAND_L_1[0]))) {
    Serial.print("xHAND_L_1: ");
    Serial.println(xHAND_L_1[step % size]);
    setServo1(HAND_L_1, xHAND_L_1[step % size]);
  }

  //COXIS_2
  if (step % size < (sizeof(xCOXIS_2) / sizeof(xCOXIS_2[0]))) {
    Serial.print("xCOXIS_2: ");
    Serial.println(xCOXIS_2[step % size]);
    setServo2(COXIS_2, xCOXIS_2[step % size]);
  }
  //HEAD_2
  if (step % size < (sizeof(xHEAD_2) / sizeof(xHEAD_2[0]))) {
    Serial.print("xHEAD_2: ");
    Serial.println(xHEAD_2[step % size]);
    setServo2(HEAD_2, xHEAD_2[step % size]);
  }
  //ARM_R_2
  if (step % size < (sizeof(xARM_R_2) / sizeof(xARM_R_2[0]))) {
    Serial.print("xARM_R_2: ");
    Serial.println(xARM_R_2[step % size]);
    setServo2(ARM_R_2, xARM_R_2[step % size]);
  }
  //HAND_R_2
  if (step % size < (sizeof(xHAND_R_2) / sizeof(xHAND_R_2[0]))) {
    Serial.print("xHAND_R_2: ");
    Serial.println(xHAND_R_2[step % size]);
    setServo2(HAND_R_2, xHAND_R_2[step % size]);
  }
  //ARM_L_2
  if (step % size < (sizeof(xARM_L_2) / sizeof(xARM_L_2[0]))) {
    Serial.print("xARM_L_2: ");
    Serial.println(xARM_L_2[step % size]);
    setServo2(ARM_L_2, xARM_L_2[step % size]);
  }
  //HAND_L_2
  if (step % size < (sizeof(xHAND_L_2) / sizeof(xHAND_L_2[0]))) {
    Serial.print("xHAND_L_2: ");
    Serial.println(xHAND_L_2[step % size]);
    setServo2(HAND_L_2, xHAND_L_2[step % size]);
  }
}
//4. En el 2000
void Rutine4(int stepInit) {
  Serial.print("Rutina Actual 4 / Step: ");
  Serial.println(stepInit);
  timerElapsed = 600;
  int size = 20;
  int duration = 60;
  int delay = 5;
  int step = stepInit - delay;

  if (step >= duration | step < 0)
    return;

  //Posiciones
  int xCOXIS_1[] = { 85, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 85 };    //60 a 120. Reposo: 85
  int xHEAD_1[] = { 90, 95, 100, 95, 90, 85, 80, 75, 70, 75, 80, 85, 90, 95, 100, 95, 90, 80, 85, 90 };   //75 a 120. Reposo: 90
  int xARM_R_1[] = { 135, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 135 };  // 30 a 135. Reposo: 135.
  int xHAND_R_1[] = { 75, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 75 };   // 00 a 80. Reposo: 75
  int xARM_L_1[] = { 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 10 };    // 10 a 130. Reposo: 10
  int xHAND_L_1[] = { 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0 };     // 00 a 80. Reposo: 00

  int xCOXIS_2[] = { 90, 85, -1, 95, -1, 85, -1, 95, -1, 85, -1, 95, -1, -1, -1, -1, -1, -1, -1, 90 };       //85 a 95. Reposo:90
  int xHEAD_2[] = { 75, 60, 90, 60, 90, 60, 90, 60, 90, 60, 90, -1, -1, -1, -1, -1, -1, -1, -1, 75 };        //60 a 90. Reposo: 75
  int xARM_R_2[] = { 90, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 180 };      //90 a 180. Reposo: 180
  int xHAND_R_2[] = { 30, -1, 80, -1, 30, -1, 80, -1, 30, -1, 80, -1, 30, -1, 80, -1, 30, -1, -1, 80 };      // 0 a 80. Reposo:80
  int xARM_L_2[] = { 90, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0 };        // 0 a 90. Reposo: 0
  int xHAND_L_2[] = { 70, -1, 130, -1, 70, -1, 130, -1, 70, -1, 130, -1, 70, -1, 130, -1, 70, -1, -1, 70 };  //70 a 150 Reposo: 70

  //COXIS_1
  if (step % size < (sizeof(xCOXIS_1) / sizeof(xCOXIS_1[0]))) {
    Serial.print("xCOXIS_1: ");
    Serial.println(xCOXIS_1[step % size]);
    setServo1(COXIS_1, xCOXIS_1[step % size]);
  }
  //HEAD_1
  if (step % size < (sizeof(xHEAD_1) / sizeof(xHEAD_1[0]))) {
    Serial.print("xHEAD_1: ");
    Serial.println(xHEAD_1[step % size]);
    setServo1(HEAD_1, xHEAD_1[step % size]);
  }
  //ARM_R_1
  if (step % size < (sizeof(xARM_R_1) / sizeof(xARM_R_1[0]))) {
    Serial.print("xARM_R_1: ");
    Serial.println(xARM_R_1[step % size]);
    setServo1(ARM_R_1, xARM_R_1[step % size]);
  }
  //HAND_R_1
  if (step % size < (sizeof(xHAND_R_1) / sizeof(xHAND_R_1[0]))) {
    Serial.print("xHAND_R_1: ");
    Serial.println(xHAND_R_1[step % size]);
    setServo1(HAND_R_1, xHAND_R_1[step % size]);
  }
  //ARM_L_1
  if (step % size < (sizeof(xARM_L_1) / sizeof(xARM_L_1[0]))) {
    Serial.print("xARM_L_1: ");
    Serial.println(xARM_L_1[step % size]);
    setServo1(ARM_L_1, xARM_L_1[step % size]);
  }
  //HAND_L_1
  if (step % size < (sizeof(xHAND_L_1) / sizeof(xHAND_L_1[0]))) {
    Serial.print("xHAND_L_1: ");
    Serial.println(xHAND_L_1[step % size]);
    setServo1(HAND_L_1, xHAND_L_1[step % size]);
  }

  //COXIS_2
  if (step % size < (sizeof(xCOXIS_2) / sizeof(xCOXIS_2[0]))) {
    Serial.print("xCOXIS_2: ");
    Serial.println(xCOXIS_2[step % size]);
    setServo2(COXIS_2, xCOXIS_2[step % size]);
  }
  //HEAD_2
  if (step % size < (sizeof(xHEAD_2) / sizeof(xHEAD_2[0]))) {
    Serial.print("xHEAD_2: ");
    Serial.println(xHEAD_2[step % size]);
    setServo2(HEAD_2, xHEAD_2[step % size]);
  }
  //ARM_R_2
  if (step % size < (sizeof(xARM_R_2) / sizeof(xARM_R_2[0]))) {
    Serial.print("xARM_R_2: ");
    Serial.println(xARM_R_2[step % size]);
    setServo2(ARM_R_2, xARM_R_2[step % size]);
  }
  //HAND_R_2
  if (step % size < (sizeof(xHAND_R_2) / sizeof(xHAND_R_2[0]))) {
    Serial.print("xHAND_R_2: ");
    Serial.println(xHAND_R_2[step % size]);
    setServo2(HAND_R_2, xHAND_R_2[step % size]);
  }
  //ARM_L_2
  if (step % size < (sizeof(xARM_L_2) / sizeof(xARM_L_2[0]))) {
    Serial.print("xARM_L_2: ");
    Serial.println(xARM_L_2[step % size]);
    setServo2(ARM_L_2, xARM_L_2[step % size]);
  }
  //HAND_L_2
  if (step % size < (sizeof(xHAND_L_2) / sizeof(xHAND_L_2[0]))) {
    Serial.print("xHAND_L_2: ");
    Serial.println(xHAND_L_2[step % size]);
    setServo2(HAND_L_2, xHAND_L_2[step % size]);
  }
}
void Rutine5(int stepInit) {
  Serial.print("Rutina Actual 5 / Step: ");
  Serial.println(stepInit);
  timerElapsed = 400;
  int size = 20;
  int duration = 80;
  int delay = 5;
  int step = stepInit - delay;

  if (step >= duration | step < 0)
    return;
  //Posiciones
  int xCOXIS_1[] = { 85, 90, 95, 100, 95, 90, 85, 80, 75, 70, 75, 80, 85, 90, 95, 100, 95, 90, 80, 85 };    //60 a 120. Reposo: 85
  int xHEAD_1[] = { 90, 95, 100, 95, 90, 85, 80, 75, 70, 75, 80, 85, 90, 95, 100, 95, 90, 80, 85, 90 };     //75 a 120. Reposo: 90
  int xARM_R_1[] = { 100, -1, -1, 75, -1, -1, 100, -1, -1, 75, -1, -1, 100, -1, -1, 75, -1, -1, 135, -1 };  // 30 a 135. Reposo: 135.
  int xHAND_R_1[] = { -1, 10, 50, -1, 10, 50, -1, 10, 50, -1, 10, 50, -1, 10, 50, -1, 75 - 1, -1, -1 };     // 00 a 80. Reposo: 75
  int xARM_L_1[] = { 100, -1, 60, -1, -1, 100, -1, -1, 60, -1, -1, 100, -1, -1, 60, -1, -1, 50, -1, -1 };   // 10 a 130. Reposo: 10
  int xHAND_L_1[] = { 80, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };     // 00 a 80. Reposo: 00

  int xCOXIS_2[] = { 90, 85, -1, 95, -1, 85, -1, 95, -1, 85, -1, 95, -1, -1, -1, -1, -1, -1, -1, 90 };       //85 a 95. Reposo:90
  int xHEAD_2[] = { 75, 60, 90, 60, 90, 60, 90, 60, 90, 60, 90, -1, -1, -1, -1, -1, -1, -1, -1, 75 };        //60 a 90. Reposo: 75
  int xARM_R_2[] = { 90, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 180 };      //90 a 180. Reposo: 180
  int xHAND_R_2[] = { 30, -1, 80, -1, 30, -1, 80, -1, 30, -1, 80, -1, 30, -1, 80, -1, 30, -1, -1, 80 };      // 0 a 80. Reposo:80
  int xARM_L_2[] = { 90, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0 };        // 0 a 90. Reposo: 0
  int xHAND_L_2[] = { 70, -1, 130, -1, 70, -1, 130, -1, 70, -1, 130, -1, 70, -1, 130, -1, 70, -1, -1, 70 };  //70 a 150 Reposo: 70

  //COXIS_1
  if (step % size < (sizeof(xCOXIS_1) / sizeof(xCOXIS_1[0]))) {
    Serial.print("xCOXIS_1: ");
    Serial.println(xCOXIS_1[step % size]);
    setServo1(COXIS_1, xCOXIS_1[step % size]);
  }
  //HEAD_1
  if (step % size < (sizeof(xHEAD_1) / sizeof(xHEAD_1[0]))) {
    Serial.print("xHEAD_1: ");
    Serial.println(xHEAD_1[step % size]);
    setServo1(HEAD_1, xHEAD_1[step % size]);
  }
  //ARM_R_1
  if (step % size < (sizeof(xARM_R_1) / sizeof(xARM_R_1[0]))) {
    Serial.print("xARM_R_1: ");
    Serial.println(xARM_R_1[step % size]);
    setServo1(ARM_R_1, xARM_R_1[step % size]);
  }
  //HAND_R_1
  if (step % size < (sizeof(xHAND_R_1) / sizeof(xHAND_R_1[0]))) {
    Serial.print("xHAND_R_1: ");
    Serial.println(xHAND_R_1[step % size]);
    setServo1(HAND_R_1, xHAND_R_1[step % size]);
  }
  //ARM_L_1
  if (step % size < (sizeof(xARM_L_1) / sizeof(xARM_L_1[0]))) {
    Serial.print("xARM_L_1: ");
    Serial.println(xARM_L_1[step % size]);
    setServo1(ARM_L_1, xARM_L_1[step % size]);
  }
  //HAND_L_1
  if (step % size < (sizeof(xHAND_L_1) / sizeof(xHAND_L_1[0]))) {
    Serial.print("xHAND_L_1: ");
    Serial.println(xHAND_L_1[step % size]);
    setServo1(HAND_L_1, xHAND_L_1[step % size]);
  }

  //COXIS_2
  if (step % size < (sizeof(xCOXIS_2) / sizeof(xCOXIS_2[0]))) {
    Serial.print("xCOXIS_2: ");
    Serial.println(xCOXIS_2[step % size]);
    setServo2(COXIS_2, xCOXIS_2[step % size]);
  }
  //HEAD_2
  if (step % size < (sizeof(xHEAD_2) / sizeof(xHEAD_2[0]))) {
    Serial.print("xHEAD_2: ");
    Serial.println(xHEAD_2[step % size]);
    setServo2(HEAD_2, xHEAD_2[step % size]);
  }
  //ARM_R_2
  if (step % size < (sizeof(xARM_R_2) / sizeof(xARM_R_2[0]))) {
    Serial.print("xARM_R_2: ");
    Serial.println(xARM_R_2[step % size]);
    setServo2(ARM_R_2, xARM_R_2[step % size]);
  }
  //HAND_R_2
  if (step % size < (sizeof(xHAND_R_2) / sizeof(xHAND_R_2[0]))) {
    Serial.print("xHAND_R_2: ");
    Serial.println(xHAND_R_2[step % size]);
    setServo2(HAND_R_2, xHAND_R_2[step % size]);
  }
  //ARM_L_2
  if (step % size < (sizeof(xARM_L_2) / sizeof(xARM_L_2[0]))) {
    Serial.print("xARM_L_2: ");
    Serial.println(xARM_L_2[step % size]);
    setServo2(ARM_L_2, xARM_L_2[step % size]);
  }
  //HAND_L_2
  if (step % size < (sizeof(xHAND_L_2) / sizeof(xHAND_L_2[0]))) {
    Serial.print("xHAND_L_2: ");
    Serial.println(xHAND_L_2[step % size]);
    setServo2(HAND_L_2, xHAND_L_2[step % size]);
  }
}
void Rutine6(int stepInit) {
  Serial.print("Rutina Actual 6 / Step: ");
  Serial.println(stepInit);
  timerElapsed = 600;
  int size = 20;
  int duration = 60;
  int delay = 4;
  int step = stepInit - delay;

  if (step >= duration | step < 0)
    return;


  //Posiciones
  int xCOXIS_1[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };   //60 a 120. Reposo: 85
  int xHEAD_1[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };    //75 a 120. Reposo: 90
  int xARM_R_1[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };   // 30 a 135. Reposo: 135.
  int xHAND_R_1[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };  // 00 a 80. Reposo: 75
  int xARM_L_1[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };   // 10 a 130. Reposo: 10
  int xHAND_L_1[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };  // 00 a 80. Reposo: 00

  int xCOXIS_2[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };   //85 a 95. Reposo:90
  int xHEAD_2[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };    //60 a 90. Reposo: 75
  int xARM_R_2[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };   //90 a 180. Reposo: 180
  int xHAND_R_2[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };  // 0 a 80. Reposo:80
  int xARM_L_2[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };   // 0 a 90. Reposo: 0
  int xHAND_L_2[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };  //70 a 150 Reposo: 70

  //COXIS_1
  if (step % size < (sizeof(xCOXIS_1) / sizeof(xCOXIS_1[0]))) {
    Serial.print("xCOXIS_1: ");
    Serial.println(xCOXIS_1[step % size]);
    setServo1(COXIS_1, xCOXIS_1[step % size]);
  }
  //HEAD_1
  if (step % size < (sizeof(xHEAD_1) / sizeof(xHEAD_1[0]))) {
    Serial.print("xHEAD_1: ");
    Serial.println(xHEAD_1[step % size]);
    setServo1(HEAD_1, xHEAD_1[step % size]);
  }
  //ARM_R_1
  if (step % size < (sizeof(xARM_R_1) / sizeof(xARM_R_1[0]))) {
    Serial.print("xARM_R_1: ");
    Serial.println(xARM_R_1[step % size]);
    setServo1(ARM_R_1, xARM_R_1[step % size]);
  }
  //HAND_R_1
  if (step % size < (sizeof(xHAND_R_1) / sizeof(xHAND_R_1[0]))) {
    Serial.print("xHAND_R_1: ");
    Serial.println(xHAND_R_1[step % size]);
    setServo1(HAND_R_1, xHAND_R_1[step % size]);
  }
  //ARM_L_1
  if (step % size < (sizeof(xARM_L_1) / sizeof(xARM_L_1[0]))) {
    Serial.print("xARM_L_1: ");
    Serial.println(xARM_L_1[step % size]);
    setServo1(ARM_L_1, xARM_L_1[step % size]);
  }
  //HAND_L_1
  if (step % size < (sizeof(xHAND_L_1) / sizeof(xHAND_L_1[0]))) {
    Serial.print("xHAND_L_1: ");
    Serial.println(xHAND_L_1[step % size]);
    setServo1(HAND_L_1, xHAND_L_1[step % size]);
  }

  //COXIS_2
  if (step % size < (sizeof(xCOXIS_2) / sizeof(xCOXIS_2[0]))) {
    Serial.print("xCOXIS_2: ");
    Serial.println(xCOXIS_2[step % size]);
    setServo2(COXIS_2, xCOXIS_2[step % size]);
  }
  //HEAD_2
  if (step % size < (sizeof(xHEAD_2) / sizeof(xHEAD_2[0]))) {
    Serial.print("xHEAD_2: ");
    Serial.println(xHEAD_2[step % size]);
    setServo2(HEAD_2, xHEAD_2[step % size]);
  }
  //ARM_R_2
  if (step % size < (sizeof(xARM_R_2) / sizeof(xARM_R_2[0]))) {
    Serial.print("xARM_R_2: ");
    Serial.println(xARM_R_2[step % size]);
    setServo2(ARM_R_2, xARM_R_2[step % size]);
  }
  //HAND_R_2
  if (step % size < (sizeof(xHAND_R_2) / sizeof(xHAND_R_2[0]))) {
    Serial.print("xHAND_R_2: ");
    Serial.println(xHAND_R_2[step % size]);
    setServo2(HAND_R_2, xHAND_R_2[step % size]);
  }
  //ARM_L_2
  if (step % size < (sizeof(xARM_L_2) / sizeof(xARM_L_2[0]))) {
    Serial.print("xARM_L_2: ");
    Serial.println(xARM_L_2[step % size]);
    setServo2(ARM_L_2, xARM_L_2[step % size]);
  }
  //HAND_L_2
  if (step % size < (sizeof(xHAND_L_2) / sizeof(xHAND_L_2[0]))) {
    Serial.print("xHAND_L_2: ");
    Serial.println(xHAND_L_2[step % size]);
    setServo2(HAND_L_2, xHAND_L_2[step % size]);
  }
}
void Rutine7(int stepInit) {
  Serial.print("Rutina Actual 7 / Step: ");
  Serial.println(stepInit);
  timerElapsed = 600;
  int size = 20;
  int duration = 60;
  int delay = 5;
  int step = stepInit - delay;

  if (step >= duration | step < 0)
    return;


  //Posiciones
  int xCOXIS_1[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };   //60 a 120. Reposo: 85
  int xHEAD_1[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };    //75 a 120. Reposo: 90
  int xARM_R_1[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };   // 30 a 135. Reposo: 135.
  int xHAND_R_1[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };  // 00 a 80. Reposo: 75
  int xARM_L_1[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };   // 10 a 130. Reposo: 10
  int xHAND_L_1[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };  // 00 a 80. Reposo: 00

  int xCOXIS_2[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };   //85 a 95. Reposo:90
  int xHEAD_2[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };    //60 a 90. Reposo: 75
  int xARM_R_2[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };   //90 a 180. Reposo: 180
  int xHAND_R_2[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };  // 0 a 80. Reposo:80
  int xARM_L_2[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };   // 0 a 90. Reposo: 0
  int xHAND_L_2[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };  //70 a 150 Reposo: 70

  //COXIS_1
  if (step % size < (sizeof(xCOXIS_1) / sizeof(xCOXIS_1[0]))) {
    Serial.print("xCOXIS_1: ");
    Serial.println(xCOXIS_1[step % size]);
    setServo1(COXIS_1, xCOXIS_1[step % size]);
  }
  //HEAD_1
  if (step % size < (sizeof(xHEAD_1) / sizeof(xHEAD_1[0]))) {
    Serial.print("xHEAD_1: ");
    Serial.println(xHEAD_1[step % size]);
    setServo1(HEAD_1, xHEAD_1[step % size]);
  }
  //ARM_R_1
  if (step % size < (sizeof(xARM_R_1) / sizeof(xARM_R_1[0]))) {
    Serial.print("xARM_R_1: ");
    Serial.println(xARM_R_1[step % size]);
    setServo1(ARM_R_1, xARM_R_1[step % size]);
  }
  //HAND_R_1
  if (step % size < (sizeof(xHAND_R_1) / sizeof(xHAND_R_1[0]))) {
    Serial.print("xHAND_R_1: ");
    Serial.println(xHAND_R_1[step % size]);
    setServo1(HAND_R_1, xHAND_R_1[step % size]);
  }
  //ARM_L_1
  if (step % size < (sizeof(xARM_L_1) / sizeof(xARM_L_1[0]))) {
    Serial.print("xARM_L_1: ");
    Serial.println(xARM_L_1[step % size]);
    setServo1(ARM_L_1, xARM_L_1[step % size]);
  }
  //HAND_L_1
  if (step % size < (sizeof(xHAND_L_1) / sizeof(xHAND_L_1[0]))) {
    Serial.print("xHAND_L_1: ");
    Serial.println(xHAND_L_1[step % size]);
    setServo1(HAND_L_1, xHAND_L_1[step % size]);
  }

  //COXIS_2
  if (step % size < (sizeof(xCOXIS_2) / sizeof(xCOXIS_2[0]))) {
    Serial.print("xCOXIS_2: ");
    Serial.println(xCOXIS_2[step % size]);
    setServo2(COXIS_2, xCOXIS_2[step % size]);
  }
  //HEAD_2
  if (step % size < (sizeof(xHEAD_2) / sizeof(xHEAD_2[0]))) {
    Serial.print("xHEAD_2: ");
    Serial.println(xHEAD_2[step % size]);
    setServo2(HEAD_2, xHEAD_2[step % size]);
  }
  //ARM_R_2
  if (step % size < (sizeof(xARM_R_2) / sizeof(xARM_R_2[0]))) {
    Serial.print("xARM_R_2: ");
    Serial.println(xARM_R_2[step % size]);
    setServo2(ARM_R_2, xARM_R_2[step % size]);
  }
  //HAND_R_2
  if (step % size < (sizeof(xHAND_R_2) / sizeof(xHAND_R_2[0]))) {
    Serial.print("xHAND_R_2: ");
    Serial.println(xHAND_R_2[step % size]);
    setServo2(HAND_R_2, xHAND_R_2[step % size]);
  }
  //ARM_L_2
  if (step % size < (sizeof(xARM_L_2) / sizeof(xARM_L_2[0]))) {
    Serial.print("xARM_L_2: ");
    Serial.println(xARM_L_2[step % size]);
    setServo2(ARM_L_2, xARM_L_2[step % size]);
  }
  //HAND_L_2
  if (step % size < (sizeof(xHAND_L_2) / sizeof(xHAND_L_2[0]))) {
    Serial.print("xHAND_L_2: ");
    Serial.println(xHAND_L_2[step % size]);
    setServo2(HAND_L_2, xHAND_L_2[step % size]);
  }
}
void Rutine8(int stepInit) {
  Serial.print("Rutina Actual 8 / Step: ");
  Serial.println(stepInit);
  timerElapsed = 600;
  int size = 20;
  int duration = 60;
  int delay = 5;
  int step = stepInit - delay;

  if (step >= duration | step < 0)
    return;
  //Posiciones
  int xCOXIS_1[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };   //60 a 120. Reposo: 85
  int xHEAD_1[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };    //75 a 120. Reposo: 90
  int xARM_R_1[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };   // 30 a 135. Reposo: 135.
  int xHAND_R_1[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };  // 00 a 80. Reposo: 75
  int xARM_L_1[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };   // 10 a 130. Reposo: 10
  int xHAND_L_1[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };  // 00 a 80. Reposo: 00

  int xCOXIS_2[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };   //85 a 95. Reposo:90
  int xHEAD_2[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };    //60 a 90. Reposo: 75
  int xARM_R_2[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };   //90 a 180. Reposo: 180
  int xHAND_R_2[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };  // 0 a 80. Reposo:80
  int xARM_L_2[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };   // 0 a 90. Reposo: 0
  int xHAND_L_2[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };  //70 a 150 Reposo: 70

  //COXIS_1
  if (step % size < (sizeof(xCOXIS_1) / sizeof(xCOXIS_1[0]))) {
    Serial.print("xCOXIS_1: ");
    Serial.println(xCOXIS_1[step % size]);
    setServo1(COXIS_1, xCOXIS_1[step % size]);
  }
  //HEAD_1
  if (step % size < (sizeof(xHEAD_1) / sizeof(xHEAD_1[0]))) {
    Serial.print("xHEAD_1: ");
    Serial.println(xHEAD_1[step % size]);
    setServo1(HEAD_1, xHEAD_1[step % size]);
  }
  //ARM_R_1
  if (step % size < (sizeof(xARM_R_1) / sizeof(xARM_R_1[0]))) {
    Serial.print("xARM_R_1: ");
    Serial.println(xARM_R_1[step % size]);
    setServo1(ARM_R_1, xARM_R_1[step % size]);
  }
  //HAND_R_1
  if (step % size < (sizeof(xHAND_R_1) / sizeof(xHAND_R_1[0]))) {
    Serial.print("xHAND_R_1: ");
    Serial.println(xHAND_R_1[step % size]);
    setServo1(HAND_R_1, xHAND_R_1[step % size]);
  }
  //ARM_L_1
  if (step % size < (sizeof(xARM_L_1) / sizeof(xARM_L_1[0]))) {
    Serial.print("xARM_L_1: ");
    Serial.println(xARM_L_1[step % size]);
    setServo1(ARM_L_1, xARM_L_1[step % size]);
  }
  //HAND_L_1
  if (step % size < (sizeof(xHAND_L_1) / sizeof(xHAND_L_1[0]))) {
    Serial.print("xHAND_L_1: ");
    Serial.println(xHAND_L_1[step % size]);
    setServo1(HAND_L_1, xHAND_L_1[step % size]);
  }

  //COXIS_2
  if (step % size < (sizeof(xCOXIS_2) / sizeof(xCOXIS_2[0]))) {
    Serial.print("xCOXIS_2: ");
    Serial.println(xCOXIS_2[step % size]);
    setServo2(COXIS_2, xCOXIS_2[step % size]);
  }
  //HEAD_2
  if (step % size < (sizeof(xHEAD_2) / sizeof(xHEAD_2[0]))) {
    Serial.print("xHEAD_2: ");
    Serial.println(xHEAD_2[step % size]);
    setServo2(HEAD_2, xHEAD_2[step % size]);
  }
  //ARM_R_2
  if (step % size < (sizeof(xARM_R_2) / sizeof(xARM_R_2[0]))) {
    Serial.print("xARM_R_2: ");
    Serial.println(xARM_R_2[step % size]);
    setServo2(ARM_R_2, xARM_R_2[step % size]);
  }
  //HAND_R_2
  if (step % size < (sizeof(xHAND_R_2) / sizeof(xHAND_R_2[0]))) {
    Serial.print("xHAND_R_2: ");
    Serial.println(xHAND_R_2[step % size]);
    setServo2(HAND_R_2, xHAND_R_2[step % size]);
  }
  //ARM_L_2
  if (step % size < (sizeof(xARM_L_2) / sizeof(xARM_L_2[0]))) {
    Serial.print("xARM_L_2: ");
    Serial.println(xARM_L_2[step % size]);
    setServo2(ARM_L_2, xARM_L_2[step % size]);
  }
  //HAND_L_2
  if (step % size < (sizeof(xHAND_L_2) / sizeof(xHAND_L_2[0]))) {
    Serial.print("xHAND_L_2: ");
    Serial.println(xHAND_L_2[step % size]);
    setServo2(HAND_L_2, xHAND_L_2[step % size]);
  }
}

//Rutina automática. Pin 4 HIGH
void Rutine9(int step) {
  Serial.print("Rutina Actual 9 / Step: ");
  Serial.println(step);
  timerElapsed = 600;
  int size = 20;
  int duration = 40;
  if (step == 2000)
    step = 0;
  if (step > duration)
    return;

  //Posiciones
  int xCOXIS_1[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };    //60 a 120. Reposo: 85
  int xHEAD_1[] = { 90, 75, -1, 120, -1, 75, -1, 120, -1, 75, -1, 120, -1, -1, -1, -1, -1, -1, -1, 90 };  //75 a 120. Reposo: 90
  int xARM_R_1[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };    // 30 a 135. Reposo: 135.
  int xHAND_R_1[] = { -1, 50, 30, 10, 1, -1, -1, 10, 30, 50, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };    // 0 a 80. Reposo:80
  int xARM_L_1[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
  int xHAND_L_1[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

  int xCOXIS_2[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };          //30 a 80. Reposo: 50
  int xHEAD_2[] = { 75, -1, -1, -1, -1, -1, -1, 90, -1, 60, -1, 90, -1, 60, -1, 90, -1, 60, -1, 75 };           //60 a 90. Reposo: 75
  int xARM_R_2[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };          //90 a 180. Reposo: 180
  int xHAND_R_2[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };         // 0 a 80. Reposo:80
  int xARM_L_2[] = { 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };          // 0 a 90. Reposo: 0
  int xHAND_L_2[] = { -1, -1, -1, -1, -1, 110, 130, 150, 170, -1, -1, 150, 130, 110, -1, -1, -1, -1, -1, -1 };  //70 a 150 Reposo: 70

  //COXIS_1
  if (step % size < (sizeof(xCOXIS_1) / sizeof(xCOXIS_1[0]))) {
    Serial.print("xCOXIS_1: ");
    Serial.println(xCOXIS_1[step % size]);
    setServo1(COXIS_1, xCOXIS_1[step % size]);
  }
  //HEAD_1
  if (step % size < (sizeof(xHEAD_1) / sizeof(xHEAD_1[0]))) {
    Serial.print("xHEAD_1: ");
    Serial.println(xHEAD_1[step % size]);
    setServo1(HEAD_1, xHEAD_1[step % size]);
  }
  //ARM_R_1
  if (step % size < (sizeof(xARM_R_1) / sizeof(xARM_R_1[0]))) {
    Serial.print("xARM_R_1: ");
    Serial.println(xARM_R_1[step % size]);
    setServo1(ARM_R_1, xARM_R_1[step % size]);
  }
  //HAND_R_1
  if (step % size < (sizeof(xHAND_R_1) / sizeof(xHAND_R_1[0]))) {
    Serial.print("xHAND_R_1: ");
    Serial.println(xHAND_R_1[step % size]);
    setServo1(HAND_R_1, xHAND_R_1[step % size]);
  }
  //ARM_L_1
  if (step % size < (sizeof(xARM_L_1) / sizeof(xARM_L_1[0]))) {
    Serial.print("xARM_L_1: ");
    Serial.println(xARM_L_1[step % size]);
    setServo1(ARM_L_1, xARM_L_1[step % size]);
  }
  //HAND_L_1
  if (step % size < (sizeof(xHAND_L_1) / sizeof(xHAND_L_1[0]))) {
    Serial.print("xHAND_L_1: ");
    Serial.println(xHAND_L_1[step % size]);
    setServo1(HAND_L_1, xHAND_L_1[step % size]);
  }

  //COXIS_2
  if (step % size < (sizeof(xCOXIS_2) / sizeof(xCOXIS_2[0]))) {
    Serial.print("xCOXIS_2: ");
    Serial.println(xCOXIS_2[step % size]);
    setServo2(COXIS_2, xCOXIS_2[step % size]);
  }
  //HEAD_2
  if (step % size < (sizeof(xHEAD_2) / sizeof(xHEAD_2[0]))) {
    Serial.print("xHEAD_2: ");
    Serial.println(xHEAD_2[step % size]);
    setServo2(HEAD_2, xHEAD_2[step % size]);
  }
  //ARM_R_2
  if (step % size < (sizeof(xARM_R_2) / sizeof(xARM_R_2[0]))) {
    Serial.print("xARM_R_2: ");
    Serial.println(xARM_R_2[step % size]);
    setServo2(ARM_R_2, xARM_R_2[step % size]);
  }
  //HAND_R_2
  if (step % size < (sizeof(xHAND_R_2) / sizeof(xHAND_R_2[0]))) {
    Serial.print("xHAND_R_2: ");
    Serial.println(xHAND_R_2[step % size]);
    setServo2(HAND_R_2, xHAND_R_2[step % size]);
  }
  //ARM_L_2
  if (step % size < (sizeof(xARM_L_2) / sizeof(xARM_L_2[0]))) {
    Serial.print("xARM_L_2: ");
    Serial.println(xARM_L_2[step % size]);
    setServo2(ARM_L_2, xARM_L_2[step % size]);
  }
  //HAND_L_2
  if (step % size < (sizeof(xHAND_L_2) / sizeof(xHAND_L_2[0]))) {
    Serial.print("xHAND_L_2: ");
    Serial.println(xHAND_L_2[step % size]);
    setServo2(HAND_L_2, xHAND_L_2[step % size]);
  }
}
void Rutine10(int step) {
  Serial.print("Rutina Actual 10 / Step: ");
  Serial.println(step);
  isPWNActive = true;
  ResetServos();
  enable = false;
}

void Cadera_1() {
  setServo1(COXIS_1, 60);
  delay(250);
  setServo1(COXIS_1, 75);
  delay(250);
  setServo1(COXIS_1, 90);
  delay(250);
  setServo1(COXIS_1, 105);
  delay(250);
  setServo1(COXIS_1, 90);
  delay(250);
  setServo1(COXIS_1, 75);
  delay(250);
}

void Cabeza_1() {
  setServo1(HEAD_1, 0);
  delay(1000);
  setServo1(HEAD_1, 160);
  delay(1000);
  setServo1(HEAD_1, 95);
  delay(1000);
}

void Brazo_R_1() {
  setServo1(ARM_R_1, 0);
  delay(1000);
  setServo1(ARM_R_1, 160);
  delay(1000);
}

void Brazo_L_1() {
  setServo1(ARM_L_1, 0);
  delay(1000);
  setServo1(ARM_L_1, 160);
  delay(1000);
}

void Manoo_L_1() {
  setServo1(HAND_L_1, 0);
  delay(1000);
  setServo1(HAND_L_1, 160);
  delay(1000);
}

void Saludo_1() {
  setServo1(ARM_R_1, 10);
  delay(1000);
  setServo1(HAND_R_1, 20);
  delay(750);
  setServo1(HAND_R_1, 50);
  delay(750);
  setServo1(HAND_R_1, 20);
  delay(750);
  setServo1(HAND_R_1, 50);
  delay(750);
  setServo1(HAND_R_1, 70);
  setServo1(ARM_R_1, 130);
  delay(2500);
}

void Saludo_2() {
  setServo1(ARM_R_1, 10);
  setServo1(COXIS_1, 60);
  delay(250);
  setServo1(COXIS_1, 75);
  delay(250);
  setServo1(COXIS_1, 90);
  delay(250);
  setServo1(COXIS_1, 105);
  delay(250);
  setServo1(HAND_R_1, 20);
  setServo1(COXIS_1, 90);
  delay(250);
  setServo1(COXIS_1, 75);
  delay(250);
  setServo1(COXIS_1, 60);
  delay(250);
  setServo1(COXIS_1, 75);
  delay(250);
  setServo1(HAND_R_1, 50);
  setServo1(COXIS_1, 90);
  delay(250);
  setServo1(COXIS_1, 105);
  delay(250);
  setServo1(COXIS_1, 90);
  delay(250);
  setServo1(HAND_R_1, 20);
  setServo1(COXIS_1, 75);
  delay(250);
  setServo1(COXIS_1, 60);
  delay(250);
  setServo1(COXIS_1, 75);
  delay(250);
  setServo1(HAND_R_1, 50);
  setServo1(COXIS_1, 90);
  delay(250);
  setServo1(COXIS_1, 105);
  delay(250);
  setServo1(COXIS_1, 90);
  delay(250);
  setServo1(HAND_R_1, 70);
  setServo1(ARM_R_1, 130);
  setServo1(COXIS_1, 85);
  delay(2500);
}

void ResetServos() {
  if (!isPWNActive) {
    return;
  }
    setServo1(COXIS_1, 85);
  setServo1(HEAD_1, 90);
  setServo1(ARM_R_1, 135);
  setServo1(HAND_R_1, 75);
  setServo1(ARM_L_1, 10);
  setServo1(HAND_L_1, 0);

  setServo2(COXIS_2, 90);
  setServo2(HEAD_2, 75);
  setServo2(ARM_R_2, 180);
  setServo2(HAND_R_2, 80);
  setServo2(ARM_L_2, 0);
  setServo2(HAND_L_2, 70);
  delay(500);
  Serial.println("Reseting Servos");
  servos_1.setPWM(COXIS_1, 0, 0);
  servos_1.setPWM(HEAD_1, 0, 0);
  servos_1.setPWM(ARM_R_1, 0, 0);
  servos_1.setPWM(HAND_R_1, 0, 0);
  servos_1.setPWM(ARM_L_1, 0, 0);
  servos_1.setPWM(HAND_L_1, 0, 0);
  servos_2.setPWM(COXIS_2, 0, 0);
  servos_2.setPWM(HEAD_2, 0, 0);
  servos_2.setPWM(ARM_R_2, 0, 0);
  servos_2.setPWM(HAND_R_2, 0, 0);
  servos_2.setPWM(ARM_L_2, 0, 0);
  servos_2.setPWM(HAND_L_2, 0, 0);
  isPWNActive = false;
}

void setServo1(uint8_t n_servo, int angulo) {
  isPWNActive = true;
  if (angulo < 0 || !masterEnable) {
    return;
  }
  int duty;
  duty = map(angulo, 0, 180, pos0, pos180);
  servos_1.setPWM(n_servo, 0, duty);
}

void setServo2(uint8_t n_servo, int angulo) {
  isPWNActive = true;
  if (angulo < 0 || !masterEnable) {
    return;
  }
  int duty;
  duty = map(angulo, 0, 180, pos0, pos180);
  servos_2.setPWM(n_servo, 0, duty);
}
