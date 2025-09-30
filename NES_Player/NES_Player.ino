// Nintendo NES player - pin PA3 audio output //

#include <HardwareTimer.h>
#include "Cartridge.h"
#include "Silver_Surfer.h"

HardwareTimer Timer(TIM1);

#define SAMPLE_RATE 44100

const int FIXED_BITS = 14;
const int FIXED_SCALE = (1 << FIXED_BITS);
const int REVERB_SIZE = 0x800;
const int REVERB_LENGTH = (int)(REVERB_SIZE * 0.99f);
const int REVERB_DECAY = (int)(FIXED_SCALE * 0.8f);

uint16_t reverbAddr;
uint16_t reverbBuffer[REVERB_SIZE];

Cartridge cart;

void timerHandler(void) {

    uint16_t sample = cart.sample_audio();

    int reverb = ((int)reverbBuffer[reverbAddr] * REVERB_DECAY) >> FIXED_BITS;
    reverb += sample;
    reverbBuffer[reverbAddr] = reverb;
    reverbAddr++;
    if (reverbAddr > REVERB_LENGTH) reverbAddr = 0;

    uint16_t output = sample + (reverbBuffer[reverbAddr]>>3);

	TIM2->CH4CVR = output >> 8;

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

    cart.play_nes(song);

}