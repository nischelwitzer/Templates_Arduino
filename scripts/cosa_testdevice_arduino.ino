
/*
  Exhibit /habitat/humidity V1.2
  (Feuchtigkeit) KLima

  Arduino WIFI Version
  Update OSC 2 MQTT

  update: 28-04-2025 NIS, MQTT IN and OUT

  https://docs.arduino.cc/libraries/wifinina/
*/

#include <SPI.h>
#include <WiFiNINA.h> 
#include "wlan_secrets.h"
WiFiClient wifi_client;

#include <PubSubClient.h>
#define MQTT_HABITAT
// #define MQTT_DMT
#include "mqtt_secrets.h"

#include "rgb_lcd.h"
rgb_lcd mylcd;

char* ssids[]     = {SECRET_SSID1, SECRET_SSID2, SECRET_SSID3, SECRET_SSID4, SECRET_SSID5};
char* passwords[] = {SECRET_PASS1, SECRET_PASS2, SECRET_PASS3, SECRET_PASS4, SECRET_PASS5};
int numNetworks = sizeof(ssids) / sizeof(ssids[0]);
String mySSID = "no SSID";

#define ARDUINO_INFO_VERSION "Arduino Uno WiFi Rev2"
#define EXHIBIT_NAME "Exhibit /habitat/arduino"

#define BAUDRATE 115200
#define LCDLENGTH 17

#include "helpers.h"

// --------------------------------------------------------------------------------
// MQTT Topics

const char* MQTT_TOPIC_XCOCOS_ALIVE   = "xcocos/alive/";
const char* MQTT_TOPIC_IN_LED         = "cosa/habitat/arduino/in/led";
const char* MQTT_TOPIC_IN_RESET       = "cosa/habitat/arduino/in/reset"; 
const char* MQTT_TOPIC_IN_DEBUGLEVEL  = "cosa/habitat/arduino/debuglevel"; 

const char* MQTT_TOPIC_OUT_TESTBUTTON = "cosa/habitat/arduino/out/testbutton";

// --------------------------------------------------------------------------------

const int ledCtrlPin    = 2;  // connector on D2
const int buttonCtrlPin = 3; // button PIN
int buttonCtrlState = 0;
int buttonCtrlPrev  = 0;

const int testled_outPin = 4;

// --------------------------------------------------------------------------------
// ##### init ######################################################################

//////////////////////
///// VARIABLES /////
////////////////////

///// Timing for Alive checks /////

#define seconds() (millis()/1000)
#define minutes() (millis()/60000)
#define hours()   (millis()/3600000)

int button_counter = 0;
int mqtt_counter   = 0;
int alive_counter  = 0;

char charSend[80];
int debugLevel = 0; // OFF=0 INFO=1, WARN=2, ERROR=3

int oldSec = 99; // check if sec has changed

///// Network & MQTT /////
// MAC Adress
byte mac[] = DEVICE_MAC;
// Fixed IP Adress of Exhibit
// IPAddress ip(DEVICE_IP_ADRESS);

// EthernetClient eth_client;
PubSubClient mqtt_client(wifi_client);

int status = WL_IDLE_STATUS;

// --------------------------------------------------------------------------------
// ################################################################################
// ################################################################################

//////////////////
///// SETUP /////
////////////////

void setup() 
{
  setup_serial();
  Serial.print(EXHIBIT_NAME);
  Serial.println(": starting setup ------------------------------------------------------------");

  setup_pins();
  setup_lcd();
  setup_wlan();
  setup_mqtt();
  ready_lcd();

  Serial.print(EXHIBIT_NAME);
  Serial.println(": setup complete ------------------------------------------------------------");
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
  Serial.begin(BAUDRATE);
  while (!Serial) { }; // wait for serial port to connect. Needed for native USB port only 
  delay(200);
  Serial.print(EXHIBIT_NAME);
  Serial.println(": starting...");
}

void setup_pins() {
  Serial.print(EXHIBIT_NAME);
  Serial.println(": setup pins...");
  
  pinMode(ledCtrlPin,     OUTPUT);
  pinMode(buttonCtrlPin,  INPUT);  
  pinMode(testled_outPin, OUTPUT);

  Serial.print(EXHIBIT_NAME);
  Serial.println(": setup pins complete.");
}

void reset_sensors()
{
  mqtt_publish(MQTT_TOPIC_IN_LED, "OFF");
  button_counter = 0;
  // mqtt_counter = 0;
  // alive_counter = 0;
  Serial.print(EXHIBIT_NAME);
  Serial.println(": MQTT Reset completed.");
}

void setup_lcd()
{
  mylcd.begin(16, 2);
  mylcd.setRGB(COLOR_RED);
  mylcd.setCursor(0, 0); 
  mylcd.print("TestDevice   1.2");
  mylcd.setCursor(0, 1); 
  mylcd.print("trying WLAN     ");
}

void ready_lcd()
{
  mylcd.setRGB(COLOR_GREEN);
  mylcd.setCursor(0, 0); 
  mylcd.print("TestDevice   1.2");
  mylcd.setCursor(0, 1); 
  mylcd.print("S# Counter:### x");
}

