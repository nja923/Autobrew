/*
 GENERAL ISSUES/QUESTIONS/CONCERNS

 1.  Is there any way to make default statements to occur only the first time coming in to a function (like what is done with all my outputs going between functions
 or do I have to manually turn whatever I've turned on before leaving the previous function?
 */

#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>

//#include <FiniteStateMachine.h>

/*This program will function to automate the home brew process
 * It will function as a state machine, with each state have a corresponding
 * functions that will operate in that state.  There are certain interrupt/flags that will
 * move the system to the next state (temperature, flow control value, level switch, etc).
 */

//Constants and variables for WIFI connecting and grabbing file data
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


/*-----( Declare Constants )-----*/
#define RELAY_ON 1
#define RELAY_OFF 0

#define Relay_1  1  // Arduino Digital I/O pin number
#define Relay_2  2
#define Relay_3  3
#define Relay_4  4
#define Relay_5  5  
#define Relay_6  6
#define Relay_7  7
#define Relay_8  8
#define Relay_9  9
#define Relay_10  10
#define Relay_11  11
#define Relay_12  29 
#define Relay_13  13
#define Relay_14  14
#define Relay_15  15
#define Relay_16  16
#define HLT_Heater_Relay    17
#define BrewPot_Heater_Relay  18
#define Fermenter_Control_Relay 19
#define HLT_Level_Switch  20
#define BrewPotLevelSwitch  21
#define HLTOutputFlowSensor 22
#define BrewPotOutputFlowSensor 23
#define HLTTempSensor 24
#define MashTunTempSensor 25
#define BrewPotTempSensor 26
#define FermenterTempSensor 27
#define Pump_Relay  28
#define Button  40

//Define the currentstate values for my state machine
#define Chill_Out_const 0
#define Fill_HLT_const 1
#define Heat_HLT_const 2
#define Transfer_To_MT_const 3
#define Maintain_MT_const 4
#define Fill_BrewPot_const 5
#define Boil_Wort_const 6
#define Cool_Wort_const 7
#define Wort_To_Fermenter_const 8
#define Fermentation_const 9
/*-----( Declare objects )-----*/
/*-----( Declare Variables )-----*/
volatile int waittime; // Delay between changes
volatile int currentstate = 0;  //will be used in switch/case statement
volatile int MashTemp = 150;  //This will store the temperature that we want to mash
volatile int FermentTemp = 70;  //This will store the temperature that we want to keep the beer fermenting at
volatile int SpargeTemp = 170;  //This will store the temperature that we want to sparge
volatile int BrewTemp = 210; //This will store the temperature that we want to brew
int seconds_var = 0;  //This will store the timer interrupt increment value.  Will be seconds.  Will reset every 60 seconds
int minutes_var = 0;  //This variable will store how many minuts have progressed since the start of the program.
int time_var; //This value is used in the program to keep track of the current time value for comparisons later
volatile int MashTime = 120;  //This will store the time duration that we want to mash
volatile int BrewTime = 120;  //This will store the time duration that we want to boil our wort
volatile int FermentTime = 500;  //This will store the time duration that we want to ferment.  Might need to get creative with this as this will likely be >2 weeks.
volatile int ProcessStart = 0;
volatile int temp_flowrate_hlt_const = 0;
volatile int temp_flowrate_bp_const = 0;
bool temp_mash_var = 0;
const byte NUMBER_OF_STATES = 4; //how many states are we cycling through?

// count how many pulses!
volatile uint16_t pulses_HLT = 0, pulses_BP = 0;
// track the state of the pulse pin
volatile uint8_t lastflowpinstate_HLT, lastflowpinstate_BP;
// you can try to keep time of how long it is between pulses
volatile uint32_t lastflowratetimer_HLT = 0, lastflowratetimer_BP = 0;
// and use that to calculate a flow rate
volatile float flowrate_HLT, flowrate_BP;

void ChillOut();
void FillHLT();
void HeatHLT();
void TransferToMT();
void MaintainMT();
void FillBrewPot();
void BoilWort();
void CoolWort();
void WortToFermenter();
void Fermenting();

/*
//initialize states
State Chill_Out = State(ChillOut);
State Fill_HLT = State(FillHLT);
State Heat_HLT = State(HeatHLT);
State Transfer_To_MT = State(TransferToMT);
State Maintain_MT = State(MaintainMT);
State Fill_BrewPot = State(FillBrewPot);
State Boil_Wort = State(BoilWort);
State Cool_Wort = State(CoolWort);
State Wort_To_Fermenter = State(WortToFermenter);
State Fermentation = State(Fermenting);
*/

