/*
GENERAL ISSUES/QUESTIONS/CONCERNS

1.  Is there any way to make default statements to occur only the first time coming in to a function (like what is done with all my outputs going between functions
or do I have to manually turn whatever I've turned on before leaving the previous function?
*/

//#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include <OneWire.h>

//#include <FiniteStateMachine.h>

/*This program will function to automate the home brew process
 * It will function as a state machine, with each state have a corresponding
 * functions that will operate in that state.  There are certain interrupt/flags that will
 * move the system to the next state (temperature, flow control value, level switch, etc).
 */

/*//Constants and variables for WIFI connecting and grabbing file data
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

*/
OneWire ds_HLT(24); //Temp sensors are located on pin 24
OneWire ds_MT(25); //Temp sensor for Mash Tun is on pin 25
OneWire ds_BP(26); //Temp sensor for Brew Pot is on pin 26
OneWire ds_Ferm(27); //Temp sensor for Fermenter is on pin 27


/*
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

  */

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
#define Relay_1_Input 11
#define Relay_2_Input 12
#define Relay_3_Input 13
#define Relay_4_Input 14
#define Relay_5_Input 15
#define Relay_6_Input 16
#define Relay_7_Input 41
#define Relay_8_Input 42
#define Relay_9_Input 43
#define Relay_10_Input 44
#define HLT_Heater_Input 45
#define BP_Heater_Input 46
#define Pump_Input 47
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
#define WIFI_Connected_led  29
#define Received_from_Pi_led  30
#define Starting_Brew_Process_led 31
#define Error_led 32
//#define TempSensorLED 41
#define Button  40

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
  digitalWrite(Relay_1_Input, RELAY_OFF);
  digitalWrite(Relay_2_Input, RELAY_OFF);
  digitalWrite(Relay_3_Input, RELAY_OFF);
  digitalWrite(Relay_4_Input, RELAY_OFF);
  digitalWrite(Relay_5_Input, RELAY_OFF);
  digitalWrite(Relay_6_Input, RELAY_OFF);
  digitalWrite(Relay_7_Input, RELAY_OFF);
  digitalWrite(Relay_8_Input, RELAY_OFF);
  digitalWrite(Relay_9_Input, RELAY_OFF);
  digitalWrite(Relay_10_Input, RELAY_OFF);
  digitalWrite(HLT_Heater_Relay, RELAY_OFF);
  digitalWrite(BrewPot_Heater_Relay, RELAY_OFF);
  digitalWrite(Fermenter_Control_Relay, RELAY_OFF);
  digitalWrite(HLT_Level_Switch, RELAY_OFF);
  digitalWrite(BrewPotLevelSwitch, RELAY_OFF);
  digitalWrite(HLTOutputFlowSensor, RELAY_OFF);
  digitalWrite(BrewPotOutputFlowSensor, RELAY_OFF);
  digitalWrite(Pump_Relay, RELAY_OFF);
  digitalWrite(Button, RELAY_OFF);
  digitalWrite(WIFI_Connected_led, RELAY_OFF);
  digitalWrite(Received_from_Pi_led, RELAY_OFF);
  digitalWrite(Starting_Brew_Process_led, RELAY_OFF);
  digitalWrite(Error_led, RELAY_OFF);
  digitalWrite(HLT_Heater_Input, RELAY_OFF);
  digitalWrite(BP_Heater_Input, RELAY_OFF);
  digitalWrite(Pump_Input, RELAY_OFF);
  //digitalWrite(TempSensorLED, LED_OFF);


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
  pinMode(Relay_1_Input, INPUT);
  pinMode(Relay_2_Input, INPUT);
  pinMode(Relay_3_Input, INPUT);
  pinMode(Relay_4_Input, INPUT);
  pinMode(Relay_5_Input, INPUT);
  pinMode(Relay_6_Input, INPUT);
  pinMode(Relay_7_Input, INPUT);
  pinMode(Relay_8_Input, INPUT);
  pinMode(Relay_9_Input, INPUT);
  pinMode(Relay_10_Input, INPUT);
  pinMode(HLT_Heater_Relay, OUTPUT);
  pinMode(BrewPot_Heater_Relay, OUTPUT);
  pinMode(Fermenter_Control_Relay, OUTPUT);
  pinMode(HLT_Level_Switch, INPUT);
  pinMode(BrewPotLevelSwitch, INPUT);
  pinMode(HLTOutputFlowSensor, INPUT);
  pinMode(BrewPotOutputFlowSensor, INPUT);
  pinMode(Pump_Relay, OUTPUT);
  pinMode(Button, INPUT);
  pinMode(WIFI_Connected_led, OUTPUT);
  pinMode(Received_from_Pi_led, OUTPUT);
  pinMode(Starting_Brew_Process_led, OUTPUT);
  pinMode(Error_led, OUTPUT);
  pinMode(HLT_Heater_Input, INPUT);
  pinMode(BP_Heater_Input, INPUT);
  pinMode(Pump_Input, INPUT);

  
  //pinMode(TempSensorLED, OUTPUT);
  delay(4000); //Check that all relays are inactive at Reset

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
  digitalWrite(Pump_Relay, RELAY_OFF);
  //digitalWrite(TempSensorLED, LED_OFF);

  /**/

  Serial.begin(115200);
  Serial.println(F("Hello, Awesome Man!\n"));

  Serial.print("AutoBrew 10/22/15");

  /* Initialise the module */
  Serial.println(F("\nInitializing..."));
  /*
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    digitalWrite(Error_led, RELAY_ON);
    while(1);
  }

   //Connection to RaspberryPi
  if (!cc3000.setStaticIPAddress(ipAddress_0, netMask_0, defaultGateway_0, dns_0)) {
    Serial.println(F("Failed to set static IP!"));
    digitalWrite(Error_led, RELAY_ON);
    while(1);
  }

  Serial.print(F("\nAttempting to connect to ")); Serial.println(WLAN_SSID_0);
  if (!cc3000.connectToAP(WLAN_SSID_0, WLAN_PASS_0, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    digitalWrite(Error_led, RELAY_ON);
    while(1);
  }

  Serial.println(F("Connected!"));
  client.close();
  digitalWrite(WIFI_Connected_led, LED_ON);
  */

  delay(2000);

  //////////////////////////////////////////////////////////////////////////////////////////
  // This section sets up the temperature sensors for the HLT, MT, Brew Pot, and Fermenter//
  //////////////////////////////////////////////////////////////////////////////////////////
 
  Serial.print("Setting up HLT temp sensor\n");

  int temp_sensor_var = 0;
  ds_HLT.reset_search();
  if(!ds_HLT.search(addr)){
    Serial.println("HLT temp sensor not found");
    ds_HLT.reset_search();
    digitalWrite(Error_led, RELAY_ON);
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
    digitalWrite(Error_led, RELAY_ON);
    temp_sensor_var++;
  }

  Serial.print("Setting up Mash Tun Temp Sensor\n");

  ds_MT.reset_search();
  if(!ds_MT.search(addr)){
    Serial.println("Mash Tun temp sensor not found");
    digitalWrite(Error_led, RELAY_ON);
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
    digitalWrite(Error_led, RELAY_ON);
    temp_sensor_var++;
  }

  Serial.print("Setting up Brew Pot Temp Sensor\n");

  ds_BP.reset_search();
  if(!ds_BP.search(addr)){
    Serial.println("Brew pot temp sensor not found");
    digitalWrite(Error_led, RELAY_ON);
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
    digitalWrite(Error_led, RELAY_ON);
    temp_sensor_var++;
  }

  Serial.print("Setting up Fermenter Temp Sensor\n");

  ds_Ferm.reset_search();
  if(!ds_Ferm.search(addr)){
    Serial.println("Fermenter temp sensor not found");
    //digitalWrite(Error_led, RELAY_ON);
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
    //digitalWrite(Error_led, RELAY_ON);
    temp_sensor_var++;
  }

  //if(temp_sensor_var == 0){digitalWrite(TempSensorLED, LED_ON);}
  
  // initialize timer1
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  //interrupt will occur once a second
  OCR1A = 15624;            // compare match register = ((16*10^6) / (1024 * 1)) - 1
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= (1 << CS12) | (1 << CS10);   // 1024 prescaler
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt

  useInterrupt(true);

  interrupts();

}//--(end setup )---

