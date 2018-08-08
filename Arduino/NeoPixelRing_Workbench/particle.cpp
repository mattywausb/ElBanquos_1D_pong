#include "particle.h"
#include "Arduino.h"

//#define TRACE_PARTICLE

#define PIXEL_COUNT 12


Particle::Particle(void)
{

}

void Particle::awake(int r,int g, int b, int pos, int vlct, byte dcr)   
{
  colorChannel[0]=r;
  colorChannel[1]=g;
  colorChannel[2]=b;
  movetick=0;
  position=pos*10;
  velocity=vlct;
  decayrate=dcr;
  life=255;
  #ifdef TRACE_PARTICLE
  Serial.println("awaken");
  #endif
}
     
void Particle::tick(void)
{
  position+=velocity*256/(512-life);
   if(position<0) position+=PIXEL_COUNT*10;
   if(position>=PIXEL_COUNT*10)position-=PIXEL_COUNT*10;
  if(life>decayrate) life-=decayrate;
  else life=0;        
}
        
void Particle::draw(Adafruit_NeoPixel &strip)
{
   int currentChannel[3];

   for(int i=0;i<3;i++)
   {
    currentChannel[i]=((int)colorChannel[i])*(int)life/255;
  #ifdef TRACE_PARTICLE    
      Serial.print(currentChannel[i]);Serial.print("|");
      #endif
   }
   strip.setPixelColor(position/10,currentChannel[0],currentChannel[1],currentChannel[2]);
     #ifdef TRACE_PARTICLE
   Serial.println();
   #endif
}

