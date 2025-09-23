// Vult DSP phase distortion oscillator - pin PA3 audio output //

#include <HardwareTimer.h>
#include "Phasedist.h"

HardwareTimer Timer(TIM1);

#define SAMPLE_RATE 44100
#define BPM         120

Phasedist_process_type phasedist;

  uint16_t cv;      // control voltage
  uint16_t detune;  // detune

void timerHandler(void) {

	int sample = Phasedist_process(phasedist, cv , detune);

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

	cv = random(4096, 16384);
  detune = random(4096, 65535);
   
  int tempo = 60000 / BPM;
  delay(tempo / 4);

}