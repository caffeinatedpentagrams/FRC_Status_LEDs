// This code written by Mark Crawford for FRC Team 2158 
// Coded during the 2015 build season for Recycle Rush
// Go AusTIN CANs!!!
//
// Some testing has been performed with this code, but I doubt that all possible combinations have been tested. Please use at your own risk.
//
// The Cylon Eye effect was adapted, with many thanks, from code written by EternalCore: https://github.com/EternalCore/NeoPixel_Cylon_Scrolling_Eye
// 
// -------------- General Description --------------
//
// This program is designed to take single characters from a serial input
// and change the color of a section of an LED strip based on the input. An optional LED
// "show" of various kinds can be started upon arduino boot or upon a specific character input. (Intended for use
// when the robot is otherwise idle.) The "show" will end when any character is passed into the serial input.
//
// Use of a WS2812 (aka. NeoPixel) compatible LED string is required (presence of the Adafruit Neopixel library assumed)
//
// The number of sections in the LED display is definable, HOWEVER:
// the number of LEDs in the string is assumed to be evenly divisible by the number of sections specified
// (code behavior is undefined if this requirement is not met)
//
// Having a single white LED between sections is optional. When enabled, relative direction of the string will be 
// indicated by the single white LED lit at the far end of status sections. This may be useful for disambiguation
// as mentioned below. This does reduce the size of each section by one LED.
//
// For usability reasons, care should be taken to avoid ambiguous status color schemes like red-white-red-white-red. It would become more difficult
// to tell which end was which and therefore which section of the display corresponded to which "function" of the robot in such a case.
//
// Be mindful that this code is intended to divide a SINGLE string of WS2812 LEDs into sections. This is probably not what you want if you're working with a
// single string that wraps to different sides of the robot, or where you've got multiple strings daisy chained together all over the robot. The working
// assumption in this code is that if multiple status displays are desired (for instance on multiple sides of the robot), they should all be divided
// in the same way and be showing the same thing. This means that the data input for all of the strings must start at the ARDUINO, not at another string.
// You can build a splitter cable to facilitate this. One partial motivation for this decision was the ability to keep some of the status 
// displays working should one of them take damage during match play. Only one output is provided, which also means that all strings are assumed to have the
// same number of pixels.
// 
// -------------- Details & Settings --------------
// Change things in this section of #define's to customize how this code operates
//
// Each character group below contains 10 characters with each character mapping to a particular color for the associated status display section
// Input chars are defined as follows:
// 0-9 control the first status display section
// a-j control the second status display section
// A-J control the third status display section
// n-w control the fourth status display section
// N-W control the fifth status display section
// Z returns the whole display to the IDLESHOW chosen below
// All other serial inputs have no status display effect, though they will cause the IDLESHOW to cease

// The color to character mappings are as follows and can be changed using the COLORx defines
// Colors are RGB Hex definitions similar to those used in HTML design, they must begin '0x' as seen below
// 0,a,A,n,N White
#define COLOR0 0xFFFFFF
// 1,b,B,o,O Red
#define COLOR1 0xFF0000
// 2,c,C,p,P Green
#define COLOR2 0x00FF00
// 3,d,D,q,Q Blue
#define COLOR3 0x0000FF
// 4,e,E,r,R Cyan
#define COLOR4 0x00FFFF
// 5,f,F,s,S Magenta
#define COLOR5 0xFF00FF
// 6,g,G,t,T Yellow
#define COLOR6 0xFFFF00
// 7,h,H,u,U 50% White (mid brightness White)
#define COLOR7 0x888888
// 8,i,I,v,V 25% White (dim brightness White)
#define COLOR8 0x444444
// 9,j,J,w,W Black
#define COLOR9 0x000000
// Z         Run the IDLESHOW chosen below


// STARTWITHSHOW defines what should happen upon Arduino boot? 
// 0=Nothing (except optional divider display)
// 1=Run chosen IDLESHOW
// 2=Light all LEDs to STARTCOLOR
#define STARTWITHSHOW 1
#define STARTCOLOR 0x00FFFF // Color to have all LEDs lit to upon boot (if set in STARTWITHSHOW) (default: Cyan)

#define BRIGHTNESS 50 // Global brightness percentage (a lower the value is better on your battery but potentially harder to see)
#define SECTIONS 3 // How many status display sections will be shown when not in initial display mode

