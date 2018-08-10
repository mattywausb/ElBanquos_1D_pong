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

enum state {
  melody1,
  melody2,
  crash,
  pong,
  ping
} current_state=pong;

void loop() {

 

    if(!digitalRead(TRIGGER_BUTTON_PIN)) {
      sound_stop();
      switch(current_state) 
      {
        case melody1: sound_start_Melody1(); break;
        case melody2: sound_start_Melody2(); break;
        case crash: sound_start_Crash(); break;
        case pong: sound_start_Pong(); break;
        case ping: sound_start_Ping(); break;

        
      }
      Serial.println("TRIGGER_BUTTON");
      delay(150); // just debounce
    }
    
    if(!digitalRead(STOP_BUTTON_PIN)) 
    {
       Serial.println("STOP_BUTTON"); 
       sound_stop();   
      switch(current_state) 
      {
        case melody1: current_state=melody2; break;
        case melody2: current_state=crash; break;
        case crash: current_state=pong; break;
        case pong: current_state=ping; break;
        case ping: current_state=melody1; break;
      }
      delay(150); // just debounce
    }
    sound_tick();    

}