void setup()   /****** SETUP: RUNS ONCE ******/
{
    waittime = 1000;
//-------( Initialize Pins so relays are inactive at reset)----
  digitalWrite(Relay_1, RELAY_OFF);
  digitalWrite(Relay_2, RELAY_OFF);
  digitalWrite(Relay_3, RELAY_OFF);
  digitalWrite(Relay_4, RELAY_OFF);  
  digitalWrite(Relay_5, RELAY_OFF);
  digitalWrite(Relay_6, RELAY_OFF);
  digitalWrite(Relay_7, RELAY_OFF);
  digitalWrite(Relay_8, RELAY_OFF);
  digitalWrite(Relay_9, RELAY_OFF);
  digitalWrite(Relay_10, RELAY_OFF);
  digitalWrite(Relay_11, RELAY_OFF);
  digitalWrite(Relay_12, RELAY_OFF);  
  digitalWrite(Relay_13, RELAY_OFF);
  digitalWrite(Relay_14, RELAY_OFF);
  digitalWrite(Relay_15, RELAY_OFF);
  digitalWrite(Relay_16, RELAY_OFF);
  digitalWrite(HLT_Heater_Relay, RELAY_OFF);
  digitalWrite(BrewPot_Heater_Relay, RELAY_OFF);
  digitalWrite(Fermenter_Control_Relay, RELAY_OFF);
  digitalWrite(HLT_Level_Switch, RELAY_OFF);
  digitalWrite(BrewPotLevelSwitch, RELAY_OFF);
  digitalWrite(HLTOutputFlowSensor, RELAY_ON);
  digitalWrite(BrewPotOutputFlowSensor, RELAY_ON);
  digitalWrite(HLTTempSensor, RELAY_OFF);
  digitalWrite(MashTunTempSensor, RELAY_OFF);
  digitalWrite(BrewPotTempSensor, RELAY_OFF);
  digitalWrite(FermenterTempSensor, RELAY_OFF);
  digitalWrite(Pump_Relay, RELAY_OFF);
  digitalWrite(Button, RELAY_OFF);
  
  //---( THEN set pins as outputs )----  
  pinMode(Relay_1, OUTPUT);   
  pinMode(Relay_2, OUTPUT);  
  pinMode(Relay_3, OUTPUT);  
  pinMode(Relay_4, OUTPUT);    
  pinMode(Relay_5, OUTPUT);   
  pinMode(Relay_6, OUTPUT);  
  pinMode(Relay_7, OUTPUT);  
  pinMode(Relay_8, OUTPUT);
  pinMode(Relay_9, OUTPUT);   
  pinMode(Relay_10, OUTPUT);  
  pinMode(Relay_11, OUTPUT);  
  pinMode(Relay_12, OUTPUT);    
  pinMode(Relay_13, OUTPUT);   
  pinMode(Relay_14, OUTPUT);  
  pinMode(Relay_15, OUTPUT);  
  pinMode(Relay_16, OUTPUT);
  pinMode(HLT_Heater_Relay, OUTPUT);
  pinMode(BrewPot_Heater_Relay, OUTPUT);
  pinMode(Fermenter_Control_Relay, OUTPUT);
  pinMode(HLT_Level_Switch, INPUT);
  pinMode(BrewPotLevelSwitch, INPUT);
  pinMode(HLTOutputFlowSensor, INPUT);
  pinMode(BrewPotOutputFlowSensor, INPUT);
  pinMode(HLTTempSensor, INPUT);
  pinMode(MashTunTempSensor, INPUT);
  pinMode(BrewPotTempSensor, INPUT);
  pinMode(FermenterTempSensor, INPUT);
  pinMode(Pump_Relay, OUTPUT);
  pinMode(Button, INPUT);
  //delay(4000); //Check that all relays are inactive at Reset
  


  /**/
  
  Serial.begin(115200);
  Serial.println(F("Hello, Awesome Man!\n")); 

  Serial.print("AutoBrew 10/22/15");
  
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
  /*Serial.println(F("Client Closed."));
  delay(5000);
  Serial.print(F("\nTrying to connect to server."));
  t = millis();
  do{
    //client.close();
    client = cc3000.connectTCP(ip, port);
    Serial.print(".");
  }
  while((!client.connected()) && ((millis() - t) < 10000));
  Serial.println(F("Done Connecting."));*/
  // initialize timer1 
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  //TCCR2A = 0;
  //TCCR2B = 0;
  //TCNT2  = 0;

  //interrupt will occur once a second
  OCR1A = 15624;            // compare match register = ((16*10^6) / (1024 * 1)) - 1
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= (1 << CS12) | (1 <<CS10);    // 1024 prescaler 
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  
  /*//Interrupt will occur once a millisecond
  OCR2A = 124;            // compare match register = ((16*10^6) / (64 * 1000)) - 1
  TCCR2B |= (1 << WGM21);   // CTC mode
  TCCR2B |= (1 << CS21) | (1 <<CS20);    // 64 prescaler 
  TIMSK2 |= (1 << OCIE2A);  // enable timer compare interrupt
  //interrupts();  */   

}//--(end setup )---

