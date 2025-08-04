
/*
   Exhibit /habitat/humidity V3.0 NIS
   (Feuchtigkeit) LUFT AIR

   Arduino UNO R4 WiFI

   update: 03-08-2025 NIS
   * starup wait for unifie WLAN - 60 sec with matrix animation
   * timeout after WLAN for IP, ca. 5 sec
   * show 4 wheels short on display
   * mqtt clientname must be unique !!! 

   last update: 16.07.2025 Arduino because of ESP Problems 
   MQTT IN and OUT

Startoutput:

Arduino MultiWiFi connected
🔹 IP address: 192.168.1.22
SSID: Habitat
BSSID: 9E:2A:6F:D0:29:30
signal strength (RSSI):-39
Encryption Type:4
IP Address: 192.168.1.22
MAC address: 84:CC:A8:2F:63:D0
NetMask: 255.255.255.0
Gateway: 192.168.1.1
Exhibit /habitat/luft: setup mqtt...
Exhibit /habitat/luft: setup mqtt complete.
Exhibit /habitat/luft: setup complete ------------------------------------------------------------
Exhibit /habitat/air: Attempting MQTT connection...
Exhibit /habitat/air: connected
S1
   
*/

#include <SPI.h>
// https://docs.arduino.cc/libraries/wifinina/
#include <WiFiS3.h>
WiFiClient wifi_client;

#include "wlan_secrets.h"

#include <PubSubClient.h>
#include "mqtt_secrets.h"

#include <Adafruit_ADS1X15.h>
Adafruit_ADS1115 ads1115;
// ADS1115 I2C address is 0x48(72)
#define Addr 0x48

#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"
ArduinoLEDMatrix matrix;
#include "matrix.h"

#include "rgb_lcd.h"
rgb_lcd mylcd;

char* ssids[]     = {SECRET_SSID1, SECRET_SSID2, SECRET_SSID3, SECRET_SSID4, SECRET_SSID5};
char* passwords[] = {SECRET_PASS1, SECRET_PASS2, SECRET_PASS3, SECRET_PASS4, SECRET_PASS5};
int numNetworks = sizeof(ssids) / sizeof(ssids[0]);
String mySSID = "no SSID";

#define ESP_INFO_VERSION "Habitat AIR 3.20"
#define EXHIBIT_NAME "Exhibit /habitat/luft"

#define BAUDRATE 115200
#define LCDLENGTH 17

#define DELTATHRESHOLD 200
// analog sensor ca. 2000..13000, dreht über
#define TURNOVERTHRESHOLD 10000 
#define DATANORMING 1000

// --------------------------------------------------------------------------------
// MQTT Topics


char* MQTT_TOPIC_XCOCOS_ALIVE = "xcocos/alive/";
char* MQTT_TOPIC_TESTBUTTON   = "cosa/testbutton/";
char* MQTT_TOPIC_DEBUGLEVEL   = "cosa/habitat/air/debuglevel"; 

char* MQTT_TOPIC_OUT_WHEEL1 = "cosa/habitat/air/out/wheel1"; // O2
char* MQTT_TOPIC_OUT_WHEEL2 = "cosa/habitat/air/out/wheel2"; // N2 - Sensor Error
char* MQTT_TOPIC_OUT_WHEEL3 = "cosa/habitat/air/out/wheel3"; // Ar
char* MQTT_TOPIC_OUT_WHEEL4 = "cosa/habitat/air/out/wheel4"; // CO2

// --------------------------------------------------------------------------------

const int ledCtrlPin    = 2; // connector PIN 
const int buttonCtrlPin = 3; // button PIN
int buttonCtrlState = 0;
int buttonCtrlPrev  = 0;

// --------------------------------------------------------------------------------

int button_counter = 0;
int mqtt_counter = 0;
int alive_counter = 0;

int counter = 0;
char charSend[80];
int debugLevel = 0; // OFF=0 INFO=1, WARN=2, ERROR=3

// ##### init ######################################################################

//////////////////////
///// VARIABLES /////
////////////////////

///// Timing for Alive checks /////

#define seconds() (millis()/1000)
#define minutes() (millis()/60000)
#define hours()   (millis()/3600000)

int oldSec = 99; // check if sec has changed
char dataInfo[60] = "Data init";

