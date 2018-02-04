/*
 @author: Justin D Vrana
 @version v2.32
 @since 2014-04-03

What is this?
The basic code needed to control a multi-output, programmable LED light device for optogenetic applications.
The the code allows for user-defined parameters to alter duration and intensity of light pulses at regular intervals.
Central to the device is an inexpensive microcontroller and LCD keypad to display and read user input to control
the duration and intensity of LED light pulses at user-defined intervals. Up to three LED outputs can be independently
programmed and different LED modules can be interchanged.

Usage:
Upload to Arduino Uno via USB. If uploaded properly, "Starting" should display on the LCD display screen and then the main page should display.
If there are issue uploading to the Arduino Uno, consult the Arduino website for help.

    To turn an output ON or OFF: From the main page on the LCD display, use the UP and DOWN buttons to select an output (indicated by a cursor)
    to edit. Move the cursor to the right using the RIGHT button and use either the UP or DOWN button to turn the output program ON
    or OFF. A countdown timer which counts down untill the next time the LED is to flash will display when the output program is running.

    To change an output's scheduled flashing: To change an output program, push SELECT to select the output highlighted by the cursor.
    Use the UP and DOWN buttons to select and attribute to edit. To edit and attribute, move the cursor to the right using the RIGHT
    button and then hold the UP or DOWN button to increase or decease a value. Move the cursor back to the left using the LEFT button
    to change attributes to edit. Push the SELECT button at any time to return to the main page.

Sources:
- The original "LiquidCrystal" 8-bit library and tutorial
    http://www.arduino.cc/en/uploads/Tutorial/LiquidCrystal.zip
    http://www.arduino.cc/en/Tutorial/LCDLibrary

Tested only with a Arduino Uno (http://arduino.cc/) and DFRobot Keypad Shield (http://dfrobot.com)

*****Important*****:
Check and change "User Defined Paramters" directly below if needed. LCD pins, output pins, and anaolog voltages
must be changed if using an LCD Keypad Shield other than the DFRobot Keypad Shield or SainSmart Keypad Shield.
Refer to the datasheet of the LCD Keypad Shield to determine correct parameter values.
Button values can be reset for using the "Button Reset Mode"

Button Reset Mode:
If using a different LCD Keypad shield, button values may need to be reset. To initiate button reset mode, during startup hold
down any button and continue holding button for at least three seconds when prompted. Follow the on screen instructions.
The screen will request you press and hold each button while it records the voltage values associated with that button.
Once completed, voltage values will be stored to the Arduino's EEPROM memory. Upon each start-up, these voltage values
will be loaded and used. To reset button values on the Arduino, simply re-upload this program using the "UPLOAD" button.

V 2.0 - New Version with new interface
V 2.1 - Re-vamped interface
V 2.2 - Expanded to three outputs
V2.3 - Created button reset mode
V2.32 (4/10/14) - Corrected coding error, cast timers to int instead of long
*/








////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////////////////// User Defined Parameters ////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

/*/////////////////// User Defined LCD Display Parameters ////////////////////
// Check LCD Keypad Shield datasheet to determine which Arduino Uno pins correspond
// to the RS, Enable, D4, D5, D6, and D7 pins on the LCD Keypad Shield. Change the
// values below. For example, DFRobot Keypad Shield for Arduino designates:
//    Digital Pin 9 on the Arduino to the reset pin (RS) on the LCD Keypad
//    Digital Pin 8 on the Arduino Uno to the Enable pin on the LCD keypad
//    Digital Pin 4 on the Arduino Uno to the D4 pin on the LCD keypad shield
//    Digital Pin 5 on the Arduino Uno to the D5 pin on the LCD keypad shield
//    Digital Pin 6 on the Arduino Uno to the D6 pin on the LCD keypad shield
//    Digital Pin 7 on the Arduino Uno to the D7 pin on the LCD keypad shield
//    Analog Pin 0 on the Arduino Uno to the Button select pin on the LCD keypad shield
///////////////////////////////////////////////////////////////////////////////*/
int RS_pin = 8;
int enable_pin = 9;
int D4_pin = 4;
int D5_pin = 5;
int D6_pin = 6;
int D7_pin = 7;
int button_select_pin = 0;


