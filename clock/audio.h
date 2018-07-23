/*
 * all of the audio code for the Music Maker Feather Wing
 */

#if(USE_AUDIO)

#include <SPI.h>
#include <SD.h>
#include <Adafruit_VS1053.h>

// These are the pins used on feather M0
#define VS1053_RESET   -1     // VS1053 reset pin (not used!)
#define VS1053_CS       6     // VS1053 chip select pin (output)
#define VS1053_DCS     10     // VS1053 Data/command select pin (output)
#define CARDCS          5     // Card chip select pin
#define VS1053_DREQ     9     // VS1053 Data request, ideally an Interrupt pin

Adafruit_VS1053_FilePlayer musicPlayer = 
  Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS);

#if(MY_DEBUG)
  #define DEBUG(s) Serial.print(s);
  #define MESSAGE(a,t) Serial.print(t);SpeakMessage(a);
#else
  #define MESSAGE(a,t) SpeakMessage(a);
  #define DEBUG(s) 
#endif

uint8_t Command; 

void SpeakMessage(const char* s) {
  BLE_DISABLE;
  musicPlayer.startPlayingFile(s);
  while(! musicPlayer.stopped()) {
    Cooperate();
  };
  BLE_ENABLE;
}
//Prototype needed by audio_menu.h
void SpeakNumber(uint8_t N,uint8_t LeadingZero); 
bool SpeakHours(uint8_t Hrs, bool Flag);
void SpeakAMPM(uint8_t Hrs);

//configuration talking menu code
#include  "audio_menu.h"

//initializes Music Maker Wing
void Audio_Setup(void) {
  if (! musicPlayer.begin()) { // initialise the music player
    DEBUG("Couldn't find VS1053, do you have the right pins defined?");
    while (1);
  }
  musicPlayer.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working
  delay(1000);
  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
  if (!SD.begin(CARDCS)) {
    DEBUG("SD card not detected.");
    while (1);  // don't do anything more
  }
  musicPlayer.setVolume(AudioDaytime,AudioDaytime);
  MESSAGE("init.mp3", "System Initialized\n");
}

//Speaks 20, 30, 40, or 50
void  SpeakDecade(uint8_t D) {
  switch(D) {
    case 2: MESSAGE("num_20.mp3", "2"); break;
    case 3: MESSAGE("num_30.mp3", "3"); break;
    case 4: MESSAGE("num_40.mp3", "4"); break;
    case 5: MESSAGE("num_50.mp3", "5"); break;
  }  
}

//Speaks numbers from 0 through 59. Flag determines if leading zero
// is spoken.
void SpeakNumber(uint8_t N,uint8_t LeadingZero= false) {
  switch(N) {
    case  0: 
      if(LeadingZero){
        MESSAGE("num_00.mp3", "00");
      } else {
        MESSAGE("num_0.mp3", "0");
      }
      break;
    case  1: 
      if(LeadingZero){
        MESSAGE("num_01.mp3", "01");
      } else {
        MESSAGE("num_1.mp3", "1");
      }
      break;
    case  2: 
      if(LeadingZero){
        MESSAGE("num_02.mp3", "02");
      } else {
        MESSAGE("num_2.mp3", "2");
      }
      break;
    case  3: 
      if(LeadingZero){
        MESSAGE("num_03.mp3", "03");
      } else {
        MESSAGE("num_3.mp3", "3");
      }
      break;
    case  4: 
      if(LeadingZero){
        MESSAGE("num_04.mp3", "04");
      } else {
        MESSAGE("num_4.mp3", "4");
      }
      break;
    case  5: 
      if(LeadingZero){
        MESSAGE("num_05.mp3", "05");
      } else {
        MESSAGE("num_5.mp3", "5");
      }
      break;
    case  6: 
      if(LeadingZero){
        MESSAGE("num_06.mp3", "06");
      } else {
        MESSAGE("num_6.mp3", "6");
      }
      break;
    case  7: 
      if(LeadingZero){
        MESSAGE("num_07.mp3", "07");
      } else {
        MESSAGE("num_7.mp3", "7");
      }
      break;
    case  8: 
      if(LeadingZero){
        MESSAGE("num_08.mp3", "08");
      } else {
        MESSAGE("num_8.mp3", "8");
      }
      break;
    case  9: 
      if(LeadingZero){
        MESSAGE("num_09.mp3", "09");
      } else {
        MESSAGE("num_9.mp3", "9");
      }
      break;
    case 10: MESSAGE("num_10.mp3", "10"); break;
    case 11: MESSAGE("num_11.mp3", "11"); break;
    case 12: MESSAGE("num_12.mp3", "12"); break;
    case 13: MESSAGE("num_13.mp3", "13"); break;
    case 14: MESSAGE("num_14.mp3", "14"); break;
    case 15: MESSAGE("num_15.mp3", "15"); break;
    case 16: MESSAGE("num_16.mp3", "16"); break;
    case 17: MESSAGE("num_17.mp3", "17"); break;
    case 18: MESSAGE("num_18.mp3", "18"); break;
    case 19: MESSAGE("num_19.mp3", "19"); break;
    default: //anything greater than 19
      SpeakDecade(N/10);
      if(N % 10) {
        SpeakNumber(N % 10);
      } else {
        DEBUG("0");
      }
  }
}