ISR(TIMER1_COMPA_vect) //Interrupt routine that will happen every second.  We want to increment a value and after every 60seconds we want to increment a minutes value
{
  seconds_var++; //increment out seconds each time interrupt fires
  //Serial.print("In 1 Second Interrupt!\n");
  /*if(currentstate == Heat_HLT_const  || currentstate == Chill_Out_const || currentstate == Maintain_MT_const){
    ds_HLT.reset();
    ds_HLT.select(addr_HLT);
    ds_HLT.write(0x44,1); //Start conversion, with parasite power on at the end
    //Serial.print("Message sent to convert HLT temp!\n");
  }
  if (currentstate == Maintain_MT_const){
    ds_MT.reset();
    ds_MT.select(addr_MT);
    ds_MT.write(0x44,1); //Start conversion, with parasite power on at the end
  }
  if (currentstate == Boil_Wort_const){
    ds_BP.reset();
    ds_BP.select(addr_BP);
    ds_BP.write(0x44,1); //Start conversion, with parasite power on at the end
  }
  if (currentstate == Fermentation_const){
    ds_Ferm.reset();
    ds_Ferm.select(addr_Ferm);
    ds_Ferm.write(0x44,1); //Start conversion, with parasite power on at the end
  }*/
  if (seconds_var == 60) //if we have 60 interrupt, that means we have 60 seconds.  We want to reset the seconds to 0 and increment our minuts variable
  {
    seconds_var = 0;
    minutes_var++;
    Serial.print(minutes_var);
    Serial.print(" minutes!\n");
  }

  //Need to toggle an LED to ensure that the system is functioning properly
}



SIGNAL(TIMER0_COMPA_vect) {
  /*uint8_t x_HLT = digitalRead(HLTOutputFlowSensor);
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
    if(lastflowratetimer_HLT == 0){
      flowrate_HLT = 0;
    }
    else {
      flowrate_HLT /= lastflowratetimer_HLT; // in hertz
    }
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
    if(lastflowratetimer_BP == 0){
      flowrate_BP = 0;
    }
    else {
      flowrate_BP /= lastflowratetimer_BP; // in hertz
    }
    lastflowratetimer_BP = 0;
  }*/

}

