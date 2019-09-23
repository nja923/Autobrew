/*
 GENERAL ISSUES/QUESTIONS/CONCERNS

 1.  Is there any way to make default statements to occur only the first time coming in to a function (like what is done with all my outputs going between functions
 or do I have to manually turn whatever I've turned on before leaving the previous function?
 */

//#include <Adafruit_CC3000.h>
//#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include <OneWire.h>

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
//Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIVIDER); // you can change this clock speed

#define WLAN_SSID_0       "FBI Surveillance Team 5"           // cannot be longer than 32 characters!
#define WLAN_PASS_0       "raspberry"
#define WLAN_SSID_1       "tcom-guest"           // cannot be longer than 32 characters!
#define WLAN_PASS_1       "TCOM@TEDrive"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

#define IDLE_TIMEOUT_MS  3000      // Amount of time to wait (in milliseconds) with no data 
                                   // received before closing the connection.  If you know the server
                                   // you're accessing is quick to respond, you can reduce this value.
//Adafruit_CC3000_Client client;

OneWire ds_HLT(24); //Temp sensors are located on pin 24
OneWire ds_MT(25); //Temp sensor for Mash Tun is on pin 25
OneWire ds_BP(26); //Temp sensor for Brew Pot is on pin 26
OneWire ds_Ferm(27); //Temp sensor for Fermenter is on pin 27

byte mac[] = { 0x70, 0xFF, 0x76, 0x02, 0x7F, 0x12 };  

  int port = 80;
  String repository = "/html/";

String currentLine = "";
String currRates = "";
boolean readingRates = false;

int network_select = 0;

uint32_t   t;


/*-----( Declare Constants )-----*/
#define RELAY_ON HIGH
#define RELAY_OFF LOW
#define LED_ON HIGH
#define LED_OFF LOW

#define Relay_1  33  // Arduino Digital I/O pin number
#define Relay_2  34
#define Relay_3  35
#define Relay_4  36
#define Relay_5  37  
#define Relay_6  38
#define Relay_7  39
#define Relay_8  8
#define Relay_9  9
#define Relay_10  10
#define HLT_Heater_Relay    17
#define BrewPot_Heater_Relay  18
#define Fermenter_Control_Relay 19
#define HLT_Level_Switch  20
#define BrewPotLevelSwitch  21
#define HLTOutputFlowSensor 22
#define BrewPotOutputFlowSensor 23
#define Pump_Relay  28
#define TempSensorLED 41
#define Button  40
#define Green_LED 29
#define Yellow_LED  30
#define Purple_LED  31
#define Red_LED   32

int HLTTempSensor, MashTunTempSensor, BrewPotTempSensor, FermenterTempSensor;

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
int temp_flow_transmit_timer = 0;
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
boolean toggle = false;
boolean toggle_1 = false;
unsigned char inByte, message_1_byte, message_6_byte, message_7_byte, message_8_byte, message_2_byte, message_3_byte, message_4_byte, message_5_byte, message_bad_byte;
int relay_array[16];
boolean MT_TS_Status = true;
boolean BP_TS_Status = true;
boolean HLT_TS_Status = true;
boolean Ferm_TS_Status = true;

int temp_count = 0;  //temporary counter for use wherever needed.

#define PitchYeastTemp  75

// count how many pulses!
volatile uint16_t pulses_HLT = 0, pulses_BP = 0;
// track the state of the pulse pin
volatile uint8_t lastflowpinstate_HLT, lastflowpinstate_BP;
// you can try to keep time of how long it is between pulses
volatile uint32_t lastflowratetimer_HLT = 0, lastflowratetimer_BP = 0;
// and use that to calculate a flow rate
volatile float flowrate_HLT, flowrate_BP;

//Variables for temp sensors
  byte i_HLT, i_BP, i_MT, i_Ferm;
  byte present_HLT, present_MT, present_BP, present_Ferm;
  byte data_HLT[12], data_MT[12], data_BP[12], data_Ferm[12];
  byte addr[8], addr_HLT[8], addr_MT[8], addr_BP[8], addr_Ferm[8];

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
void message_1();
void message_2();
void read_HLT_TS();
void read_MT_TS();
void read_BP_TS();
void read_Ferm_TS();

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
  }
}

