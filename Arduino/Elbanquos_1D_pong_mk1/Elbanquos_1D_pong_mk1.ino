
#include "MainSettings.h"
#include "PongGame.h"

#ifdef TRACE_ON
#define TRACE_MAIN
#endif

PongGame theGame(PIXEL_COUNT<<7) ;
  

 
/*   *****************  setup *****************************+
 *    
 */

void setup() {
  #ifdef TRACE_ON 
    char compile_signature[] = "--- START (Build: " __DATE__ " " __TIME__ ") ---";   
    Serial.begin(9600);
    Serial.println(compile_signature); 
  #endif


  input_setup() ;
  output_setup(&theGame);
  sound_setup();
  output_begin_GAME_SELECT_SCENE();
}

void loop() {

   if(theGame.isActive())
   {
   unsigned long game_tick_delay=theGame.process_tick();
   unsigned long output_frame_delay=output_frame_tick();
   unsigned long output_sound_delay=sound_tick();;
   delay(min(game_tick_delay,min(output_frame_delay,output_sound_delay )) );    
   return;
   }

   if(theGame.isClosing())
   {
    output_begin_GAME_SELECT_SCENE();
   }

   input_capture_tick();  
   if(input_button_A_gotPressed()) {
      #ifdef TRACE_MAIN
         Serial.println(F("Loop: Button A for Select pressed"));
      #endif 
      theGame.setupGame();
      theGame.startGame();
      return;
   }
   sound_tick();
   output_frame_tick(); 
}