ISR(TIMER1_COMPA_vect) //Interrupt routine that will happen every second.  We want to increment a value and after every 60seconds we want to increment a minutes value
{
  seconds_var++; //increment out seconds each time interrupt fires
  if(seconds_var == 60) //if we have 60 interrupt, that means we have 60 seconds.  We want to reset the seconds to 0 and increment our minuts variable
  {
    seconds_var = 0;
    minutes_var++;
  }

  //Need to toggle an LED to ensure that the system is functioning properly
}

/*//ISR(TIMER2_COMPA_vect)
//{
//  //Interrupt will monitor flow sensors
//  uint8_t x1 = digitalRead(HLTOutputFlowSensor);
//  uint8_t x2 = digitalRead(BrewPotOutputFlowSensor);
//
//  int NoHLTFlow;
//
//  if (x1 == lastflowpinstate_HLT)
//  {
//    if (lastflowratetimer_HLT >= 1000)
//    {
//      NoHLTFlow = 1;
//    }
//    else
//    {
//      lastflowratetimer_HLT++;
//      NoHLTFlow = 0;
//    }
//  }
//
//  if (x1 == HIGH)
//  {
//    lastflowratetimer_HLT = 0;
//  }
//
//}*/

SIGNAL(TIMER0_COMPA_vect) {
  uint8_t x_HLT = digitalRead(HLTOutputFlowSensor);
  uint8_t x_BP = digitalRead(BrewPotOutputFlowSensor);

  if(currentstate == Transfer_To_MT_const) {
    if (x_HLT == lastflowpinstate_HLT) {
      lastflowratetimer_HLT++;
      return; //nothing changed!
    }

    if (x_HLT == HIGH) {
      //low to high transition!
      pulses_HLT++;
    }
    lastflowpinstate_HLT = x_HLT;
    flowrate_HLT = 1000.0;
    flowrate_HLT /= lastflowratetimer_HLT; // in hertz
    lastflowratetimer_HLT = 0;
  }
  if(currentstate == Wort_To_Fermenter_const) {
    if (x_HLT == lastflowpinstate_BP) {
      lastflowratetimer_BP++;
      return; //nothing changed!
    }

    if (x_BP == HIGH) {
      //low to high transition!
      pulses_BP++;
    }
    lastflowpinstate_BP = x_BP;
    flowrate_BP = 1000.0;
    flowrate_BP /= lastflowratetimer_BP; // in hertz
    lastflowratetimer_BP = 0;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(2000);
  //Need to  increment currentstate whenever I want to move into another state
  if(currentstate == 0 && Button == HIGH){ //If we're in the state of 0 and we press the button, we want the Arduino to try to grab data from RaspiPi
    delay(5000);
    Serial.print(F("\nTrying to connect to server."));
    t = millis();
    do{
    client = cc3000.connectTCP(ip, port);
    Serial.print(".");
    }
    while((!client.connected()) &&
    ((millis() - t) < 10000));
    if (client.connected()) {
      String request = "GET /tempandtime.txt HTTP/1.0\r\nConnection: close\r\n\r\n";
      client.println("GET /tempandtime.txt HTTP/1.0\r\nConnection: close\r\n\r\n");
      client.println("Host: localhost");
      client.println("Connection: close");
      client.println();
      Serial.println("...Sending request:");
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
    //Serial.println(F("...closing socket"));
    client.close();
    //wait some amount of time before sending temperature/humidity to the PHP service.
    //delay(50000);
    //currentstate = 1;
  }
  
  if (currentstate >9) {currentstate = 0;}
  
  //if(ProcessStart==1){
    switch(currentstate)
  {
    /*case Chill_Out_const: stateMachine.transitionTo(Chill_Out);
    case Fill_HLT_const: stateMachine.transitionTo(Fill_HLT);
    case Heat_HLT_const: stateMachine.transitionTo(Heat_HLT);
    case Transfer_To_MT_const = stateMachine.transitionTo(Transfer_To_MT);
    case Maintain_MT_const = stateMachine.transitionTo(Maintain_MT);
    case Fill_BrewPot_const = stateMachine.transitionTo(Fill_BrewPot);
    case Boil_Wort_const = stateMachine.transitionTo(Boil_Wort);
    case Cool_Wort_const = stateMachine.transitionTo(Cool_Wort);
    case Wort_To_Fermenter_const = stateMachine.transitionTo(Wort_To_Fermenter);
    case Fermentation_const = stateMachine.transitionTo(Fermentation);
    */

    case Chill_Out_const: 
      Serial.println(F("Entering Chill Out"));
      ChillOut();
    case Fill_HLT_const:  
      Serial.println(F("Entering Fill HLT"));
      FillHLT();
    case Heat_HLT_const:  
      Serial.println(F("Entering Heat HLT"));
      HeatHLT();
    case Transfer_To_MT_const:  
      Serial.println(F("Entering Transfer to MT"));
      TransferToMT();
    case Maintain_MT_const:  
      Serial.println(F("Entering Maintain MT"));
      MaintainMT();
    case Fill_BrewPot_const:  
      Serial.println(F("Entering Fill Brew Pot"));
      FillBrewPot();
    case Boil_Wort_const:  
      Serial.println(F("Entering Boil Wort"));
      BoilWort();
    case Cool_Wort_const:  
      Serial.println(F("Entering Cool Wort"));
      CoolWort();
    case Wort_To_Fermenter_const:  
      Serial.println(F("Entering Wort to Fermenter"));
      WortToFermenter();
    case Fermentation_const:  
      Serial.println(F("Entering Fermentation"));
      Fermenting();
  //}
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
        
        MashTemp = justRates.substring(0,3).toFloat();
        SpargeTemp = justRates.substring(3,6).toFloat();
        BrewTemp = justRates.substring(6,9).toFloat();
        FermentTemp = justRates.substring(9,12).toFloat();
        MashTime = justRates.substring(12,15).toFloat();
        BrewTime = justRates.substring(15,18).toFloat();
        FermentTime = justRates.substring(18,21).toFloat();
        ProcessStart = justRates.substring(21,22).toFloat();

        Serial.println("Mash Temp: " + String(MashTemp));
        Serial.println("Sparge Temp: " + String(SpargeTemp));
        Serial.println("Brew Temp: " + String(BrewTemp));
        Serial.println("Ferment Temp: " + String(FermentTemp));
        Serial.println("Mash Time: " + String(MashTime));
        Serial.println("Brew Time: " + String(BrewTime));
        Serial.println("Ferment Time: " + String(FermentTime));
        Serial.println("Process Start: " + String(ProcessStart));
        currRates = "";
        //client.stop();
        currentstate = 1;
        return;
      }
    }
  }
}