#define DIVIDERS 1 // Have white divider LEDs between status display sections? 1=yes, 0=no
#define DIVIDERCOLOR 0x888888 // Color of the divider LEDs (default: mid brightness white)


// Uncomment ONE, and only ONE, of the following two define lines (UART or I2C) to specify the Arduino serial input to be used
// If both are uncommented, no testing has been performed and bugs are more likely, though it may work fine
#define UART
#define I2C

// Set parameters for the input types here as needed
#define UART_SPEED 115200   // Ignored if UART input isn't used
#define I2C_SLAVE_ADDR 0x10 // Ignored if I2C input isn't used

// Tell me about your WS2812 light strip
#define NPIXEL 40  // How many pixels on the LED strip
#define LED_PIN 6  // What Arduino pin is connected to the data line of the LED strip?
                   // This is always Pin 6 if using the LED header of the REX Robotics RioDuino.

// Choose which Idle time LED show/effect is used
// This can be run in two possible ways: At Arduino boot (see STARTWITHSHOW), or by passing the character 'Z' into the input
// 0=None, all LEDs are off except for (optional) section dividers. Can be used by way of 'Z' char to reset all sections to black.  
// 1=Cylon Eye effect going up and down the string
// 2=Cylon Eye effect going only one direction, then wrapping back to the start of the string
// 3=Breathing effect using brightness increase/decrease and a single color
#define IDLESHOW 1


// Things that control the look of the Cylon Eye effects (ignored if Cylon effects not used)
#define CYLONCOLOR1 0xFFFF00 // Center pixel color
#define CYLONCOLOR2 0x444400 // 2 pixels each side of center (usually just a dimmer version of the center color)
#define CYLONCOLOR3 0x0a0a00 // Background pixels (all pixels not the "eye"
#define CYLONSPEED 50 // Inverse speed, lower = faster (actually a loop delay in millisec)

// Things that control the look of the Breathing effect (ignored if breathing effect not used)
#define BREATHCOLOR 0xFFFF00
#define BREATHSPEED 18 // Inverse speed, lower = faster (actually a loop delay in millisec, note that this delay is executed 5 times between brightness changes)
#define BREATHLOWTIME 1500 // How long (millisec) should the LEDs remain at min brightness? Set to same value as BREATHSPEED to get a sinusoidial pattern.
#define BREATHMAXBRIGHTNESS 128 // How bright should the LEDs get? Set to a value between 4 and 255 (2% to 100% brightness)


// -------------- End of Settings --------------


/////////////////////////////////////////////////////////////////////////////////////
//////////////////                                                 //////////////////
////////////////// Things below here shouldn't need to be modified //////////////////
////////////////// If you make modifications, please share them    //////////////////
//////////////////                                                 //////////////////
/////////////////////////////////////////////////////////////////////////////////////


#include <Adafruit_NeoPixel.h>
#ifdef I2C
#include <Wire.h>
#endif

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NPIXEL, LED_PIN, NEO_GRB + NEO_KHZ800); //initializes the strip, as seen in example code
boolean dropInitDisplay=0;
unsigned int storedValues[3] = {}; //stores values from communcation protocol
//unsigned int storeRGB[3] = {}; //values stored in this array by function getRGB(), order R, G, B
//4 strips, one on each side of robot. Strip does not need to be specified, all strips show the same output
//Section needs to be specified on each strip, section is an arbitrary number which forty is a multiple of, this should be done in the setSectionColor() function already
void setSectionColor(unsigned int, uint32_t); 
void updateEffect(unsigned int section, unsigned int effect, uint32_t color); //where color is a result of getRGB

void setup() {
#ifdef UART
  Serial.begin(UART_SPEED);
#endif
#ifdef I2C
  Wire.begin(I2C_SLAVE_ADDR);
  Wire.onReceive(receiveEvent); // register event
#endif
  pinMode(13,OUTPUT);
  strip.begin();
  strip.setBrightness(255*(BRIGHTNESS/100.001));
  strip.show();
#if STARTWITHSHOW == 1
  doInitDisplay();
#elif STARTWITHSHOW == 2
  doInitColor();
#else
  initDividers();
#endif
}