void loop() {

  delay(3000);
  
  Serial.print("\n");

//  If we press the Relay On button (contact, whatever), then we want to display that the relay is on, but only if the relay is not currently on.  Then we turn it on.  If
// the relay is on and the button is not pressed, then we want to display that the relay is off, and then turn it off.
  if(digitalRead(Relay_1_Input)){
    if(digitalRead(!Relay_1)) {
      Serial.println(F("Relay 1 On"));
    }
    digitalWrite(Relay_1, RELAY_ON);
  }
  else{
    if(digitalRead(Relay_1)) {
      Serial.println(F("Relay 1 Off"));
    }
    digitalWrite(Relay_1, RELAY_OFF); 
  }

  //Relay 2
  if(digitalRead(Relay_2_Input)){
    if(digitalRead(!Relay_2)) {
      Serial.println(F("Relay 2 On"));
    }
    digitalWrite(Relay_2, RELAY_ON);
  }
  else{
  if(digitalRead(Relay_2)) {
      Serial.println(F("Relay 2 Off"));
    }
    digitalWrite(Relay_2, RELAY_OFF); 
  }
    
  //Relay 3
  if(digitalRead(Relay_3_Input)){
    if(digitalRead(!Relay_3)) {
      Serial.println(F("Relay 3 On"));
    }
    digitalWrite(Relay_3, RELAY_ON);
  }
  else{
    if(digitalRead(Relay_3)) {
      Serial.println(F("Relay 3 Off"));
    }
    digitalWrite(Relay_3, RELAY_OFF); 
  }

  //Relay 4
  if(digitalRead(Relay_4_Input)){
    if(digitalRead(!Relay_4)) {
      Serial.println(F("Relay 4 On"));
    }
    digitalWrite(Relay_4, RELAY_ON);
  }
  else{
    if(digitalRead(Relay_4)) {
      Serial.println(F("Relay 4 Off"));
    }
    digitalWrite(Relay_4, RELAY_OFF); 
  }

  //Relay 5
  if(digitalRead(Relay_5_Input)){
    if(digitalRead(!Relay_5)) {
      Serial.println(F("Relay 5 On"));
    }
    digitalWrite(Relay_5, RELAY_ON);
  }
  else{
    if(digitalRead(Relay_5)) {
      Serial.println(F("Relay 5 Off"));
    }
    digitalWrite(Relay_5, RELAY_OFF); 
  }

  //Relay 6
  if(digitalRead(Relay_6_Input)){
    if(digitalRead(!Relay_6)) {
      Serial.println(F("Relay 6 On"));
    }
    digitalWrite(Relay_6, RELAY_ON);
  }
  else{
    if(digitalRead(Relay_6)) {
      Serial.println(F("Relay 6 Off"));
    }
    digitalWrite(Relay_6, RELAY_OFF); 
  }

  //Relay 7
  if(digitalRead(Relay_7_Input)){
    if(digitalRead(!Relay_7)) {
      Serial.println(F("Relay 7 On"));
    }
    digitalWrite(Relay_7, RELAY_ON);
  }
  else{
    if(digitalRead(Relay_7)) {
      Serial.println(F("Relay 7 Off"));
    }
    digitalWrite(Relay_7, RELAY_OFF); 
  }

  //Relay 8
  if(digitalRead(Relay_8_Input)){
    if(digitalRead(!Relay_8)) {
      Serial.println(F("Relay 8 On"));
    }
    digitalWrite(Relay_8, RELAY_ON);
  }
  else{
    if(digitalRead(Relay_8)) {
      Serial.println(F("Relay 8 Off"));
    }
    digitalWrite(Relay_8, RELAY_OFF); 
  }

  //Relay 9
  if(digitalRead(Relay_9_Input)){
    if(digitalRead(!Relay_9)) {
      Serial.println(F("Relay 9 On"));
    }
    digitalWrite(Relay_9, RELAY_ON);
  }
  else{
    if(digitalRead(Relay_9)) {
      Serial.println(F("Relay 9 Off"));
    }
    digitalWrite(Relay_9, RELAY_OFF); 
  }

  //Relay 10
  if(digitalRead(Relay_10_Input)){
    if(digitalRead(!Relay_10)) {
      Serial.println(F("Relay 10 On"));
    }
    digitalWrite(Relay_10, RELAY_ON);
  }
  else{
    if(digitalRead(Relay_10)) {
      Serial.println(F("Relay 10 Off"));
    }
    digitalWrite(Relay_10, RELAY_OFF); 
  }

  //HLT Heater
  if(digitalRead(HLT_Heater_Input)){
    if(digitalRead(!HLT_Heater_Relay)) {
      Serial.println(F("HLT Heater On"));
    }
    digitalWrite(HLT_Heater_Relay, RELAY_ON);
  }
  else{
    if(digitalRead(HLT_Heater_Relay)) {
      Serial.println(F("HLT Heater Off"));
    }
    digitalWrite(HLT_Heater_Relay, RELAY_OFF); 
  }

  //BP Heater
  if(digitalRead(BP_Heater_Input)){
    if(digitalRead(!BrewPot_Heater_Relay)) {
      Serial.println(F("BP Heater On"));
    }
    digitalWrite(BrewPot_Heater_Relay, RELAY_ON);
  }
  else{
    if(digitalRead(BrewPot_Heater_Relay)) {
      Serial.println(F("BP Heater Off"));
    }
    digitalWrite(BrewPot_Heater_Relay, RELAY_OFF); 
  }

  //Pump
  if(digitalRead(Pump_Input)){
    if(digitalRead(!Pump_Relay)) {
      Serial.println(F("Pump On"));
    }
    digitalWrite(Pump_Relay, RELAY_ON);
  }
  else{
    if(digitalRead(Pump_Relay)) {
      Serial.println(F("Pump Off"));
    }
    digitalWrite(Pump_Relay, RELAY_OFF); 
  }

  read_and_show_HLT_temp();
  read_and_show_MT_Temp();
  read_and_show_BP_Temp();
  read_and_show_Ferm_Temp();
 
  return;
}

