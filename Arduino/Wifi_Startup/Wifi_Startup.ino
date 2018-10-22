#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
//#include "utility/debug.h"
//#include "mysql.h"



// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER); // you can change this clock speed

#define WLAN_SSID_0       "FBI Surveillance Team 5"           // cannot be longer than 32 characters!
#define WLAN_PASS_0       "raspberry"
#define WLAN_SSID_1       "tcom-guest"           // cannot be longer than 32 characters!
#define WLAN_PASS_1       "TCOM@TEDrive"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

#define IDLE_TIMEOUT_MS  3000      // Amount of time to wait (in milliseconds) with no data 
                                   // received before closing the connection.  If you know the server
                                   // you're accessing is quick to respond, you can reduce this value.
Adafruit_CC3000_Client client;

byte mac[] = { 0x70, 0xFF, 0x76, 0x02, 0x7F, 0x12 };  

uint32_t ipAddress_0 = cc3000.IP2U32(192, 168, 0, 111);
  uint32_t netMask_0 = cc3000.IP2U32(255, 255, 255, 0);
  uint32_t defaultGateway_0 = cc3000.IP2U32(192, 168, 0, 1);
  uint32_t dns_0 = cc3000.IP2U32(8, 8, 8, 8);
  uint32_t ipAddress_1 = cc3000.IP2U32(0, 0, 0, 0);
  uint32_t netMask_1 = cc3000.IP2U32(255, 255, 255, 0);
  uint32_t defaultGateway_1 = cc3000.IP2U32(192, 168, 12, 1);
  uint32_t dns_1 = cc3000.IP2U32(0, 0, 0, 0);
  uint32_t server_addr = (192, 168, 0, 105);
  uint32_t ip = cc3000.IP2U32(192,168,0,105);
  int port = 80;
  String repository = "/html/";

String currentLine = "";
String currRates = "";
boolean readingRates = false;

int network_select = 0;

uint32_t   t;

void setup(void)
{
 Serial.begin(115200);
  Serial.println(F("Hello, Awesome Man!\n")); 

  Serial.print("Wifi_Startup 10/21/15");
  
  /* Initialise the module */
  Serial.println(F("\nInitializing..."));
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }

   //Connection to RaspberryPi
  if(!network_select){
    if (!cc3000.setStaticIPAddress(ipAddress_0, netMask_0, defaultGateway_0, dns_0)) {
      Serial.println(F("Failed to set static IP!"));
      while(1);
    }
    
    Serial.print(F("\nAttempting to connect to ")); Serial.println(WLAN_SSID_0);
    if (!cc3000.connectToAP(WLAN_SSID_0, WLAN_PASS_0, WLAN_SECURITY)) {
      Serial.println(F("Failed!"));
      while(1);
    }
  }
  //connection to tcom network
  else{
    if (!cc3000.setStaticIPAddress(ipAddress_1, netMask_1, defaultGateway_1, dns_1)) {
      Serial.println(F("Failed to set static IP!"));
      while(1);
    }
    
    Serial.print(F("\nAttempting to connect to ")); Serial.println(WLAN_SSID_1);
    if (!cc3000.connectToAP(WLAN_SSID_1, WLAN_PASS_1, WLAN_SECURITY)) {
      Serial.println(F("Failed!"));
      while(1);
    }
  }
   
  Serial.println(F("Connected!"));
  client.close();
  
}



void loop(void)
{
 delay(10000);
 Serial.print(F("\nTrying to connect to server."));
 t = millis();
 do{
  client = cc3000.connectTCP(ip, port);
  Serial.print(".");
 }
 while((!client.connected()) &&
  ((millis() - t) < 10000));
  if (client.connected()) {
    Serial.println("Connected"); 
    String request = "GET /tempandtime.txt HTTP/1.0\r\nConnection: close\r\n\r\n";
    //String request = "GET "+ repository + "tempandtime.txt" + " HTTP/1.0\r\nConnection: close\r\n\r\n";
    client.println("GET /tempandtime.txt HTTP/1.0\r\nConnection: close\r\n\r\n");
    client.println("Host: localhost");
    client.println("Connection: close");
    client.println();
    Serial.print("...Sending request:");
    //Serial.println(request);
    //send_request(request);
  }
  else {
    Serial.print(F("\nClient not connected"));
    return;
  }
  Serial.println("...Reading response");
  show_response();

  Serial.println(F("Cleaning up..."));
  Serial.println(F("...closing socket"));
  //client.close();
  //wait some amount of time before sending temperature/humidity to the PHP service.
  //delay(50000);
}

bool send_request (String request) {
  // Transform to char
  char requestBuf[request.length()+1];
  request.toCharArray(requestBuf,request.length()); 
  // Send request
  if (client.connected()) {
    client.fastrprintln(requestBuf);
    client.println("Host: localhost");
  } 
  else {
    Serial.println(F("Connection failed"));    
    return false;
  }
  return true;
  free(requestBuf);
}

void show_response_1() {
  Serial.println(F("-------------------------------------"));
  while (client.connected() || client.available()) {
    int text_array[21];
    int i = 0;
    char c_raw = client.read();
    if (c_raw == ';'){
      c_raw = client.read();
      while (c_raw != ';'){
        text_array[i]=(int)c_raw;
        c_raw = client.read();
        i++;
      }
    }
    for (i=0;i<21;i++){
      Serial.print((char)text_array[i]);
    }
    Serial.println(F("Done Parsing Data"));
  }
}


void show_response() {
  Serial.println(F("-------------------------------------"));
  while (client.connected() || client.available()) {
    char inChar = client.read();
    currentLine += inChar;
    // if you get a newline, clear the line:
    if (inChar == '\n') {
      currentLine = "";
    }
    if (currentLine.endsWith("<start>")) {
      readingRates = true;
    }
    else if (readingRates) {
      if (!currentLine.endsWith("<stop>")) { //'>' is our ending character
      currRates += inChar;
      }
      else {
        readingRates = false;
        String justRates = currRates.substring(0, currRates.length()-5);
        
        Serial.println("Data grabbed.");
        
        int MashTemp = justRates.substring(0,3).toFloat();
        int SpargeTemp = justRates.substring(3,6).toFloat();
        int BrewTemp = justRates.substring(6,9).toFloat();
        int FermentTemp = justRates.substring(9,12).toFloat();
        int MashTime = justRates.substring(12,15).toFloat();
        int BrewTime = justRates.substring(15,18).toFloat();
        int FermentTime = justRates.substring(18,21).toFloat();

        Serial.println("Mash Temp: " + String(MashTemp));
        Serial.println("Sparge Temp: " + String(SpargeTemp));
        Serial.println("Brew Temp: " + String(BrewTemp));
        Serial.println("Ferment Temp: " + String(FermentTemp));
        Serial.println("Mash Time: " + String(MashTime));
        Serial.println("Brew Time: " + String(BrewTime));
        Serial.println("Ferment Time: " + String(FermentTime));
        currRates = "";
        //client.stop();
        return;
      }
    }
  }
}

