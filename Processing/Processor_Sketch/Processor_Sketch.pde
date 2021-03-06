import controlP5.*; //import ControlP5 library //<>// //<>// //<>// //<>// //<>// //<>// //<>// //<>//
import processing.serial.*;

Serial port;

ControlP5 cp5; //create ControlP5 object
PFont font, font_title, font_buttons, font_0, font_labels;

int button_y_size = 50;    //y size of the button
int button_x_size = 120;   //x size of the button
int temp_sensor_4 = 54;
String flow_meter_1 = "55";//, flow_meter_2, temp_sensor_1, temp_sensor_2, temp_sensor_3;
String flow_meter_2 = "55";
String temp_sensor_1 = "55";
String temp_sensor_2 = "55";
String temp_sensor_3= "55";
String level_switch_1 = "OFF";
String level_switch_2 = "OFF";
boolean BV1, BV2, BV3, BV4, BV5, BV6, BV7, BV8, BV9, BV10, PR1, PR2, PR3, PR4, ALL_PR_OFF, BV_ALL_OFF;
boolean ball_valve_1_status;
boolean ball_valve_2_status;
boolean ball_valve_3_status;
boolean ball_valve_4_status;
boolean ball_valve_5_status;
boolean ball_valve_6_status;
boolean ball_valve_7_status;
boolean ball_valve_8_status;
boolean ball_valve_9_status;
boolean level_switch_1_status;
boolean level_switch_2_status; 
boolean HLT_Temp_Up;
boolean HLT_Temp_Down;
boolean BP_Temp_Up;
boolean BP_Temp_Down;
int byte_1, byte_2, byte_3, byte_4, old_byte_1, old_byte_2, old_byte_3, old_byte_4, inByte_3, inByte_4, inByte_5, inByte_6, id_byte, inByte_7, inByte_8;
int HLT_Temp_Setpoint = 130;
int BP_Temp_Setpoint = 210;

