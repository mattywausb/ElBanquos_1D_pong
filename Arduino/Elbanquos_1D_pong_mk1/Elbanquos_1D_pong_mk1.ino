
#include "MainSettings.h"
#include "PongGame.h"

#ifdef TRACE_ON
#define TRACE_MAIN
#endif

PongGame
        theGame ;
  

 
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
}

void loop() {
   unsigned long game_tick_delay=theGame.process_tick();
   unsigned long output_frame_delay=output_frame_tick();
   
   delay(min(game_tick_delay,output_frame_delay ) );
}
