// libraries
#include <DFRobot_RGBMatrix.h>    // graphics library for RGB LED matrix board
#include <Wire.h>                 // library for I2C communication
#include <math.h>                 // library for calculations
#include <DFRobotDFPlayerMini.h>  // library for MP3 player
#include <SoftwareSerial.h>       // library for serial communication

// Defining MATRIX BOARD pins and variables connected to elegoo mega 2560 
#define OE    9                 // Output enable
#define LAT   10                // Latch
#define CLK   11                // Clock
#define A     A0                // A line selection
#define B     A1                // B line selection
#define C     A2                // C line selection
#define D     A3                // D line selection
#define E     A4                // ground?
#define WIDTH 64                // matrix board 64 px width
#define HIGH  64                // matrix board 64 px height
// set MATRIX BOARD pin settings & size
DFRobot_RGBMatrix matrix(A, B, C, D, E, CLK, LAT, OE, false, WIDTH, HIGH); 

// Define ROTARY ENCODER pins
// Note: Arduino MEGA 2560 interrupt pins: 2, 3, 18,19,20,21 (20,21 not available when I2C is used)
#define ENC_CLK 18            // encoder CLK used for rotation and diretion detection
#define ENC_DT 19             // encoder DT used for rotation and diretion detection
#define ENC_SW 20             // Click Button
#define BTN_Pause 21          // Pause Button
// Statuses for the ROTARY ENCODER
unsigned long _lastIncReadTime = micros(); 
unsigned long _lastDecReadTime = micros(); 
int _pauseLength = 25000;
int _fastIncrement = 5;
volatile int counter = 0;

// Define MP3 player pins and create object for MP3 player
SoftwareSerial mySoftwareSerial(53, 52);  // (TX, RX)
DFRobotDFPlayerMini MP3_Player;           // create MP3_Player object

// MENU variables
// Note: if menu item is selected/deselected colour changes blue/white
int8_t menuCounter = 0;            // counts menu items (0-6) = (RESTm,RESTs,UPm,Ups,SETS,START,VOLUME)
int8_t menu_RESTm_Value = 0;       // rest minutes (0-59)
int8_t menu_RESTs_Value = 0;       // rest seconds (0-59)
int8_t menu_UPm_Value = 0;         // UP minutes (0-59)
int8_t menu_UPs_Value = 0;         // UP seconds (0-59)
int8_t menu_SETS_Value = 1;        // number of sets (1-99)
int8_t menu_start_Value = 0;       // start button
int8_t menu_volume_Value = 20;     // volume (0/10/20/30)
volatile bool flagPause;           // pause button flag

int8_t menu_RESTm_Value_old = menu_RESTm_Value;       // rest minutes (0-59)
int8_t menu_RESTs_Value_old = menu_RESTs_Value;       // rest seconds (0-59)
int8_t menu_UPm_Value_old = menu_UPm_Value;           // UP minutes (0-59)
int8_t menu_UPs_Value_old = menu_UPs_Value;           // UP seconds (0-59)
int8_t menu_SETS_Value_old = menu_SETS_Value;         // number of sets (1-99)
int8_t menu_start_Value_old = menu_start_Value;       // start button
int8_t menu_volume_Value_old = menu_volume_Value;     // volume (0/10/20/30)

// enable/disable selected menu item on start up
bool menu_RESTm_selected = false;   
bool menu_RESTs_selected = false;
bool menu_UPm_selected = false;
bool menu_UPs_selected = false;
bool menu_SETS_selected = false;
bool menu_start_selected = false;
bool menu_volume_selected = false;
bool refreshMATRIX = true;      // refreshes display on startup
bool refreshSelection = false;  // refreshes selection (white to blue) on start up