void ChillOut()  //This function is my idle function where nothing is done
{
  digitalWrite(Relay_1, RELAY_OFF);
  digitalWrite(Relay_2, RELAY_OFF);
  digitalWrite(Relay_3, RELAY_OFF);
  digitalWrite(Relay_4, RELAY_OFF);  
  digitalWrite(Relay_5, RELAY_OFF);
  digitalWrite(Relay_6, RELAY_OFF);
  digitalWrite(Relay_7, RELAY_OFF);
  digitalWrite(Relay_8, RELAY_OFF);
  digitalWrite(Relay_9, RELAY_OFF);
  digitalWrite(Relay_10, RELAY_OFF);
  digitalWrite(Relay_11, RELAY_OFF);
  digitalWrite(Relay_12, RELAY_OFF);  
  digitalWrite(Relay_13, RELAY_OFF);
  digitalWrite(Relay_14, RELAY_OFF);
  digitalWrite(Relay_15, RELAY_OFF);
  digitalWrite(Relay_16, RELAY_OFF);
  digitalWrite(HLT_Heater_Relay, RELAY_OFF);
  digitalWrite(BrewPot_Heater_Relay, RELAY_OFF);
  digitalWrite(Fermenter_Control_Relay, RELAY_OFF);
  digitalWrite(Pump_Relay, RELAY_OFF);

  seconds_var = 0;
  minutes_var = 0;

  if(!Button){
    Serial.println(F("Still in Chill Out."));
    delay(1000);
  }
  else {
    delay(2000);
    currentstate++;
  }
  //currentstate++;
  return;
}