void read_and_show_MT_Temp() {
  //Start to grab the Mash Tun Temperature
  int present = ds_MT.reset();
  ds_MT.select(addr_MT);
  ds_MT.write(0xBE);         // Read Scratchpad

  for ( int i = 0; i < 9; i++) {           // we need 9 bytes
    data_MT[i] = ds_MT.read();
  }

  int HighByte, LowByte, TReading;

  LowByte = data_MT[0];
  HighByte = data_MT[1];
  TReading = (HighByte << 8) + LowByte;
  int MashTunTempSensor = (((6 * TReading) + TReading / 4) / 100) * (9 / 4) + 32;
  Serial.print("Mash Temperature is: ");
  Serial.print(MashTunTempSensor);
  Serial.print(" Degrees Fahrenheit!\n");
}

void read_and_show_BP_Temp() {
  //Read the BrewPot Temp Sensor for initially going into next while loop
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
  TReading = (HighByte << 8) + LowByte;
  int BrewPotTempSensor = (((6 * TReading) + TReading / 4) / 100) * (9 / 4) + 32;
  Serial.print("\nBrew Pot Temperature is: ");
  Serial.print(BrewPotTempSensor);
  Serial.print(" Degrees Fahrenheit!\n");
}

void read_and_show_Ferm_Temp() {
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
  TReading = (HighByte << 8) + LowByte;
  int FermenterTempSensor = (((6 * TReading) + TReading / 4) / 100) * (9 / 4) + 32;
  Serial.print("\nFermenter Temperature is: ");
  Serial.print(FermenterTempSensor);
  Serial.print(" Degrees Fahrenheit!\n");

}

void read_and_show_HLT_temp() {

  int present = ds_HLT.reset();
  ds_HLT.select(addr_HLT);
  ds_HLT.write(0xBE);         // Read Scratchpad

  for ( int i = 0; i < 9; i++) {           // we need 9 bytes
    data_HLT[i] = ds_HLT.read();
  }

  int HighByte, LowByte, TReading;
  
  LowByte = data_HLT[0];
  HighByte = data_HLT[1];
  TReading = (HighByte << 8) + LowByte;
  int HLTTempSensor = (((6 * TReading) + TReading / 4) / 100) * (9 / 4) + 32;
  Serial.print("HLT Temperature is: ");
  Serial.print(HLTTempSensor);
  Serial.print(" Degrees Fahrenheit!\n");
  
}

void show_response_no_conn() {



  Serial.println(F("-------------------------------------"));


  Serial.println("Data grabbed.");

  MashTemp = 70;
  SpargeTemp = 85;
  BrewTemp = 200;
  FermentTemp = 70;
  MashTime = 30;
  BrewTime = 120;
  FermentTime = 500;
  //ProcessStart = justRates.substring(21,22).toFloat();
  ProcessStart = 1;

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
  digitalWrite(Received_from_Pi_led, LED_ON);
  return;
}

/*void show_response() {



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
        String justRates = currRates.substring(0, currRates.length() - 5);

        Serial.println("Data grabbed.");

        MashTemp = justRates.substring(0, 3).toFloat();
        SpargeTemp = justRates.substring(3, 6).toFloat();
        BrewTemp = justRates.substring(6, 9).toFloat();
        FermentTemp = justRates.substring(9, 12).toFloat();
        MashTime = justRates.substring(12, 15).toFloat();
        BrewTime = justRates.substring(15, 18).toFloat();
        FermentTime = justRates.substring(18, 21).toFloat();
        //ProcessStart = justRates.substring(21,22).toFloat();
        ProcessStart = 1;

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
        digitalWrite(Received_from_Pi_led, LED_ON);
        return;
      }
    }
  }
}*/

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
  //digitalWrite(TempSensorLED, LED_OFF);

  //  if(digitalRead(Button) || ProcessStart){
  //    //Serial.println(F("Still in Chill Out."));
  //    currentstate++;
  //    Serial.print("Moving to next state");
  //    delay(5000);
  //  }

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
  minutes_var = 0;  //Reset the minutes variable at the beginning of the brew cycle so we don't have to worry about any overflow during the brew cycle.

  delay(5000);  //Wait 1 second before looking for the level switch

  Serial.print("Waiting for level switch signal!");

  while (!digitalRead(HLT_Level_Switch)) {
    if (digitalRead(Button)) //If we press the button, we want to move into the next state
    {
      delay(2000);
      if (!digitalRead(Button)) { //We wait until we've released the button before moving into next state
        currentstate++;
        digitalWrite(Relay_1, RELAY_OFF);
        return;
      }
    } //otherwise we want to sit in this while loop until the level switch is activated
  }

  Serial.println("HLT level switch has been activated");
  digitalWrite(Relay_1, RELAY_OFF);
  delay(5000);
  currentstate++;


  return;
}

void HeatHLT()  //This function will Heat the HLT to a temperature x degrees above what I want to mash at
{
  temp_flowrate_hlt_const = 0;

  digitalWrite(HLT_Heater_Relay, RELAY_ON);

  int present = ds_HLT.reset();
  ds_HLT.select(addr_HLT);
  ds_HLT.write(0xBE);         // Read Scratchpad

  for ( int i = 0; i < 9; i++) {           // we need 9 bytes
    data_HLT[i] = ds_HLT.read();
  }

  int HighByte, LowByte, TReading;

  LowByte = data_HLT[0];
  Serial.print(LowByte);
  Serial.print("\n");
  HighByte = data_HLT[1];
  Serial.print(HighByte);
  Serial.print("\n");
  TReading = (HighByte << 8) + LowByte;
  Serial.print("HLT Raw Data Value is: ");
  Serial.print(TReading);
  Serial.print("\n");
  HLTTempSensor = (((6 * TReading) + TReading / 4) / 100) * (9 / 4) + 32;
  Serial.print("HLT Temperature is: ");
  Serial.print(HLTTempSensor);
  Serial.print(" Degrees Fahrenheit!\n");

  if (HLTTempSensor >= (MashTemp + 15)) //We want to heat the HLT to a temperature 15 degrees higher than what we want to mash at.  This is because the temperature will decrease 10-15 degrees when transferring into grains
  {
    currentstate++;
    Serial.println("HLT Temperature is > Mash Temperature + 15");
    digitalWrite(HLT_Heater_Relay, RELAY_OFF);
  }

  if (digitalRead(Button)) //If we press the button, we want to move into the next state
  {
    delay(2000);
    if (!digitalRead(Button)) { //We want to make sure we've released the button before moving into the next state.
      currentstate++;
      Serial.println("Button pressed in HEAT HLT");
      digitalWrite(HLT_Heater_Relay, RELAY_OFF);
      delay(5000);
    }
  }
  return;
}

