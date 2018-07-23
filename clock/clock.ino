/*
 * Talking Musical Neo-Pixel Clock with Infrared, BLE, and Touch Controls
 *    by Chris Young
 *  released to the public domain
 */
//Various options. Set to zero to disable or one to enable.
//Enables Serial Monitor Debugging
#define MY_DEBUG 0

//Enables Bluetooth controls must use Feather M0 BLA if enabled.
//If disabled, can use Feather M0 basic or M0 express
#define USE_BLE 1

//Enables IR controls. Must connect IR receiver to a digital input pin
#define USE_IR 1

//Enables audio output. Requires Music Maker Feather Wing
#define USE_AUDIO 1

//Enables photocell
#define USE_PHOTOCELL 1

//Enables Touch Controls
#define USE_TOUCH 1

//Pin numbers
#define BLUEFRUIT_SPI_CS   8
#define BLUEFRUIT_SPI_IRQ  7
#define BLUEFRUIT_SPI_RST  4
#define NEO_PIXEL_PIN     11
#define IR_PIN            12
#define TOUCH_SETUP_PIN   A3
#define TOUCH_RIGHT_PIN   A2
#define TOUCH_UP_PIN      A1
#define TOUCH_DOWN_PIN    A0 
#define PHOTOCELL_PIN     A4

//Audio, Music and Animation modes
#define MODE_OFF      0
#define MODE_ON       1
#define MODE_TIMED    2
#define MODE_LIGHT    3
#define MODE_RANDOM   4
#define MODE_WESTMIN  5
#define MODE_CUCKOO   6

//Initial default settings. These values are set in "audio_menu.h" and other places
// but the defaults are defined here for easy access.
#define AUDIO_DEFAULT       MODE_TIMED
#define ANIMATION_DEFAULT   MODE_RANDOM
#define MUSIC_DEFAULT       MODE_RANDOM
#define CHIMES_DEFAULT      MODE_WESTMIN
#define VOICE_DEFAULT       true
#define QUARTERLY_DEFAULT   true
#define HOURLY_DEFAULT      true
#define MARKS_DEFAULT       true
#define ALARM_DEFAULT       true
#define ALARM_HOUR          16
#define ALARM_MINUTES       30
#define DAYTIME_START_DEFAULT   10
#define NIGHT_BEGINS_DEFAULT    22
#define VOLUME_DAY_DEFAULT      20
#define VOLUME_NIGHT_DEFAULT    40
#define VOLUME_THRESHOLD        50
#define BRIGHT_DAY_DEFAULT       6
#define BRIGHT_NIGHT_DEFAULT     2
#define HOURS_DEFAULT           12
#define MINUTES_DEFAULT          0
#define LIGHT_THRESHOLD        100

//fudge factor in case you don't get your pixel ring oriented properly
#define TOP_PIXEL 3

//real-time clock module for SAMD21
#include <RTCZero.h>
RTCZero rtc;

//DMA version of Neo-Pixel Library
#include <Adafruit_NeoPixel_ZeroDMA.h>
Adafruit_NeoPixel_ZeroDMA strip(60,NEO_PIXEL_PIN, NEO_GRB);

#if(USE_IR)
  //set up IRLib2 to use NEC protocol and Adafruit mini-remote
  #include "IRLibRecvPCI.h"
  #include "IRLibDecodeBase.h"
  #include "IRLib_P01_NEC.h"
  #include "adafruit_mini_codes.h"
  IRrecvPCI myReceiver(IR_PIN); 
  IRdecodeNEC myDecoder;   
#endif

#if(USE_TOUCH)
  #include "Adafruit_FreeTouch.h"
  #define TOUCH_SETUP_THRESHOLD 650
  #define TOUCH_RIGHT_THRESHOLD 550
  #define TOUCH_UP_THRESHOLD    550
  #define TOUCH_DOWN_THRESHOLD  750
  Adafruit_FreeTouch qt_setup = Adafruit_FreeTouch(TOUCH_SETUP_PIN, OVERSAMPLE_4, RESISTOR_50K, FREQ_MODE_NONE);
  Adafruit_FreeTouch qt_right = Adafruit_FreeTouch(TOUCH_RIGHT_PIN, OVERSAMPLE_4, RESISTOR_50K, FREQ_MODE_NONE);
  Adafruit_FreeTouch qt_up    = Adafruit_FreeTouch(TOUCH_UP_PIN,    OVERSAMPLE_4, RESISTOR_50K, FREQ_MODE_NONE);
  Adafruit_FreeTouch qt_down  = Adafruit_FreeTouch(TOUCH_DOWN_PIN,  OVERSAMPLE_4, RESISTOR_50K, FREQ_MODE_NONE);
#endif