void FillHLT()  //This function will Fill the HLT with water.  Denoted by Red in schematic
{
  digitalWrite(Relay_1, RELAY_ON);
  /*
  digitalWrite(Relay_2, RELAY_OFF);
  digitalWrite(Relay_3, RELAY_OFF);
  digitalWrite(Relay_4, RELAY_OFF);  
  digitalWrite(Relay_5, RELAY_OFF);
  digitalWrite(Relay_6, RELAY_OFF);
  digitalWrite(Relay_7, RELAY_OFF);
  digitalWrite(Relay_8, RELAY_OFF);
  digitalWrite(Relay_9, RELAY_OFF);
  digitalWrite(Relay_10, RELAY_OFF);
  digitalWrite(Relay_11, RELAY_OFF);
  digitalWrite(Relay_12, RELAY_OFF);  
  digitalWrite(Relay_13, RELAY_OFF);
  digitalWrite(Relay_14, RELAY_OFF);
  digitalWrite(Relay_15, RELAY_OFF);
  digitalWrite(Relay_16, RELAY_OFF);
  digitalWrite(HLT_Heater_Relay, RELAY_OFF);
  digitalWrite(BrewPot_Heater_Relay, RELAY_OFF);
  digitalWrite(Fermenter_Control_Relay, RELAY_OFF);
  digitalWrite(Pump_Relay, RELAY_OFF);
*/
  delay(1000);  //Wait 1 second before looking for the level switch

  while(HLT_Level_Switch == 1)  //This is intended to "debounce the level switch to make sure it is completely activated for 2 seconds before moving us into the next state
  {
    delay(2000);
    currentstate++;
    digitalWrite(Relay_1, RELAY_OFF);
  }
  
  if(Button) //If we press the button, we want to move into the next state
  {
    delay(2000);
    currentstate++;
    digitalWrite(Relay_1, RELAY_OFF);
  }
  return;
}

void HeatHLT()  //This function will Heat the HLT to a temperature x degrees above what I want to mash at
{
//  digitalWrite(Relay_1, RELAY_OFF);
//  digitalWrite(Relay_2, RELAY_OFF);
//  digitalWrite(Relay_3, RELAY_OFF);
//  digitalWrite(Relay_4, RELAY_OFF);  
//  digitalWrite(Relay_5, RELAY_OFF);
//  digitalWrite(Relay_6, RELAY_OFF);
//  digitalWrite(Relay_7, RELAY_OFF);
//  digitalWrite(Relay_8, RELAY_OFF);
//  digitalWrite(Relay_9, RELAY_OFF);
//  digitalWrite(Relay_10, RELAY_OFF);
//  digitalWrite(Relay_11, RELAY_OFF);
//  digitalWrite(Relay_12, RELAY_OFF);  
//  digitalWrite(Relay_13, RELAY_OFF);
//  digitalWrite(Relay_14, RELAY_OFF);
//  digitalWrite(Relay_15, RELAY_OFF);
//  digitalWrite(Relay_16, RELAY_OFF);
  digitalWrite(HLT_Heater_Relay, RELAY_ON);
//  digitalWrite(BrewPot_Heater_Relay, RELAY_OFF);
//  digitalWrite(Fermenter_Control_Relay, RELAY_OFF);
//  digitalWrite(Pump_Relay, RELAY_OFF);

  if(HLTTempSensor >= (MashTemp+15))  //We want to heat the HLT to a temperature 15 degrees higher than what we want to mash at.  This is because the temperature will decrease 10-15 degrees when transferring into grains
  {
    currentstate++;
    digitalWrite(HLT_Heater_Relay, RELAY_OFF);
  }
  
  if (Button == 1) //If we press the button, we want to move into the next state
  {
    currentstate++;
    digitalWrite(HLT_Heater_Relay, RELAY_OFF);
  }
  return;
}

