/*
 * 8 Channel Servo Driver
 * 2/1/2016, Joe Cummings
 * This sketch will drive up to eight servos for the purpose of controlling model railroad turnouts.
 * Each channel consists of a pushbutton, servo, LED1 (green), LED2 (red).
 * Pushbuttons are wired to parallel input lines of 74HC165.
 * During each iteration of main loop: 
 * - Reads serial output stream of 74HC165.
 * - Create loop to process each servo channel.
 *    -- Read value for appropriate bit position from 74HC165. 
 *    -- If state HIGH, process as button press, else ignore.
 * 
 * 
 * Button press process:
 * - If servo channel is in low range position, move to high range position; set LED1 high; set LED2 low, 
 *   else move servo to low range position; set LED1 low; set LED2 high 
 */

#include <VarSpeedServo.h>

#define NUMBER_OF_SHIFT_CHIPS   1               // number of 165/595 shift chips
#define DATA_WIDTH   NUMBER_OF_SHIFT_CHIPS * 8  // Width of 165 data (how many ext lines)
#define PULSE_WIDTH_USEC   5                    // Width of pulse to trigger the shift register to read and latch.
#define POLL_DELAY_MSEC   1                     // Optional delay between shift register reads.
#define BYTES_VAL_T unsigned int                // You will need to change the "int" to "long" If the NUMBER_OF_SHIFT_CHIPS is higher than 2

// 74HC595 connections
const int latchPin_595	= 5;
const int clockPin_595	= 6;
const int dataPin_595	= 4;
byte leds				= 0;

// 74HC165 connections
const int shld_165		= 8;						// Connect Pin 8 to SH/!LD		(pin 1, shift or active low load)
const int ce_165		= 9;						// Connect Pin 9 to !CE			(pin 15, clock enable, active low)
const int data_165		= 11;						// Connect Pin 11 to SER_OUT	(pin 9, serial data out)
const int clk_165		= 12;						// Connect Pin 12 to CLK		(pin 2, the clock that times the shifting)


byte mask               = 1;						// bitmask

const byte NumberServos           =     DATA_WIDTH; // the total number of servos
const unsigned long DetachTime    =     10000;		// detach servo after 10 seconds
const byte LedLightPin            =     13;         // internal test LED is on pin 13
byte i                            =     0;          // Loop counter
byte bitDelay                     =     100;
byte pos, old_pos                 =     0;

byte buttons_new[DATA_WIDTH];						// button state array
byte buttons_old[DATA_WIDTH];						// button state array

byte masks[DATA_WIDTH]            = {1,2,4,8,16,32,64,128};         // masks to extract bits

// Servo nr               =    {0,  1,  2 }
//byte buttons[]            =    {14, 15, 16}; // , 17, 18, 19}; // the analog 0-5 pins are also known as 14-19
byte ServoPins[]          =    {2,  3,  4};
byte SignalPins[]         =    {5, 6, 7};
byte RangeLow[]           =    {60, 60, 50};
byte RangeHigh[]          =    {90, 90, 110};
byte TravelSpeed[]        =    {10,  10,  10};

byte pinValues;
byte oldPinValues;

/* 
 * This function is essentially a "shift-in" routine reading the
 * serial Data from the shift register chips and representing
 * the state of those pins in an unsigned integer (or long).
*/
byte read_shift_regs()
{
	/* read out parallel inputs of 74HC165 return as byte to caller */

	byte shift_val = 0;										// byte-sized container to hold register values

	digitalWrite(shld_165, LOW);							// take shld low to load parallel registers into serial register
	delayMicroseconds(5);									// delay required by chip
	digitalWrite(shld_165, HIGH);
	delayMicroseconds(5);

	// Required initial states of these two pins according to the datasheet timing diagram
	pinMode(clk_165, OUTPUT);
	pinMode(data_165, INPUT);
	digitalWrite(clk_165, HIGH);							// clock initial state HIGH
	digitalWrite(ce_165, LOW);								// clock enable initial state low (negative logic)

	shift_val = shiftIn(data_165, clk_165, MSBFIRST);		// read in register D0-D7 values
	digitalWrite(ce_165, HIGH);								// Disable the clock

	return shift_val;
}


/* Dump the list of zones along with their current status. */
void display_pin_values()
{
    Serial.print("Pin States:\r\n");

    for(i = 0; i < DATA_WIDTH; i++)
    {
        Serial.print("  Pin-");
        Serial.print(i);
        Serial.print(": ");

        if((pinValues >> i) & 1)
            Serial.print("HIGH");
        else
            Serial.print("LOW");

        Serial.print("\r\n");
    }

    Serial.print("\r\n");
}

void setup()
{
    Serial.begin(9600);

    /* Initialize digital pins for 74HC165 */
	pinMode(shld_165, OUTPUT);
	pinMode(ce_165, OUTPUT);
	pinMode(clk_165, OUTPUT);
	pinMode(data_165, INPUT);

	/* Required initial states of these two pins according to the datasheet timing diagram */
	digitalWrite(clk_165, HIGH);
	digitalWrite(shld_165, HIGH);

    /* initialize digital pins for 74HC595 */
    pinMode(latchPin_595, OUTPUT);
    pinMode(dataPin_595, OUTPUT);  
    pinMode(clockPin_595, OUTPUT);

    for(i=0;i<DATA_WIDTH;i++)                               // initialize state arrays to known state
    {
      buttons_new[i] = 0;
      buttons_old[i] = 0;
    }

    /* Read in and display the pin states at startup. */
    pinValues = read_shift_regs();
    display_pin_values();
    oldPinValues = pinValues;
}

void loop()
{
    pinValues = read_shift_regs();                            // get button states

	/*
	for (int x = 0; x < 8; x++) {
		buttons_new[x] = pinValues << 1
	}
	*/
    
    for(i=0; i<DATA_WIDTH; i++) {  
      if(pinValues & masks[i]) {                              // get button state from pinValues
        buttons_new[i] = 1;
      } else {
        buttons_new[i] = 0;
    }

      if(buttons_new[i] == 1 && buttons_old[i] != 1)          // react on first button press only
      {
        leds += masks[i];

        if (leds&masks[i]) {
          Serial.print("Clearing bit ");
          Serial.println(i);
          bitClear(leds, i);
        } else {
          Serial.print("Setting bit ");
          Serial.println(i);
          bitSet(leds, i);
        } 

        
        //updateShiftRegister();
        delay(500);
                
        Serial.print("(button-");
        Serial.print(i, DEC);
        Serial.print("): ");
        Serial.print("led value: " );
        Serial.println(leds, DEC);
        
        /*
        if(digitalRead(TEST_LED_PIN) == HIGH)                
        {
          Serial.print("(button-");
          Serial.print(i, DEC);
          Serial.print("): ");
          Serial.println("LED is on, turn off");
          digitalWrite(TEST_LED_PIN, LOW);
        } 
        else 
        {
          Serial.print("(button-");
          Serial.print(i, DEC);
          Serial.print("): ");
          Serial.println("LED is off, turn on");
          digitalWrite(TEST_LED_PIN, HIGH);
        }
        */
      }
      buttons_old[i] = buttons_new[i];   
    }
    
    delay(POLL_DELAY_MSEC);
}