//Codes returned by Get_Command()
#define NO_COMMAND  0
#define SETUP       1
#define PLAY        2
#define RESET       3
#define SELECT      4   //Unused as of now
#define UP_ARROW    5
#define DOWN_ARROW  6
#define LEFT_ARROW  7
#define RIGHT_ARROW 8

//prototypes
uint8_t Get_Command(void);
void Cooperate(void);
void updateDisplay(void);

//BLE and Music Maker Wing don't get along very well. You have to disable BLE
//before every use of and reenable it afterwards. These macros make it easy.
#define BLE_DISABLE  digitalWrite(BLUEFRUIT_SPI_CS,HIGH);
#if(USE_BLE)
  #define BLE_ENABLE   digitalWrite(BLUEFRUIT_SPI_CS,LOW);
#else
  #define BLE_ENABLE 
#endif

//Global variables for time and pixel display
int16_t Brightness= BRIGHT_DAY_DEFAULT;
int16_t BrightInverse= 255/BRIGHT_DAY_DEFAULT;
int16_t Bright_Day= BRIGHT_DAY_DEFAULT;
int16_t Bright_Night= BRIGHT_NIGHT_DEFAULT;
uint8_t Day_Begins_Hour= DAYTIME_START_DEFAULT;
uint8_t Night_Begins_Hour =NIGHT_BEGINS_DEFAULT;
uint8_t Marks= MARKS_DEFAULT;
uint8_t Hours=HOURS_DEFAULT;
uint8_t Minutes=MINUTES_DEFAULT;
uint8_t Seconds=0;
uint8_t pSeconds=0;
uint8_t Animation_State= ANIMATION_DEFAULT;

//Include code for audio, BLE and animation. 
//Conditional compiles inside will turn them off as necessary.
#include "audio.h"
#include "BLESetup.h"
#include "animation.h"


//Checks to see if display needs updating and does so if necessary.
//This allows for update in background while other things are going on
void Cooperate(void) {
  if((Seconds=rtc.getSeconds()) != pSeconds){
    pSeconds= Seconds; 
    updateDisplay();
  }
}

//main setup
void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  randomSeed(1234);//Set up random number generator
  #if(MY_DEBUG)
    Serial.begin(9600);
    delay(2000); while (!Serial); 
    Serial.println("Debug output ready.");
  #endif
  rtc.begin(); // initialize RTC
  rtc.setTime(Hours,Minutes,Seconds);
  #if(USE_IR)
    myReceiver.enableIRIn();
  #endif
  pinMode(BLUEFRUIT_SPI_CS, INPUT_PULLUP);
  BLE_DISABLE;
  Audio_Setup();
  BLE_ENABLE;
  #if(USE_BLE)
    if (BluefruitSetup()) {
      MESSAGE("ble_init.mp3", "BLE Initialized.\n");
    } else {
      MESSAGE("ble_fail.mp3", "BLE failed\n");
    }
  #endif
  #if(USE_TOUCH)
    if(!qt_setup.begin()) DEBUG("Failed to begin touch on setup pin.\n");
    if(!qt_right.begin()) DEBUG("Failed to begin touch on right pin.\n");
    if(!qt_up.begin())    DEBUG("Failed to begin touch on up pin.\n");
    if(!qt_down.begin())  DEBUG("Failed to begin touch on down pin.\n");
  #endif
}

//Redraws all of the neo-pixels to show the current time
//Gets called when seconds change or as needed.
void updateDisplay(void) {
  uint8_t i;
  Minutes= rtc.getMinutes();
  Hours= rtc.getHours();
  //Don't mess with brightness during configuration
  if(! Doing_Configuration) {
    if((Hours<Day_Begins_Hour)  || (Hours>Night_Begins_Hour)) {//Nighttime
      Brightness=Bright_Night;
    } else {
      Brightness=Bright_Day;
    }
    BrightInverse= 255/Brightness;
  }
  #if(MY_DEBUG)
    Serial.print("BrightInverse:"); Serial.print(BrightInverse,DEC); 
    Serial.print("  Brightness:"); Serial.println(Brightness,DEC);
  #endif
  strip.clear();    //erase everything
  if (Marks) {      //draw the tick marks every five minutes if enabled
    uint8_t val=(Brightness)?max(Brightness*0.3,1):0;
    for(i=0;i<60;i=i+5) {
      strip.setPixelColor((TOP_PIXEL+i)%60,val,val,val);
    }
  }
  //Use blue for AM and cyan for PM
  //Draw five pixels to designate the hour
  uint32_t temp=strip.Color(0,0,Brightness);
  if(Hours>12) temp= strip.Color(0,Brightness,Brightness);
  for(i=0;i<5;i++) {
    strip.setPixelColor( (TOP_PIXEL+60+(Hours%12)*5+i-2) % 60,temp);
  }
  //Drawn minutes in red and seconds in green
  strip.setPixelColor((TOP_PIXEL+Minutes)%60, Brightness,0,0);
  strip.setPixelColor((TOP_PIXEL+Seconds)%60, 0,Brightness,0);
  strip.show();  
  #if(MY_DEBUG)
    Serial.print("Time="); 
    if(Hours<10) Serial.print("0");
    Serial.print(Hours,DEC); Serial.print(":"); 
    if(Minutes<10) Serial.print("0");
    Serial.print(Minutes,DEC); Serial.print(":"); 
    if(Seconds<10) Serial.print("0");
    Serial.println(Seconds,DEC); 
  #endif
 if (Seconds==0) {
    do_Audio();
    if (Minutes==0) {
      do_Animation();
      do_Music();
    }
  }
}