void setup()   /****** SETUP: RUNS ONCE ******/
{
    seconds_var = 0;
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
  digitalWrite(HLT_Heater_Relay, RELAY_OFF);
  digitalWrite(BrewPot_Heater_Relay, RELAY_OFF);
  digitalWrite(Fermenter_Control_Relay, RELAY_OFF);
  digitalWrite(HLT_Level_Switch, RELAY_OFF);
  digitalWrite(BrewPotLevelSwitch, RELAY_OFF);
  digitalWrite(HLTOutputFlowSensor, RELAY_OFF);
  digitalWrite(BrewPotOutputFlowSensor, RELAY_OFF);
  digitalWrite(Pump_Relay, RELAY_OFF);
  digitalWrite(Button, RELAY_OFF);
  digitalWrite(Green_LED, RELAY_OFF);
  digitalWrite(Yellow_LED, RELAY_OFF);
  digitalWrite(Purple_LED, RELAY_OFF);
  digitalWrite(Red_LED, RELAY_OFF);
  
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
  pinMode(HLT_Heater_Relay, OUTPUT);
  pinMode(BrewPot_Heater_Relay, OUTPUT);
  pinMode(Fermenter_Control_Relay, OUTPUT);
  pinMode(HLT_Level_Switch, INPUT);
  pinMode(BrewPotLevelSwitch, INPUT);
  pinMode(HLTOutputFlowSensor, INPUT);
  pinMode(BrewPotOutputFlowSensor, INPUT);
  pinMode(Pump_Relay, OUTPUT);
  pinMode(Button, INPUT);
  pinMode(Button, INPUT); 
  pinMode(Green_LED, OUTPUT);   
  pinMode(Yellow_LED, OUTPUT);  
  pinMode(Purple_LED, OUTPUT);  
  pinMode(Red_LED, OUTPUT);
  //delay(4000); //Check that all relays are inactive at Reset
  
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
  //unuseddigitalWrite(Relay_11, RELAY_OFF);
  //unuseddigitalWrite(Relay_12, RELAY_OFF);  
  //unuseddigitalWrite(Relay_13, RELAY_OFF);
  //unuseddigitalWrite(Relay_14, RELAY_OFF);
  //unuseddigitalWrite(Relay_15, RELAY_OFF);
  //unuseddigitalWrite(Relay_16, RELAY_OFF);
  digitalWrite(HLT_Heater_Relay, RELAY_OFF);
  digitalWrite(BrewPot_Heater_Relay, RELAY_OFF);
  digitalWrite(Fermenter_Control_Relay, RELAY_OFF);
  digitalWrite(Pump_Relay, RELAY_OFF);

  /**/
  
  Serial.begin(115200);
  Serial.println(F("Hello, Awesome Man!\n")); 

  Serial.print("AutoBrew 10/22/18");
  
  /* Initialise the module */
  Serial.println(F("\nInitializing..."));
  
  Serial.print("Setting up HLT temp sensor\n");

  int temp_sensor_var = 0;
  ds_HLT.reset_search();
  if(!ds_HLT.search(addr)){
    Serial.println("HLT temp sensor not found");
    ds_HLT.reset_search();
    digitalWrite(Purple_LED, RELAY_ON);
    HLT_TS_Status = false;
    temp_sensor_var++;
  }

  Serial.print("HLT Temp Sensor address: ");
  
  for(int i=0; i<8;i++){
    addr_HLT[i]=addr[i];
    Serial.print(addr_HLT[i],HEX);
  }
  Serial.print("\n");

  if(OneWire::crc8(addr, 7)!=addr[7]){
    Serial.print("HLT CRC is not valid!\n");
    digitalWrite(Purple_LED, RELAY_ON);
    HLT_TS_Status = false;
    temp_sensor_var++;
  }

  
  Serial.print("Setting up Mash Tun Temp Sensor\n");

  ds_MT.reset_search();
  if(!ds_MT.search(addr)){
    Serial.println("Mash Tun temp sensor not found");
    digitalWrite(Purple_LED, RELAY_ON);
    MT_TS_Status = false;
    ds_MT.reset_search();
    temp_sensor_var++;
  }

  Serial.print("Mash Tun Temp Sensor address: ");
  for(int i=0; i<8;i++){
    addr_MT[i]=addr[i];
    Serial.print(addr_MT[i],HEX);
  }
  Serial.print("\n");

  if(OneWire::crc8(addr, 7)!=addr[7]){
    Serial.print("Mash Tun CRC is not valid!\n");
    digitalWrite(Purple_LED, RELAY_ON);
    MT_TS_Status = false;
    temp_sensor_var++;
  }

  Serial.print("Setting up Brew Pot Temp Sensor\n");

  ds_BP.reset_search();
  if(!ds_BP.search(addr)){
    Serial.println("Brew pot temp sensor not found");
    digitalWrite(Purple_LED, RELAY_ON);
    BP_TS_Status = false;
    ds_BP.reset_search();
    temp_sensor_var++;
  }

  Serial.print("Brew Pot Temp Sensor address: ");
  for(int i=0; i<8;i++){
    addr_BP[i]=addr[i];
    Serial.print(addr_BP[i],HEX);
  }
  Serial.print("\n");

  if(OneWire::crc8(addr, 7)!=addr[7]){
    Serial.print("Brew Pot CRC is not valid!\n");
    digitalWrite(Purple_LED, RELAY_ON);
    BP_TS_Status = false;
    temp_sensor_var++;
  }

  /*Serial.print("Setting up Fermenter Temp Sensor\n");

  ds_Ferm.reset_search();
  if(!ds_Ferm.search(addr)){
    Serial.println("Fermenter temp sensor not found");
    digitalWrite(Purple_LED, RELAY_ON);
    Ferm_TS_Status = false;
    ds_Ferm.reset_search();
    temp_sensor_var++;
  }

  Serial.print("Fermenter Temp Sensor address: ");
  for(int i=0; i<8;i++){
    addr_Ferm[i]=addr[i];
    Serial.print(addr_Ferm[i],HEX);
  }
  Serial.print("\n");

  if(OneWire::crc8(addr, 7)!=addr[7]){
    Serial.print("Fermenter CRC is not valid!\n");
    digitalWrite(Purple_LED, RELAY_ON);
    Ferm_TS_Status = false;
    temp_sensor_var++;
  }
*/
  //if(temp_sensor_var == 0){digitalWrite(Green_LED, LED_ON);}
  
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
  TIMSK2 |= (1 << OCIE2A);  // enable timer compare interrupt*/
  useInterrupt(true);
  
  interrupts(); 

  digitalWrite(Red_LED, LED_ON);
  

}//--(end setup )---

