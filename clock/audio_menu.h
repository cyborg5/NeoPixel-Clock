/*
 * Implements an audio menu system for setting configuration
 */
uint8_t Audio_State= AUDIO_DEFAULT;
uint8_t Chimes_State= CHIMES_DEFAULT;
uint8_t Voice_Enable= VOICE_DEFAULT;
uint8_t Quarterly_Enable= QUARTERLY_DEFAULT;
uint8_t Hourly_Enable= HOURLY_DEFAULT;
uint8_t Alarm_Enable= ALARM_DEFAULT;
uint8_t Alarm_Hour= ALARM_HOUR;
uint8_t Alarm_Minutes= ALARM_MINUTES;
uint8_t Music_State= MUSIC_DEFAULT;
uint8_t AudioDaytime= VOLUME_DAY_DEFAULT;
uint8_t AudioNighttime= VOLUME_NIGHT_DEFAULT;
uint8_t Doing_Configuration= false;

#define MAX_ITEMS 4

//Defines a menu item
typedef const struct {
  const char* audioFile;  //The audio file to play
  const char* audioText;  //Text message for debugging purposes
  const uint8_t value;    //Numeric value associated with this item.
                          //Note: not necessarily the index of the item
} item_t;

//Defines a menu system
typedef const struct{
  const char* titleAudio; //Audio description of this menu
  const char* titleText;  //Text description for debugging
  uint8_t *Variable;      //address of the variable we want to set
  uint8_t nItems;         //the number of options for this menu item
  item_t Items[MAX_ITEMS];//The items in the menu
} audioMenuItem_t;

//Our menu system
const audioMenuItem_t Menus[]={
  {"audio.mp3", "Audio mode is currently ",&Audio_State,4,
    { {"disabled.mp3", "disabled.\n",MODE_OFF},
      {"continuo.mp3", "continuous.\n",MODE_ON},
      {"tbased.mp3", "time based.\n",MODE_TIMED},
      {"lbased.mp3", "light based.\n",MODE_LIGHT}
    }
  },
  {"animate.mp3", "Animation mode is currently ",&Animation_State,3,
    { {"disabled.mp3", "disabled.\n",MODE_OFF},
      {"sequence.mp3", "sequential.\n",MODE_ON},
      {"random.mp3", "random.\n",MODE_RANDOM}
    }
  },
  {"chimes.mp3", "Chimes ",&Chimes_State,3,
    { {"disabled.mp3", "disabled.\n",MODE_OFF},
      {"westmins.mp3", "Westminster chimes.\n",MODE_WESTMIN},
      {"cuckoo.mp3", "Cuckoo clock chimes.\n",MODE_CUCKOO}
    }
  },
  {"hourly.mp3", "Hourly ",&Hourly_Enable,2,
    { {"disabled.mp3", "disabled.\n",false},
      {"enabled.mp3", "enabled.\n",true}
    }
  },
  {"quarter.mp3", "Quarterly ",&Quarterly_Enable,2,
    { {"disabled.mp3", "disabled.\n",false},
      {"enabled.mp3", "enabled.\n",true}
    }
  },
  {"voice.mp3", "Voice ",&Voice_Enable,2,
    { {"disabled.mp3", "disabled.\n",false},
      {"enabled.mp3", "enabled.\n",true}
    }
  },
  {"alarmis.mp3", "Alarm ",&Alarm_Enable,2,
    { {"disabled.mp3", "disabled.\n",false},
      {"enabled.mp3", "enabled.\n",true}
    }
  },
  {"music.mp3", "Music mode is currently ",&Music_State,3,
    { {"disabled.mp3", "disabled.\n",MODE_OFF},
      {"sequence.mp3", "sequential.\n",MODE_ON},
      {"random.mp3", "random.\n",MODE_RANDOM}
    }
  }
};