///// Network & MQTT /////
// MAC Adress
byte mac[] = DEVICE_MAC;
// Fixed IP Adress of Exhibit
IPAddress ip(DEVICE_IP_ADRESS);

// EthernetClient eth_client;
PubSubClient mqtt_client(wifi_client);

int status = WL_IDLE_STATUS;

// --------------------------------------------------------------------------------

const int colorR = 255;
const int colorG = 255;
const int colorB = 0;

// ################################################################################

int16_t RotationSensorONE = 0;
int16_t OldRotationSensorONE = 0;

int16_t RotationSensorTWO = 0;
int16_t OldRotationSensorTWO = 0;

int16_t RotationSensorTHREE = 0;
int16_t OldRotationSensorTHREE = 0;

int16_t RotationSensorFOUR = 0;
int16_t OldRotationSensorFOUR = 0;

int16_t THISDeltaRotationSensorONE = 0;
int16_t DeltaRotationSensorONE = 0;

int16_t THISDeltaRotationSensorTWO = 0;
int16_t DeltaRotationSensorTWO = 0;

int16_t THISDeltaRotationSensorTHREE = 0;
int16_t DeltaRotationSensorTHREE = 0;

int16_t THISDeltaRotationSensorFOUR = 0;
int16_t DeltaRotationSensorFOUR = 0;

unsigned long timeNow = 0;
unsigned long SensorTimestampONE = 0;
unsigned long SensorTimestampTWO = 0;
unsigned long SensorTimestampTHREE = 0;
unsigned long SensorTimestampFOUR = 0;

float rotate1 = 0;
float rotate1old = 0;
float rotate2 = 0;
float rotate2old = 0;
float rotate3 = 0;
float rotate3old = 0;
float rotate4 = 0;
float rotate4old = 0;
const float rotateThreshold = 0.50;

boolean LoginState = false;

// ################################################################################

//////////////////
///// SETUP /////
////////////////

void setup() 
{
  setup_serial();
  Serial.println("Exhibit /habitat/luft: starting setup ------------------------------------------------------------");
  
  setup_matrix();
  show_matrix("init", 2000);

  show_matrix("lcd", 2000);
  setup_lcd();

  show_matrix("pin", 2000);
  setup_pins();

  show_matrix("clock1", 2000);
  wlan_delay(60); // wait till unifi dreambox is also ready
  show_matrix("net", 1000);
  setup_multiwlan(); // with small delay for IP

  show_matrix("mqtt", 2000);
  setup_mqtt(); // unique client name 
  
  Serial.println("Exhibit /habitat/luft: setup complete ------------------------------------------------------------");
}

/////////////////
///// LOOP /////
///////////////

// MAIN LOOP

void loop() 
{
  check_testbutton();

  handle_connection();
  check_alive();
  check_inputs();

  check_timer();
  delay(100);
}

/////////////////////////
///// SETUP HELPER /////
///////////////////////

void setup_serial() {
  Serial.begin(BAUDRATE); // or slow: 9600
  while (!Serial) { }; // wait for serial port to connect. Needed for native USB port only 
  delay(200);
  Serial.println("Exhibit /habitat/luft starting...");
 
  Serial.println("");
  Serial.print(F("Starting UP File: "));
  Serial.println(__FILE__);
  Serial.print(F("code creation [NIS]: "));
  Serial.println(": " __DATE__ " @ " __TIME__);  // predefined macors
  Serial.println(F("--------------------------------------------------------------------------------"));
}

void setup_pins() {
  Serial.println("Exhibit /habitat/luft arduino: setup pins...");
  ads1115.begin();

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ledCtrlPin, OUTPUT);
  pinMode(buttonCtrlPin, INPUT);

  RotationSensorONE = ads1115.readADC_SingleEnded(0);
  OldRotationSensorONE = RotationSensorONE;

  RotationSensorTWO = ads1115.readADC_SingleEnded(1);
  OldRotationSensorTWO = RotationSensorTWO;

  RotationSensorTHREE = ads1115.readADC_SingleEnded(2);
  OldRotationSensorTHREE = RotationSensorTHREE;

  RotationSensorFOUR = ads1115.readADC_SingleEnded(3);
  OldRotationSensorFOUR = RotationSensorFOUR;

  Serial.println("Exhibit /habitat/luft arduino: setup pins complete.");
}

