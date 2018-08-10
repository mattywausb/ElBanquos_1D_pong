//#ifdef TRACE_ON
#define TRACE_SOUND
//#endif

/*            Musical Note Freqency table  (its just defines, dont worry about the size */
#define FREQ_B0  31
#define FREQ_C1  33
#define FREQ_CS1 35
#define FREQ_D1  37
#define FREQ_DS1 39
#define FREQ_E1  41
#define FREQ_F1  44
#define FREQ_FS1 46
#define FREQ_G1  49
#define FREQ_GS1 52
#define FREQ_A1  55
#define FREQ_AS1 58
#define FREQ_B1  62
#define FREQ_C2  65
#define FREQ_CS2 69
#define FREQ_D2  73
#define FREQ_DS2 78
#define FREQ_E2  82
#define FREQ_F2  87
#define FREQ_FS2 93
#define FREQ_G2  98
#define FREQ_GS2 104
#define FREQ_A2  110
#define FREQ_AS2 117
#define FREQ_B2  123
#define FREQ_C3  131
#define FREQ_CS3 139
#define FREQ_D3  147
#define FREQ_DS3 156
#define FREQ_E3  165
#define FREQ_F3  175
#define FREQ_FS3 185
#define FREQ_G3  196
#define FREQ_GS3 208
#define FREQ_A3  220
#define FREQ_AS3 233
#define FREQ_B3  247
#define FREQ_C4  262
#define FREQ_CS4 277
#define FREQ_D4  294
#define FREQ_DS4 311
#define FREQ_E4  330
#define FREQ_F4  349
#define FREQ_FS4 370
#define FREQ_G4  392
#define FREQ_GS4 415
#define FREQ_A4  440
#define FREQ_AS4 466
#define FREQ_B4  494
#define FREQ_C5  523
#define FREQ_CS5 554
#define FREQ_D5  587
#define FREQ_DS5 622
#define FREQ_E5  659
#define FREQ_F5  698
#define FREQ_FS5 740
#define FREQ_G5  784
#define FREQ_GS5 831
#define FREQ_A5  880
#define FREQ_AS5 932
#define FREQ_B5  988
#define FREQ_C6  1047
#define FREQ_CS6 1109
#define FREQ_D6  1175
#define FREQ_DS6 1245
#define FREQ_E6  1319
#define FREQ_F6  1397
#define FREQ_FS6 1480
#define FREQ_G6  1568
#define FREQ_GS6 1661
#define FREQ_A6  1760
#define FREQ_AS6 1865
#define FREQ_B6  1976
#define FREQ_C7  2093
#define FREQ_CS7 2217
#define FREQ_D7  2349
#define FREQ_DS7 2489
#define FREQ_E7  2637
#define FREQ_F7  2794
#define FREQ_FS7 2960
#define FREQ_G7  3136
#define FREQ_GS7 3322
#define FREQ_A7  3520
#define FREQ_AS7 3729
#define FREQ_B7  3951
#define FREQ_C8  4186
#define FREQ_CS8 4435
#define FREQ_D8  4699
#define FREQ_DS8 4978

/* Frequencies we will use in this program */
int note_palette[] = {
   FREQ_B3, 
   FREQ_C4,
   FREQ_D4,
   FREQ_E4,
   FREQ_F4,
   FREQ_G4,
   FREQ_A4,
   FREQ_B4,
   FREQ_C5
}; 

/* Tranlastion of note name to index in the note_palette */

#define NOTE_B3 0
#define NOTE_C4 1
#define NOTE_D4 2
#define NOTE_E4 3
#define NOTE_F4 4
#define NOTE_G4 5
#define NOTE_A4 6
#define NOTE_B4 7
#define NOTE_C5 8
#define NOTE_PAUSE 255


/* Base Timing */
unsigned int sound_full_note_duration=60000; // Ridiculus high so we notice when we forget to calculate

/* now we can start composing melodys */

typedef struct  {
  byte note_index;
  byte note_length;   // Meaning 1 = Full note 4 quater, 8 = 8th and so on
} note_t;

note_t melody_1[] = { 
  {NOTE_A4,8},{NOTE_PAUSE,8},
  {NOTE_C5,16},{NOTE_B4,16},{NOTE_C5,4},
  {NOTE_E4,4},{NOTE_G4,8},{NOTE_A4,8}
};

note_t melody_2[] = { 
  {NOTE_C5,16},{NOTE_PAUSE,16},{NOTE_C5,16},{NOTE_C5,16},
  {NOTE_B4,16},{NOTE_PAUSE,16},{NOTE_B4,16},{NOTE_B4,16},
  {NOTE_A4,16},{NOTE_PAUSE,16},{NOTE_A4,16},{NOTE_A4,16},
  {NOTE_G4,16},{NOTE_PAUSE,16},{NOTE_G4,16},{NOTE_PAUSE,16},
  {NOTE_A4,8}
};

/* variables for effects */
int sound_pong_base_freq; 

/* All the hardware stuff */

#define SOUND_OUT_PIN 11
#define TRACE_MONITOR_LED_PIN LED_BUILTIN

/* management Variables */

enum sound_effects {
  sound_off,
  sound_melody,
  sound_crash,
  sound_pong
};
sound_effects sound_current_effect=sound_off;