void TransferToMT()  //This function will transfter the hot water from the hlt to the mash tun for mashing
{
//  digitalWrite(Relay_1, RELAY_OFF);
//  digitalWrite(Relay_2, RELAY_OFF);
//  digitalWrite(Relay_3, RELAY_OFF);
  digitalWrite(Relay_4, RELAY_ON);  
//  digitalWrite(Relay_5, RELAY_OFF);
//  digitalWrite(Relay_6, RELAY_OFF);
//  digitalWrite(Relay_7, RELAY_OFF);
//  digitalWrite(Relay_8, RELAY_OFF);
//  digitalWrite(Relay_9, RELAY_OFF);
//  digitalWrite(Relay_10, RELAY_OFF);
//  digitalWrite(Relay_11, RELAY_OFF);
//  digitalWrite(Relay_12, RELAY_OFF);  
//  digitalWrite(Relay_13, RELAY_OFF);
//  digitalWrite(Relay_14, RELAY_OFF);
//  digitalWrite(Relay_15, RELAY_OFF);
//  digitalWrite(Relay_16, RELAY_OFF);
//  digitalWrite(HLT_Heater_Relay, RELAY_OFF);
//  digitalWrite(BrewPot_Heater_Relay, RELAY_OFF);
//  digitalWrite(Fermenter_Control_Relay, RELAY_OFF);
//  digitalWrite(Pump_Relay, RELAY_OFF);

////////////////////////////////////////////////////////////
//Need to add in coding for the HLT Flow sensor interrupts//
////////////////////////////////////////////////////////////

  while(flowrate_HLT == 0 && temp_flowrate_hlt_const == 0){}; //when we initially come into this state, we dont' want to do anything if we haven't started to flow yet
  temp_flowrate_hlt_const = 1;

  while(flowrate_HLT == 0 && temp_flowrate_hlt_const == 1)  //We've already had some flow but now it's stopped, we need to make sure we're completely done and then move to next state
  {
    delay(2000);
    currentstate++;
    digitalWrite(Relay_4, RELAY_OFF); 
    time_var = minutes_var;//insert current time variable from interrupt.  This will be used in the next state
  }
  
  if (Button == 1) //If we press the button, we want to move into the next state
  {
    currentstate++;
    digitalWrite(Relay_4, RELAY_OFF); 
    time_var = minutes_var;//insert current time variable from interrupt.  This will be used in the next state
  }
  return;
}

void MaintainMT()  //This function will maintain the temp in the mash tun by sending it out through coils in hlt
{
//  digitalWrite(Relay_1, RELAY_OFF);
//  digitalWrite(Relay_2, RELAY_OFF);
//  digitalWrite(Relay_3, RELAY_OFF);
//  digitalWrite(Relay_4, RELAY_OFF);  
//  digitalWrite(Relay_5, RELAY_OFF);
//  digitalWrite(Relay_6, RELAY_OFF);
//  digitalWrite(Relay_7, RELAY_OFF);
//  digitalWrite(Relay_8, RELAY_OFF);
//  digitalWrite(Relay_9, RELAY_OFF);
//  digitalWrite(Relay_10, RELAY_OFF);
//  digitalWrite(Relay_11, RELAY_OFF);
//  digitalWrite(Relay_12, RELAY_OFF);  
//  digitalWrite(Relay_13, RELAY_OFF);
//  digitalWrite(Relay_14, RELAY_OFF);
//  digitalWrite(Relay_15, RELAY_OFF);
//  digitalWrite(Relay_16, RELAY_OFF);
//  digitalWrite(HLT_Heater_Relay, RELAY_OFF);
//  digitalWrite(BrewPot_Heater_Relay, RELAY_OFF);
//  digitalWrite(Fermenter_Control_Relay, RELAY_OFF);
//  digitalWrite(Pump_Relay, RELAY_OFF);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Need to figure out a way to keep track of time, because the total duration of this function is dependent on time, not a specific action happening//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  while(HLT_Level_Switch != 1 && temp_mash_var == 0)
  {
    digitalWrite(Relay_1, RELAY_ON);  //While the HLT is not full, allow water to flow into HLT
  }
  digitalWrite(Relay_1, RELAY_OFF);  //Once HLT is full, turn off the relay allowing water into the HLT

  //Next we want to start to heat up the Mash Tun.  
  //This while loop is for the initial heating of the HLT before we start to cycle liquid through it.
  while(HLTTempSensor < MashTemp && temp_mash_var == 0)
  {
    digitalWrite(HLT_Heater_Relay, RELAY_ON);
  }
  digitalWrite(HLT_Heater_Relay, RELAY_OFF);

  //Next we want to start to cycle liquid from the Mash Tun through the coils in the HLT
  digitalWrite(Relay_5, RELAY_ON);
  digitalWrite(Relay_10, RELAY_ON);
  digitalWrite(Relay_14, RELAY_ON);
  delay(5000);
  digitalWrite(Pump_Relay, RELAY_ON);
  temp_mash_var = 1;
  
  //The if statement covers any case that the temperature might start to decrease.
  if(HLTTempSensor < MashTemp)
  {
    digitalWrite(HLT_Heater_Relay, RELAY_ON);
  }
  else
  {
    digitalWrite(HLT_Heater_Relay, RELAY_OFF);
  }

  int time_compare_mash = minutes_var - time_var;

  if (time_compare_mash >= MashTime)
  {
    currentstate++;
    digitalWrite(Relay_5, RELAY_OFF);
    digitalWrite(Relay_10, RELAY_OFF);
    digitalWrite(Relay_14, RELAY_OFF);
    digitalWrite(Pump_Relay, RELAY_OFF);
    digitalWrite(HLT_Heater_Relay, RELAY_OFF);
  }
  
  if (Button == 1) //If we press the button, we want to move into the next state
  {
    currentstate++;
  }
  return;
}