//Gets a command from either IR, BLE, or touch controls and returns a value
uint8_t Get_Command(void) {
 uint8_t Value=NO_COMMAND;
  #if(USE_IR)
    if (myReceiver.getResults()) {//was a command received by IR?
      if (myDecoder.decode()){    //is that the right protocol?
        switch(myDecoder.value) {
          case ADAF_MINI_UP_ARROW:    Value= UP_ARROW; break; 
          case ADAF_MINI_DOWN_ARROW:  Value= DOWN_ARROW; break; 
          case ADAF_MINI_LEFT_ARROW:  Value= LEFT_ARROW; break; 
          case ADAF_MINI_RIGHT_ARROW: Value= RIGHT_ARROW; break; 
          case ADAF_MINI_ENTER_SAVE:  Value= SELECT; break; 
          case ADAF_MINI_SETUP:       Value= SETUP; break; 
          case ADAF_MINI_PLAY_PAUSE:  Value= PLAY; break; 
          case ADAF_MINI_REPEAT:      Value= RESET; break; 
        }
     }
     #if (MY_DEBUG)
       //myDecoder.dumpResults(false);
     #endif
     myReceiver.enableIRIn(); 
    }
  #endif
  #if (USE_BLE)
    if(Update_BLE_Connected()) {   //see if it's connected
      //See if we got a packet and if it's big enough
      if (readPacket(&ble, BLE_READPACKET_TIMEOUT)>2) {
        if (packetbuffer[1]=='B') {
          uint8_t Button= packetbuffer[2]-'0';
          bool Pressed= packetbuffer[3]-'0';
          #if (MY_DEBUG)
            Serial.print("Button:"); Serial.print(Button,DEC);
            if(Pressed) {
              Serial.println(" pressed.");
            } else {
              Serial.println(" released.");
            }
          #endif
          if(Pressed) {
            return Button;
          }
        }
      }
    }
  #endif
  #if(USE_TOUCH)
    uint16_t T;
    if((T=qt_setup.measure()) > TOUCH_SETUP_THRESHOLD) {
      DEBUG("Setup Pressed:");DEBUG(T);DEBUG("\n");
      Value=SETUP;
    } else if((T=qt_right.measure()) > TOUCH_RIGHT_THRESHOLD) {
      DEBUG("Right Pressed:");DEBUG(T);DEBUG("\n");
      Value=RIGHT_ARROW;
    } else if((T=qt_up.measure()) > TOUCH_UP_THRESHOLD) {
      DEBUG("Up Pressed:");DEBUG(T);DEBUG("\n");
      Value=UP_ARROW;
    } else if((T=qt_down.measure()) > TOUCH_DOWN_THRESHOLD) {
      DEBUG("Down Pressed:");DEBUG(T);DEBUG("\n");
      Value=DOWN_ARROW;
    }
  #endif
  return Value;
}

//main loop gets a command if any man handles it.
void loop() {
  switch(Get_Command()) {
    case RIGHT_ARROW://increase hours
      Hours=(Hours+1)%24;
      rtc.setHours(Hours);
      updateDisplay();
      if (SpeakHours(Hours,true)) {
        SpeakAMPM(Hours);
      }
      break;
    case LEFT_ARROW://Decrease hours
      Hours=(Hours+23)%24;
      rtc.setHours(Hours);
      updateDisplay();
      if (SpeakHours(Hours,true)) {
        SpeakAMPM(Hours);
      }
      break;
    case UP_ARROW://Increase Minutes
      Minutes=(Minutes+1)%60;
      rtc.setMinutes(Minutes);
      updateDisplay();
      SpeakNumber(Minutes,true);
      DEBUG("\n");
      break;
    case DOWN_ARROW://Decrease Minutes
      Minutes=(Minutes+59)%60;
      rtc.setMinutes(Minutes);
      updateDisplay();
      SpeakNumber(Minutes,true);
      DEBUG("\n");
      break;
    case PLAY://speak time
      SpeakTime(Hours, Minutes);
      do_Animation();
      do_Music();
      break;          
    case RESET://reset seconds
      rtc.setSeconds(Seconds=59);
      updateDisplay();
      break;
    case SETUP:
      do_Configuration();
      break;
  }
  //if necessary, update display
  Cooperate();
}