boolean commsProtocol(byte x) //returns boolean, so an if could check whether or not the storedValues array has the values to process
{
	static word buffer;
	static word process; //figure out how to do conversion from byte data type to int data type
	static int bufferSubscript = 0;
	if (x == 255){
		//move everything from buffer array to process array and wipe the buffer
		for (int z = 0; z < 2; z++){
			process = buffer;			
			buffer &= 0b0000000000000000; //this line wipes the buffer
		}
		//process the bytes into readable values here
		int copy = process;
		storedValues[0] = (copy>>8) & 0b0000000000000111; //8 possible values for section
		storedValues[1] = (process<<1)>>12; // shifts so the four split can be read, eliminates reserve char
		storedValues[2] = process & 0b0000000011111111; //makes color value readable
		return true;
	}
	else {
		buffer = (buffer<<8) | x; //shift left 8, the OR over x;
		return false;
	}
}

uint32_t getRGB(unsigned int RGB) //where x is the color val, function converts single value to RGB, three values, maps to outside of color wheel
{ //wrote more effecient version that travels in a "color triangle" but still gets an accurate color in
	if (0<=RGB && RGB<=42){
		//storeRGB[0] = 255;
		//storeRGB[1] = 6*x;
		//storeRGB[2] = 0;
                return strip.Color(255,6*RGB,0);
	}
	else if (42<RGB && RGB<=85){
		//storeRGB[0] = 255-6*(x-42);
		//storeRGB[1] = 255;
		//storeRGB[2] = 0;
                //return ((255-6*RGB*(RGB-42)<<24|255<<16|0<<8|255));
                return strip.Color(255-6*(RGB-42), 255,0);
	}
	else if (85<RGB && RGB<=127){
		//storeRGB[0] = 0;
		//storeRGB[1] = 255;
		//storeRGB[2] = 6*(x-85);
                //return (0<<24|255<<16|(6*(RGB-85))<<8|255);
                return strip.Color(0, 255, 6*(RGB-85));
	}
	else if (127<RGB && RGB<=170){
		//storeRGB[0] = 0;
		//storeRGB[1] = 255-6*(x-127);
		//storeRGB[2] = 255;
                //return (0<<24|255-6*(RGB-127)<<16|255<<8|255);
                return strip.Color(0, 255-6*(RGB-127), 255);
        }
	else if (170<RGB && RGB<=212){
		//storeRGB[0] = 6*(x-170);
		//storeRGB[1] = 0;
		//storeRGB[2] = 255;
                //return (6*(RGB-170)<<24|0<<16|255<<8|255);
                return strip.Color(6*(RGB-170), 0, 255);
	}
	else if (212<RGB && RGB<=254){
		//storeRGB[0] = 255;
		//storeRGB[1] = 0;
		//storeRGB[2] = 255-6*(x-212);
                //return (255<<24|0<<16|255-6*(RGB-212)<<8|255);
                return strip.Color(255, 0, 255-6*(RGB-212));
	}
	else{
		//storeRGB[0] = 0;
		//storeRGB[1] = 0;
		//storeRGB[2] = 0;
                //return (0<<24|0<<16|0<<8|0);
                return strip.Color(0, 0, 0);
		//this is incase of impossible error, sets to off/black, but error should never be reached because of inability to exceed 255 in 8 bits
                //also, forcing an error by passing the function a number greater than 254 will set the color to black, or off.
	}
}

unsigned long lastMillis = millis(); //necessary for flashing functions
void loop() {
#ifdef UART  
  // see if there's incoming serial data:
  if (Serial.available() > 0) {
    // read the oldest byte in the serial buffer:
    byte incomingByte = Serial.read();
    if(commsProtocol(incomingByte)){ //commsProtocol returns bool, so if a message is received, then do stuff
  //    paramEval(storedValues[0], storedValues[1], storedValues[2]); //obsolete function
      
      
    }
  }
#endif
//  colorWipe(getRGB(41));
  updateEffect(storedValues[0], storedValues[1], getRGB(storedValues[2]));
  delay(10);
  strip.show();
}

void colorWipe(uint32_t color)//wipes strip to one color, where color is the result of getRGB()
{
  for (int z = 0; z < strip.numPixels(); z++){
    strip.setPixelColor(z, color);
  } 
  strip.show(); //updates strip
}