note_t *sound_active_melody;

unsigned int sound_active_melody_length=0; /* max index in the note Array */
unsigned int  sound_current_note;
unsigned int sound_wait_millis=0;


unsigned long sound_frame_tick_millis=0;

byte sound_current_frame=0;


/*  ************************  interface  ********************************
 *  *********************************************************************
 */

bool sound_isPlaying()
{
  return sound_current_effect!=sound_off;
}

void sound_stop()
{
  sound_current_effect=sound_off;
  noTone(SOUND_OUT_PIN);
  #ifdef TRACE_SOUND
    Serial.print(F("sound_stopMelody. Current note:")); Serial.println(sound_current_note);         
  #endif
}

/* ------------  Melody1 ------------------- */

void sound_start_GameStartMelody()
{
  sound_current_effect=sound_melody;
  sound_current_note=0;
  sound_wait_millis=0;
  sound_calculate_full_note_duration(120);
  sound_active_melody=&melody_1[0];
  sound_active_melody_length=sizeof(melody_1)/sizeof(melody_1[0]);
  #ifdef TRACE_SOUND
         Serial.print(F("sound_active_sound_lenth=")); Serial.println(sound_active_melody_length);
  #endif 
}

void sound_start_GameOverMelody()
{
  sound_current_effect=sound_melody;
  sound_current_note=0;
  sound_wait_millis=0;
  sound_calculate_full_note_duration(120);
  sound_active_melody=&melody_2[0];
  sound_active_melody_length=sizeof(melody_2)/sizeof(melody_2[0]);
}

/* ------------  Pong & Ping  ------------------- */

void sound_start_Pong()
{
  sound_current_effect=sound_pong;
  sound_current_note=0;
  sound_wait_millis=0;
  sound_pong_base_freq=random(108,115);
}

void sound_start_Ping()
{
  sound_current_effect=sound_pong;
  sound_current_note=0;
  sound_wait_millis=0;
  sound_pong_base_freq=random(208,230);
}

void sound_play_pong(){
  tone(SOUND_OUT_PIN, sound_pong_base_freq+sound_current_note<<2);
  sound_wait_millis=10;
  if(++sound_current_note>8) sound_stop();
}

/* ------------  crash ------------------- */

void sound_start_Crash()
{
  sound_current_effect=sound_crash;
  sound_current_note=0;
  sound_wait_millis=0;
}

void sound_play_crash(){
  tone(SOUND_OUT_PIN, random(550-(sound_current_note<<1),800-(sound_current_note)));
  sound_wait_millis=1;
  if(++sound_current_note>250) sound_stop();
  
}

/*  ************************  internals  ********************************
 *  *********************************************************************
 */



void sound_calculate_full_note_duration(long beats_per_minute)
{
  // 4 beat per minute = 60000 millis per beat
  sound_full_note_duration=240000/beats_per_minute;
  #ifdef TRACE_SOUND
         Serial.print(F("sound_full_note_duration=")); Serial.println(sound_full_note_duration);
  #endif
   
}

void sound_play_next_note()
{
   if(sound_current_note>=sound_active_melody_length) 
    {
      sound_stop();
      return;
    }
  
   int note_duration=0;
  
      #ifdef TRACE_SOUND
          Serial.print(sound_current_note);Serial.print(F(":"));
          Serial.print(sound_active_melody[sound_current_note].note_index);Serial.print(F("/"));
          Serial.print(sound_active_melody[sound_current_note].note_length);Serial.print(F("="));
      #endif
          
    if(sound_active_melody[sound_current_note].note_index!=NOTE_PAUSE)
    {
      note_duration=sound_full_note_duration/sound_active_melody[sound_current_note].note_length;
      sound_wait_millis=note_duration+3; // This creates a small gap bewen all tones
      tone(SOUND_OUT_PIN, note_palette[sound_active_melody[sound_current_note].note_index], note_duration);
      #ifdef TRACE_SOUND

          Serial.print(sound_wait_millis);Serial.println(F("ms"));
      #endif
    } 
    ++sound_current_note;
}
  

/* *************************** TICK *************************************
   Must be called every tick of the core loop
   Manages playing the melody in the background 
*/

unsigned long sound_tick()
{
  if(sound_current_effect==sound_off) return 1000; // external timers may force earlier call to get me check for work
  
  unsigned long current_frame_duration=millis()-sound_frame_tick_millis;
   
  if(current_frame_duration<sound_wait_millis) return sound_wait_millis-current_frame_duration;
  sound_frame_tick_millis=millis();  // globally fixed timestamp for this tick

  switch (sound_current_effect) {
      case sound_melody: sound_play_next_note(); break;
      case sound_crash: sound_play_crash();break;
      case sound_pong: sound_play_pong();break;
    }
}

/*  ************************  setup  ************************************
 *  *********************************************************************
 */

void sound_setup() {
  pinMode(SOUND_OUT_PIN,OUTPUT);
  digitalWrite(SOUND_OUT_PIN,LOW);   
  #ifdef TRACE_SOUND
  pinMode(TRACE_MONITOR_LED_PIN,OUTPUT);
  digitalWrite(TRACE_MONITOR_LED_PIN,LOW);    
  #endif
}