void TransferToMT()  //This function will transfter the hot water from the hlt to the mash tun for mashing
{
  digitalWrite(Relay_4, RELAY_ON);

  while (flowrate_HLT == 0 && temp_flowrate_hlt_const == 0) {
    Serial.print("No flow yet!\n");
    delay(2000);
  } //when we initially come into this state, we dont' want to do anything if we haven't started to flow yet
  temp_flowrate_hlt_const = 1;

  Serial.print("We have flow! = ");
  Serial.print(flowrate_HLT);
  Serial.print("\n");

  while (flowrate_HLT == 0 && temp_flowrate_hlt_const == 1) //We've already had some flow but now it's stopped, we need to make sure we're completely done and then move to next state
  {
    delay(2000);
    currentstate++;
    digitalWrite(Relay_4, RELAY_OFF);
    time_var = minutes_var;//insert current time variable from interrupt.  This will be used in the next state
    temp_flowrate_hlt_const = 0;
    Serial.println("Flow from HLT to Mash tun has finished");

    return;
  }

  flowrate_HLT = 0;

  if (digitalRead(Button) == 1) //If we press the button, we want to move into the next state
  {
    delay(2000);
    if (!digitalRead(Button)) {
      currentstate++;
      digitalWrite(Relay_4, RELAY_OFF);
      time_var = minutes_var;//insert current time variable from interrupt.  This will be used in the next state
      temp_flowrate_hlt_const = 0;
      Serial.println("Button pressed while transferring from HLT to Mash Tun");
    }
  }
  //delay(5000);
  return;
}