//Function to set effect, should be called after everything else in paramEval because it contains overrides of color and section values depending on effect
void updateEffect(unsigned int section, unsigned int effect, uint32_t color) //where color is a result of getRGB()
{
          static boolean toggle;
          unsigned long temp = millis();
          int sectionLength=(NPIXEL / SECTIONS);
          int sectionStart=sectionLength*section; //cp'd setSectionColor() code in the event that a sweep should only effect a single section
          int sectionEnd=(sectionLength*(section+1))-DIVIDERS;
          switch(effect){
                case 0: //slow flash
                        if ((temp-lastMillis) > 500){
                          lastMillis = temp;
                          if (toggle){
                           toggle = false;
                           setSectionColor(section, getRGB(256)); //256 forces error in getRGB, making getRGB return 0 0 0, which turns LEDs off 
                          }
                         else {
                          toggle = true;
                          setSectionColor(section, color);//because flashing only targets first section
                         }
                        }
			break; //so only sets section and color
		case 1: //fast flash, critical error
                        if ((temp-lastMillis) > 166){
                          lastMillis = temp;
                          if (toggle){
                           toggle = false;
                           setSectionColor(section, getRGB(256)); //256 forces error in getRGB, making getRGB return 0 0 0, which turns LEDs off 
                          }
                         else {
                          toggle = true;
                          setSectionColor(section, color);//because flashing only targets first section
                         }
                        }
			break;
		case 2: //set specified section to solid color
                        setSectionColor(section, color);
			break;
		case 3: //set entire strip to solid color
			colorWipe(color);
			break;
		case 4:
                        for(uint16_t i=sectionStart; i<sectionEnd;) { //sets every pixel in section to color
                          strip.setPixelColor(i, color);
                          if (i>0)
                            strip.setPixelColor(i-1, getRGB(256)); //returns off previous pixel, except on first pass
                          if (temp-lastMillis > 100){
                            lastMillis = temp;
                            i++; //only increases i if 100 miliseconds has passed
                          }
                        }
                        //every tenth of a second, turn previous LED off and next LED on
			break;
                case 5:
                        //full strip sweep
                        for(int i=0; i < strip.numPixels();){
                          strip.setPixelColor(i, color);
                          if(i>0)
                            strip.setPixelColor(i-1, getRGB(256));
                          if (temp-lastMillis > 100){ //turns of prev pix
                           lastMillis = temp;
                           i++;
                          }
                        }
                        break;
		case 9:
			//sleep breathing (fade in and out, use transparency)
			break;
		case 10:
			//Single Direction/FollowingZip
			break;
		case 11:
			//Theatre Chase
			break;
		case 12:
			//Rainbow Sweep
			break;
		case 13:
			//Sleep breathing
			break;
		case 14:
			//sound Meter
			break;
		case 15:
			//Cylon eye
			break;
		default:
			break;
	}
}


//This function will never be called
// This function always processes input for 5 input ranges/display sections
// MAC: low priority: Look at pre-compile directives to only run the code for the defined number of sections
void paramEval(unsigned int section, unsigned int effect, uint32_t color) {
  // Figure out which color should be assigned and set the designated section to that color
  //determine effect after section and color assignment because of the black or white effects that will be required due to lack of three byte RGB value
  switch(section) {
  case 0:
    setSectionColor(section, color);
    break;
  case 1: 
    setSectionColor(section, color);
    break;
  case 2: 
    setSectionColor(section, color);
    break;
  case 3: 
    setSectionColor(section, color);
    break;
  case 4: 
    setSectionColor(section, color);
    break;
  case 5: 
    setSectionColor(section, color);
    break;
  case 6: 
    setSectionColor(section, color);
    break;
  case 7: 
    setSectionColor(section, color);
    break;
  case 8: 
    setSectionColor(section, color);
    break;
  case 9: 
    setSectionColor(section, color);
    break;
  case 12: //what is this case for
    if (section == 4) { // This corresponds to a 'Z' character being received
      dropInitDisplay=0;
      doInitDisplay();
    }
    break;
  default:
	break;
  }
}

void setSectionColor(unsigned int section, uint32_t color) { //defin
  if (section < SECTIONS) { // Don't do anything if we're told to affect a section that shouldn't exist
    int sectionLength=(NPIXEL / SECTIONS);
    int sectionStart=sectionLength*section;
    int sectionEnd=(sectionLength*(section+1))-DIVIDERS;
    for(uint16_t i=sectionStart; i<sectionEnd; i++) { //sets every pixel in section to color
      strip.setPixelColor(i, color);
    }
  }
}

void initDividers() {
  for(int i=0; i<NPIXEL; i++) {
    strip.setPixelColor(i, 0x000000); //Clears the strand to Black
  }
#if DIVIDERS > 0
  int sectionLength=(NPIXEL / SECTIONS);
  for(uint16_t i=0; i<SECTIONS; i++) {
    int dividerLoc=(sectionLength*(i+1))-1;
    strip.setPixelColor(dividerLoc, DIVIDERCOLOR);
  }
#endif
}

