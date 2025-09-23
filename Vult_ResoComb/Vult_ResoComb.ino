// Vult DSP resonant comb filter - pin PA3 audio output //

#include <HardwareTimer.h>
#include "Noise.h"
#include "Rescomb.h"

HardwareTimer Timer(TIM1);

#define SAMPLE_RATE 44100
#define BPM         120

Noise_process_type noise;
Rescomb_process_type comb;

  uint16_t color = 0;   // noise color
  uint16_t combcv = 0;  // comb cv
  uint16_t tones = 0;   // comb tone
  uint16_t res = 65000; // comb resonance
  bool trig = false;

void timerHandler(void) {

  int noiseout = 0;

  if (trig) noiseout = Noise_process(noise, color) >> 2;

	int sample = Rescomb_process(comb, noiseout, combcv, tones, res);

  TIM2->CH4CVR = 128 + (sample >> 9);

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

}

void loop() {

  color = random(0, 65535);
  combcv = random(0, 16384);
  tones = 65535 - random(0, 8192);
  trig = true;

  delay(5);

  trig = false;
   
  int tempo = 60000 / BPM;
  delay((tempo / 3) - 5);

}