void MaintainMT()  //This function will maintain the temp in the mash tun by sending it out through coils in hlt
{
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  while (!digitalRead(HLT_Level_Switch) && temp_mash_var == 0) //If we haven't filled the HLT, we want to fill it
  {
    Serial.print("\nWaiting for HLT Level Switch");
    digitalWrite(Relay_1, RELAY_ON);  //While the HLT is not full, allow water to flow into HLT
    if (digitalRead(Button)) //If we press the button, we want to move into the next state
    {
      delay(2000);
      if (!digitalRead(Button)) { //We wait until we've released the button before moving into next state
        currentstate++;
        digitalWrite(Relay_1, RELAY_OFF);
        return;
      }
    } //otherwise we want to sit in this while loop until the level switch is activated
  }
  //Serial.print("\nHLT Level Switch Activated");
  digitalWrite(Relay_1, RELAY_OFF);  //Once HLT is full, turn off the relay allowing water into the HLT

  delay(3000);
  Serial.print("\nReading HLT Scratchpad\n");

  //Next we want to start to heat up the HLT Tun.
  int present = ds_HLT.reset();
  ds_HLT.select(addr_HLT);
  ds_HLT.write(0xBE);         // Read Scratchpad

  for ( int i = 0; i < 9; i++) {           // we need 9 bytes
    data_HLT[i] = ds_HLT.read();
  }

  Serial.print("\nScratchpad Read\n");

  int HighByte, LowByte, TReading;

  LowByte = data_HLT[0];
  HighByte = data_HLT[1];
  TReading = (HighByte << 8) + LowByte;
  HLTTempSensor = (((6 * TReading) + TReading / 4) / 100) * (9 / 4) + 32;
  Serial.print("HLT Temperature is: ");
  Serial.print(HLTTempSensor);
  Serial.print(" Degrees Fahrenheit!\n");

  Serial.print("\nHeating up the HLT before moving on to cycling.\n The temp_mash_var = ");
  Serial.print(temp_mash_var);
  Serial.print("\n");

  delay(2000);

  Serial.print("\nReading HLT Scratchpad\n");

  //Next we want to start to heat up the HLT Tun.
  present = ds_HLT.reset();
  ds_HLT.select(addr_HLT);
  ds_HLT.write(0xBE);         // Read Scratchpad

  for ( int i = 0; i < 9; i++) {           // we need 9 bytes
    data_HLT[i] = ds_HLT.read();
  }

  Serial.print("\nScratchpad Read\n");

  LowByte = data_HLT[0];
  HighByte = data_HLT[1];
  TReading = (HighByte << 8) + LowByte;
  HLTTempSensor = (((6 * TReading) + TReading / 4) / 100) * (9 / 4) + 32;
  Serial.print("HLT Temperature is: ");
  Serial.print(HLTTempSensor);
  Serial.print(" Degrees Fahrenheit!\n");

  Serial.print("\nHeating up the HLT before moving on to cycling.\n The temp_mash_var = ");
  Serial.print(temp_mash_var);
  Serial.print("\n");

  //This while loop is for the initial heating of the HLT before we start to cycle liquid through it.
  while (HLTTempSensor < MashTemp && temp_mash_var == 0)
  {
    Serial.print("\nWaiting for HLT Temp to reach Mash Temp before cycling Mash");
    digitalWrite(HLT_Heater_Relay, RELAY_ON);

    present = ds_HLT.reset();
    ds_HLT.select(addr_HLT);
    ds_HLT.write(0xBE);         // Read Scratchpad

    for ( int i = 0; i < 9; i++) {           // we need 9 bytes
      data_HLT[i] = ds_HLT.read();
    }

    Serial.print("\nScratchpad Read\n");

    LowByte = data_HLT[0];
    HighByte = data_HLT[1];
    TReading = (HighByte << 8) + LowByte;
    Serial.print("HLT Raw Data Value is: ");
    Serial.print(TReading);
    Serial.print("\n");
    HLTTempSensor = (((6 * TReading) + TReading / 4) / 100) * (9 / 4) + 32;
    Serial.print("HLT Temperature is: ");
    Serial.print(HLTTempSensor);
    Serial.print(" Degrees Fahrenheit!\n");
    if (digitalRead(Button)) //If we press the button, we want to move into the next state
    {
      delay(2000);
      if (!digitalRead(Button)) { //We wait until we've released the button before moving into next state
        currentstate++;
        digitalWrite(Relay_1, RELAY_OFF);
        return;
      }
    } //otherwise we want to sit in this while loop until the level switch is activated
    delay(2000);
  }
  Serial.print("\nHLT Temp is = Desired Mash Temperature");

  digitalWrite(HLT_Heater_Relay, RELAY_OFF);

  //Next we want to start to cycle liquid from the Mash Tun through the coils in the HLT
  digitalWrite(Relay_5, RELAY_ON);
  digitalWrite(Relay_8, RELAY_ON);
  delay(5000);
  digitalWrite(Pump_Relay, RELAY_ON);
  temp_mash_var = 1;

  Serial.print("\nReading MT Scratchpad\n");

  //Start to grab the Mash Tun Temperature
  present = ds_MT.reset();
  ds_MT.select(addr_MT);
  ds_MT.write(0xBE);         // Read Scratchpad

  for ( int i = 0; i < 9; i++) {           // we need 9 bytes
    data_MT[i] = ds_MT.read();
  }

  Serial.print("\nScratchpad Read\n");

  LowByte = data_MT[0];
  HighByte = data_MT[1];
  TReading = (HighByte << 8) + LowByte;
  MashTunTempSensor = (((6 * TReading) + TReading / 4) / 100) * (9 / 4) + 32;
  Serial.print("Mash Temperature is: ");
  Serial.print(MashTunTempSensor);
  Serial.print(" Degrees Fahrenheit!\n");

  //The if statement covers any case that the temperature might start to decrease.
  if (HLTTempSensor < MashTemp)
  {
    Serial.print("\nMashTun Temp is too low\n");
    digitalWrite(HLT_Heater_Relay, RELAY_ON);
  }
  else
  {
    Serial.print("\nMashTun Temp is ok!\n");
    digitalWrite(HLT_Heater_Relay, RELAY_OFF);
  }

  int time_compare_mash = minutes_var - time_var;

  Serial.print("\nCurrent Time = ");
  Serial.print(time_compare_mash);

  if (time_compare_mash >= MashTime)
  {
    currentstate++;
    digitalWrite(Relay_5, RELAY_OFF);
    digitalWrite(Relay_8, RELAY_OFF);
    digitalWrite(Pump_Relay, RELAY_OFF);
    digitalWrite(HLT_Heater_Relay, RELAY_OFF);
    Serial.println("\nMashing Time has completed");
  }

  if (digitalRead(Button)) //If we press the button, we want to move into the next state
  {
    delay(2000);
    if (!digitalRead(Button)) { //We wait until we've released the button before moving into next state
      currentstate++;
      digitalWrite(Relay_1, RELAY_OFF);
      Serial.print("Button pressed while maintaining the Mash!");
    }
  } //otherwise we want to sit in this while loop until the level switch is activated
  return;
}

void FillBrewPot()  //This function will fill the brew pot with the sweet wort from the mash tun
{
  //Need to open the relay to allow flow from mash tun to brew pot
  digitalWrite(Relay_6, RELAY_ON);
  delay(1000);

  //Need to open the relay to allow the HLT liquid to flow through the pump, back into the coils and into the mash tun
  digitalWrite(Relay_3, RELAY_ON);
  digitalWrite(Relay_8, RELAY_ON);
  digitalWrite(Pump_Relay, RELAY_ON);

  //Need to monitor the level switch

  Serial.print("\nBrewPotLevelSwitch = \n");
  Serial.print(digitalRead(BrewPotLevelSwitch));
  delay(10000);


  while (!digitalRead(BrewPotLevelSwitch))
  {
    Serial.print("\nWaiting for BrewPotLevelSwitch");
    delay(2000);
    while (digitalRead(Button)) //If we press the button, we want to move into the next state
    {
      delay(2000);
      if (!digitalRead(Button)) { //We wait until we've released the button before moving into next state
        currentstate++;
        digitalWrite(Relay_6, RELAY_OFF);
        digitalWrite(Relay_3, RELAY_OFF);
        digitalWrite(Relay_8, RELAY_OFF);
        digitalWrite(Pump_Relay, RELAY_OFF);
        delay(5000);
        time_var = minutes_var;
        Serial.print("Button pressed while filling the brew pot!");
        return;
      }
    }
  }//Turn off all relays
  Serial.print("\nBrewPotLevelSwitch = \n");
  Serial.print(digitalRead(BrewPotLevelSwitch));
  delay(10000);

  digitalWrite(Relay_6, RELAY_OFF);
  digitalWrite(Relay_3, RELAY_OFF);
  digitalWrite(Relay_8, RELAY_OFF);
  digitalWrite(Pump_Relay, RELAY_OFF);
  delay(2000);
  currentstate++;
  Serial.println("Brew pot has been filled");
  delay(5000);
  time_var = minutes_var;
  //time_var = minutes_var;//insert current time variable from interrupt.  This will be used in the next state. //Not used here.  Want to wait until we're boiling before we store value

  while (digitalRead(Button)) //If we press the button, we want to move into the next state
  {
    delay(2000);
    if (!digitalRead(Button)) { //We wait until we've released the button before moving into next state
      currentstate++;
      digitalWrite(Relay_1, RELAY_OFF);
      delay(5000);
      time_var = minutes_var;
      Serial.print("Button pressed while filling the brew pot!");
      return;
    }
  }
  return;
}

