

#define TRACE_ON
//#define TRACE_OUTPUT
//#define TRACE_OUTPUT_HIGH

#include <Adafruit_NeoPixel.h>
#include "particle.h"

#define PIXEL_PIN    7    // Digital IO pin connected to the NeoPixels.

#define PIXEL_COUNT 12

// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick

#define FPS 30
#define FRAME_DELAY 1000/FPS
#define BALL_COLOR strip.Color(200, 200, 0)  // Yellow
#define BALL_COLOR_AFTER_GLOW strip.Color(20, 10, 0)  // Yellow
#define BLACK_COLOR strip.Color(0, 0, 0)
#define BASE_A_COLOR(i) dimmedColor(0, 255, 255,i)
#define BASE_B_COLOR(i) dimmedColor(50, 00,0,i)

  #define PARTICLE_COUNT 10
  static Particle particles[PARTICLE_COUNT];
  
unsigned long output_frame_millis=0;
unsigned long output_frame_number=0;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

enum program_t {
  dimgrading_scene,
  particle_party_1,
  particle_party_2
};

program_t current_program=particle_party_2;
//program_t current_program=dimgrading_scene;

/*  ************************  dimgrading_scene **************************
 *  *********************************************************************
 */

unsigned long output_draw_dimgrading_scene()
{ 
  static int intensity=255;
  static int change_velocity=-10;
  static int contrast=10;
  static int contrast_velocity=3;


  if(input_button_A_gotPressed()) {
    contrast+=contrast_velocity;
    #ifdef TRACE_OUTPUT 
      Serial.print(F("Contrast:"));Serial.println(contrast);
    #endif
    if(contrast<1)
    { 
      contrast_velocity= - contrast_velocity;
      contrast=2;
    }
    if(contrast>60) 
    {
      contrast_velocity= - contrast_velocity;
      contrast=57;      
    }
  }

  if(input_button_B_gotPressed()) {
    intensity+=change_velocity;
    #ifdef TRACE_OUTPUT 
      Serial.print(F("Intensity:"));Serial.println(intensity);
    #endif
    if(intensity<0)
    { 
      change_velocity= - change_velocity;
      intensity=0;
    }
    if(intensity>255) 
    {
      change_velocity= - change_velocity;
      intensity=255;      
    }
  }
  
  unsigned long current_frame_duration=millis()-output_frame_millis;
  if(current_frame_duration<FRAME_DELAY) return FRAME_DELAY-current_frame_duration;
  output_frame_millis=millis();
  output_frame_number++;
    #ifdef TRACE_OUTPUT_HIGH
      Serial.print(F("Frame:"));Serial.println(output_frame_number);
    #endif
  for(int i=0;i<PIXEL_COUNT;i++) {
    strip.setPixelColor(i,BASE_A_COLOR(intensity-i*contrast));
  }
  strip.show();
}

/*  ************************  output_draw_particle_party_1 **************************
 *  *********************************************************************
 */
void output_draw_particle_party_1()
{
    

  int i;

  if(input_button_A_gotPressed()) {

      for(i=0;i<PARTICLE_COUNT;i++)
      {
        if(!particles[i].isAlive()) {
          particles[i].igniteRandom(11,-600);
          break;
        }
     }
  }
  if(input_button_B_gotPressed()) {

      for(i=0;i<PARTICLE_COUNT;i++)
      {
        if(!particles[i].isAlive()) {
          particles[i].igniteRandom(0,600);
          break;
        }
     }
  }
   
  unsigned long current_frame_duration=millis()-output_frame_millis;
  if(current_frame_duration<FRAME_DELAY) return FRAME_DELAY-current_frame_duration;
  output_frame_millis=millis();
  output_frame_number++;
  #ifdef TRACE_OUTPUT
      Serial.print(F("Frame:"));Serial.println(output_frame_number);
  #endif 

 
  

  for(i=0;i<PIXEL_COUNT;i++) {// clear strip
    strip.setPixelColor(i,0);
  }
  for(i=0;i<PARTICLE_COUNT;i++) {
    if(particles[i].isAlive()) {
      particles[i].draw(strip);
      particles[i].frameTick(output_frame_number);
    }
  }
  strip.show();
}

/*  ************************  output_draw_particle_party_2 **************************
 *  *********************************************************************
 */
void output_draw_particle_party_2()
{

  int i;

  if(input_button_A_gotPressed()) {

      for(i=0;i<PARTICLE_COUNT;i++)
      {
        if(!particles[i].isAlive()&&i%2) {
          particles[i].ignite(255,0,0,11,-100-150*i);
        }
     }
  }
  if(input_button_B_gotPressed()) {

      for(i=0;i<PARTICLE_COUNT;i++)
      {
        if(!particles[i].isAlive()&&i%2==0) {
          particles[i].ignite(0,0,255,0+(i)/3,230+50*i);
        }
     }
  }
   
  unsigned long current_frame_duration=millis()-output_frame_millis;
  if(current_frame_duration<FRAME_DELAY) return FRAME_DELAY-current_frame_duration;
  output_frame_millis=millis();
  output_frame_number++;
  #ifdef TRACE_OUTPUT
      Serial.print(F("Frame:"));Serial.println(output_frame_number);
  #endif 

 
  

  for(i=0;i<PIXEL_COUNT;i++) {// clear strip
    strip.setPixelColor(i,0);
  }
  for(i=0;i<PARTICLE_COUNT;i++) {
    if(particles[i].isAlive()) {
      particles[i].draw(strip);
      particles[i].frameTick(output_frame_number);
    }
  }
  strip.show();
}
/*  ************************  Helper  ************************************
 *  *********************************************************************
 */


uint32_t dimmedColor(int r,int g,int b,int i)
{
  if(i<0)i=0;
  if(i>255)i=255;
  return strip.Color((r*i)/255, (g*i)/255, (b*i)/255);
}

/*  ************************  setup  ************************************
 *  *********************************************************************
 */

void setup() {

   input_setup();
  #ifdef TRACE_ON 
    char compile_signature[] = "--- START (Build: " __DATE__ " " __TIME__ ") ---";   
    Serial.begin(9600);
    Serial.println(compile_signature); 
  #endif

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(64);
}

/*  ************************  loop  ************************************
 *  *********************************************************************
 */
void loop() {
   input_capture_tick();

   switch (current_program) {
    case dimgrading_scene: 
              // TODO Check buttons to switch to next scene
              output_draw_dimgrading_scene();
             if(input_button_B_gotReleased() && input_getLastPressDuration()>3000)
                 current_program=particle_party_1;
              break;  
    case particle_party_1:

              output_draw_particle_party_1();
              if(input_button_B_gotReleased() && input_getLastPressDuration()>3000)
                 current_program=particle_party_1;
              
              break;  
    case particle_party_2:

              output_draw_particle_party_2();
              if(input_button_B_gotReleased() && input_getLastPressDuration()>3000)
                 current_program=dimgrading_scene;
              
              break;  
    default: 
         Serial.println(F("Bad program")); delay(1000);
   }
}