/*//////////////////// User Defined LCD Display Parameters ////////////////////
// Check LCD Keypad Shield datasheet to determine which Arduino Uno pins are
// available for use as output pins. Pins designated to the RS, Enable, D4, D5,
// D6, D7, and Button select pins (see above) on the LCD Keypad shield are
// unavailable for use as LED output pins. For example, DFRobot Keypad Shield
// has pins 12, 11, and 3 pins free.
///////////////////////////////////////////////////////////////////////////////*/
int LED_output_pin_1 = 12;
int LED_output_pin_2 = 11;
int LED_output_pin_3 = 3;


/*//////////////////// User Defined Analog Button Parameters ////////////////////
// Buttons on the LCD Keypad Shield change the voltage to the button_select_pin.
// Different values being recorded correspond to which button is being pressed
// Depending on LCD Keypad shield, the following values may be different. Button
// values can be reset live if desired by pressing and holding any button for
// three seconds during startup.
///////////////////////////////////////////////////////////////////////////////*/

//DFRobot LCD Keypad Shield Values
int btnNONE_upper_voltage = 1000;
int btnRIGHT_upper_voltage = 0;
int btnUP_upper_voltage = 150;
int btnDOWN_upper_voltage = 330;
int btnLEFT_upper_voltage = 500;
int btnSELECT_upper_voltage = 750;
int button_threshold = 50;

/*
//SainSmart LCD Keypad Shield Values
int btnNONE_upper_voltage = 1023;
int btnRIGHT_upper_voltage = 0;
int btnUP_upper_voltage = 144;
int btnDOWN_upper_voltage = 329;
int btnLEFT_upper_voltage = 505;
int btnSELECT_upper_voltage = 742;
*/

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////////////// END User Defined Parameters ////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
///////////////////// VARIABLES ////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

#include <LiquidCrystal.h>
#include <EEPROM.h>

LiquidCrystal lcd(RS_pin, enable_pin, D4_pin, D5_pin, D6_pin, D7_pin); // Used to display information to LCD Display.

///////////////////// struct output ////////////////////
// Defines LED output parameters
// Name: name of output
// pin: which pin controls this output
// runs_status: is this output program currently running?
// run_state: is the LED currently powered on?
// attributes: defines duration, interval, and intensity for this input
// attribute_labels: labels for interval, interval, and intensity
// interval_timer: counter used to determine when to power ON led (change state = true)
// duration_timer: counter used to determine when to power OFF led (change state = false)
///////////////////// END struct output ////////////////////
struct output{
  String name;
  int pin;
  boolean run_status;
  boolean run_state;
  int attributes[3];
  String attribute_labels[3];
  long interval_timer;
  long duration_timer;
} out_1, out_2, out_3; // define outputs 1, 2, and 3

int LED_output_pins[3] = {LED_output_pin_1, LED_output_pin_2, LED_output_pin_3}; // Designates output pins on Arduino used to control LED outputs
output LED_outputs[3] = {out_1, out_2, out_3}; // Array for each output. Index is used later in program to grab corresponding output.
int selected_item = 0; // Used to designate item highlighted on LCD Display.
int current_page = 0; // Current page beging viewed (0 = top, 1 = edit).
int selected_output = 0; // Designate last selected output. Index to determine which output structure to retrieve from "LED_outputs" array.
int top_item_displayed = 0; // Which item is displayed at top of page.
int cursorPositionY = 0; // Designates where the cursor should be (0=top row, 1=bottom row).
int cursorPositionX = 0; // Designates where the cursor should be (0=1st character, 1=12th character).
int lcd_key     = 0; // Analog input key pressed. Example: btnRIGHT = 0
int prev_lcd_key = 0; // Previous analog input key pressed.
// long button_hold_interval_counter: Used to increase or decrease a value while holding the UP or DOWN buttons.
// Designates total time since value was increased while holding button.
long button_hold_interval_counter = 0;
// long button_hold_start_counter: Used to increase or decrease a value while holding the UP or DOWN buttons.
// Designates total time button is held.
long button_hold_start_counter = 0;
int button_values[6] = {btnRIGHT_upper_voltage, btnUP_upper_voltage, btnDOWN_upper_voltage,
  btnLEFT_upper_voltage, btnSELECT_upper_voltage, btnNONE_upper_voltage}; //Contains voltage values for each button