ISR(TIMER1_COMPA_vect) //Interrupt routine that will happen every second.  We want to increment a value and after every 60seconds we want to increment a minutes value
{
  toggle = !toggle;
  digitalWrite(Red_LED, toggle);
  seconds_var++; //increment out seconds each time interrupt fires
  temp_flow_transmit_timer++;
  if(temp_flow_transmit_timer == 1) {

    //Run this when we want to transmit information to Processing
    Serial.print(temp_count);
    Serial.print(" - c");
    Serial.write(message_3_byte);  //hlt flow rate
    Serial.print("d");
    Serial.write(message_4_byte);   //brewpot flow rate
    Serial.print("e");
    Serial.write(message_5_byte);   //HLT temp sensor
    Serial.print("f");
    Serial.write(message_6_byte);   //Mash Tun Temp Sensor
    Serial.print("g");
    Serial.write(message_7_byte);   //Brewpot Temp Sensor
    //Serial.print("Temp - ");
    //Serial.print(flowrate_HLT);

    /*//Run this when we want detailed info in Arduino Serial Monitor
    Serial.print(temp_count);
    Serial.print(" - HLT Flow Rate - ");
    Serial.print(message_3_byte); 
    Serial.print(" , Brewpot Flow Rate - ");
    Serial.print(message_4_byte);
    Serial.print(" , HLT Temp Sensor - ");
    Serial.print(message_5_byte);
    Serial.print(" , Mash Tun Temp Sensor - ");
    Serial.print(message_6_byte);
    Serial.print(" , Brewpot Temp Sensor - ");
    Serial.print(message_7_byte);
*/

    message_3_byte = message_4_byte = 0;
    Serial.println("");
    temp_flow_transmit_timer = 0;
    //temp_count++;
    //digitalWrite(Relay_1, toggle_1);
    //digitalWrite(Green_LED, toggle_1);
    //toggle_1 = !toggle_1;
  }

  if(HLT_TS_Status) {
    temp_count++;
    ds_HLT.reset();
    ds_HLT.select(addr_HLT);
    ds_HLT.write(0x44,1); //Start conversion, with parasite power on at the end
  }
  if(MT_TS_Status) {
    ds_MT.reset();
    ds_MT.select(addr_MT);
    ds_MT.write(0x44,1); //Start conversion, with parasite power on at the end
  }
  if(BP_TS_Status) {
    ds_BP.reset();
    ds_BP.select(addr_BP);
    ds_BP.write(0x44,1); //Start conversion, with parasite power on at the end 
  }
  if(Ferm_TS_Status) {
    ds_Ferm.reset();
    ds_Ferm.select(addr_Ferm);
    ds_Ferm.write(0x44,1); //Start conversion, with parasite power on at the end 
  }
  /*if(seconds_var == 60) //if we have 60 interrupt, that means we have 60 seconds.  We want to reset the seconds to 0 and increment our minuts variable
  {
    seconds_var = 0;
    minutes_var++;
    Serial.print(minutes_var);
    Serial.print(" minutes!\n");
  }*/

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

  //message_3_byte = message_4_byte = 0;

  
    if (x_HLT == lastflowpinstate_HLT) {
      lastflowratetimer_HLT++;
      //return; //nothing changed!
    }
    else{

      if (x_HLT == HIGH) {
        //low to high transition!
        pulses_HLT++;
      }
      lastflowpinstate_HLT = x_HLT;
      flowrate_HLT = 1000.0;
      flowrate_HLT /= lastflowratetimer_HLT; // in hertz
      message_3_byte = flowrate_HLT;
      lastflowratetimer_HLT = 0;
      flowrate_HLT = 0;
    }
  
  
    if (x_BP == lastflowpinstate_BP) {
      lastflowratetimer_BP++;
      //return; //nothing changed!
    }
    else {

      if (x_BP == HIGH) {
        //low to high transition!
        pulses_BP++;
        
      }
      lastflowpinstate_BP = x_BP;
      flowrate_BP = 1000.0;
      flowrate_BP /= lastflowratetimer_BP; // in hertz
      message_4_byte = flowrate_BP;
      lastflowratetimer_BP = 0;
      flowrate_BP = 0;
    }
  
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(50);
  //Need to  increment currentstate whenever I want to move into another state
    //toggle = !toggle;
  if (Serial.available() > 0) {
    // get incoming byte:
    inByte = Serial.read();
//    Serial.println(inByte);
    if (inByte == 'a') {
      message_1_byte = Serial.read();
//      Serial.print("Message byte 1 received - ");
//      Serial.println(message_1_byte);
      //digitalWrite(Yellow_LED, LED_ON);
      message_1();
    }
    else if (inByte == 'b') {
      message_2_byte = Serial.read();
      //Serial.print("Message byte 2 received - ");
      //Serial.println(message_2_byte);
      message_2();
    
    }
    else {
      message_bad_byte = Serial.read();
      message_bad_byte = 64;
//      Serial.print("Invalid message received - ");
//      Serial.write(message_bad_byte);
    }
//    Serial.println("");
  }

  if(HLT_TS_Status) {read_HLT_TS();}
  delay(1000);
  if(MT_TS_Status) {read_MT_TS();}
  delay(1000);
  if(BP_TS_Status) {read_BP_TS();}
  delay(1000);
  if(Ferm_TS_Status)  {read_Ferm_TS();}
  delay(1000);
  
}

