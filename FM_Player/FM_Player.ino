// FM polyphonic player - pin PA3 audio output //

#include <HardwareTimer.h>
#include "playtune.h"
#include "wavetable.h"
#include "songdata.h"
#include "tuningwords.h"
#include "envelope.h"

HardwareTimer Timer(TIM1);

#define SAMPLE_RATE 44100

const int FIXED_BITS = 14;
const int FIXED_SCALE = (1 << FIXED_BITS);
const int REVERB_SIZE = 0x800;
const int REVERB_LENGTH = (int)(REVERB_SIZE * 0.99f);
const int REVERB_DECAY = (int)(FIXED_SCALE * 0.8f);

int16_t reverbAddr;
int16_t reverbBuffer[REVERB_SIZE];

extern FMchannel ch[NUM_OF_CHANNELS];

  unsigned int timePlay = 0;
  unsigned int timePlayCount = 0;
  unsigned char isPlaying = 1;
  unsigned int songIndex = 0;
  float speed = 1.5f;


void timerHandler(void) {

  isPlaying = 1;

  int16_t sample = generateFModTask();

  int reverb = ((int)reverbBuffer[reverbAddr] * REVERB_DECAY) >> FIXED_BITS;
      reverb += sample;
      reverbBuffer[reverbAddr] = reverb;
      reverbAddr++;
      if (reverbAddr > REVERB_LENGTH) reverbAddr = 0;

      int16_t output = sample + (reverbBuffer[reverbAddr]>>3);

	TIM2->CH4CVR = 128 + (output >> 8);

}


void PWM_init() {

	RCC->APB2PCENR |= RCC_APB2Periph_GPIOA;
	RCC->APB1PCENR |= RCC_APB1Periph_TIM2;

  GPIOA->CFGLR &= ~(0xf<<(4*3));
	GPIOA->CFGLR |= (GPIO_Speed_50MHz | GPIO_Mode_AF_PP)<<(4*3);

	RCC->APB1PRSTR |= RCC_APB1Periph_TIM2;
	RCC->APB1PRSTR &= ~RCC_APB1Periph_TIM2;

	TIM2->PSC = 0x0000;
	TIM2->ATRLR = 255;
  TIM2->CHCTLR2 |= TIM_OC4M_2 | TIM_OC4M_1 | TIM_OC4PE;
	TIM2->CTLR1 |= TIM_ARPE;
  TIM2->CCER |= TIM_CC4E | (TIM_CC4P & 0xff);
	TIM2->SWEVGR |= TIM_UG;
	TIM2->CTLR1 |= TIM_CEN;

}

void setup() {

	PWM_init();
	
	Timer.setOverflow(SAMPLE_RATE, HERTZ_FORMAT);
  Timer.attachInterrupt(timerHandler);
  Timer.resume();

  for (int i = 0; i < NUM_OF_CHANNELS; i++){
    ch[i].setFreqMult_c(4.000f);
    ch[i].setFreqMult_m(1.000f);
    ch[i].setModMultiplier(4096);
    ch[i].setADSR(0.05f, 0.15f, 0.15f, 0.25f);
  }

}

void loop() {

  if (isPlaying){
    if (timePlayCount > timePlay){
      timePlayCount = 0;
      updateNote(isPlaying, timePlay, timePlayCount, songIndex, speed);
    } else timePlayCount++;
     delay(1);
  }

}