String button_names[6] = {"RIGHT", "UP", "DOWN", "LEFT", "SELECT", "NONE"}; // Contains names for each button
int button_addresses[6] = {101, 102, 103, 104, 105, 106}; // Contains EEPROM address for each button value
byte button_override_true = 111; // Contains value to override button values (after button reset mode)
int button_override_address = 111; // Address of whether to overrid buttons



////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
///////////////////// DEFINITIONS ////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

// Definitions used to determine pages.
// Definitions used to determine button pushed on lcd keypad
#define EDITPAGE 1
#define TOPPAGE 0
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5
#define btnOverride 111



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/////////////////// MAIN METHODS (setup and loop //////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////// method: setup() ////////////////////
// Method initializes upon Arduino Uno startup and
// initialized variables for LED_outputs and LCD display
// Returns: void
///////////////////// END Setup ////////////////////
void setup() {
  Serial.begin(9600);
  // Initialize output values for each LED output
  for (int i = 0; i < 3; i++) {
    LED_outputs[i].name = "Output ";
    LED_outputs[i].name = LED_outputs[i].name + (i + 1); // Name of this output to be displayed on the LCD display
    LED_outputs[i].pin = LED_output_pins[i]; // Designates this output to a pin on the Arduino Uno
    LED_outputs[i].run_status = false; // Determines if the output program is currently running (true) or not running (false)
    LED_outputs[i].run_state = false; // Determines if the LED is powered (true) or not powered (false)
    LED_outputs[i].attributes[0] = 1; // Value defines the duration of the LED light pulse
    LED_outputs[i].attributes[1] = 10; // Value defines the interval duration of the LED light pulse
    LED_outputs[i].attributes[2] = 100; // Value defines the intensity of the LED light pulse (0%-100%)
    LED_outputs[i].attribute_labels[0] = "Dura(s)"; // Corresponding label for duration to be displayed on the LCD display
    LED_outputs[i].attribute_labels[1] = "Int(s)"; // Corresponding label for interval to be displayed on the LCD display
    LED_outputs[i].attribute_labels[2] = "Intensity(%)"; // Corresponding label for intensity to be displayed on the LCD display
  }
  lcd.begin(16,2);
  lcd.print("Starting");
  delay(300);
  lcd.clear();
  lcd.print("Starting.");
  delay(300);
  lcd.clear();
  lcd.print("Starting..");
  delay(300);
  lcd.clear();
  lcd.print("Starting...");
  Serial.print("Checking for button reset mode\n");
  buttonReset(); // Check for button and activate button reset if needed.
  Serial.print("Loading values from memory\n");
  loadButtonValuesFromMemory(); // Only loads if button reset has been previously activated.
  delay(300);
  lcd.clear();
  lcd.blink(); // Sets cursor blinker ON
  lcd.cursor(); // Sets underline cursor ON
  printScreen();
  button_hold_interval_counter = millis();
}

