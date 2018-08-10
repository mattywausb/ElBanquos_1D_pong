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
   FREQ_B2, 
   FREQ_C3,
   FREQ_D3,
   FREQ_E3,
   FREQ_F3,
   FREQ_G3,
   FREQ_B3,
   FREQ_C4
}; 

/* Tranlastion of note name to index in the note_palette */

#define NOTE_B2 0
#define NOTE_C3 1
#define NOTE_D3 2
#define NOTE_E3 3
#define NOTE_F3 4
#define NOTE_G3 5
#define NOTE_H3 6
#define NOTE_C4 7
#define NOTE_PAUSE 255


/* Base Timing */
unsigned int sound_full_note_duration=60000; // Ridiculus high so we notice when we forget to calculate

/* not we can start composing melodys */

typedef struct  {
  byte note_index;
  byte note_length;   // Meaning 1 = Full note 4 quater, 8 = 8th and so on
} note_t;

note_t melody_1[] = { 
  {NOTE_C3,8},{NOTE_C3,8},{NOTE_C3,8},{NOTE_F3,4}
};


/* All the hardware stuff */

#define SOUND_OUT_PIN 11
#define TRACE_MONITOR_LED_PIN LED_BUILTIN

/* management Variables */

byte sound_active_melody=255;  /* 255 means off */ 
unsigned int sound_active_melody_length=0; /* max index in the note Array */
byte sound_current_note;
unsigned int sound_note_total_length_millis=0;


unsigned long sound_last_tick_time=0;

byte sound_current_frame=0;

/*  ************************  interface  ********************************
 *  *********************************************************************
 */

void sound_playMelody1()
{
  sound_active_melody=0;
  sound_current_note=0;
  sound_note_total_length_millis=0;
  sound_calculate_full_note_duration(120);
  sound_active_melody_length=sizeof(melody_1)/sizeof(melody_1[0]);
  #ifdef TRACE_SOUND
         Serial.print(F("sound_active_melody_lenth=")); Serial.println(sound_active_melody_length);
  #endif
  
}

void sound_stopMelody()
{
  sound_active_melody=255;
  noTone(SOUND_OUT_PIN);
    #ifdef TRACE_SOUND
         Serial.println(F("sound_stopMelody")); 
  #endif
}

void sound_calculate_full_note_duration(long beats_per_minute)
{
  // 4 beat per minute = 60000 millis per beat
  sound_full_note_duration=240000/beats_per_minute;
  #ifdef TRACE_SOUND
         Serial.print(F("sound_full_note_duration=")); Serial.println(sound_full_note_duration);
  #endif
   
}

/* *************************** TICK *************************************
   Must be called every tick of the core loop
   Manages playing the melody in the background 
*/

void sound_tick()
{
  int note_duration=0;
  
  if(sound_active_melody==255) return;

  if(millis()-sound_last_tick_time>sound_note_total_length_millis) 
  {
    sound_last_tick_time=millis();
    if(sound_current_note>=sound_active_melody_length) 
    {
      sound_stopMelody();
      return;
    }
    
    if(melody_1[sound_current_note].note_index!=NOTE_PAUSE)
    {
      note_duration=sound_full_note_duration/melody_1[sound_current_note].note_length;
      sound_note_total_length_millis=note_duration+3; // This creates a small gap bewen all tones
      tone(SOUND_OUT_PIN, note_palette[melody_1[sound_current_note].note_index], note_duration);
      #ifdef TRACE_SOUND
          Serial.print(sound_current_note);Serial.print(F(":"));
          Serial.print(melody_1[sound_current_note].note_index);Serial.print(F("/"));
          Serial.print(melody_1[sound_current_note].note_length);Serial.print(F("="));
          Serial.print(sound_note_total_length_millis);Serial.println(F("ms"));
      #endif
    } 
    ++sound_current_note;
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