void message_1() {
  for(int x=0;x<8;x++) {
    relay_array[x] = bitRead(message_1_byte, x);
//    Serial.print("Relay ");
//    Serial.print(x);
//    Serial.print(" is ");
//    Serial.println(relay_array[x]);
  }
  digitalWrite(Relay_1, relay_array[0]);
  digitalWrite(Relay_2, relay_array[1]);
  digitalWrite(Relay_3, relay_array[2]);
  digitalWrite(Relay_4, relay_array[3]);
  digitalWrite(Relay_5, relay_array[4]);
  digitalWrite(Relay_6, relay_array[5]);
  digitalWrite(Relay_7, relay_array[6]);
  digitalWrite(Relay_8, relay_array[7]);
  return;
}

void message_2() {
 // for(int x=0;x<8;x++) {
   // relay_array[x] = bitRead(message_2_byte, x);
//    Serial.print("Relay ");
//    Serial.print(x);
//    Serial.print(" is ");
//    Serial.println(relay_array[x]);
  //}
  if(message_2_byte == 1  && message_2_byte == 3 && message_2_byte == ) {digitalWrite(HLT_Heater_Relay, RELAY_ON);}
  else
  if(message_2_byte == 1) {digitalWrite(BrewPot_Heater_Relay, relay_array[1]);
  digitalWrite(Fermenter_Control_Relay, relay_array[2]);
  digitalWrite(Pump_Relay, relay_array[3]);
  
  digitalWrite(HLT_Heater_Relay, relay_array[0]);
  digitalWrite(BrewPot_Heater_Relay, relay_array[1]);
  digitalWrite(Fermenter_Control_Relay, relay_array[2]);
  digitalWrite(Pump_Relay, relay_array[3]);
  return;
}

void show_response() {

   
  
  Serial.println(F("-------------------------------------"));
  /*while (client.connected() || client.available()) {
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
  }*/
}

void read_HLT_TS() {
  int present = ds_HLT.reset();
  ds_HLT.select(addr_HLT);    
  ds_HLT.write(0xBE);         // Read Scratchpad
  
  for ( int i = 0; i < 9; i++) {           // we need 9 bytes
    data_HLT[i] = ds_HLT.read();
  }

  int HighByte, LowByte, TReading;

  LowByte = data_HLT[0];
  HighByte = data_HLT[1];
  TReading = (HighByte<<8) + LowByte;
  HLTTempSensor = (((6*TReading) + TReading/4)/100)*(9/4) + 32;
  message_5_byte = HLTTempSensor;
}

void read_MT_TS(){
  
  int HighByte, LowByte, TReading;
  
  //Start to grab the Mash Tun Temperature
  int present = ds_MT.reset();
  ds_MT.select(addr_MT);    
  ds_MT.write(0xBE);         // Read Scratchpad

  for ( int i = 0; i < 9; i++) {           // we need 9 bytes
    data_MT[i] = ds_MT.read();
  }

  //Serial.print("\nScratchpad Read\n");

  LowByte = data_MT[0];
  HighByte = data_MT[1];
  TReading = (HighByte<<8) + LowByte;
  MashTunTempSensor = (((6*TReading) + TReading/4)/100)*(9/4) + 32;
  //Serial.print("Mash Temperature is: ");
  //Serial.print(MashTunTempSensor);
  //Serial.print(" Degrees Fahrenheit!\n");
  message_6_byte = MashTunTempSensor;
}

void read_BP_TS() {
  int HighByte, LowByte, TReading;
  
  //Start to grab the Mash Tun Temperature
  int present = ds_BP.reset();
  ds_BP.select(addr_BP);    
  ds_BP.write(0xBE);         // Read Scratchpad

  for ( int i = 0; i < 9; i++) {           // we need 9 bytes
    data_BP[i] = ds_BP.read();
  }

  LowByte = data_BP[0];
  HighByte = data_BP[1];
  TReading = (HighByte<<8) + LowByte;
  BrewPotTempSensor = (((6*TReading) + TReading/4)/100)*(9/4) + 32;
  //Serial.print("Brew Pot Temperature is: ");
  //Serial.print(BrewPotTempSensor);
  //Serial.print(" Degrees Fahrenheit!\n");
  message_7_byte = BrewPotTempSensor;
}

void read_Ferm_TS() {
  int HighByte, LowByte, TReading, drink_var;
  
  //Start to grab the Mash Tun Temperature
  int present = ds_Ferm.reset();
  ds_Ferm.select(addr_Ferm);    
  ds_Ferm.write(0xBE);         // Read Scratchpad

  for ( int i = 0; i < 9; i++) {           // we need 9 bytes
    data_Ferm[i] = ds_Ferm.read();
  }

  LowByte = data_Ferm[0];
  HighByte = data_Ferm[1];
  TReading = (HighByte<<8) + LowByte;
  FermenterTempSensor = (((6*TReading) + TReading/4)/100)*(9/4) + 32;
  //Serial.print("\nFermenter Temperature is: ");
  //Serial.print(FermenterTempSensor);
  //Serial.print(" Degrees Fahrenheit!\n");
  message_8_byte = FermenterTempSensor;
}