void setup_wlan()
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
      Serial.println("");
      Serial.println("Arduino Uno WiFI Rev2 MultiWiFi connected");
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
  Serial.print(EXHIBIT_NAME);
  Serial.println(": setup mqtt...");

  mqtt_client.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt_client.setCallback(handle_received_message);
  
  Serial.print(EXHIBIT_NAME);
  Serial.println(": setup mqtt complete.");
}

// WIFI Helper

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("🔹 SSID: ");
  mySSID = WiFi.SSID();
  Serial.println(mySSID);
  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid); // Basic Service Set Identifier - MAC des access points
  Serial.print("🔹 BSSID: ");
  printMacAddress(bssid);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("🔹 Signalstärke (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void printWifiData() 
{
  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("🔹 MAC address: ");
  printMacAddress(mac);
  // print your subnet mask:
  IPAddress subnet = WiFi.subnetMask();
  Serial.print("🔹 NetMask: ");
  Serial.println(subnet);

  // print your gateway address:
  IPAddress gateway = WiFi.gatewayIP();
  Serial.print("🔹 Gateway: ");
  Serial.println(gateway);

  Serial.print("🔹 DNS: ");
  Serial.println(WiFi.dnsIP());
  Serial.println();
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

void handle_connection() 
{
  if (!mqtt_client.connected()) {
    mqtt_connect();
    ready_lcd();
  }
  mqtt_client.loop();
}

void check_inputs() {
  // 
  // sensor checks
  //
}

void check_alive() {
  int mySec = seconds() % 60;
  if (mySec != oldSec)  // only when sec changed
  {
    oldSec = mySec;

    if ((mySec%60) == 0)  // alle 60 sec
    {
      alive_counter++;
      sprintf(charSend,"%s %d", ALIVE_ID, alive_counter); 
      mqtt_publish(MQTT_TOPIC_XCOCOS_ALIVE, charSend); // alive Message to XCOCOS
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
      button_counter++;
      mylcd.setCursor(11, 1);
      sprintf(charSend, "%03d", button_counter); 
      mylcd.print(charSend);
    }

    sprintf(charSend, "%1d %04d", buttonCtrlState, button_counter); 
    mqtt_publish(MQTT_TOPIC_OUT_TESTBUTTON, charSend);
  }
}

void check_timer()
{
    // helper functions - time 
    int mySec = seconds() % 60;
    int myMin = minutes();

    int mySecPuls = seconds() % 2;
    mylcd.setCursor(15, 1);
    
    if (mySecPuls == 1)
    {
      mylcd.print("x");
      digitalWrite(ledCtrlPin, true);
    }
    else
    {
      mylcd.print("o");  
      digitalWrite(ledCtrlPin, false);
    }
}

/////////////////
///// MQTT /////
///////////////


void mqtt_connect() {
  mylcd.setCursor(0, 1);  
  mylcd.print("try MQTT connect");
 
  while (!mqtt_client.connected()) {
    Serial.print(EXHIBIT_NAME);
    Serial.println(": Attempting MQTT connection...");

    if (mqtt_client.connect(MQTT_CLIENT, MQTT_USER, MQTT_PASS)) {
      Serial.print(EXHIBIT_NAME);
      Serial.println(": connected");
      // list of MQTT topics to handle als input
      mqtt_client.subscribe(MQTT_TOPIC_IN_LED);
      mqtt_client.subscribe(MQTT_TOPIC_IN_DEBUGLEVEL);
      mqtt_client.subscribe(MQTT_TOPIC_IN_RESET);
    } else {
      Serial.print(EXHIBIT_NAME);
      Serial.print(": failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void handle_received_message(char* topic, byte* payload, unsigned int length) 
{
  Serial.print(EXHIBIT_NAME);
  Serial.print(": !!! Handle Message from topic [");
  Serial.print(topic);
  Serial.print("]: ");

  String payload_string = "";
  for (int i = 0; i < length; i++) {
    payload_string += (char)payload[i];
  }
  Serial.println(payload_string);
  if (strcmp(topic, MQTT_TOPIC_IN_LED) == 0) {
     set_info_led(payload_string);
  } 
  else if (strcmp(topic, MQTT_TOPIC_IN_RESET) == 0) 
  {
     reset_sensors();
  } else if (strcmp(topic, MQTT_TOPIC_IN_DEBUGLEVEL) == 0) 
  {
     debugLevel = payload_string.toInt();
     Serial.print("DEBUG LEVEL set to: ");
     Serial.println(debugLevel);
  } 
}

void mqtt_publish(char* topic, String payload) {
  mqtt_counter++;
  Serial.print(EXHIBIT_NAME);
  Serial.print(": Publishing MQTT message to topic [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.print(payload);
  Serial.print(" -- ");
  Serial.println(mqtt_counter);
  
  mqtt_client.publish(topic, payload.c_str(), true);
}

void set_info_led(String payload) 
{
  Serial.print(EXHIBIT_NAME);
  Serial.print(": Set 'Info LED'... ");

  if (payload == "ON") {
    Serial.print("ON");
    digitalWrite(testled_outPin, HIGH);
  } else if (payload == "OFF") {
    Serial.print("OFF");
    digitalWrite(testled_outPin, LOW);
  } else {
    // analogWrite(CONTROLLINO_D4, payload.toFloat());
    Serial.print("INVALID SIGNAL - Must be 'ON' or 'OFF'!");
  }

  Serial.println();
}

///////////////////
// Data Processing
///////////////////


///////////////////
// EOF
///////////////////