void wlan_delay(int secGot)
{
  for (int i = secGot; i > 0; i-=5) // ca. 5sec für animation
  {
    // needs around 5 sec
    matrix.beginDraw();
    matrix.stroke(0xFFFFFFFF);
    matrix.textScrollSpeed(100);
    sprintf(charSend,"  >-%02d-<", int(i/5)); 
    matrix.textFont(Font_5x7);
    matrix.beginText(0, 1, 0xFFFFFF);
    matrix.println(charSend);
    matrix.endText(SCROLL_LEFT);
    matrix.endDraw();
  
    delay(100); // wait stepper msec
  }
}

void setup_multiwlan()
{
  for (int i = 0; i < numNetworks; i++) 
  {
    Serial.print("Versuche Verbindung mit: ");
    Serial.println(ssids[i]);

    mylcd.setCursor(0, 1); 
    snprintf(charSend, LCDLENGTH,"try %-12s", ssids[i]); 
    mylcd.print(charSend);

    int status = WiFi.begin(ssids[i], passwords[i]);

    // Warte auf Verbindung (max. 10 Sekunden)
    int retries = 0;
    while (status != WL_CONNECTED && retries < 20) 
    {
      delay(500);
      retries++;
      status = WiFi.status();
    }

    if (WiFi.status() == WL_CONNECTED) 
    {
      delay(5000); // wait a little to geht IP
      Serial.println("");
      Serial.println("Arduino MultiWiFi connected");
      Serial.print("🔹 IP address: ");
      Serial.println(WiFi.localIP());
      break; // end for loop - NW found
    }
  }

  if (WiFi.status() != WL_CONNECTED) 
  {
    Serial.println("Keine Verbindung zu einem Netzwerk möglich.");
    mylcd.setCursor(0, 1);  
    mylcd.print("ERROR no network");
    while (true) {}; // stop 
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) 
  {
    Serial.print(fv);
    Serial.print(" vs. newest FW ");
    Serial.print(WIFI_FIRMWARE_LATEST_VERSION);
    Serial.println(" -- Please upgrade the firmware");
  }

  printCurrentNet();
  printWifiData();
}


void setup_mqtt() {
  Serial.println("Exhibit /habitat/luft: setup mqtt...");

  mqtt_client.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt_client.setCallback(handle_received_message);
  LoginState = true;
  
  Serial.println("Exhibit /habitat/luft: setup mqtt complete.");
}

void setup_lcd()
{
  mylcd.begin(16, 2);
  mylcd.setRGB(colorR, colorG, colorB);
  mylcd.setCursor(0, 0); 
  //           ################
  mylcd.print("Habitat-Luft 3.0");
  mylcd.setCursor(0, 1); 
  mylcd.print("S# ##/##/##/## x");
}

void setup_matrix()
{
  matrix.begin();
  setup_symbols(); // in matrix.h
}

void show_matrix(String matrix_name,int time)
{
  matrix.loadFrame(getMatrix(matrix_name));
  delay(time);
}


// WIFI Helper

void printWifiData() {

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);
  // print your subnet mask:
  IPAddress subnet = WiFi.subnetMask();
  Serial.print("NetMask: ");
  Serial.println(subnet);
  // print your gateway address:
  IPAddress gateway = WiFi.gatewayIP();
  Serial.print("Gateway: ");
  Serial.println(gateway);
}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);
  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);
  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}

////////////////////////
///// LOOP HELPER /////
//////////////////////

void handle_connection() {
  // if (Ethernet.linkStatus() == LinkOFF) {
  //   Serial.println("Exhibit /habitat/luft: Ethernet cable disconnected!");
  // }
  if (!mqtt_client.connected()) {
    mqtt_login();
  }
  mqtt_client.loop();
}

void check_inputs() {
  timeNow = millis();

  RotationSensorONE   = ads1115.readADC_SingleEnded(0);
  RotationSensorTWO   = ads1115.readADC_SingleEnded(1);
  RotationSensorTHREE = ads1115.readADC_SingleEnded(2);
  RotationSensorFOUR  = ads1115.readADC_SingleEnded(3);
  
  // show raw data from wheels
  if (debugLevel==1)
  {
    sprintf(charSend,"ADS1115: %05d %05d %05d %05d",RotationSensorONE, RotationSensorTWO, RotationSensorTHREE, RotationSensorFOUR); 
    Serial.println(charSend); 
  }

  CheckInput1(); // Sensor Error 
  CheckInput2(); // repaired, was Error
  CheckInput3();
  CheckInput4();
}