void setup() 
{
  // Steer input pin to known state using built in pullup 20 k Ohm resistor
  pinMode(ENC_CLK, INPUT_PULLUP);   // RotaryCLK
  pinMode(ENC_DT, INPUT_PULLUP);    // RotaryDT
  pinMode(ENC_SW, INPUT_PULLUP);    // ButtonSW (Button)
  pinMode(BTN_Pause,INPUT_PULLUP);  // Pause Button

  // Attach interrupt pins    
  attachInterrupt(digitalPinToInterrupt(ENC_CLK), read_encoder, CHANGE);       
  attachInterrupt(digitalPinToInterrupt(ENC_DT), read_encoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_SW), buttonPush_encoder, FALLING);  
  attachInterrupt(digitalPinToInterrupt(BTN_Pause), buttonPush_Pause, FALLING);

  // Settings for MP3 player
  mySoftwareSerial.begin(9600);             // Initialize USB serial port for debugging
  Serial.begin(115200);                     // Initialize serial port for DFPlayer Mini
  if (MP3_Player.begin(mySoftwareSerial))   // Start communication with DFPlayer Mini
  { 
    Serial.println("MP3 Player Ready..."); 
    MP3_Player.EQ(5);                       // set equalizer to bass
    MP3_Player.volume(menu_volume_Value);   // set volume 0-30
  }
  else
  {
    Serial.println("Connecting to DFPlayer Mini failed!");
  }
  
  // Initiate LED matrix, display start screen & selection screen
  matrix.begin();                    // initialize matrix display 
  displayStartScreen();              // display start screen
  displaySelectionScreen();          // display selection screen

}
// 1. updates screen & cursor position with:  updateCursorPosition()
// 2. updates selected item with:             updateSelection()
void loop() 
{
  if(refreshMATRIX == true) // If we are allowed to update the MATRIX...
  {
    updateMATRIX(); // ... we update the MATRIX ...

    // ... also, if one of the menus are already selected...
    if(menu_RESTm_selected == true || menu_RESTs_selected == true || menu_UPm_selected == true || menu_UPs_selected == true || menu_SETS_selected == true || menu_start_selected == true)
    {
     // do nothing
    }
    else
    {
      updateCursorPosition(); // update the position
    }
    
    refreshMATRIX = false; // reset the variable - wait for a new trigger
  }

  if(refreshSelection == true) // if the selection is changed
  {
    updateSelection(); // update the selection on the MATRIX
    refreshSelection = false; // reset the variable - wait for a new trigger
  }
  
}
// change menu_item_selected on button press
void buttonPush_encoder()
{
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts is faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200) 
  {
  switch(menuCounter)
  {
     case 0:
     menu_RESTm_selected = !menu_RESTm_selected;  // change the status of the variable to the opposite
     break;

     case 1:
     menu_RESTs_selected = !menu_RESTs_selected;
     break;

     case 2:
     menu_UPm_selected = !menu_UPm_selected;
     break;

     case 3:
     menu_UPs_selected = !menu_UPs_selected;
     break;

     case 4:
     menu_SETS_selected = !menu_SETS_selected;
     break;

     case 5:
     menu_start_selected = !menu_start_selected;
     break;

     case 6:
     menu_volume_selected = !menu_volume_selected;
     break;
  } 
  refreshMATRIX = true; // Refresh MATRIX after changing the value of the menu items
  refreshSelection = true; // refresh the selection
  }
  last_interrupt_time = interrupt_time;
}
// pause during countdown
void buttonPush_Pause()
{
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts is faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200) 
  {
    flagPause = !flagPause;
  }
  last_interrupt_time = interrupt_time;
}
// actions when encoder is rotated
void read_encoder()
{  
  //-----MENU REST minutes DISPLAY--------------------------------------------------------------
  if(menu_RESTm_selected == true)
  {
    // Encoder interrupt routine for both pins. Updates counter
    // if they are valid and have rotated a full indent
    static uint8_t old_AB = 3;  // Lookup table index (3 corresponds to "00000011" so A == (ENC_CLK) && B == (ENC_DT) are high)
    static int8_t encval = 0;   // Encoder value  
    static const int8_t enc_states[]  = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0}; // Lookup table

    old_AB <<=2;  // Remember previous state (shift 2 places to left) 
    // NOTE: to switch direction change ENC_DT with ENC_CLK below so A becomes B & B becomes A
    if (digitalRead(ENC_DT)) old_AB |= 0x02;   // Add current state of pin A 
    if (digitalRead(ENC_CLK)) old_AB |= 0x01;    // Add current state of pin B
    encval += enc_states[( old_AB & 0x0f )];    // convert encval e.g. : "xxxxyyyyy" to "0000yyyy" to use enc_states[] properly

    // Update counter if encoder has rotated a full indent, that is at least 4 steps
    if( encval > 3 )    // Four steps forward----------------------------------
    {        
      int changevalue = 1;
      if((micros() - _lastIncReadTime) < _pauseLength) 
      {
        changevalue = _fastIncrement * changevalue;   // if rotation is faster changevalue is larger
      }
      _lastIncReadTime = micros();           
      if(counter <59) // do not go above 59
      {
        counter = counter + changevalue;  // Update counter
      }
      else
      {
        counter = 0;  
      }  
      menu_RESTm_Value = counter;
      encval = 0;
      refreshMATRIX = true;
    }
    else if( encval < -3 )  // Four steps backward------------------------------
    {        
      int changevalue = -1;
      if((micros() - _lastDecReadTime) < _pauseLength) 
      {
        changevalue = _fastIncrement * changevalue; 
      }
      _lastDecReadTime = micros();
      if(counter < 1) // do not go below 0
      {
        counter = 59; // changes to 0 in next cycle of loop
      }
      else
      {
      // Encoder is rotating B direction so decrease
      counter = counter + changevalue;              // Update counter      
      }  
      menu_RESTm_Value = counter;
      encval = 0;
      refreshMATRIX = true;
    }
  }
  //---MENU REST seconds DISPLAY---------------------------------------------------------------
  else if(menu_RESTs_selected == true)
  {
    static uint8_t old_AB = 3;
    static int8_t encval = 0; 
    static const int8_t enc_states[]  = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
    
    old_AB <<=2;
    if (digitalRead(ENC_DT)) old_AB |= 0x02;   
    if (digitalRead(ENC_CLK)) old_AB |= 0x01;    
    encval += enc_states[( old_AB & 0x0f )];    

    if( encval > 3 )    // Four steps forward----------------------------------
    {        
      int changevalue = 1;
      if((micros() - _lastIncReadTime) < _pauseLength) 
      {
        changevalue = _fastIncrement * changevalue;   
      }
      _lastIncReadTime = micros();           
      if(counter <59) // do not go above 59
      {
        counter = counter + changevalue;  // Update counter
      }
      else
      {
        counter = 0;  
      }  
      menu_RESTs_Value = counter;
      encval = 0;
      refreshMATRIX = true;
    }
    else if( encval < -3 )  // Four steps backward------------------------------
    {        
      int changevalue = -1;
      if((micros() - _lastDecReadTime) < _pauseLength) 
      {
        changevalue = _fastIncrement * changevalue; 
      }
      _lastDecReadTime = micros();
      if(counter < 1) // do not go below 0
      {
        counter = 59; // changes to 0 in next cycle of loop
      }
      else
      {
      // Encoder is rotating B direction so decrease
      counter = counter + changevalue;              // Update counter      
      }  
      menu_RESTs_Value = counter;
      encval = 0;
      refreshMATRIX = true;
    }
  }
  //---MENU UP minutes DISPLAY---------------------------------------------------------------
  else if(menu_UPm_selected == true)
  {
    static uint8_t old_AB = 3;
    static int8_t encval = 0; 
    static const int8_t enc_states[]  = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
    
    old_AB <<=2;
    if (digitalRead(ENC_DT)) old_AB |= 0x02;   
    if (digitalRead(ENC_CLK)) old_AB |= 0x01;    
    encval += enc_states[( old_AB & 0x0f )];    

    if( encval > 3 )    // Four steps forward----------------------------------
    {        
      int changevalue = 1;
      if((micros() - _lastIncReadTime) < _pauseLength) 
      {
        changevalue = _fastIncrement * changevalue;   
      }
      _lastIncReadTime = micros();           
      if(counter <59) // do not go above 59
      {
        counter = counter + changevalue;  // Update counter
      }
      else
      {
        counter = 0;  
      }  
      menu_UPm_Value = counter;
      encval = 0;
      refreshMATRIX = true;
    }
    else if( encval < -3 )  // Four steps backward------------------------------
    {        
      int changevalue = -1;
      if((micros() - _lastDecReadTime) < _pauseLength) 
      {
        changevalue = _fastIncrement * changevalue; 
      }
      _lastDecReadTime = micros();
      if(counter < 1) // do not go below 0
      {
        counter = 59; // changes to 0 in next cycle of loop
      }
      else
      {
      // Encoder is rotating B direction so decrease
      counter = counter + changevalue;              // Update counter      
      }  
      menu_UPm_Value = counter;
      encval = 0;
      refreshMATRIX = true;
    }
  }
  //---MENU UP seconds DISPLAY---------------------------------------------------------------
  else if(menu_UPs_selected == true)
  {
    static uint8_t old_AB = 3;
    static int8_t encval = 0; 
    static const int8_t enc_states[]  = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
    
    old_AB <<=2;
    if (digitalRead(ENC_DT)) old_AB |= 0x02;   
    if (digitalRead(ENC_CLK)) old_AB |= 0x01;    
    encval += enc_states[( old_AB & 0x0f )];    

    if( encval > 3 )    // Four steps forward----------------------------------
    {        
      int changevalue = 1;
      if((micros() - _lastIncReadTime) < _pauseLength) 
      {
        changevalue = _fastIncrement * changevalue;   
      }
      _lastIncReadTime = micros();           
      if(counter <59) // do not go above 59
      {
        counter = counter + changevalue;  // Update counter
      }
      else
      {
        counter = 0;  
      }  
      menu_UPs_Value = counter;
      encval = 0;
      refreshMATRIX = true;
    }
    else if( encval < -3 )  // Four steps backward------------------------------
    {        
      int changevalue = -1;
      if((micros() - _lastDecReadTime) < _pauseLength) 
      {
        changevalue = _fastIncrement * changevalue; 
      }
      _lastDecReadTime = micros();
      if(counter < 1) // do not go below 0
      {
        counter = 59; // changes to 0 in next cycle of loop
      }
      else
      {
      // Encoder is rotating B direction so decrease
      counter = counter + changevalue;              // Update counter      
      }  
      menu_UPs_Value = counter;
      encval = 0;
      refreshMATRIX = true;
    }
  }
  //---MENU SETS DISPLAY---------------------------------------------------------------
  else if(menu_SETS_selected == true)
  {
    static uint8_t old_AB = 3;
    static int8_t encval = 0; 
    static const int8_t enc_states[]  = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
    
    old_AB <<=2;
    if (digitalRead(ENC_DT)) old_AB |= 0x02;   
    if (digitalRead(ENC_CLK)) old_AB |= 0x01;    
    encval += enc_states[( old_AB & 0x0f )];    

    if( encval > 3 )    // Four steps forward----------------------------------
    {        
      int changevalue = 1;
      if((micros() - _lastIncReadTime) < _pauseLength) 
      {
        changevalue = _fastIncrement * changevalue;   
      }
      _lastIncReadTime = micros();           
      if(counter <99) // do not go above 99
      {
        counter = counter + changevalue;  // Update counter
      }
      else
      {
        counter = 1;  
      }  
      menu_SETS_Value = counter;
      encval = 0;
      refreshMATRIX = true;
    }
    else if( encval < -3 )  // Four steps backward------------------------------
    {        
      int changevalue = -1;
      if((micros() - _lastDecReadTime) < _pauseLength) 
      {
        changevalue = _fastIncrement * changevalue; 
      }
      _lastDecReadTime = micros();
      if(counter < 2) // do not go below 1
      {
        counter = 99; // changes to 99 in next cycle of loop
      }
      else
      {
      // Encoder is rotating B direction so decrease
      counter = counter + changevalue;              // Update counter      
      }  
      menu_SETS_Value = counter;
      encval = 0;
      refreshMATRIX = true;
    }
  }
  //---MENU START DISPLAY---------------------------------------------------------------
  else if(menu_start_selected == true)
  {
    // do nothing
  }
  //---MENU Volume DISPLAY---------------------------------------------------------------
 else if(menu_volume_selected == true)
  {
    static uint8_t old_AB = 3;
    static int8_t encval = 0; 
    static const int8_t enc_states[]  = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
    
    old_AB <<=2;
    if (digitalRead(ENC_DT)) old_AB |= 0x02;   
    if (digitalRead(ENC_CLK)) old_AB |= 0x01;    
    encval += enc_states[( old_AB & 0x0f )];    

    if( encval > 3 )    // Four steps forward----------------------------------
    {        
      int changevalue = 5;
      if((micros() - _lastIncReadTime) < _pauseLength) 
      {
        changevalue = _fastIncrement * changevalue;   
      }
      _lastIncReadTime = micros();           
      if(counter <30) // do not go above 30
      {
        counter = counter + changevalue;  // Update counter
      }
      else
      {
        counter = 30;  
      }  
      menu_volume_Value = counter;
      encval = 0;
      refreshMATRIX = true;
    }
    else if( encval < -3 )  // Four steps backward------------------------------
    {        
      int changevalue = -5;
      if((micros() - _lastDecReadTime) < _pauseLength) 
      {
        changevalue = _fastIncrement * changevalue; 
      }
      _lastDecReadTime = micros();
      if(counter < 1) // do not go below 1
      {
        counter = 0; // changes to 30 in next cycle of loop
      }
      else
      {
      // Encoder is rotating B direction so decrease
      counter = counter + changevalue;              // Update counter      
      }  
      menu_volume_Value = counter;
      encval = 0;
      refreshMATRIX = true;
    }
  }
  //---MENU COUNTER---------------------------------------------------------------
  else
  {
    static uint8_t old_AB = 3;
    static int8_t encval = 0; 
    static const int8_t enc_states[]  = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
    
    old_AB <<=2;
    if (digitalRead(ENC_DT)) old_AB |= 0x02;   
    if (digitalRead(ENC_CLK)) old_AB |= 0x01;    
    encval += enc_states[( old_AB & 0x0f )];    

    if( encval > 7 )    // Four steps forward----------------------------------
    {        
      int changevalue = 1;
      if(counter < 6) // do not go above 6
      {
        counter = counter + changevalue;  // Update counter
      }
      else
      {
        counter = 0;  
      }  
      menuCounter = counter;
      encval = 0;
      refreshMATRIX = true;
    }
    else if( encval < -7 )  // Four steps backward------------------------------
    {        
      int changevalue = -1;
      if(counter < 1) // do not go below 0
      {
        counter = 6; // changes to 0 in next cycle of loop
      }
      else
      {
      // Encoder is rotating B direction so decrease
      counter = counter + changevalue;              // Update counter      
      }  
      menuCounter = counter;
      encval = 0;
      refreshMATRIX = true;
    }
  }
  // Refresh MATRIX after changing the counter's value
  //refreshMATRIX = true;
}
// start screen settings
void displayStartScreen()
{ 
  // play intro music: 0001_intro.wav
  MP3_Player.playMp3Folder(1);
  //display timer icon (64x64 px) with costumizeZH-method
  const uint8_t StartupScreen[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x07, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xe0, 0x07, 0xf0, 0x00, 0x00, 
  0x00, 0x00, 0x3f, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x1e, 0x00, 0x00, 
  0x00, 0x01, 0xf0, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x03, 0xc0, 0x00, 
  0x00, 0x07, 0x80, 0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 
  0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 
  0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 
  0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 
  0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 
  0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 
  0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 
  0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 
  0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 
  0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 
  0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 
  0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 
  0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 
  0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 
  0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 
  0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 
  0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x01, 0xe0, 0x00, 
  0x00, 0x03, 0xc0, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x0f, 0x00, 0x00, 
  0x00, 0x00, 0x7c, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0xfc, 0x00, 0x00, 
  0x00, 0x00, 0x0f, 0xf0, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xc0, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };
  matrix.customizeZH(StartupScreen, 64, 0, 0, matrix.Color333(7,7,7));
  //Start up animation
  const char title[] = {"TIMER"};
  for(int i = 0; i<5; i++){   //animation pressing button & writing "T I M E R"
    matrix.fillRect(25, 4, 14, 4, 0);
    matrix.drawLine(30, 7, 33, 7, matrix.Color333(7,7,7));
    matrix.fillRect(26, 3, 12, 4, matrix.Color333(0,7,0));
    matrix.fillRect(25, 4, 14, 2, matrix.Color333(0,7,0));
    delay(300);
    displayText(15+i*7,31,1,777,title[i]);
    matrix.fillRect(25, 3, 14, 4, 0);
    matrix.fillRect(26, 4, 12, 4, matrix.Color333(7,0,0));
    matrix.fillRect(25, 5, 14, 2, matrix.Color333(7,0,0));
    delay(500);
    }
  matrix.drawRect(14, 40, 35, 4, matrix.Color333(7,7,7));
  for(int i = 0; i<33; i++){
    matrix.drawPixel(15+i, 41,matrix.Color333(0,7,0));  
    matrix.drawPixel(15+i, 42,matrix.Color333(0,7,0));  
    delay(200);
  }
  delay(100);                     //wait 
  matrix.fillScreen(0);           //clear the whole MATRIX
}
// selection screen, these are the values which are not changing the operation
void displaySelectionScreen()
{
    // draw rectangles and dots for "00:00"-display
    matrix.drawRect(1, 1, 62, 13, matrix.Color333(2,2,2));  
    matrix.drawRect(1, 15, 62, 48, matrix.Color333(2,2,2)); 
    matrix.drawPixel(47, 20, matrix.Color333(7,7,7));
    matrix.drawPixel(47, 22, matrix.Color333(7,7,7));
    matrix.drawPixel(47, 30, matrix.Color333(7,7,7));
    matrix.drawPixel(47, 32, matrix.Color333(7,7,7));
    // display INTERVAL,REST,UP,SETS and START button with rectangle
    displayText(9,4,1,777,"INTERVAL");    // display INTERVAL on top
    displayText(4,18,1,700,"REST");       // red REST counter
    displayText(4,28,1,70,"UP");         // green UP counter
    displayText(4,38,1,770,"SETS");       // yellow SETS counter
    displayText(11,52,1,777,"START");     // white START button
    matrix.drawRect(9, 50, 33, 11, matrix.Color333(7,7,7)); //rectangle around START
}
// whenever we press or rotate we call updateMatrix()
// when updateMATRIX() == true, erase old counter-display of REST,UP,SET and volume
void updateMATRIX()
{  
  // update counter-display of REST minutes -----------------------------
  if(menu_RESTm_Value != menu_RESTm_Value_old)  // when values of menu items are changed...
  {
    matrix.fillRect(35, 18, 11, 7, 0);    // clear old value
    if(menu_RESTm_Value < 10)             // displayText prints from left to right so if value X < 10 add 0 for: "0X"-layout
    {
      if (menu_RESTm_selected == true)    // if selected display in turquoise color
      {
        displayText(35,18,1,77,0); 
        displayText(41,18,1,77,menu_RESTm_Value);
      }
      else                                // else in white
      {
        displayText(35,18,1,777,0); 
        displayText(41,18,1,777,menu_RESTm_Value);
      }
    } 
    else                                  // otherwise for X > = 10 print value without leading 0
    {
      if (menu_RESTm_selected == true)
      {
      displayText(35,18,1,77,menu_RESTm_Value);
      }
      else
      {
      displayText(35,18,1,777,menu_RESTm_Value);
      }
    }
  }
  else
  {
    if(menu_RESTm_Value < 10)           
    {
      displayText(35,18,1,777,0); 
      displayText(41,18,1,777,menu_RESTm_Value); 
    } 
    else
    {
      displayText(35,18,1,777,menu_RESTm_Value);
    }
  }
  // update counter-display of REST seconds -----------------------------           
  if(menu_RESTs_Value != menu_RESTs_Value_old)
  {
    matrix.fillRect(49, 18, 11, 7, 0);
    if(menu_RESTs_Value < 10)           
    {
      if (menu_RESTs_selected == true)
      {
        displayText(49,18,1,77,0); 
        displayText(55,18,1,77,menu_RESTs_Value);
      }
      else
      {
        displayText(49,18,1,777,0); 
        displayText(55,18,1,777,menu_RESTs_Value);
      }
    } 
    else
    {
      if (menu_RESTs_selected == true)
      {
        displayText(49,18,1,77,menu_RESTs_Value);
      }
      else
      {
        displayText(49,18,1,777,menu_RESTs_Value);
      }
    }
  }
  else
  {
    if(menu_RESTs_Value < 10)
    {
      displayText(49,18,1,777,0); 
      displayText(55,18,1,777,menu_RESTs_Value);
    } 
    else
    {
      displayText(49,18,1,777,menu_RESTs_Value);
    }  
  }
  // update counter-display of UP minutes -------------------------------          
   if(menu_UPm_Value != menu_UPm_Value_old)
  {
    matrix.fillRect(35, 28, 11, 7, 0);
    if(menu_UPm_Value < 10)           
    {
      if (menu_UPm_selected == true)
      {
        displayText(35,28,1,77,0); 
        displayText(41,28,1,77,menu_UPm_Value);
      }
      else
      {
        displayText(35,28,1,777,0); 
        displayText(41,28,1,777,menu_UPm_Value);
      }
    } 
    else
    {
      if (menu_UPm_selected == true)
      {
        displayText(35,28,1,77,menu_UPm_Value);
      }
      else
      {
        displayText(35,28,1,777,menu_UPm_Value);
      }
    }
  }
  else
  {
    if(menu_UPm_Value < 10)         
    {
      displayText(35,28,1,777,0); 
      displayText(41,28,1,777,menu_UPm_Value);
    } 
    else
    {
      displayText(35,28,1,777,menu_UPm_Value);
    }  
  }
  // update counter-display of UP seconds -------------------------------          
  if(menu_UPs_Value != menu_UPs_Value_old)
  {
    matrix.fillRect(49, 28, 11, 7, 0);
    if(menu_UPs_Value < 10)
    {
      if (menu_UPs_selected == true)
      {
        displayText(49,28,1,77,0); 
        displayText(55,28,1,77,menu_UPs_Value);
      }
      else
      {
        displayText(49,28,1,777,0); 
        displayText(55,28,1,777,menu_UPs_Value);
      }
    } 
    else
    {
      if (menu_UPs_selected == true)
      {
        displayText(49,28,1,77,menu_UPs_Value);
      }
      else
      {
        displayText(49,28,1,777,menu_UPs_Value);
      }
    }
  }
  else
  {
    if(menu_UPs_Value < 10)
    {
      displayText(49,28,1,777,0); 
      displayText(55,28,1,777,menu_UPs_Value);
    } 
    else
    {
      displayText(49,28,1,777,menu_UPs_Value);
    }  
  }
  // update counter-display of SETS ---------------------------------          
  if(menu_SETS_Value != menu_SETS_Value_old)
  {
    matrix.fillRect(42, 38, 11, 7, 0);
    if(menu_SETS_Value < 10)
    {
      if (menu_SETS_selected == true)
      {
        displayText(42,38,1,77,0); 
        displayText(48,38,1,77,menu_SETS_Value);
      }
      else
      {
        displayText(42,38,1,777,0); 
        displayText(48,38,1,777,menu_SETS_Value);
      }
    } 
    else
    {
      if (menu_SETS_selected == true)
      {
        displayText(42,38,1,77,menu_SETS_Value);
      }
      else
      {
        displayText(42,38,1,777,menu_SETS_Value);
      }
    }
  }
  else
  {
    if(menu_SETS_Value < 10)
  {
    displayText(42,38,1,777,0); 
    displayText(48,38,1,777,menu_SETS_Value);
  } 
  else
  {
    displayText(42,38,1,777,menu_SETS_Value);
  }  
  }
  // reset START BUTTON colour ---------------------------------
  displayText(11,52,1,777,"START");    
  matrix.drawRect(9, 50, 33, 11,matrix.Color333(7,7,7));
  // update volume setting ---------------------------------
  if (menu_volume_Value != menu_volume_Value_old)
  {
    matrix.fillCircle(54, 54, 6, 0);
    if(menu_volume_selected == true)
    {
      displayVolume(54,54,menu_volume_Value,77);
    }
    else
    {
      displayVolume(54,54,menu_volume_Value,777);
    }    
  }
  else
  {
    displayVolume(54,54,menu_volume_Value,777);
  }

  menu_RESTm_Value_old = menu_RESTm_Value;       // rest minutes (0-59)
  menu_RESTs_Value_old = menu_RESTs_Value;       // rest seconds (0-59)
  menu_UPm_Value_old = menu_UPm_Value;           // UP minutes (0-59)
  menu_UPs_Value_old = menu_UPs_Value;           // UP seconds (0-59)
  menu_SETS_Value_old = menu_SETS_Value;         // number of sets (1-99)
  menu_start_Value_old = menu_start_Value;       // start button
  menu_volume_Value_old = menu_volume_Value;     // volume (0/10/20/30)
}
// clear old value of selected item (i.e. menuCounter) & make new selected value blue
void updateCursorPosition()
{
  switch(menuCounter) //this checks the value of the counter (0-6)
  {
    // update counter-display of REST minutes -----------------------------
    case 0: 
    matrix.fillRect(35,18,11,7,0);   // erase block (x,y,width,lenght)
    if(menu_RESTm_Value < 10)
    {
      displayText(35,18,1,7,0);    // add 0 if value < 10 for "00" display-layout
      displayText(41,18,1,7,menu_RESTm_Value);
    } 
    else
    {
      displayText(35,18,1,7,menu_RESTm_Value);
    }               
      break;
    // update counter-display of REST seconds -----------------------------
    case 1:
    matrix.fillRect(49,18,11,7,0);             
    if(menu_RESTs_Value < 10)
    {
      displayText(49,18,1,7,0); 
      displayText(55,18,1,7,menu_RESTs_Value);
    } 
    else
    {
      displayText(49,18,1,7,menu_RESTs_Value);
    }       
    break;
    // update counter-display of UP minutes -------------------------------        
    case 2:  
    matrix.fillRect(35,28,11,7, 0);
    if(menu_UPm_Value < 10)
    {
      displayText(35,28,1,7,0); 
      displayText(41,28,1,7,menu_UPm_Value);
    } 
    else
    {
      displayText(35,28,1,7,menu_UPm_Value);
    }  
    break;
    // update counter-display of UP seconds ---------------------------          
    case 3:
    matrix.fillRect(49,28,11,7, 0);   
    if(menu_UPs_Value < 10)
    {
      displayText(49,28,1,7,0); 
      displayText(55,28,1,7,menu_UPs_Value);
    } 
    else
    {
      displayText(49,28,1,7,menu_UPs_Value);
    }  
    break;
    // update counter-display of SETS ---------------------------------    
    case 4:
    matrix.fillRect(42,38,11,7, 0);      
    if(menu_SETS_Value < 10)
    {
      displayText(42,38,1,7,0); 
      displayText(48,38,1,7,menu_SETS_Value);
    } 
    else
    {
      displayText(42,38,1,7,menu_SETS_Value);
    }  
    break;
    // reset START BUTTON colour ---------------------------------
    case 5:
    displayText(11,52,1,7,"START");    
    matrix.drawRect(9, 50, 33, 11,matrix.Color333(0,0,7));
    break;
    // update Volume setting --------------------------------------
    case 6:
    matrix.fillCircle(54, 54, 5, 0);
    if(menu_volume_selected == true)
    {
      displayVolume(54,54,menu_volume_Value,77);
    }
    else
    {
      displayVolume(54,54,menu_volume_Value,7);
    }
    break;
  }
}
// make selected menu-item value turquoise & perform action under start menu
void updateSelection()
{
  //-------------------RESTm is SELECTED---------------------------
  if(menu_RESTm_selected)
  {
    if(menu_RESTm_Value < 10)
    {
      displayText(35,18,1,77,0);    // add 0 if value < 10 for "00" display-layout
      displayText(41,18,1,77,menu_RESTm_Value);
    } 
    else
    {
      displayText(35,18,1,77,menu_RESTm_Value);
    }          
  }
  //-------------------RESTs is SELECTED---------------------------
  if(menu_RESTs_selected)
  {
    if(menu_RESTs_Value < 10)
    {
      displayText(49,18,1,77,0);    // add 0 if value < 10 for "00" display-layout
      displayText(55,18,1,77,menu_RESTs_Value);
    } 
    else
    {
      displayText(49,18,1,77,menu_RESTs_Value);
    }          
  }
  //-------------------UPm   is SELECTED---------------------------
  if(menu_UPm_selected)
  {
    if(menu_UPm_Value < 10)
    {
      displayText(35,28,1,77,0);    // add 0 if value < 10 for "00" display-layout
      displayText(41,28,1,77,menu_UPm_Value);
    } 
    else
    {
      displayText(35,28,1,77,menu_UPm_Value);
    }          
  }
  //-------------------UPs   is SELECTED---------------------------
  if(menu_UPs_selected)
  {
    if(menu_UPs_Value < 10)
    {
      displayText(49,28,1,77,0);    // add 0 if value < 10 for "00" display-layout
      displayText(55,28,1,77,menu_UPs_Value);
    } 
    else
    {
      displayText(49,28,1,77,menu_UPs_Value);
    }          
  }
  //-------------------SETS  is SELECTED---------------------------
  if(menu_SETS_selected)
  {
    if(menu_SETS_Value < 10)
    {
      displayText(42,38,1,77,0);    // add 0 if value < 10 for "00" display-layout
      displayText(48,38,1,77,menu_SETS_Value);
    } 
    else
    {
      displayText(48,38,1,77,menu_SETS_Value);
    }          
  }
  //-------------------START BUTTON selected---------------------------
  if(menu_start_selected == true)
  {
    // UP seconds not 0 start interval timer 
    if(menu_UPs_Value != 0 || menu_UPm_Value != 0)
    {
      // Change start button color to green for 1000 ms and clear display
      displayText(11,52,1,70,"START");                                
      matrix.drawRect(9, 50, 33, 11,matrix.Color333(0,7,0)); 
      delay(1000);                                            
      matrix.fillScreen(0);                                   
      // selecting start menu initiates intervalTimerScreen() and clear display  
      intervalTimerScreen(menu_RESTm_Value, menu_RESTs_Value, menu_UPm_Value, menu_UPs_Value, menu_SETS_Value);
      matrix.fillScreen(0);         
      // go back to start menu
      displaySelectionScreen();                // print the stationary parts on the screen
      refreshMATRIX = true;         // refresh matrix
      menu_start_selected = false;  // deselect start
    }
    //blink "UP" text and set cursor & selection to UP-seconds if total UP time is 0
    if(menu_UPs_Value == 0 && menu_UPm_Value == 0)
    {
      for(int i = 0; i<3;i++)
      {
        displayText(4,28,1,0,"UP");    // erase UP counter
        delay(300);
        displayText(4,28,1,7,"UP");    // blue UP counter 
        delay(300);
      }
        displayText(4,28,1,70,"UP");
      menuCounter = 3;
      menu_UPs_selected = true;
      menu_start_selected = false;
      updateCursorPosition();
    }
    MP3_Player.volume(menu_volume_Value); // set volume
  }
   //-------------------volume is SELECTED---------------------------
  if(menu_volume_selected == true)
  {
    displayVolume(54,54,menu_volume_Value,77);
  }
  
  
}
// display input (seconds) as "00:00" in rgb-colour at center of screen for 1 second
void printTime(int t, int colour)
{
  // calculate minute and second digits
  int m0,m1,m2,s0,s1,s2;     
  m0 = (t - (t % 60))/60;             // minutes
  m1 = (m0 - (m0 % 10))/10;           // 1st digit of m0
  m2 = m0 % 10;                       // 2nd digit of m0
  s0 = t - (m0*60);                       // seconds
  s1 = (s0 - (s0 % 10)) /10;            // 1st digit of s0
  s2 = s0 % 10;                        // 2nd digit of s0
  // draw timer and scale font size up if input time < 1 minute
  if(t > 59)
  {
    displayText(3,25,2,colour, m1);
    displayText(15,25,2,colour, m2);
    displayText(27,25,2,colour, ":");
    displayText(39,25,2,colour, s1);
    displayText(51,25,2,colour, s2); 
    delay(1000);
    matrix.fillRect(3, 25, 22, 14, 0);
    matrix.fillRect(39, 25, 22, 14, 0);
    if(t == 60){        // remove ":" from display from "1:00" to "59" 
    displayText(27,25,2,0, ":");
    }
  }
  else
  {
    displayText(15,22,3,colour, s1);
    displayText(33,22,3,colour, s2);
    delay(1000);
    matrix.fillRect(15, 22, 33, 21, 0);
  }
}
// displays text with input: (x-coordinate,y-coordinate,font-size,rgb-colour,text) 
// rgb color input: (r,g,b) == (0,7,0) set as 70 without leading zeros
template <class T>
void displayText(int x, int y, int size, int colour, T text_input)
{
  int r = (colour - colour % 100)/100;          // calculate r,g,b per digit from colour 
  int g = (colour % 100 - colour % 10)/10;      // for easier input in displayText()
  int b = colour % 10;
  matrix.setCursor(x,y);                        // display at (x,y)
  matrix.setTextSize(size);                     // font size (8 px height)
  matrix.setTextWrap(true);                     // Do not wrap text automatically at end of line
  matrix.setTextColor(matrix.Color333(r,g,b));  // set colour
  matrix.print(text_input);                     // display text_input
}
// display timer interval screen
// display countdown, SETS*(UP/REST) cycles and ends last SET with UP and STOP screen
// NOTE: UP and REST cycles can be paused with pause button during timer countdown loops
void intervalTimerScreen(int RESTm, int RESTs, int UPm, int UPs, int SETS)
{
  int countdown = 10;       // countdown in s
  int i = countdown;        // i is initialied for while loop
  int sets_total = SETS;    // store total # of sets for while
  // display countdown------------------------------------------------------------------------
  displayText(8,9,1,777,"START IN");
  matrix.drawRect(4, 54, 56, 6, matrix.Color333(7,7,7)); 
  //display countown and loading bar simultaneously
  while (i > 0)
    {
    loadingBar(i,countdown,7,0,0);
    printTime(i,700); 
    if(i == 4)
    {
      MP3_Player.playMp3Folder(2);    // play 3s beep 0002_3sbeep.mp3
    }
    i--;
    } 
  matrix.fillRect(4, 54, 56, 6, 0); //clear loading bar
  matrix.fillScreen(0);
  // display up/rest cycle--------------------------------------------------------------------
  while (SETS > 0)
    {
    // display green screen with "GO", used two displayText-methods to align "GO" properly
    matrix.fillScreen(matrix.Color333(0, 7, 0));   
    displayText(4,15,5,0,"G");
    displayText(35,15,5,0,"O");
    delay(1000);
    matrix.fillScreen(0);
    // temporary variables to go through loop while SETS > 0
    int rest_total = RESTm*60+RESTs;
    int up_total = UPm*60+UPs;
    // display up countdown
    displayText(27,9,1,70,"UP");
    if(sets_total-SETS+1 > 9)
    {
      displayText(17,45,1,770,sets_total-SETS+1);
      displayText(29,45,1,770,"/");
      displayText(35,45,1,770,sets_total);
    }
    else
    {
      displayText(23,45,1,770,sets_total-SETS+1);
      displayText(29,45,1,770,"/");
      displayText(35,45,1,770,sets_total);
    }
    while (up_total > 0)  
      {
        if(!flagPause)    //if pause button isn't pressed
        {
          loadingBar(up_total,UPm*60+UPs,0,7,0);
          printTime(up_total,70);
          if(up_total == 4)
          {
            MP3_Player.playMp3Folder(2);    // play 3s beep 0002_3sbeep.mp3
          }
          up_total--;
        }
        else            //if pause button is pressed
        {
          loadingBar(up_total,UPm*60+UPs,0,0,7);
          printTime(up_total,7);
        }      
      } 
      matrix.fillRect(3, 53, 58, 8, 0); //clear loading bar
    // display rest while not on the last set cycle
    if(SETS > 1 && (rest_total != 0))
    {
      // display red screen with "REST"
      matrix.fillScreen(matrix.Color333(7, 0, 0));
      displayText(9,25,2,0,"REST");
      delay(1000);
      matrix.fillScreen(0);
      // display rest countdown
      displayText(21,9,1,700,"REST");
      while (rest_total > 0)   
      {
        if(!flagPause)    //if pause button isn't pressed
        {
          loadingBar(rest_total,RESTm*60+RESTs,7,0,0);
          printTime(rest_total,700);
          if(rest_total == 4)
          {
            MP3_Player.playMp3Folder(2);    // play 3s beep 0002_3sbeep.mp3
          }          
          rest_total--;
        }
        else
        {
          loadingBar(rest_total,RESTm*60+RESTs,0,0,7);
          printTime(rest_total,7);
        }
      }
    }
    SETS--; 
    }
  // display stop sign when SETS = 0
    MP3_Player.playMp3Folder(3);  // play stop sound: 0003_stop.mp3
    matrix.fillCircle(31, 31, 30, matrix.Color333(7,7,7));
    matrix.fillCircle(31, 31, 26, matrix.Color333(7,0,0));
    displayText(9,25,2,777,"STOP");
    delay(2500);
    matrix.fillScreen(0);
}
// display loading Bar animation per second at bottom of screen, 
// NOTE: use in same loop with printTime
void loadingBar(int t_now, int t_total, int r, int g, int b)
{
  matrix.fillRect(4, 54, 56, 6, 0);
  matrix.drawRect(3, 53, 58, 8, matrix.Color333(7,7,7)); 
  double i = floor(54*(1-((double)t_total-(double)t_now)/(double)t_total));
  matrix.fillRect(5, 55, i, 4, matrix.Color333(r,g,b)); 
}
// display Volume at center (x,y) with volume v 10/20/30 
void displayVolume(int x, int y, int v, int colour)
{
    int r = (colour - colour % 100)/100;          // calculate r,g,b per digit from colour 
    int g = (colour % 100 - colour % 10)/10;      // for easier input for colour selection
    int b = colour % 10;

    matrix.drawCircle(x, y, 6, matrix.Color333(r,g,b));
    if(v > 0)
    {
      matrix.fillTriangle(x-4, y, x-1, y-3, x-1, y+3, matrix.Color333(r,g,b));
      matrix.drawLine(x-4, y-1, x-4, y+1, matrix.Color333(r,g,b));  
      if ( v > 10)
      {
        matrix.drawLine(x+1,y-1,x+1,y+1,matrix.Color333(r,g,b));
      }
      if ( v > 20)
      {
        matrix.drawLine(x+3,y-2,x+3,y+2,matrix.Color333(r,g,b));
      }
    }
}