//Speaks the hours including midnight or noon
//returns false if it was midnight or noon which means don't say a.m. or p.m.
bool SpeakHours(uint8_t Hrs, bool Flag) {
  if(Flag) {
    if(Hrs==0) {
      MESSAGE("12mid.mp3", "12 midnight\n");
      return false;
    } else {
      if(Hrs==12) {
        MESSAGE("12noon.mp3", "12 noon\n");
        return false;
      }
    }
  }
  SpeakNumber( ((Hrs % 12)==0)?12: Hrs % 12);
  return true;
}

void SpeakAMPM(uint8_t Hrs) {
  if(Hrs<12) {
    MESSAGE("am.mp3", " a.m.\n");
  } else {
    MESSAGE("pm.mp3", " p.m.\n");
  }
}

//Speaks the hours and minutes passed to it. Uses parameters because
// when selecting alarms you aren't necessarily speaking the current time.
void SpeakTime(uint8_t Hrs,uint8_t Mins) {
  MESSAGE("current.mp3", "The current time is ");
  bool Not12=SpeakHours(Hrs,Mins==0);
  if(Mins) {
    DEBUG(":");
    SpeakNumber(Mins, true);
    SpeakAMPM(Hrs);
  } else {
    if(Not12) {
      SpeakAMPM(Hrs);
    }
  }
}

//Plays a music clip based on the current hour either sequentially or randomly
//Is invoked at the top of the hour and anytime you get the "Play" button.
void do_Music(void) {
  uint8_t H= Hours% 12;
  if(Music_State==MODE_OFF) return;
  if(Music_State==MODE_RANDOM) H=random(12);
  DEBUG("Playing music:"); DEBUG(H); 
  switch(H) {
    case  1: MESSAGE("music01.mp3","\n");break;
    case  2: MESSAGE("music02.mp3","\n");break;
    case  3: MESSAGE("music03.mp3","\n");break;
    case  4: MESSAGE("music04.mp3","\n");break;
    case  5: MESSAGE("music05.mp3","\n");break;
    case  6: MESSAGE("music08.mp3","\n");break;
    case  7: MESSAGE("music07.mp3","\n");break;
    case  8: MESSAGE("music08.mp3","\n");break;
    case  9: MESSAGE("music09.mp3","\n");break;
    case 10: MESSAGE("music10.mp3","\n");break;
    case 11: MESSAGE("music11.mp3","\n");break;
    case  0: MESSAGE("music12.mp3","\n");break;
  }
}