void FillBrewPot()  //This function will fill the brew pot with the sweet wort from the mash tun
{

//  digitalWrite(Relay_1, RELAY_OFF);
//  digitalWrite(Relay_2, RELAY_OFF);
//  digitalWrite(Relay_3, RELAY_OFF);
//  digitalWrite(Relay_4, RELAY_OFF);  
//  digitalWrite(Relay_5, RELAY_OFF);
//  digitalWrite(Relay_6, RELAY_OFF);
//  digitalWrite(Relay_7, RELAY_OFF);
//  digitalWrite(Relay_8, RELAY_OFF);
//  digitalWrite(Relay_9, RELAY_OFF);
//  digitalWrite(Relay_10, RELAY_OFF);
//  digitalWrite(Relay_11, RELAY_OFF);
//  digitalWrite(Relay_12, RELAY_OFF);  
//  digitalWrite(Relay_13, RELAY_OFF);
//  digitalWrite(Relay_14, RELAY_OFF);
//  digitalWrite(Relay_15, RELAY_OFF);
//  digitalWrite(Relay_16, RELAY_OFF);
//  digitalWrite(HLT_Heater_Relay, RELAY_OFF);
//  digitalWrite(BrewPot_Heater_Relay, RELAY_OFF);
//  digitalWrite(Fermenter_Control_Relay, RELAY_OFF);
//  digitalWrite(Pump_Relay, RELAY_OFF);

//Need to open the relay to allow flow from mash tun to brew pot
//Need to open the relay to allow the HLT liquid to flow through the pump, back into the coils and into the mash tun
//Need to monitor the level switch 

  if (BrewPotLevelSwitch == 1) {
    //Turn off all relays
    delay(2000);
    currentstate++;
    //time_var = minutes_var;//insert current time variable from interrupt.  This will be used in the next state. //Not used here.  Want to wait until we're boiling before we store value
  }
    
  if (Button == 1) //If we press the button, we want to move into the next state
  {
    currentstate++;
    //time_var = minutes_var;//insert current time variable from interrupt.  This will be used in the next state. //Not used here.  Want to wait until we're boiling before we store value
  }
  return;
}

void BoilWort()  //This function will start to boil the water in the brew pot
{
  /*digitalWrite(Relay_1, RELAY_OFF);
  digitalWrite(Relay_2, RELAY_OFF);
  digitalWrite(Relay_3, RELAY_OFF);
  digitalWrite(Relay_4, RELAY_OFF);  
  digitalWrite(Relay_5, RELAY_OFF);
  digitalWrite(Relay_6, RELAY_OFF);
  digitalWrite(Relay_7, RELAY_OFF);
  digitalWrite(Relay_8, RELAY_OFF);
  digitalWrite(Relay_9, RELAY_OFF);
  digitalWrite(Relay_10, RELAY_OFF);
  digitalWrite(Relay_11, RELAY_OFF);
  digitalWrite(Relay_12, RELAY_OFF);  
  digitalWrite(Relay_13, RELAY_OFF);
  digitalWrite(Relay_14, RELAY_OFF);
  digitalWrite(Relay_15, RELAY_OFF);
  digitalWrite(Relay_16, RELAY_OFF);
  digitalWrite(HLT_Heater_Relay, RELAY_OFF);
  digitalWrite(BrewPot_Heater_Relay, RELAY_OFF);
  digitalWrite(Fermenter_Control_Relay, RELAY_OFF);
  digitalWrite(Pump_Relay, RELAY_OFF);
*/

  if(BrewPotTempSensor <= BrewTemp) {
     digitalWrite(BrewPot_Heater_Relay, RELAY_ON);
  }

  while(BrewPotTempSensor <= (BrewTemp - 20)){
    if (BrewPotTempSensor >= (BrewTemp - 23)) {
      time_var = minutes_var;
    }
  }
  int time_compare_brew = minutes_var - time_var;

  if (time_compare_brew >= BrewTime)
  {
    currentstate++;
    //Turn off all relays
    digitalWrite(BrewPot_Heater_Relay, RELAY_ON);
  }
  
  if (Button == 1) //If we press the button, we want to move into the next state
  {
    currentstate++;
  }
  return;
}