////////////////////////////////////////////////
///
/// Below are functions that only get compiled in if certain settings are specified, in some cases 
///
////////////////////////////////////////////////

#ifdef I2C
// function that executes whenever data is received from I2C master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
//  Serial.println("Got I2C Data");
  if (! dropInitDisplay) {
    initDividers();
  }
  dropInitDisplay=1;
  while(Wire.available()) // loop through all but the last
  {
    byte incomingByte = Wire.read(); // receive byte as a character
//    Serial.println("Got I2C Byte");
    if(commsProtocol(incomingByte) == true){
/*      Serial.print("Setting: ");
      Serial.print(storedValues[0]);
      Serial.print(" ");
      Serial.print(storedValues[1]);
      Serial.print(" ");
      Serial.println(storedValues[2]); */
      paramEval(storedValues[0], storedValues[1], storedValues[2]);
    }
  }
}
#endif

// Funcion to wipe whole string to a specified color
#if STARTWITHSHOW == 2
void doInitColor() {
  for(int i=0; i<NPIXEL; i++) { // Sets whole LED strand to specified color
    strip.setPixelColor(i, STARTCOLOR);
  }
  strip.show();
  while(true) {
#ifdef I2C
    if (dropInitDisplay) {
      return;
    }
#endif
#ifdef UART
    if (Serial.available() > 0) {
      dropInitDisplay=1;
      initDividers();
      return;
    }   
#endif
    delay(50);
  }
}
#endif // STARTWITHSHOW


// Compile in the right doInitDisplay function based on the setting of IDLESHOW as follows
// 0=No effect/show, just init dividers and return
// 1=Cylon Eye effect going up and down the string
// 2=Cylon Eye effect going only one direction, then wrapping back to the start of the string
// 3=Breathing effect using brightness increase/decrease and a single color

#if IDLESHOW == 0

// This is the code called when no Idle show/effect is desired
/*void doInitDisplay() {
  if (dropInitDisplay) { 
    return; 
  }
  initDividers();
} */
void doInitDisplay() {
  boolean pinFlash;
  while(true) {
      delay(50);
      if (pinFlash) {
        digitalWrite(13,0);
        pinFlash=false;
      }
      else {
        digitalWrite(13,1);
        pinFlash=true;
      }
    }
}

// End No effect/show

#elif IDLESHOW == 1

// This is the code for the bi-directional Cylon Eye effect
// The eye runs from one end of the LED string to the other end, the runs in the opposite direction back to the starting end
// Rinse and repeat
// ... Think "Night Rider"
void doInitDisplay() {
  boolean pinFlash;
  if (dropInitDisplay) { 
    return; 
  }
  for(int i=0; i<NPIXEL; i++) {
    strip.setPixelColor(i, CYLONCOLOR3); //Clears the dots after the 3rd color
  }
  while(true) {
    for(int i=0; i<NPIXEL; i++) {
#ifdef I2C
      if (dropInitDisplay) {
        return;
      }
#endif
#ifdef UART
      if (Serial.available() > 0) {
        dropInitDisplay=1;
        initDividers();
        return;
      }   
#endif
      strip.setPixelColor(i+2, CYLONCOLOR2); 
      strip.setPixelColor(i+1, CYLONCOLOR2); //Second Dot Color
      strip.setPixelColor(i,   CYLONCOLOR1); //Center Dot Color
      strip.setPixelColor(i-1, CYLONCOLOR2); //Second Dot Color
      strip.setPixelColor(i-2, CYLONCOLOR2); 
      strip.setPixelColor(i-3, CYLONCOLOR3); //Clears the dots to BG color
      strip.show();
      delay(CYLONSPEED);
    }
    for(int i=NPIXEL-1; i>0; i--) {
#ifdef I2C
      if (dropInitDisplay) { // When using I2C, the receiveEvent function tells us when to drop out
        return;
      }
#endif
#ifdef UART
      if (Serial.available() > 0) { // When using UART, check for input and drop out if any found
        dropInitDisplay=1; // Remember that we did this
        initDividers();
        return;
      }
#endif
      strip.setPixelColor(i-2, CYLONCOLOR2); 
      strip.setPixelColor(i-1, CYLONCOLOR2); //Second Dot Color
      strip.setPixelColor(i,   CYLONCOLOR1); //Center Dot Color
      strip.setPixelColor(i+1, CYLONCOLOR2); //Second Dot Color
      strip.setPixelColor(i+2, CYLONCOLOR2); 
      strip.setPixelColor(i+3, CYLONCOLOR3); //Clears the dots to the BG color
      strip.show();
      delay(CYLONSPEED);
      if (pinFlash) {
        digitalWrite(13,0);
        pinFlash=false;
      }
      else {
        digitalWrite(13,1);
        pinFlash=true;
      }
    }
  }
}