//Handles all audio that is time based such as chimes, quarterly or hourly voice etc.
//Only called when seconds==0
void do_Audio(void) {
  if((Audio_State==MODE_OFF) || Doing_Configuration) return;
  uint8_t Count;
  #if(MY_DEBUG && USE_PHOTOCELL)
    if((Minutes % 15)==0) {
      Serial.print(analogRead(PHOTOCELL_PIN),DEC);Serial.print("\t");
    }
  #endif
  if((Hours<Day_Begins_Hour)  || (Hours>Night_Begins_Hour) && (Audio_State==MODE_TIMED)) {//Nighttime
    if(AudioNighttime>=VOLUME_THRESHOLD) {//Note lower numbers mean higher volume
      //essentially muted so do nothing
      return;
    }
    musicPlayer.setVolume(AudioNighttime,AudioNighttime);
  } else {
    #if(USE_PHOTOCELL)
      if(Audio_State== MODE_LIGHT) {
        uint16_t Light= analogRead(PHOTOCELL_PIN);
        #if(MY_DEBUG)
          Serial.print("Photocell value:"); Serial.println(Light,DEC);
        #endif
        if (Light<LIGHT_THRESHOLD) {  //nighttime
          if(AudioNighttime<VOLUME_THRESHOLD) {//essentially muted so do nothing
            return;
          }
          musicPlayer.setVolume(AudioNighttime,AudioNighttime);
        } 
      } else {  //Play normally
        musicPlayer.setVolume(AudioDaytime,AudioDaytime);
      }
    #else
      musicPlayer.setVolume(AudioDaytime,AudioDaytime);
    #endif
  }
  if(Quarterly_Enable && (Chimes_State==MODE_WESTMIN)) {
    switch(Minutes) {
      case 15: MESSAGE("west1q.mp3", "Quarter after the hour\n"); break;
      case 30: MESSAGE("west2q.mp3", "Half past the hour\n"); break;
      case 45: MESSAGE("west3q.mp3", "Three quarter after the hour\n"); break;
      case 00: MESSAGE("west4q.mp3", "Top of the hour\n"); 
    }
  }
  if(Chimes_State && Hourly_Enable && (Minutes==0)) {
    Count=Hours%12;
    if (Count==0) Count=12;
    if(Chimes_State==MODE_WESTMIN) {
      Count--;//Final gong is different so only do the first Count-1
      for(uint8_t i=0; i< Count;i++) {
        MESSAGE("wgong_1.mp3", "Gong, ");
      }
      MESSAGE("wgong_x.mp3", "Final Gong.\n");
    } else {//Must be cuckoo clock
      for(uint8_t i=0; i< Count;i++) {
        MESSAGE("cuckoo1.mp3", "Cuckoo ");
      }
      DEBUG (".\n");      
    }
  }
  if(Voice_Enable) {
    switch(Minutes) {
      case 15:
      case 30:
      case 45:  if(Quarterly_Enable) SpeakTime(Hours, Minutes);break;
      case 00:  if(Quarterly_Enable || Hourly_Enable){
                  SpeakTime(Hours,Minutes);
                };
                break;
    }
  }
  if(Alarm_Enable) {
    if((Hours== Alarm_Hour) && (Minutes== Alarm_Minutes)) {
      MESSAGE("alarm.mp3", "ALARM!\n");
    }
  }
}

#else  // Don't USE_AUDIO
  //Dummy routines used if audio is not implemented
  void Audio_Setup(void) {};
  void do_Music(void) {};
  void do_Audio (void) {};
  void do_Configuration (void) {};
  void SpeakNumber(uint8_t N,uint8_t LeadingZero){}; 
  bool SpeakHours(uint8_t Hrs, bool Flag){};
  void SpeakAMPM(uint8_t Hrs){};
  void SpeakTime(uint8_t Hrs,uint8_t Mins) {};  
  #if(MY_DEBUG)
    #define DEBUG(s) Serial.print(s);
    #define MESSAGE(a,t) Serial.print(t);
  #else
    #define MESSAGE(a,t) 
    #define DEBUG(s) 
  #endif
#endif