void setup(){ //same as arduino program

  size(800, 900);    //window size, (width, height)
  smooth();
  printArray(Serial.list());   //prints all available serial ports
  
  port = new Serial(this, "COM14", 115200);  //i have connected arduino to com3, it would be different in linux and mac os
  
  //lets add buton to empty window
  
  cp5 = new ControlP5(this);
  font = createFont("calibri light bold", 20);    // custom fonts for buttons and title
  font_title = createFont("calibri light bold", 40);  //custom font for title
  font_buttons = createFont("calibri light bold", 12);  //custom font for title
  font_0 = createFont("calibri light bold", 1);  //custom font for title
  font_labels = createFont("calibri light bold", 18);  //custom font for title
 
////////////////////////////////
//Set up Flow Meter Text Boxes//
////////////////////////////////
      
  //cp5.addNumberbox("Flow_Meter_1")
  //   .setPosition(200,70)
  //   .setSize(120,30)
  //   .setScrollSensitivity(1.1)
  //   //.setValue(flow_meter_1)
  //   //.setFont(font_buttons)
  // ;
   
   //cp5.addToggle("Flow Meter 1")
   // .setPosition(200, 70)
   // .setSize(120,30)
   // .setFont(font_buttons)
   // ;
   
   //cp5.addNumberbox("Flow_Meter_2")
   //  .setPosition(200,130)
   //  .setSize(120,30)
   //  .setScrollSensitivity(1.1)
   //  .setValue(flow_meter_2)
   //  .setFont(font_buttons)
   //;
     
///////////////////////////////////
//Set Temperature Sensor Boxes Up//
///////////////////////////////////

   //cp5.addNumberbox("Temp_Sensor_1")
   //  .setPosition(375,70)
   //  .setSize(120,30)
   //  .setScrollSensitivity(1.1)
   //  .setValue(temp_sensor_1)
   //  .setFont(font_buttons)
   //;
   
   //cp5.addNumberbox("Temp_Sensor_2")
   //  .setPosition(375,130)
   //  .setSize(120,30)
   //  .setScrollSensitivity(1.1)
   //  .setValue(temp_sensor_2)
   //  .setFont(font_buttons)
   //;
   
   //cp5.addNumberbox("Temp_Sensor_3")
   //  .setPosition(375,190)
   //  .setSize(120,30)
   //  .setScrollSensitivity(1.1)
   //  .setValue(temp_sensor_3)
   //  .setFont(font_buttons)
   //;
  
///////////////////////////
//Set Ball Valve Boxes Up//
///////////////////////////

  cp5.addToggle("BV1")
    .setPosition(60, 70)
    .setSize(75,button_y_size)
    .setFont(font_0)
    ;
   cp5.addToggle("BV2")
    .setPosition(60, 130)
    .setSize(75,button_y_size)
    .setFont(font_0)
    ;
   cp5.addToggle("BV3")
    .setPosition(60, 190)
    .setSize(75,button_y_size)
    .setFont(font_0)
    ;
   cp5.addToggle("BV4")
    .setPosition(60, 250)
    .setSize(75,button_y_size)
    .setFont(font_0)
    ;
   cp5.addToggle("BV5")
    .setPosition(60, 310)
    .setSize(75,button_y_size)
    .setFont(font_0)
    ;
   cp5.addToggle("BV6")
    .setPosition(60, 370)
    .setSize(75,button_y_size)
    .setFont(font_0)
    ;
   cp5.addToggle("BV7")
    .setPosition(60, 430)
    .setSize(75,button_y_size)
    .setFont(font_0)
    ;
   cp5.addToggle("BV8")
    .setPosition(60, 490)
    .setSize(75,button_y_size)
    .setFont(font_0)
    ;
   cp5.addToggle("BV9")
    .setPosition(60, 550)
    .setSize(75,button_y_size)
    .setFont(font_0)
    ;
   cp5.addToggle("BV_ALL_OFF")
    .setPosition(60, 610)
    .setSize(75,button_y_size)
    .setFont(font_0)
    ;
   
/////////////////////////////////////////
//Set up the power relay toggle buttons//
////////////////////////////////////////

   cp5.addToggle("PR1")
    .setPosition(595, 70)
    .setSize(75,button_y_size)
    .setFont(font_0)
    ;
   cp5.addToggle("PR2")
    .setPosition(595,130)
    .setSize(75,button_y_size)
    .setFont(font_0)
    ;
   cp5.addToggle("PR3")
    .setPosition(595, 190)
    .setSize(75,button_y_size)
    .setFont(font_0)
    ;
   cp5.addToggle("PR4")
    .setPosition(595, 250)
    .setSize(75,button_y_size)
    .setFont(font_0)
    ;
   cp5.addToggle("ALL_PR_OFF")
    .setPosition(595, 310)
    .setSize(75,button_y_size)
    .setFont(font_0)
    ;
    //Add button to increase or decrease temp setpoint for HLT up
    cp5.addToggle("HLT_Temp_Up")
    .setPosition(240, 320)
    .setSize(25,25)
    .setFont(font_0)
    ;
    //Add button to increase or decrease temp setpoint for HLT down
    cp5.addToggle("HLT_Temp_Down")
    .setPosition(270, 320)
    .setSize(25,25)
    .setFont(font_0)
    ;
    //Add button to increase or decrease temp setpoint for BP up
    cp5.addToggle("BP_Temp_Up")
    .setPosition(240, 380)
    .setSize(25,25)
    .setFont(font_0)
    ;
    //Add button to increase or decrease temp setpoint for BP down
    cp5.addToggle("BP_Temp_Down")
    .setPosition(270, 380)
    .setSize(25,25)
    .setFont(font_0)
    ;
    
    

   
}