void CoolWort()  //This function will start to cool the wort by pushing cool water through the wort chiller and pumping wort out and into the brew pot
{
 /* digitalWrite(Relay_1, RELAY_OFF);
  digitalWrite(Relay_2, RELAY_OFF);
  digitalWrite(Relay_3, RELAY_OFF);
  digitalWrite(Relay_4, RELAY_OFF);  
  digitalWrite(Relay_5, RELAY_OFF);
  digitalWrite(Relay_6, RELAY_OFF);
  digitalWrite(Relay_7, RELAY_OFF);
  digitalWrite(Relay_8, RELAY_OFF);
  digitalWrite(Relay_9, RELAY_OFF);
  digitalWrite(Relay_10, RELAY_OFF);
  digitalWrite(Relay_11, RELAY_OFF);
  digitalWrite(Relay_12, RELAY_OFF);  
  digitalWrite(Relay_13, RELAY_OFF); 
  digitalWrite(Relay_14, RELAY_OFF);
  digitalWrite(Relay_15, RELAY_OFF);
  digitalWrite(Relay_16, RELAY_OFF);
  digitalWrite(HLT_Heater_Relay, RELAY_OFF);
  digitalWrite(BrewPot_Heater_Relay, RELAY_OFF);
  digitalWrite(Fermenter_Control_Relay, RELAY_OFF);
  digitalWrite(Pump_Relay, RELAY_OFF);
*/

  //Need to activate the relays to allow the water to flow through wort chiller and back up into HLT 
  
  //Need to activate the relays to allow the wort to flow through pump and back into brew pot.
  
  //Need to activate the pump relay
  digitalWrite(Pump_Relay, RELAY_ON);

  //Check to see if the temperature in the brew pot has decreased below setpoint for fermentation
  

  
  if (Button == 1) //If we press the button, we want to move into the next state
  {
    currentstate++;
  }
}

void WortToFermenter()  //This function will transfer wort from the brew pot to the fermenter
{
  digitalWrite(Relay_1, RELAY_OFF);
  digitalWrite(Relay_2, RELAY_OFF);
  digitalWrite(Relay_3, RELAY_OFF);
  digitalWrite(Relay_4, RELAY_OFF);  
  digitalWrite(Relay_5, RELAY_OFF);
  digitalWrite(Relay_6, RELAY_OFF);
  digitalWrite(Relay_7, RELAY_OFF);
  digitalWrite(Relay_8, RELAY_OFF);
  digitalWrite(Relay_9, RELAY_OFF);
  digitalWrite(Relay_10, RELAY_OFF);
  digitalWrite(Relay_11, RELAY_OFF);
  digitalWrite(Relay_12, RELAY_OFF);  
  digitalWrite(Relay_13, RELAY_OFF);
  digitalWrite(Relay_14, RELAY_OFF);
  digitalWrite(Relay_15, RELAY_OFF);
  digitalWrite(Relay_16, RELAY_OFF);
  digitalWrite(HLT_Heater_Relay, RELAY_OFF);
  digitalWrite(BrewPot_Heater_Relay, RELAY_OFF);
  digitalWrite(Fermenter_Control_Relay, RELAY_OFF);
  digitalWrite(Pump_Relay, RELAY_OFF);

  if (Button == 1) //If we press the button, we want to move into the next state
  {
    currentstate++;
  }
}

void Fermenting()  //This function will maintain the temperature of the fermenting refridgerator
{
  digitalWrite(Relay_1, RELAY_OFF);
  digitalWrite(Relay_2, RELAY_OFF);
  digitalWrite(Relay_3, RELAY_OFF);
  digitalWrite(Relay_4, RELAY_OFF);  
  digitalWrite(Relay_5, RELAY_OFF);
  digitalWrite(Relay_6, RELAY_OFF);
  digitalWrite(Relay_7, RELAY_OFF);
  digitalWrite(Relay_8, RELAY_OFF);
  digitalWrite(Relay_9, RELAY_OFF);
  digitalWrite(Relay_10, RELAY_OFF);
  digitalWrite(Relay_11, RELAY_OFF);
  digitalWrite(Relay_12, RELAY_OFF);  
  digitalWrite(Relay_13, RELAY_OFF);
  digitalWrite(Relay_14, RELAY_OFF);
  digitalWrite(Relay_15, RELAY_OFF);
  digitalWrite(Relay_16, RELAY_OFF);
  digitalWrite(HLT_Heater_Relay, RELAY_OFF);
  digitalWrite(BrewPot_Heater_Relay, RELAY_OFF);
  digitalWrite(Fermenter_Control_Relay, RELAY_OFF);
  digitalWrite(Pump_Relay, RELAY_OFF);

  if(Button == 1) //If we press the button, we want to move into the next state
  {
    currentstate++;
  }
}

