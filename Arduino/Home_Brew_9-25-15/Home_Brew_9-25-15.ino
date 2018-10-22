#include <FiniteStateMachine.h>

/*This program will function to automate the home brew process
 * It will function as a state machine, with each state have a corresponding
 * functions that will operate in that state.  There are certain interrupt/flags that will
 * move the system to the next state (temperature, flow control value, level switch, etc).
 */

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
#define Relay_12  12 
#define Relay_13  13
#define Relay_14  14
#define Relay_15  15
#define Relay_16  16

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
int waittime; // Delay between changes
int currentstate;  //will be used in switch/case statement
const byte NUMBER_OF_STATES = 4; //how many states are we cycling through?


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
  delay(4000); //Check that all relays are inactive at Reset

}//--(end setup )---

void loop() {
  // put your main code here, to run repeatedly:

  //Need to  increment currentstate whenever I want to move into another state

  switch(currentstate)
  {
    case Chill_Out_const: stateMachine.transitionTo(Chill_Out);
    case Fill_HLT_const: stateMachine.transitionTo(Fill_HLT);
    case Heat_HLT_const: stateMachine.transitionTO(Heat_HLT);
    State Transfer_To_MT = State(TransferToMT);
    State Maintain_MT = State(MaintainMT);
    State Fill_BrewPot = State(FillBrewPot);
    State Boil_Wort = State(BoilWort);
    State Cool_Wort = State(CoolWort);
    State Wort_To_Fermenter = State(WortToFermenter);
    State Fermentation
      
    
  }

}

void ChillOut()  //This function is my idle function where nothing is done
{
  
}

void FillHLT()  //This function will Fill the HLT with water.  Denoted by Red in schematic
{
  
}

void HeatHLT()  //This function will Heat the HLT to a temperature x degrees above what I want to mash at
{
  
}

void TransferToMT()  //This function will transfter the hot water from the hlt to the mash tun for mashing
{
  
}

void MaintainMT()  //This function will maintain the temp in the mash tun by sending it out through coils in hlt
{
  
}

void FillBrewPot()  //This function will fill the brew pot with the sweet wort from the mash tun
{
  
}

void BoilWort()  //This function will start to boil the water in the brew pot
{
  
}

void CoolWort()  //This function will start to cool the wort by pushing cool water through the wort chiller and pumping wort out and into the brew pot
{
  
}

void WortToFermenter()  //This function will transfer wort from the brew pot to the fermenter
{
  
}

void Fermenting()  //This function will maintain the temperature of the fermenting refridgerator
{
  
}