void BoilWort()  //This function will start to boil the water in the brew pot
{
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
  TReading = (HighByte << 8) + LowByte;
  BrewPotTempSensor = (((6 * TReading) + TReading / 4) / 100) * (9 / 4) + 32;
  Serial.print("Brew Pot Temperature is: ");
  Serial.print(BrewPotTempSensor);
  Serial.print(" Degrees Fahrenheit!\n");

  if (BrewPotTempSensor <= BrewTemp) {
    digitalWrite(BrewPot_Heater_Relay, RELAY_ON);
  }

  Serial.print("\nWaiting for Brew Pot Temp to reach Brew Temp - 23");
  while (BrewPotTempSensor <= (BrewTemp - 20)) {
    //Start to grab the Mash Tun Temperature
    present = ds_BP.reset();
    ds_BP.select(addr_BP);
    ds_BP.write(0xBE);         // Read Scratchpad

    for ( int i = 0; i < 9; i++) {           // we need 9 bytes
      data_BP[i] = ds_BP.read();
    }

    LowByte = data_BP[0];
    HighByte = data_BP[1];
    TReading = (HighByte << 8) + LowByte;
    MashTunTempSensor = (((6 * TReading) + TReading / 4) / 100) * (9 / 4) + 32;
    Serial.print("Brew Pot Temperature is: ");
    Serial.print(BrewPotTempSensor);
    Serial.print(" Degrees Fahrenheit!\n");

    if (BrewPotTempSensor >= (BrewTemp - 23)) {
      time_var = minutes_var;
    }
  }
  Serial.print("\nBrew Temp is ok!");

  int time_compare_brew = minutes_var - time_var;

  Serial.print("\nCurrent Time : ");
  Serial.print(time_compare_brew);

  while (time_compare_brew < BrewTime) { //While loop to wait for the brew time to finish
    delay(1000);
    if (seconds_var == 30) {
      time_compare_brew = minutes_var - time_var;
      Serial.print("\nCurrent Time : ");
      Serial.print(time_compare_brew);
    }
    while (digitalRead(Button)) //If we press the button, we want to move into the next state
    {
      delay(2000);
      if (!digitalRead(Button)) { //We wait until we've released the button before moving into next state
        currentstate++;
        digitalWrite(Relay_1, RELAY_OFF);
        delay(5000);
        time_var = minutes_var;
        Serial.print("Button pressed while filling the brew pot!");
        return;
      }
    }
  }

  if (time_compare_brew >= BrewTime)
  {
    currentstate++;
    Serial.println("Brew Time has completed");
    //Turn off all relays
    digitalWrite(BrewPot_Heater_Relay, RELAY_ON);
  }

  while (digitalRead(Button)) //If we press the button, we want to move into the next state
  {
    delay(2000);
    if (!digitalRead(Button)) { //We wait until we've released the button before moving into next state
      currentstate++;
      digitalWrite(Relay_1, RELAY_OFF);
      delay(5000);
      time_var = minutes_var;
      Serial.print("Button pressed while filling the brew pot!");
      return;
    }
  }
  return;
}