///////////////////// method: loop() ////////////////////
// Method is automatically looped by the Arduino Uno microcontroller continuously
// returns: void
///////////////////// END loop() ////////////////////
void loop() {
  flash();
  printScreen();
  displayCursor();
  delay(50);
  lcd_key = read_LCD_buttons(); //Read analog input from buttons
  if (lcd_key != prev_lcd_key)  { //New key is pressed
     button_hold_start_counter = millis(); // Timer for how long button is held
     switch (lcd_key) {               // depending on which button was pushed, we perform an action

         case btnRIGHT:{
              if (cursorPositionX == 0) { cursorPositionX = 1; }
              delay(50);
              break;
         }
         case btnLEFT:{
               if (cursorPositionX == 1) {
                 cursorPositionX = 0;
               }
               break;
         }
         case btnUP:{
               if (cursorPositionX == 0) {
                 lcd.clear();
                 prevItem();
               }
               else if (cursorPositionX == 1 && current_page == TOPPAGE) {
                 if (LED_outputs[selected_item].run_status == false) {
                   turnOnOutput(selected_item);
                 } else {
                   turnOffOutput(selected_item);
                 }
               }
               lcd.clear();
               printScreen();
               break;
         }
         case btnDOWN:{
               if (cursorPositionX == 0) {
                 lcd.clear();
                 nextItem();
               }
               else if (cursorPositionX == 1 && current_page == TOPPAGE) {
                 if (LED_outputs[selected_item].run_status == false) {
                   turnOnOutput(selected_item);
                 } else {
                   turnOffOutput(selected_item);
                 }
               }
               lcd.clear();
               printScreen();
               break;
         }
         case btnSELECT:{
               if (current_page == TOPPAGE) {
                 selected_output = selected_item;
                 toEditPage();
               }
               else if (current_page == EDITPAGE) {
                 toTopPage();
               }
               break;
         }
         case btnNONE:{
               break;
         }
     }
   }
   // Else if on edit page, with cursor fartherest right and the UP or DOWN button is pressed
   else if (cursorPositionX == 1 && current_page == EDITPAGE && (lcd_key == btnDOWN || lcd_key == btnUP)) {
      int * value;
      value = &LED_outputs[selected_output].attributes[selected_item];
      increaseValueWhileHoldingButton(value); // Either increase or decease value of attribute while holding the UP or DOWN button
      lcd.clear(); // Clear screen
      printScreen(); // Print to screen
      delay(50);
   }
   prev_lcd_key = lcd_key; // Save previous key pressed
}

void turnOffOutput(int which_output) {
  LED_outputs[which_output].run_status = false;
  LED_outputs[which_output].run_state = false;
  LED_outputs[which_output].interval_timer = millis();
  LED_outputs[which_output].duration_timer = millis();
}

void turnOnOutput(int which_output) {
  LED_outputs[which_output].run_status = true;
  LED_outputs[which_output].run_state = true;
  LED_outputs[which_output].interval_timer = millis();
  LED_outputs[which_output].duration_timer = millis();
}