void check_alive() {
  

  int mySec = seconds() % 60;

  if (mySec != oldSec)  // only when sec changed
  {
    oldSec = mySec;

    if ((mySec%60) == 0)  // alle 60 sec
    {
      sprintf(dataInfo,"%s", ALIVE_ID); 
      mqtt_publish(MQTT_TOPIC_XCOCOS_ALIVE, dataInfo); // alive Message to XCOCOS
    }
  }
}

void check_testbutton()
{
  buttonCtrlState = !digitalRead(buttonCtrlPin);
  if (buttonCtrlState != buttonCtrlPrev) 
  {
    sprintf(charSend,"S%1d",buttonCtrlState); 
    Serial.println(charSend); // send to pc/unity
    buttonCtrlPrev = buttonCtrlState;
    mylcd.setCursor(1, 1);
    mylcd.print(buttonCtrlState);

    if (buttonCtrlState)
    {
      counter++;
      // sprintf(charSend,"%03d",counter); 
      // mylcd.setCursor(11, 1);
      // mylcd.print(charSend);

      sprintf(dataInfo,"%d", counter); 
      mqtt_publish(MQTT_TOPIC_TESTBUTTON, dataInfo); 
    }
  }
}

void check_timer()
{
    // ###################################################
    // helper functions - time 
    int mySec = seconds() % 60;
    int myMin = minutes();
    int mySecPuls = seconds() % 2;

    mylcd.setCursor(15, 1);
    
    if (mySecPuls == 1)
    {
      mylcd.print("x");
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(ledCtrlPin, true);

      matrix.loadFrame(matrix_ok);
      // matrix.loadFrame(matrix_karo1);
      // matrix.loadFrame(matrix_line1);
      // matrix.loadFrame(matrix_full);
      // matrix.loadFrame(matrix_happy);
    }
    else
    {
      mylcd.print("o");  
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(ledCtrlPin, false);

      matrix.loadFrame(matrix_herz);
      // matrix.loadFrame(matrix_karo2);
      // matrix.loadFrame(matrix_line2);
      // matrix.loadFrame(matrix_empty);
      // matrix.loadFrame(matrix_heart);
    }
}

/////////////////
///// MQTT /////
///////////////


void handle_received_message(char* topic, byte* payload, unsigned int length) 
{
  Serial.print(EXHIBIT_NAME);
  Serial.print(": handle Message from topic [");
  Serial.print(topic);
  Serial.print("]: ");

  String payload_string = "";
  for (int i = 0; i < length; i++) {
    payload_string += (char)payload[i];
  }
  Serial.println(payload_string);

  if (strcmp(topic, MQTT_TOPIC_DEBUGLEVEL) == 0) {
     debugLevel = payload_string.toInt();
     Serial.print("DEBUG LEVEL set to: ");
     Serial.println(debugLevel);
  } 

}