//Enumerates the menus
#define AUDIO_MENU      0
#define ANIMATION_MENU  1
#define CHIMES_MENU     2
#define HOURLY_MENU     3
#define QUARTERLY_MENU  4
#define VOICE_MENU      5
#define ALARM_MENU      6
#define MUSIC_MENU      7

//Sets a value to one of a number of optional choices
void Configure_Menu_Items(audioMenuItem_t &Menu) {
  uint8_t i;
  uint8_t Index=-1;
  //Menu.Items[i].value could be anything and is not necessarily the index
  //of the item in the list of possible options. So we have to search through list
  //to determine the current item index.
  for (i=0;i<Menu.nItems;i++){
    if(*Menu.Variable==Menu.Items[i].value) {
      Index=i;
      break;
    }
  }
  if(Index<0)  {
    //Value wasn't properly set. Attempt to recover By defaulting to the first item
    MESSAGE("error.mp3","Internal error\n");
    Index=0;
    *Menu.Variable= Menu.Items[Index].value;
  }
  while(true) {
    MESSAGE(Menu.titleAudio,Menu.titleText);
    MESSAGE(Menu.Items[Index].audioFile,Menu.Items[Index].audioText);
    while((Command=Get_Command())==NO_COMMAND) {};
    switch(Command) {
      case UP_ARROW:    Index= (Index+1) % Menu.nItems; break;
      case DOWN_ARROW:  Index= (Index+Menu.nItems-1) % Menu.nItems; break;
      case RIGHT_ARROW: *Menu.Variable= Menu.Items[Index].value;return;
      default: 
        MESSAGE ("invalid.mp3", "\nInvalid keypress\n");
        MESSAGE("choice.mp3", "\nPress arrows to change or advance to next item\n");
    }
  }
}

//Increment or decrement an integer value. Uses a callback function
//because each item requires different visual or auditory feedback.
//Callback function also enforces upper and lower limits or performs wraparound.
void Configure_Integer(void(*Update)(int)) {
  Update(0);//Speak and display the current value
  while(true) {
    while((Command=Get_Command())==NO_COMMAND) {};
    switch(Command) {
      case UP_ARROW:    Update(+1); break;
      case DOWN_ARROW:  Update(-1); break;
      case RIGHT_ARROW: 
        return;
      default: 
        MESSAGE ("invalid.mp3", "\nInvalid keypress\n");
        MESSAGE("choice.mp3", "\nPress arrows to change or advance to next item\n");
    }
  }
}

//Callback function for daytime volume
void Update_Day_Volume(int Amount) {
  AudioDaytime+= Amount;
  AudioDaytime=max(1,min(VOLUME_THRESHOLD,AudioDaytime));
  MESSAGE("daytime.mp3","Daytime ");
  MESSAGE("volume.mp3", "volume:");
  SpeakNumber(AudioDaytime,false);
  DEBUG("\n");
  musicPlayer.setVolume(AudioDaytime,AudioDaytime);
}

//Callback function for nighttime volume
void Update_Night_Volume(int Amount) {
  AudioNighttime+= Amount;
  AudioNighttime=max(1,min(VOLUME_THRESHOLD,AudioNighttime));
  MESSAGE("night.mp3","Nighttime ");
  MESSAGE("volume.mp3", "volume:");
  SpeakNumber(AudioNighttime,false);
  DEBUG("\n");
  musicPlayer.setVolume(AudioNighttime,AudioNighttime);
}

//Callback function for alarm hours
void Update_Alarm_Hour(int Amount) {
  Alarm_Hour=(Alarm_Hour+24+Amount) % 24;
  if(Amount==0)MESSAGE("alarmh.mp3","Alarm hour is ");
  if(SpeakHours(Alarm_Hour,true)) {
    SpeakAMPM(Alarm_Hour);
  }
  DEBUG("\n");
}

