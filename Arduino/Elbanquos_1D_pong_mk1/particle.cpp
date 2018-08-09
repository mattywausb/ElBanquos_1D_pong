#include "particle.h"
#include "Arduino.h"

//#define TRACE_PARTICLE

#define PARTICLE_PIXEL_COUNT 12


Particle::Particle(void)
{

}

void Particle::ignite(int r,int g, int b, int pos, int vlct, byte dcr)   
{
  colorChannel[0]=r;
  colorChannel[1]=g;
  colorChannel[2]=b;
  
  position=pos*POSITION_TO_PIXEL_FACTOR;
  velocity=vlct;
  decayrate=dcr;   
  if(decayrate>99) decayrate=99; // using values >99 would result in infinity life
  
  life=1000;
   if(velocity>0) acceleration=-15;
  else acceleration=15;
  
  #ifdef TRACE_PARTICLE
  Serial.println("ignite");
  #endif
}


void Particle::igniteRandom(int pos, int velocityBase=0)
{
  igniteRandom( 255-random(200)  // r
               ,255-random(200)  // g
               ,255-random(200)  // b
               ,pos
               ,velocityBase);
}

void Particle::igniteRandom(int r,int g, int b,int pos, int velocityBase=0)
{
  colorChannel[0]=r;
  colorChannel[1]=g;
  colorChannel[2]=b;
  
  position=pos*POSITION_TO_PIXEL_FACTOR;
  if(velocityBase>0) velocity=velocityBase+ random(0,300);
  if(velocityBase<0) velocity=velocityBase- random(0,300);
  if(velocityBase==0) velocity=((random(20)%2)==0?1:-1)*random(700,1000);
  decayrate=random(85,93);  // decayrate
  
  life=1000-random(0,100);
  acceleration=-(15-(3-random(0,6)))*velocity/1000;
}

     
void Particle::frameTick(unsigned long frame_number)
{
  long long_life=0;

  if(frame_number%PARTICLE_FRAMES_PER_TICK!=0) return;  
  
  position+=velocity;
  if(position<0) position+=max_position;
  if(position>=max_position)  position-=max_position;
   
  velocity+=acceleration;

  if(life>0) {
    long_life=life;
    life=(long_life*decayrate)/100;   // Percentace in integer arithmetic: multiply first
  }       
}
        
void Particle::draw(Adafruit_NeoPixel &strip)
{
   int currentChannel[3];
   unsigned int colorValueAsInt;
   long long_life=life;

   for(int i=0;i<3;i++)
   {
    currentChannel[i]=long_life*colorChannel[i]/1000;
    #ifdef TRACE_PARTICLE    
      Serial.print(currentChannel[i]);Serial.print("|");
    #endif
   }
   if(currentChannel[0]|currentChannel[1]|currentChannel[2]) strip.setPixelColor(position/POSITION_TO_PIXEL_FACTOR,currentChannel[0],currentChannel[1],currentChannel[2]);
   #ifdef TRACE_PARTICLE
   Serial.println(life);
   #endif
}