void CoolWort()  //This function will start to cool the wort by pushing cool water through the wort chiller and pumping wort out and into the brew pot
{
  temp_flowrate_bp_const = 0;

  //Need to activate the relays to allow the water to flow through wort chiller and back up into HLT
  digitalWrite(Relay_2, RELAY_ON);

  //Need to activate the relays to allow the wort to flow through pump and back into brew pot.
  digitalWrite(Relay_7, RELAY_ON);
  digitalWrite(Relay_9, RELAY_ON);

  //Need to activate the pump relay
  digitalWrite(Pump_Relay, RELAY_ON);

  Serial.print("Waiting for the Brew Pot Temperature to be greater than ");
  Serial.print(PitchYeastTemp);

  //Read the BrewPot Temp Sensor for initially going into next while loop
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
  TReading = (HighByte << 8) + LowByte;
  BrewPotTempSensor = (((6 * TReading) + TReading / 4) / 100) * (9 / 4) + 32;
  Serial.print("\nBrew Pot Temperature is: ");
  Serial.print(BrewPotTempSensor);
  Serial.print(" Degrees Fahrenheit!\n");

  //Check to see if the temperature in the brew pot has decreased below setpoint for fermentation
  while (BrewPotTempSensor >= PitchYeastTemp) {
    //Start to grab the Mash Tun Temperature
    present = ds_BP.reset();
    ds_BP.select(addr_BP);
    ds_BP.write(0xBE);         // Read Scratchpad

    for ( int i = 0; i < 9; i++) {           // we need 9 bytes
      data_BP[i] = ds_BP.read();
    }

    LowByte = data_BP[0];
    HighByte = data_BP[1];
    TReading = (HighByte << 8) + LowByte;
    BrewPotTempSensor = (((6 * TReading) + TReading / 4) / 100) * (9 / 4) + 32;
    Serial.print("Brew Pot Temperature is: ");
    Serial.print(BrewPotTempSensor);
    Serial.print(" Degrees Fahrenheit!\n\n");
    while (digitalRead(Button)) { //If we press the button, we want to move into the next state
      delay(2000);
      if (!digitalRead(Button)) {
        currentstate++;
        //Moving to next state, need to turn all relays off
        digitalWrite(Pump_Relay, RELAY_OFF);
        digitalWrite(Relay_7, RELAY_OFF);
        digitalWrite(Relay_9, RELAY_OFF);
        digitalWrite(Relay_2, RELAY_OFF);
        Serial.println("Button pressed while cooling wort");
        return;
      }
    }
    Serial.print("\nBrewPotLevelSwitch = \n");
    Serial.print(digitalRead(BrewPotLevelSwitch));
    delay(1000);
  }

  while (digitalRead(Button)) //If we press the button, we want to move into the next state
  {
    delay(2000);
    if (!digitalRead(Button)) {
      currentstate++;
      //Moving to next state, need to turn all relays off
      digitalWrite(Pump_Relay, RELAY_OFF);
      digitalWrite(Relay_7, RELAY_OFF);
      digitalWrite(Relay_9, RELAY_OFF);
      digitalWrite(Relay_2, RELAY_OFF);
      Serial.println("Button pressed while cooling wort");
      return;
    }
  }

  currentstate++;
  //Moving to next state, need to turn all relays off
  digitalWrite(Pump_Relay, RELAY_OFF);
  digitalWrite(Relay_7, RELAY_OFF);
  digitalWrite(Relay_9, RELAY_OFF);
  digitalWrite(Relay_2, RELAY_OFF);
  Serial.println("Wort is cooled");
  return;

}

void WortToFermenter()  //This function will transfer wort from the brew pot to the fermenter
{
  digitalWrite(Relay_7, RELAY_OFF);
  digitalWrite(Relay_10, RELAY_OFF);

  while (flowrate_BP == 0 && temp_flowrate_bp_const == 0) {
    Serial.print("No flow yet!\n");
    delay(2000);
  } //when we initially come into this state, we dont' want to do anything if we haven't started to flow yet
  temp_flowrate_bp_const = 1;

  Serial.print("We have flow! = ");
  Serial.print(flowrate_BP);
  Serial.print("\n");

  while (flowrate_BP == 0 && temp_flowrate_bp_const == 1) //We've already had some flow but now it's stopped, we need to make sure we're completely done and then move to next state
  {
    delay(2000);
    currentstate++;
    digitalWrite(Relay_7, RELAY_OFF);
    digitalWrite(Relay_10, RELAY_OFF);
    time_var = minutes_var;//insert current time variable from interrupt.  This will be used in the next state
    temp_flowrate_bp_const = 0;
    Serial.println("We have emptied out the Brew pot");

    return;
  }

  flowrate_BP = 0;

  while (digitalRead(Button)) //If we press the button, we want to move into the next state
  {
    delay(2000);
    if (!digitalRead(Button)) { //We wait until we've released the button before moving into next state
      currentstate++;
      digitalWrite(Relay_7, RELAY_OFF);
      digitalWrite(Relay_10, RELAY_OFF);
      time_var = minutes_var;//insert current time variable from interrupt.  This will be used in the next state
      temp_flowrate_bp_const = 0;
      Serial.println("Button pressed while transferring to fermeter");
    }
  }
  return;


}

void Fermenting()  //This function will maintain the temperature of the fermenting refridgerator
{
  //digitalWrite(Fermenter_Control_Relay, RELAY_ON);

  Serial.print("Waiting for Fermenter time to finish.  While waiting, if the temp drops, the fridge will turn on.\n");

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
  TReading = (HighByte << 8) + LowByte;
  FermenterTempSensor = (((6 * TReading) + TReading / 4) / 100) * (9 / 4) + 32;
  Serial.print("\nFermenter Temperature is: ");
  Serial.print(FermenterTempSensor);
  Serial.print(" Degrees Fahrenheit!\n");

  if (FermenterTempSensor >= FermentTemp) { //If the fermenter temperature is too high, we want to turn on the mini fridge
    digitalWrite(Fermenter_Control_Relay, RELAY_ON);
    drink_var = 0;
  }
  else {
    digitalWrite(Fermenter_Control_Relay, RELAY_OFF);
  }

  int time_compare_ferment = minutes_var - time_var;
  Serial.print("\nCurrent Time : ");
  Serial.print(time_compare_ferment);

  if (time_compare_ferment >= FermentTime)
  {
    //currentstate++;
    //Turn off all relays
    digitalWrite(Fermenter_Control_Relay, RELAY_OFF);
    Serial.println("Done fermenting\n");
    drink_var = 1;
  }

  while (digitalRead(Button)) //If we press the button, we want to move into the next state
  {
    delay(2000);
    if (!digitalRead(Button)) { //We wait until we've released the button before moving into next state
      //currentstate++;
      Serial.println("Button pressed while fermenting\n");
      digitalWrite(Fermenter_Control_Relay, RELAY_OFF);
      drink_var = 1;
    }
  }

  if (drink_var == 1) { //We've finished fermenting, now we want to keep the temp appropriate for drinking ~45°C
    if (FermenterTempSensor >= 40) { //If the fermenter temperature is too high, we want to turn on the mini fridge
      digitalWrite(Fermenter_Control_Relay, RELAY_ON);
    }
    else {
      digitalWrite(Fermenter_Control_Relay, RELAY_OFF);
    }
  }
  return;
}

