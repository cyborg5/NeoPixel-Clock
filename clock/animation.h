/*
 * Routines to create animated displays on the neo-pixel ring.
 * Depends on "Brightness" and "BrightInverse" values.
 */
 
void CandyCane(uint8_t sets, uint8_t width, uint16_t wait, bool reversed=false) {
  int L;
  for(int j=0;j<(sets*width);j++) {
    for(int i=0;i< strip.numPixels();i++) {
      L=(reversed)?strip.numPixels()-i-1:i;
      if ( ((i+j) % (width*2) )<width)
        strip.setPixelColor(L,Brightness,0,0);
      else
        strip.setPixelColor(L,Brightness,Brightness, Brightness);
    };
    strip.show();
    delay(wait); 
  };
};

//Create sets of random white or gray pixels
void RandomWhite (int sets, int wait) {
  int V,i,j;
  for (i=0;i<sets;i++) {
    for(j=0;j<strip.numPixels();j++) {
      V=random(Brightness);
      strip.setPixelColor(j,V,V,V);
    }
    strip.show();
    delay(wait);
  }
};
//Create sets of random colors
void RandomColor (int sets, int wait) {
  int i,j;
  for (i=0;i<sets;i++) {
    for(j=0;j<strip.numPixels();j++) {
      strip.setPixelColor(j,random(255)/BrightInverse,random(255)/BrightInverse,random(255)/BrightInverse);
    }
    strip.show();
    delay(wait);
  }
};

void RainbowStripe (uint8_t sets, uint8_t width, uint16_t wait, bool reversed= false) {
  uint8_t L;
  for(int j=0;j<(sets*width*6);j++) {
    for(int i=0;i< strip.numPixels();i++) {
      L=(reversed)?strip.numPixels()-i-1:i;
      switch ( ( (i+j)/width) % 6 ) {
        case 0: strip.setPixelColor(L,Brightness,0,0);break;//Red
        case 1: strip.setPixelColor(L,Brightness,Brightness,0);break;//Yellow
        case 2: strip.setPixelColor(L,0,Brightness,0);break;//Green
        case 3: strip.setPixelColor(L,0,Brightness,Brightness);break;//Cyan
        case 4: strip.setPixelColor(L,0,0,Brightness);break;//Blue
        case 5: strip.setPixelColor(L,Brightness,0,Brightness);break;//Magenta
//        default: strip.setPixelColor(L,0,0,0);//Use for debugging only
      }
    };
    strip.show();
    delay(wait);
  };
};

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait, bool reversed= false) {
  uint8_t L;
  for(uint8_t i=0; i<strip.numPixels(); i++) {
      L=(reversed)?strip.numPixels()-i-1:i;
      strip.setPixelColor(L, c);
      strip.show();
      delay(wait);
  }
}
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color((WheelPos * 3)/BrightInverse, (255 - WheelPos * 3)/BrightInverse, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color((255 - WheelPos * 3)/BrightInverse, 0, (WheelPos * 3)/BrightInverse);
  } else {
   WheelPos -= 170;
   return strip.Color(0,(WheelPos * 3)/BrightInverse, (255 - WheelPos * 3)/BrightInverse);
  }
}

void rainbowCycle(uint8_t sets, uint8_t wait, bool reversed= false) {
  uint16_t i, j;
  for(j=0; j<256*sets; j++) { //cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor((reversed)?strip.numPixels()-i-1:i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}
void WipeColors(uint8_t sets, uint8_t wait, bool reversed= false) {
  for (uint8_t i=0;i< sets;i++) {
    colorWipe(strip.Color(Brightness, 0, 0), wait, reversed); // Red
    colorWipe(strip.Color(Brightness, Brightness, 0), wait, reversed); // Yellow
    colorWipe(strip.Color(0, Brightness, 0), wait, reversed); // Green
    colorWipe(strip.Color(0, Brightness, Brightness), wait, reversed); // Cyan
    colorWipe(strip.Color(0, 0, Brightness), wait, reversed); // Blue
    colorWipe(strip.Color(Brightness, 0, Brightness), wait, reversed); // Magenta
  }
}

void crissCross(uint8_t sets, uint8_t wait,uint32_t c1,uint32_t c2) {
  uint8_t L,i,j,k;
  for(j=0;j<sets*strip.numPixels();j++) {
    for(i=0;i<strip.numPixels();i++) strip.setPixelColor(i,0,0,0);//erase everything
    for(i=0;i<strip.numPixels();i+=12) {
      for(k=0;k<2;k++) {
        strip.setPixelColor((i+k+j)% strip.numPixels(),c1);
      }
      for(k=0;k<2;k++) {
        strip.setPixelColor((i+k-j+ strip.numPixels()*sets)% strip.numPixels(),c2);
      }
    }
    strip.show();
    delay(wait);
  }
}

//Generates an animation based on the current hour either sequentially or randomly
//Is invoked at the top of the hour and anytime you get the "Play" button.
void do_Animation(void) {
  uint8_t H= Hours% 12;
  if(Animation_State==MODE_OFF) return;
  if(Animation_State==MODE_RANDOM) H=random(12);
  switch(H) {
    case  0: crissCross(3, 100, strip.Color(Brightness,0,0),strip.Color(0,Brightness,0)); break;
    case  1: crissCross(3, 100, strip.Color(Brightness,0,Brightness),strip.Color(0,Brightness,Brightness)); break;
    case  2: rainbowCycle(5, 10, false); break;
    case  3: rainbowCycle(5, 10, true); break;
    case  4: WipeColors(3, 20, false); break;
    case  5: WipeColors(3, 20, true); break;
    case  6: RainbowStripe(10,5, 50, true); break;
    case  7: RainbowStripe(10,5, 50, false); break;
    case  8: RandomColor(40, 400); break;
    case  9: RandomWhite(40, 400); break;
    case 10: CandyCane(25,5,100, true); break;
    case 11: CandyCane(25,5,100, false);break;
  }
  DEBUG("Animation Completed\n");
}


