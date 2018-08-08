
#include "MainSettings.h"
#include "PongGame.h"

#ifdef TRACE_ON
#define TRACE_GAME
#endif

PongGame theGame;
  
unsigned long game_tick()
{
  static long tick_count=0;
  unsigned long current_tick_duration=millis()-last_game_tick_millis;
  static byte button_a_previous_state=HIGH;
  static byte button_b_previous_state=HIGH;
  
  if(current_tick_duration<GAME_TICK_DELAY) return GAME_TICK_DELAY-current_tick_duration;
  tick_count++;

 
      
  /* logic */

  if(button_a_got_pressed && millis()-base_A_trigger_millis>BASE_HOT_RECOVERY) {
    base_A_trigger_millis=millis();
    base_A_hot=true;
  }
  if(base_A_hot &&  millis()-base_A_trigger_millis>BASE_HOT_DURATION) base_A_hot=false;

  if(button_b_got_pressed && millis()-base_B_trigger_millis>BASE_HOT_RECOVERY) {
    base_B_trigger_millis=millis();
    base_B_hot=true;
  }
  if(base_B_hot &&  millis()-base_B_trigger_millis>BASE_HOT_DURATION) base_B_hot=false;
  

  if((tick_count%10)==0) 
  {
    ball_position+=ball_direction;
    if(ball_position<0) {
      if(base_A_hot)     {
        ball_position-=2*ball_direction;
        ball_direction = -ball_direction;
      } else {
       base_b_points+=1;
       delay(500);
        ball_position=PIXEL_COUNT-1;
      }
    }
    
    if(ball_position>=PIXEL_COUNT) 
    {
     if(base_B_hot)     {
        ball_position-=2*ball_direction;
        ball_direction = -ball_direction;
      } else {
        base_a_points+=1;
        delay(500);
        ball_position=0;
      }      
     }
    }
  
  last_game_tick_millis=millis();
  return GAME_TICK_DELAY;
}
/*   *****************  setup *****************************+
 *    
 */

void setup() {
  #ifdef TRACE_ON 
    char compile_signature[] = "--- START (Build: " __DATE__ " " __TIME__ ") ---";   
    Serial.begin(9600);
    Serial.println(compile_signature); 
  #endif
  
  pinMode(BUTTON_A_PIN, INPUT_PULLUP);
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
   
   delay(min(game_tick(),output_draw_game_frame()));
}
