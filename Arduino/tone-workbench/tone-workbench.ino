/* Workbench to develop Sounds and music for the game
*/

#define TRACE_ON

#define TRIGGER_BUTTON_PIN 8
#define STOP_BUTTON_PIN 6



void setup() {
  #ifdef TRACE_ON 
    char compile_signature[] = "--- START (Build: " __DATE__ " " __TIME__ ") ---";   
    Serial.begin(9600);
    Serial.println(compile_signature); 
  #endif
  
   pinMode(TRIGGER_BUTTON_PIN, INPUT_PULLUP);
   pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
   void sound_setup() ;
}

void loop() {

    if(!digitalRead(TRIGGER_BUTTON_PIN)) {
      sound_playMelody1();
      Serial.println("TRIGGER_BUTTON");
      delay(500); // just debounce
    }
    if(!digitalRead(STOP_BUTTON_PIN)) 
    {
       Serial.println("STOP_BUTTON"); 
       sound_stopMelody();    
      delay(500); // just debounce
    }
    sound_tick();    

}