//Callback function for alarm minutes
void Update_Alarm_Minutes(int Amount) {
  Alarm_Minutes=(Alarm_Minutes+60+Amount) % 60;
  if(Amount==0)MESSAGE("alarmm.mp3","Alarm minutes is ");
  SpeakNumber(Alarm_Minutes,true);
  DEBUG("\n");
}

//Callback function for daytime brightness
void Update_Day_Bright(int Amount) {
  Bright_Day+= Amount;
  Bright_Day=min(59, max(Bright_Day,0));
  MESSAGE("daytime.mp3","Daytime ");
  MESSAGE("bright.mp3", "brightness:");
  SpeakNumber(Bright_Day,false);
  DEBUG("\n");
  Brightness=Bright_Day;
  updateDisplay();
}

//Callback function for nighttime brightness
void Update_Night_Bright(int Amount) {
  Bright_Night+= Amount;
  Bright_Night=min(59, max(Bright_Night,0));
  MESSAGE("night.mp3","Nighttime ");
  MESSAGE("bright.mp3", "brightness:");
  SpeakNumber(Bright_Night,false);
  DEBUG("\n");
  Brightness=Bright_Night;
  updateDisplay();
}

//Callback function for daytime begins hour
void Update_Day_Begins(int Amount) {
  Day_Begins_Hour=(Day_Begins_Hour+24+Amount) % 24;
  if(Amount==0) {
    MESSAGE("daytime.mp3","Daytime ");
    MESSAGE("begins.mp3","begins at:");
  }
  if(SpeakHours(Day_Begins_Hour,true)) {
    SpeakAMPM(Day_Begins_Hour);
  }
  DEBUG("\n");
}

//Callback function for nighttime begins hour
void Update_Night_Begins(int Amount) {
  Night_Begins_Hour=(Night_Begins_Hour+24+Amount) % 24;
  if(Amount==0) {
    MESSAGE("night.mp3","Nighttime ");
    MESSAGE("begins.mp3","begins at:");
  }
  if(SpeakHours(Night_Begins_Hour,true)) {
    SpeakAMPM(Night_Begins_Hour);
  }
  DEBUG("\n");
}




//Do the actual setup configuration. It's called when you press the setup button
void do_Configuration(void) {
  Doing_Configuration= true;//turnoff chimes and other audio while configuring
  musicPlayer.setVolume(AudioDaytime,AudioDaytime);
  MESSAGE("setup.mp3", "Entering setup mode\n");
  MESSAGE("choice.mp3", "\nPress arrows to change or advance to next item\n");
  delay(500);
  Configure_Menu_Items(Menus[AUDIO_MENU]);
  Configure_Integer(Update_Day_Begins);
  Configure_Integer(Update_Night_Begins);
  Configure_Integer(Update_Day_Volume);
  if((Audio_State== MODE_TIMED)||(Audio_State== MODE_LIGHT)) {
    musicPlayer.setVolume(AudioNighttime,AudioNighttime);
    Configure_Integer(Update_Night_Volume);
    musicPlayer.setVolume(AudioDaytime,AudioDaytime);
  }
  Configure_Integer(Update_Day_Bright);
  Configure_Integer(Update_Night_Bright);
  Brightness=Bright_Day;
  BrightInverse=255/Brightness;
  Configure_Menu_Items(Menus[CHIMES_MENU]);
  Configure_Menu_Items(Menus[HOURLY_MENU]);
  Configure_Menu_Items(Menus[QUARTERLY_MENU]);
  Configure_Menu_Items(Menus[VOICE_MENU]);
  Configure_Menu_Items(Menus[ANIMATION_MENU]);
  Configure_Menu_Items(Menus[ALARM_MENU]);
  if(Alarm_Enable) {
    Configure_Integer(Update_Alarm_Hour);
    Configure_Integer(Update_Alarm_Minutes);
  }
  Configure_Menu_Items(Menus[MUSIC_MENU]);
  MESSAGE("complete.mp3", "Setup completed\n");
  Doing_Configuration= false;
}