///////////////////// method: flash() ////////////////////
// Uses interval_timer and duration_timer of each output to determine when to
// power LEDs ON or OFF. Duration and interval attributes (attributes[0] and attributes[1] respectively)
// are used to determine the length of the duration and interval. Intensity (attribute[2]) is used to
// determine the how much power goes to the LEDs.
// returns: void
///////////////////// END flash()  ////////////////////
void flash() {
  for (int i = 0; i < 3; i++) {

    unsigned long current_time = millis();
    /*  //Serial output
    if (i == i) {
      Serial.print("Output: ");
      Serial.print(i);
      Serial.print( " Run Status: ");
      Serial.print(LED_outputs[i].run_status);
      Serial.print( " LED States: ");
      Serial.print(LED_outputs[i].run_state);
      Serial.print(" Current time: ");
      Serial.print(millis()/1000);
      Serial.print( " Interval Timer: ");
      Serial.print(LED_outputs[i].interval_timer/1000);
      Serial.print( " Duration Timer: ");
      Serial.print(LED_outputs[i].duration_timer/1000);
      Serial.print( " Interval: ");
      Serial.print(LED_outputs[i].attributes[1]);
      Serial.print( " Duration: ");
      Serial.print(LED_outputs[i].attributes[0]);
      Serial.print("  ");
      Serial.print(current_time - LED_outputs[i].interval_timer);
      Serial.print( " > " );
      unsigned long this_interval = LED_outputs[i].attributes[1] * 1000.0;
      Serial.print(this_interval);
      Serial.print("\n");
    }
    /**/
    if (LED_outputs[i].run_status) {
      boolean state = LED_outputs[i].run_state;
      if (current_time - LED_outputs[i].interval_timer > LED_outputs[i].attributes[1]*1000.0 && !state) {
          LED_outputs[i].run_state = true;
          LED_outputs[i].interval_timer = current_time; // reset interval timer
          LED_outputs[i].duration_timer = current_time; // reset duration timer
      }
      if (millis() - LED_outputs[i].duration_timer > LED_outputs[i].attributes[0] * 1000.0 && state) {
        LED_outputs[i].run_state = false;
        LED_outputs[i].duration_timer = current_time; // reset duration timer
      }
      if (LED_outputs[i].run_state) {
        analogWrite(LED_outputs[i].pin, map(LED_outputs[i].attributes[2], 0, 100, 0, 255));
      }
      else {
        digitalWrite(LED_outputs[i].pin, LOW);
      }
    }
    else {
      LED_outputs[i].interval_timer = current_time;
      LED_outputs[i].duration_timer = current_time;
      digitalWrite(LED_outputs[i].pin, LOW);
    }
  }
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/////////////////// METHODS FOR USER DISPLAY AND INTERACTION //////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////// method: toTopPage() ////////////////////
// Simply returns to the first page to display the outputs.
// Text is scrolled left and cursor position is reset.
// returns: void
///////////////////// END toTopPage() ////////////////////
void toTopPage() {
  scrollText(false, 50, 16);
  lcd.clear();
  cursorPositionY = 0;
  cursorPositionX = 0;
  current_page = TOPPAGE;
  selected_item = 0;
  top_item_displayed = 0;
  printScreen();
}

///////////////////// method: toEditPage() ////////////////////
// Simply returns to the edit page to display the output attributes
// Text is scrolled right and cursor position is reset.
// returns: void
///////////////////// END toEditPage() ////////////////////
void toEditPage() {
  scrollText(true, 50, 16);
  lcd.clear();
  cursorPositionY = 0;
  cursorPositionX = 0;
  current_page = EDITPAGE;
  selected_item = 0;
  top_item_displayed = 0;
  printScreen();
}

///////////////////// method: display() ////////////////////
// Displays cursor at position as determined by cursorPositionX
// and cursorPositionY
// returns: void
///////////////////// END displayCursor() ////////////////////
void displayCursor() {
  if (cursorPositionY == 0 && cursorPositionX == 0) {
    lcd.setCursor(0,0);
  }
  else if (cursorPositionY == 1 && cursorPositionX == 0) {
    lcd.setCursor(0,1);
  }
   else if (cursorPositionY == 0 && cursorPositionX == 1) {
    lcd.setCursor(13,0);
  }
  else if (cursorPositionY == 1 && cursorPositionX == 1) {
    lcd.setCursor(13,1);
  }
}

///////////////////// method: printScreen() ////////////////////
// Prints to LCD screen. Output depends on which page and which
// item is selected.
// returns: void
///////////////////// END printScreen() ////////////////////
void printScreen() {
  if (current_page == EDITPAGE) {
      lcd.setCursor(0,0);
      lcd.print(LED_outputs[selected_output].attribute_labels[top_item_displayed]);
      lcd.setCursor(9,0);
      lcd.print(convertSeconds(LED_outputs[selected_output].attributes[top_item_displayed]));
      lcd.setCursor(0,1);
      lcd.print(LED_outputs[selected_output].attribute_labels[top_item_displayed+1]);
      if (top_item_displayed + 1 == 2) {
        lcd.setCursor(13,1);
        lcd.print(LED_outputs[selected_output].attributes[top_item_displayed+1]);
      } else {
        lcd.setCursor(9,1);
        lcd.print(convertSeconds(LED_outputs[selected_output].attributes[top_item_displayed+1]));
      }
  } else if (current_page == TOPPAGE) {
    lcd.setCursor(0,0);
      lcd.print(LED_outputs[top_item_displayed].name);
      if (LED_outputs[top_item_displayed].run_state) {
        lcd.print("*");
      }
      else {
        lcd.print(" ");
      }
      lcd.setCursor(13,0);
      int out_status = LED_outputs[top_item_displayed].run_status;
      if (out_status == 1) {
        lcd.setCursor(10,0);
        lcd.print(convertSeconds(LED_outputs[top_item_displayed].attributes[1] - (millis() - LED_outputs[top_item_displayed].interval_timer)/1000));
        lcd.print("       ");
      }
      else {
        lcd.print("OFF");
      }
      lcd.setCursor(0,1);
      lcd.print(LED_outputs[top_item_displayed+1].name);
      if (LED_outputs[top_item_displayed+1].run_state) {
        lcd.print("*");
      }
      else {
        lcd.print(" ");
      }
      lcd.setCursor(13,1);
      out_status = LED_outputs[top_item_displayed+1].run_status;
      if (out_status == 1) {
        lcd.setCursor(10,1);
        lcd.print(convertSeconds(LED_outputs[top_item_displayed+1].attributes[1] - (millis() - LED_outputs[top_item_displayed+1].interval_timer)/1000));
        lcd.print("       ");
      }
      else {
        lcd.print("OFF");
      }
  }
}

///////////////////// method: convertSeconds() ////////////////////
// Converts an integer to a 00:00 format
// seconds: integer of how many seconds to convert
// returns: String time in format of 00:00
///////////////////// END convertSeconds() ////////////////////
String convertSeconds(int seconds) {
   int m = seconds/60;
   int s = seconds - m * 60.0;
   String time = "";
   if (m < 10) {
     time = time + "0" + m;
   }
   else {
     time = time + m;
   }
   time = time + ":";
   if (s < 10) {
     time = time + "0" + s;
   } else {
     time = time + s;
   }
   return time;
}

///////////////////// method: nextItem() ////////////////////
// Moves to next item. Moves cursor down. If cursor is down, scroll
// page down.
// returns: void
///////////////////// END nextItem() ////////////////////
void nextItem() {
  int pageLength = 3;
  if (cursorPositionY == 1) {
    top_item_displayed = top_item_displayed + 1;
    if (top_item_displayed > pageLength - 2) {
      top_item_displayed = pageLength - 2;
    }
    else {
      cursorPositionY = 0;
    }
  }
  else {
    cursorPositionY = 1;
  }
  selected_item = cursorPositionY + top_item_displayed;
  printScreen();
}

///////////////////// method: prevItem() ////////////////////
// Moves to previous item. Moves cursor up. If cursor is up, scroll
// page up.
// returns: void
///////////////////// END prevItem() ////////////////////
void prevItem() {
  if (cursorPositionY == 0) {
    top_item_displayed = top_item_displayed - 1;
    if (top_item_displayed < 0) {
      top_item_displayed = 0;
    }
    else {
      cursorPositionY = 1;
    }
  }
  else {
    cursorPositionY = 0;
  }
  selected_item = cursorPositionY + top_item_displayed;
  printScreen();
}

///////////////////// method: increaseValueWhileHoldingButton() ////////////////////
// If button is being held, increase or decrease value designated by *value depending
// on how long the UP or DOWN button is held. The longer the button is held, the faster
// the value is changed.
// int *value: pointer to value to decrease or increase
// returns: void
///////////////////// END increaseValueWhileHoldingButton() ////////////////////
void increaseValueWhileHoldingButton(int *value) {
  if (millis()-button_hold_interval_counter > 100) {
           int increment = 0;
           if (millis() - button_hold_start_counter < 2000 || *value % 10 > 0) {
             increment = 1;
           }
           else if (millis() - button_hold_start_counter > 2000 && millis() - button_hold_start_counter < 5000) {
             increment = 10;
           }
           else if (millis() - button_hold_start_counter > 5000 && millis() - button_hold_start_counter < 10000) {
             increment = 30;
           }
           else if (millis() - button_hold_start_counter > 10000) {
             increment = 60;
           }
           if (lcd_key == btnDOWN) {
             increment = increment * -1;
           }
           *value = *value + increment;
           if (*value < 0) {
             *value = 0;
           }
           button_hold_interval_counter = millis();
     }
}

///////////////////// method: scrollText() ////////////////////
// Scrolls the current text left or right
// boolean scrollLeft: which direction to scroll text
// int scrollDelay: how quickly to scroll text (how many ms to move to next position)
// int number: how far to scroll the text
// returns: void
///////////////////// END scrollText() ////////////////////
void scrollText(boolean scrollLeft, int scrollDelay, int number) {
  for (int i = 0; i < number; i++) {
    // wait a bit:
    delay(scrollDelay);
    if (!scrollLeft) {
      lcd.scrollDisplayRight();
    }
    else {
      lcd.scrollDisplayLeft();
    }
  }
  lcd.clear();
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/////////////////// METHODS FOR ANALOG INPUT AND BUTTON RESET MODE/////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////



///////////////////// method: read_LCD_buttons() ////////////////////
// Returns which button was pressed.
// returns: int which button was pushed
///////////////////// END read_LCD_buttons() ////////////////////
int read_LCD_buttons() {               // read the buttons
    button_select_pin = analogRead(0);       // read the value from the sensor
    if (button_select_pin > btnNONE_upper_voltage) return btnNONE;
    for (int i = 0; i < 5; i++) {

      if (button_select_pin < button_values[i] + button_threshold &&
        button_select_pin > button_values[i] - button_threshold) {
          return i;
        }
    }
    return btnNONE;                // when all others fail, return this.
}


///////////////////// method: buttonReset() ////////////////////
// Used during Button Reset Mode.
// If button is pressed and held for 3 seconds, initial button reset mode
// For each button, promt a request for voltage value of each button.
// Save values to EEPROM memory
// returns: void
///////////////////// END buttonReset() ////////////////////
void buttonReset() {
  Serial.print("Button Reset mode activated\n");
  button_select_pin = analogRead(0);
  if (button_select_pin < btnNONE_upper_voltage) { //If button is being pressed
    int countdownTimer = 5;
    lcd.clear();
    while (button_select_pin < btnNONE_upper_voltage && countdownTimer >= 1) {
      lcd.setCursor(0,0);
      lcd.print("Btn Rst in ");
      lcd.print(countdownTimer);
      delay(1000);
      countdownTimer = countdownTimer - 1;
      lcd_key = read_LCD_buttons();
    }
    if (countdownTimer <= 1) {
      //Btn Rst activated
      lcd.clear();
      lcd.print("Btn Rst");
      lcd.setCursor(0,1);
      lcd.print("activated");
      delay(1000);
      for (int i = 0; i < 5; i++) {
        waitForButtonRelease();
        delay(500);
        button_values[i] = retrieveAverageButtonValue(button_names[i]);
        delay(500);
      }
      lcd.clear();
      lcd.print("Btn Rst");
      lcd.setCursor(0,1);
      lcd.print("Completed");
      saveButtonValuesToMemory();
      delay(2000);
    }
  }
}

///////////////////// method: waitForButtonRelease() ////////////////////
// Used during Button Reset Mode.
// Requests button release for program continues
// returns: void
///////////////////// END waitForButtonRelease() ////////////////////
void waitForButtonRelease() {
  lcd.clear();
  button_select_pin = analogRead(0);
  while (button_select_pin < btnNONE_upper_voltage) {
    lcd.setCursor(0,0);
    lcd.print("Release");
    lcd.setCursor(0,1);
    lcd.print("all buttons");
    lcd_key = read_LCD_buttons();
    delay(50);
  }
  lcd.clear();
  lcd.print("Btn released");
}

///////////////////// method: retrieveAverageValue() ////////////////////
// Used during Button Reset Mode.
// Demands a button to be pressed.
// Records up to 50 values and averages value.
// param: name of button requested
// returns: average voltage value of button
///////////////////// END retrieveAverageValue() ////////////////////
int retrieveAverageButtonValue(String whichButton) {
  long total = 0;
  int count = 1;
  int numValues = 50;
  int average_value = 0;
  waitForButtonRelease();
  delay(500);
  demandButtonPress(whichButton);
  button_select_pin = analogRead(0);
  lcd.clear();
  while (button_select_pin < btnNONE_upper_voltage && count <= numValues) {
    button_select_pin = analogRead(0);
    lcd.setCursor(0,0);
    lcd.print("Hold btn ");
    lcd.print(count);
    lcd.print("/");
    lcd.print(numValues);
    lcd.print("        ");
    total = total + button_select_pin;
    average_value = total / count;
    lcd.setCursor(0,1);
    lcd.print("Avg: ");
    lcd.print(average_value);
    lcd.print("         ");
    count = count + 1;
    delay(50);
  }
  lcd.clear();
  lcd.print("button set:");
  lcd.setCursor(0,1);
  lcd.print(average_value);
  return average_value;
}

///////////////////// method: demandButtonPress() ////////////////////
// Used during Button Reset Mode.
// Demands a button to be pressed.
// param: name of button requested
// returns: voltage value of button
///////////////////// END demandButtonPress() ////////////////////
int demandButtonPress(String whichButton) {
  lcd.clear();
  button_select_pin = analogRead(0);
  while (button_select_pin > btnNONE_upper_voltage) {
    lcd.setCursor(0,0);
    lcd.print("Press and hold .[");
    lcd.setCursor(0,1);
    lcd.print(whichButton);
    lcd_key = read_LCD_buttons();
    delay(50);
  }
  lcd.clear();
  lcd.print("btn pressed");
  lcd.setCursor(0,1);
  lcd.print(button_select_pin);
  return button_select_pin;
}

///////////////////// method: loadButtonValuesFromMemory() ////////////////////
// Used during setup()
// Load button voltage values from EEPROM memory. Finds an appropriate threshold
// for those values. Displays those values on screen.
// returns: void
///////////////////// END loadButtonValuesFromMemory() ////////////////////
void loadButtonValuesFromMemory() {
  byte overrideButtons = EEPROM.read(button_override_address);
  if (overrideButtons == button_override_true) {
    for (int i = 0; i < 6; i++) {
      button_values[i] = EEPROM.read(button_addresses[i]) * 10; // Multiple by 10 since EEPROM can only store values up to 255
    }
    // Copy button_values and sort values.
    int copy_array[5] = {0,0,0,0,0}; // a copy array of button values to be sorted
    for (int i = 0; i < 5; i++) {
      copy_array[i] = button_values[i];
    }
    for (int nStartIndex = 0; nStartIndex < 5; nStartIndex++) {
      int nSmallestIndex = nStartIndex;
      for (int nCurrentIndex = nStartIndex + 1; nCurrentIndex < 5; nCurrentIndex++) {
        if (copy_array[nCurrentIndex] < copy_array[nSmallestIndex]) {
          nSmallestIndex = nCurrentIndex;
        }
      }
      int temp_value = copy_array[nStartIndex];
      copy_array[nStartIndex] = copy_array[nSmallestIndex];
      copy_array[nSmallestIndex] = temp_value;
    }
    // Calculate average distance between values. Set threshold to 1/3 of that distance.
    int total_distance = 0;
    for (int i = 0; i < 4; i++) {
      total_distance = total_distance + (copy_array[i+1] - copy_array[i])/3;
    }
    button_threshold = total_distance/3;
    lcd.clear();
    lcd.print("Button values");
    lcd.setCursor(0,1);
    lcd.print("loaded");
    delay(1000);
    lcd.clear();
    lcd.print("Btn thrshld set");
    lcd.setCursor(0,1);
    lcd.print(button_threshold);
    delay(500);
    lcd.clear();
    for (int i = 0; i < 3; i++) {
      lcd.print(button_values[i]);
      lcd.print(" ");
    }
    lcd.setCursor(0,1);
    for (int i = 3; i < 6; i++) {
      lcd.print(button_values[i]);
      lcd.print(" ");
    }
    delay(500);
  }
  // Else use default values
}

///////////////////// method: saveButtonValuesToMemory() ////////////////////
// Used during Button Reset Mode.
// Saves each button value to designate button address on EEPROM memory.
// Saves that these values were altered (button_override_true) so next start-up
// will load EEPROM button values.
// returns: void
///////////////////// END saveButtonValuesToMemory() ////////////////////
void saveButtonValuesToMemory() {
  for (int i = 0; i < 6; i++) {
    EEPROM.write(button_addresses[i], button_values[i]/10); //Divide by 10 since EEPROM can only store values up to 255
  }
  EEPROM.write(button_override_address, button_override_true); //From now on, program will use stored EPPROM values
}