void mqtt_login() 
{
  mylcd.setCursor(0, 1);     
  //                            ################  
  snprintf(charSend, LCDLENGTH,"try MQTT connect"); 
  mylcd.print(charSend);

  while (!mqtt_client.connected()) {
    Serial.print(EXHIBIT_NAME);
    Serial.println(": Attempting MQTT connection...");

    if (mqtt_client.connect(MQTT_CLIENT, MQTT_USER, MQTT_PASS)) {
      Serial.print(EXHIBIT_NAME);
      mqtt_client.subscribe(MQTT_TOPIC_DEBUGLEVEL);
      Serial.println(": connected");
    } else {
      Serial.print(EXHIBIT_NAME);
      Serial.print(": failed, rc=");
      Serial.print(mqtt_client.state()); 
      Serial.print(" to ");
      Serial.print(MQTT_BROKER);
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
  
  setup_lcd();
}

void mqtt_publish(char* topic, String payload) 
{
  mqtt_counter++;
  // sprintf(charSend,"%s: Publishing %03d", EXHIBIT_NAME, mqtt_counter); 
  // Serial.println(charSend);
  sprintf(charSend,"##### MQTT message: %s %s ", topic, payload.c_str()); 
  Serial.println(charSend);
  
  mqtt_client.publish(topic, payload.c_str(), true);
  matrix.loadFrame(matrix_pfeil3);
  delay(100);
}

///////////////////


// #################################################################
// New NIS

///////////////////
///// EVENTS ///// 
/////////////////

void CheckInput1() 
{
  DeltaRotationSensorONE = OldRotationSensorONE - RotationSensorONE;
  if (abs(DeltaRotationSensorONE)>TURNOVERTHRESHOLD) // check turn over
  {
    OldRotationSensorONE = RotationSensorONE;
  }
  else 
  if (abs(DeltaRotationSensorONE) > DELTATHRESHOLD) // change big enough
  {
    float value = (float)DeltaRotationSensorONE/DATANORMING;
    dtostrf(value, 6, 3, charSend);  // (value, width, precision, buffer) 
    mqtt_publish(MQTT_TOPIC_OUT_WHEEL1, charSend);
    OldRotationSensorONE = RotationSensorONE;

    mylcd.setCursor(3, 1); 
    int sensor = int(value*10);
    snprintf(charSend, 3,"%+2d", sensor); 
    mylcd.print(charSend);
  }
  else
  {
    mylcd.setCursor(3, 1); 
    mylcd.print("**");
  }
}

void CheckInput2() 
{
  DeltaRotationSensorTWO = OldRotationSensorTWO - RotationSensorTWO;
  if (abs(DeltaRotationSensorTWO)>TURNOVERTHRESHOLD) // check turn over
  {
    OldRotationSensorTWO = RotationSensorTWO;
  }
  else 
  if (abs(DeltaRotationSensorTWO) > DELTATHRESHOLD) // change big enough
  {
    float value = (float)DeltaRotationSensorTWO/DATANORMING;
    dtostrf(value, 6, 3, charSend);  // (value, width, precision, buffer) 
    mqtt_publish(MQTT_TOPIC_OUT_WHEEL2, charSend);
    OldRotationSensorTWO = RotationSensorTWO;

    mylcd.setCursor(6, 1); 
    int sensor = int(value*10);
    snprintf(charSend, 3, "%+2d", sensor); 
    mylcd.print(charSend);
  }
  else
  {
    mylcd.setCursor(6, 1); 
    mylcd.print("**");
  }
}

void CheckInput3() 
{
  DeltaRotationSensorTHREE = OldRotationSensorTHREE - RotationSensorTHREE;
  if (abs(DeltaRotationSensorTHREE)>TURNOVERTHRESHOLD) 
  {
    OldRotationSensorTHREE = RotationSensorTHREE;
  }
  else 
  if (abs(DeltaRotationSensorTHREE) > DELTATHRESHOLD) 
  {
    float value = (float)DeltaRotationSensorTHREE/DATANORMING;
    dtostrf(value, 6, 3, charSend);  // (value, width, precision, buffer)  
    mqtt_publish(MQTT_TOPIC_OUT_WHEEL3, charSend);
    OldRotationSensorTHREE = RotationSensorTHREE;
   
    mylcd.setCursor(3+2*3, 1); 
    int sensor = int(value*10);
    snprintf(charSend, 3,"%+2d", sensor ); 
    mylcd.print(charSend);
  }
  else
  {
    mylcd.setCursor(9, 1); 
    mylcd.print("**");
  }}

void CheckInput4() 
{
  DeltaRotationSensorFOUR = OldRotationSensorFOUR - RotationSensorFOUR;
  if (abs(DeltaRotationSensorFOUR)>TURNOVERTHRESHOLD) // check turn over
  {
    OldRotationSensorFOUR = RotationSensorFOUR;
  }
  else 
  if (abs(DeltaRotationSensorFOUR) > DELTATHRESHOLD) 
  {
    float value = (float)DeltaRotationSensorFOUR/DATANORMING;
    dtostrf(value, 6, 3, charSend);  // (value, width, precision, buffer) 
    mqtt_publish(MQTT_TOPIC_OUT_WHEEL4, charSend);
    OldRotationSensorFOUR = RotationSensorFOUR;
 
    // mylcd.print("S# ##/##/##/## x");
    mylcd.setCursor(12, 1); 
    int sensor = int(value*10);
    snprintf(charSend, 3, "%+2d", sensor); 
    mylcd.print(charSend);
  }
  else
  {
    mylcd.setCursor(12, 1); 
    mylcd.print("**");
  }
}

// #################################################################
// EOP