void draw(){  //same as loop in arduino

  background(0, 0 , 150); // background color of window (r, g, b) or (0 to 255)
  
  //lets give title to our window
  fill(255, 0, 0);               //text color (r, g, b)
  textFont(font_title);
  text("AUTOBREW", 250, 40);  // ("text", x coordinate, y coordinat)
  textFont(font);
  text("Ball Valves", 25, 60); //title for the ball valve column
  text("Flow Meter", 200, 60); //title for the ball valve column
  text("Temp Sensors", 375, 60); //title for the ball valve column
  text("Power Relays", 550, 60); //title for the ball valve column
  textFont(font_labels);
  text("BV1",15,105);
  text("BV2",15,165);
  text("BV3",15,225);
  text("BV4",15,285);
  text("BV5",15,345);
  text("BV6",15,405);
  text("BV7",15,465);
  text("BV8",15,525);
  text("BV9",15,585);
  text("BV",15,635);
  text("OFF",15,650);
  text("PR1",550,105);
  text("PR2",550,165);
  text("PR3",550,225);
  text("PR4",550,285);
  text("PR",550,335);
  text("OFF",550,350);
  textFont(font_labels);
  text("Flow Meter 1", 200, 100);
  text(flow_meter_1, 200, 120);//, 540, 300);
  text("Flow Meter 2", 200, 160);
  text(flow_meter_2, 200, 180);
  text("Temp Sensor 1", 375, 100);
  text(temp_sensor_1, 375, 120);
  text("Temp Sensor 2", 375, 160);
  text(temp_sensor_2, 375, 180);
  text("Temp Sensor 3", 375, 220);
  text(temp_sensor_3, 375, 240);
  text("Level Switch 1", 375, 320);
  text(level_switch_1, 375, 340);
  text("Level Switch 2", 375, 380);
  text(level_switch_2, 375, 400);
  text("HLT Setpoint", 200, 320);
  text("BP Setpoint", 200, 380);
  text(Integer.toString(HLT_Temp_Setpoint), 200, 340);
  text(Integer.toString(BP_Temp_Setpoint), 200, 400);
 
 //Need to parse the toggle values to set up the message that we want to send to the Serial port
 //Need to have a lot of if statements looking at value of toggle value, and then have that set or reset a value
 //that will then be combined into the overall message.
 if(BV1==true) {byte_1 |= 1 << 0;}  //this will set the bit in the nth position
 else {byte_1 &= ~(1 << 0); }        //this will clear the bit in the nth position
 if(BV2==true) {byte_1 |= 1 << 1;}
 else {byte_1 &= ~(1 << 1); }
 if(BV3==true) {byte_1 |= 1 << 2;}
 else {byte_1 &= ~(1 << 2); }
 if(BV4==true) {byte_1 |= 1 << 3;}
 else {byte_1 &= ~(1 << 3); }
 if(BV5==true) {byte_1 |= 1 << 4;}
 else {byte_1 &= ~(1 << 4); }
 if(BV6==true) {byte_1 |= 1 << 5;}
 else {byte_1 &= ~(1 << 5); }
 if(BV7==true) {byte_1 |= 1 << 6;}
 else {byte_1 &= ~(1 << 6); }
 if(BV8==true) {byte_1 |= 1 << 7;}
 else {byte_1 &= ~(1 << 7); }
 //if(BV9==true) {byte_2 |= 1 << 0;}
 //else {byte_2 &= ~(1 << 0); }
 if(BV_ALL_OFF==true) {byte_1 &= ~(255);
   //byte_2 &= ~(1 << 0);
 }  //this will clear all the bits in position 0-8.  Equates to 511 decimal
 else {}//transmit_message_1 |= 0 << 7; }
 
 
 if(HLT_Temp_Setpoint > int(temp_sensor_1)) {
   if(PR1==true) {byte_2 |= 1 << 0;}
   else {byte_2 &= ~(1 << 0); }
 }
 else {byte_2 &= ~(1 << 0);} 
 if(BP_Temp_Setpoint > int(temp_sensor_3)) {
   if(PR2==true) {byte_2 |= 1 << 1;}
   else {byte_2 &= ~(1 << 1); } 
 }
 else {byte_2 &= ~(1 << 1); } 
   if(PR3==true) {byte_2 |= 1 << 2;}
   else {byte_2 &= ~(1 << 2); }
   if(PR4==true) {byte_2 |= 1 << 3;}
   else {byte_2 &= ~(1 << 3); }
   if(ALL_PR_OFF==true) {byte_2 &= ~(255);} //this will clear all the bits in position 9-11.
   else {}//transmit_message_1 |= 0 << 7; }
 
//Temperature Setpoints Adjust 
 if(HLT_Temp_Up==true) {HLT_Temp_Setpoint += 1;
   HLT_Temp_Up = false;
   byte_3 = HLT_Temp_Setpoint;
   cp5.addToggle("HLT_Temp_Up")
    .setPosition(240, 320)
    .setSize(25,25)
    .setFont(font_0);
 }
 else if (HLT_Temp_Down==true) {HLT_Temp_Setpoint -= 1;
   HLT_Temp_Down = false;
   byte_3 = HLT_Temp_Setpoint;
   cp5.addToggle("HLT_Temp_Down")
    .setPosition(270, 320)
    .setSize(25,25)
    .setFont(font_0);
 }
 if(BP_Temp_Up==true) {BP_Temp_Setpoint += 1;
   BP_Temp_Up = false;
   byte_4 = BP_Temp_Setpoint;
   cp5.addToggle("BP_Temp_Up")
    .setPosition(240, 380)
    .setSize(25,25)
    .setFont(font_0);
 }
 else if (BP_Temp_Down==true) {BP_Temp_Setpoint -= 1;
   BP_Temp_Down = false;
   byte_4 = BP_Temp_Setpoint;
   cp5.addToggle("BP_Temp_Down")
    .setPosition(270, 380)
    .setSize(25,25)
    .setFont(font_0);
 }
 
 print("message 3 is ");
      println(byte_3);
       print("message 4 is ");
      println(byte_4);
  
  if ((byte_1 != old_byte_1)) { //| (byte_2 != old_byte_2) ){
    //port.write('x');
    //println("Now Sending Byte 1 - ", byte_1);
    port.write('a');
    port.write(Integer.toString(byte_1));
    //port.write("_");
    //println("Now Sending Byte 2 - ", byte_2);
    //port.write(Integer.toString(byte_2));
    //port.write('x');
  }
  if (byte_2 != old_byte_2) {
   port.write('b');
   port.write(Integer.toString(byte_2));
  }
  if (byte_3 != old_byte_3) {
   port.write('l');
   port.write(byte_3);
  }
  if (byte_4 != old_byte_4) {
   port.write('m');
   port.write(Integer.toString(byte_4));
  }
    
  old_byte_1 = byte_1;
  old_byte_2 = byte_2;
  old_byte_3 = byte_3;
  old_byte_4 = byte_4;
  
  while (port.available() > 0) {
    id_byte = port.read();
    char byte_value =char(id_byte);
    if (id_byte == 99) {
      inByte_3 = port.read();
      print("Got a c");
      print(" - ");
      println(inByte_3);
      flow_meter_1 = str(inByte_3);
    }
    if (id_byte == 'd') {
      inByte_4 = port.read();
      //print("Got a d");
      //print(" - ");
      //println(inByte_4);
      flow_meter_2 = str(inByte_4);
    }
    if (id_byte == 'e') {
      inByte_5 = port.read();
      print("Got a e");
      print(" - ");
      println(inByte_5);
      temp_sensor_1 = str(inByte_5);
    }
    if (id_byte == 'f') {
      inByte_6 = port.read();
      //print("Got a f");
      //print(" - ");
      //println(inByte_6);
      temp_sensor_2 = str(inByte_6);
    }
    if (id_byte == 'g') {
      inByte_7 = port.read();
      print("Got a g");
      print(" - ");
      println(inByte_7);
      temp_sensor_3 = str(inByte_7);
    }
    if (id_byte == 'h') {
      inByte_8 = port.read();
      print("Got a h");
      print(" - ");
      println(inByte_8);
      switch (inByte_8) {
         case 0: 
           level_switch_1 = "OFF";
           level_switch_2 = "OFF";
           break;
         case 1: 
           level_switch_1 = "ON";
           level_switch_2 = "OFF";
           break;
         case 2: 
           level_switch_1 = "OFF";
           level_switch_2 = "ON";
           break;
         case 3: 
           level_switch_1 = "ON";
           level_switch_2 = "ON";     
           break;  
      }
    }
  }
    
    /*
  while (port.available() > 0) {
    int inByte_3 = port.read();
    println(inByte_3);
  }
  while (port.available() > 0) {
    int inByte_4 = port.read();
    println(inByte_4);
  }
  while (port.available() > 0) {
    int inByte_5 = port.read();
    println(inByte_5);
  }
  while (port.available() > 0) {
    int inByte_6 = port.read();
    println(inByte_6);
  }*/
  
  /*byte[] inBuffer = new byte[6];
  while (port.available() > 0) {
    inBuffer = port.readBytes();
    port.readBytes(inBuffer);
    if (inBuffer != null) {
      String myString = new String(inBuffer);
      println(myString);
    }
  }*/
  
  //flow_meter_1 = 70;
}

//lets add some functions to our buttons
//so whe you press any button, it sends perticular char over serial port
