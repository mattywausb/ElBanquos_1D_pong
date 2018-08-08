#include <Adafruit_NeoPixel.h>

#include "MainSettings.h"

#ifdef TRACE_ON
#define TRACE_OUTPUT
#endif


#define PIXEL_PIN    7    // Digital IO pin connected to the NeoPixels.

#define FPS 50
#define FRAME_DELAY 1000/FPS
#define BALL_COLOR strip.Color(200, 200, 0)  // Yellow
#define BALL_COLOR_AFTER_GLOW strip.Color(20, 10, 0)  // Yellow
#define BLACK_COLOR strip.Color(0, 0, 0)
#define BASE_A_COLOR strip.Color(0, 40, 40)
#define BASE_B_COLOR strip.Color(50, 00,0)
#define BASE_HOT_COLOR strip.Color(180,90,0)
#define POINT_COLOR strip.Color(15,0,30)

unsigned long last_frame_millis=0;


Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

unsigned long output_draw_game_frame()
{ 
  static byte afterglow_position=255;
  static byte previous_ball_position=255;
  
    unsigned long current_frame_duration=millis()-last_frame_millis;
    if(current_frame_duration<FRAME_DELAY) return FRAME_DELAY-current_frame_duration;

    for(int i=0;i<PIXEL_COUNT;i++) {
      if(i==ball_position) {
        strip.setPixelColor(i, BALL_COLOR);
        #ifdef TRACE_ON
          Serial.println(i);
        #endif
        continue;
      }

      switch(i) {
        case 0 :  if(base_A_hot) strip.setPixelColor(i,BASE_HOT_COLOR);
                  else strip.setPixelColor(i,BASE_A_COLOR);
                  break;
        case PIXEL_COUNT-1 :  if(base_B_hot) strip.setPixelColor(i,BASE_HOT_COLOR);
                              else strip.setPixelColor(i,BASE_B_COLOR); 
                              break;
        default: 
            if(i==afterglow_position) strip.setPixelColor(i,BALL_COLOR_AFTER_GLOW);
            else {
                  strip.setPixelColor(i,BLACK_COLOR);
                  if(i<base_a_points+1) strip.setPixelColor(i, POINT_COLOR);
                  if(i>=PIXEL_COUNT-1-base_b_points) strip.setPixelColor(i, POINT_COLOR);
            }
        }
    }   
    strip.show();
    if(previous_ball_position!=ball_position) {
       afterglow_position=previous_ball_position;
       previous_ball_position=ball_position;      
    }
    last_frame_millis=millis();
    return FRAME_DELAY;
}