// End bi-directional Cylon Eye effect

#elif IDLESHOW == 2

// This is the code for the looping Cylon Eye effect
// The eye runs from one end of the LED string to the other end
// Once there it keeps going in the same direction, but the eye "wraps" back to the beginning end of the string
// Rinse and repeat
// ... If LED strip(s) are set on a single plane wrapping all the way around the robot, this produces an eye(s) that go all the way around the robot
// ... Keep in mind that there will be as many eyes as you have non-daisy chained LED strips
void doInitDisplay() {
  if (dropInitDisplay) { 
    return; 
  }
  for(int i=0; i<NPIXEL; i++) {
    strip.setPixelColor(i, CYLONCOLOR3); //Clears the dots after the 3rd color
  }
  while(true) {
    for(int i=3; i<NPIXEL+3; i++) {
      for(int f,j=-2; j<3; j++){
        if ((i+j) >= NPIXEL) {
          f=j-NPIXEL;
        } 
        else {
          f=j;
        }
#ifdef I2C
        if (dropInitDisplay) { // When using I2C, the receiveEvent function tells us when to drop out
          return;
        }
#endif
        strip.setPixelColor(i+f, CYLONCOLOR2); //Second Dot Color
      }
      if (i >= NPIXEL) {
        strip.setPixelColor(i-NPIXEL, CYLONCOLOR1); //Center Dot Color
      } 
      else {
        strip.setPixelColor(i, CYLONCOLOR1); //Center Dot Color
      }
      strip.setPixelColor(i-3, CYLONCOLOR3); //Clears the dots after the 3rd color
      strip.show();
      delay(CYLONSPEED);
#ifdef I2C
      if (dropInitDisplay) { // When using I2C, the receiveEvent function tells us when to drop out
        return;
      }
#endif
#ifdef UART
      if (Serial.available() > 0) { // When using UART, check for input and drop out if any found
        dropInitDisplay=1; // Remember that we did this
        initDividers();
        return;
      }
#endif
    }
  }
}

// End looping Cylon Eye effect

#elif IDLESHOW == 3

// This is the code for the breathing effect
// The LEDs are all set to half brightness and all the same color
// The LEDs are slowly dimmed and brightened 
// ... You get a gentle "breathing/snoring" effect similar to that sometimes seen on an LED in the sleep mode of a computer
void doInitDisplay() {
  if (dropInitDisplay) { 
    return; 
  }
  int brightness=64;
  int changeFactor=1;

  strip.setBrightness(brightness);
  for(int i=0; i<NPIXEL; i++) {
    if (dropInitDisplay) { 
      return; 
    }
    strip.setPixelColor(i, BREATHCOLOR);
  }

  while(true) {
    strip.show();
#ifdef I2C
    if (dropInitDisplay) {
      strip.setBrightness(255*(BRIGHTNESS/100.001));
      return;
    }
#endif
#ifdef UART
    if (Serial.available() > 0) {
      dropInitDisplay=1;
      initDividers();
      strip.setBrightness(255*(BRIGHTNESS/100.001));
      return;
    }   
#endif
    delay(BREATHSPEED);
    brightness=brightness+changeFactor;
    if (brightness > BREATHMAXBRIGHTNESS) {
      changeFactor *= -1;
      brightness=brightness+changeFactor;
    }
    if (brightness <= 1) {
      changeFactor *= -1;
      brightness=brightness+changeFactor;
      strip.setBrightness(1);
      strip.show();
      for (int j=0; j < BREATHLOWTIME; j+=20) {
        delay(20);
#ifdef I2C
        if (dropInitDisplay) {
          strip.setBrightness(255*(BRIGHTNESS/100.001));
          return;
        }
#endif
#ifdef UART
        if (Serial.available() > 0) {
          dropInitDisplay=1;
          initDividers();
          strip.setBrightness(255*(BRIGHTNESS/100.001));
          return;
        }   
#endif
      }
    }
    strip.setBrightness(brightness);
  }
}

// End breathing effect

#endif




