// Granular noise synth with delay - pin PA3 audio output //

#include <HardwareTimer.h>

HardwareTimer Timer(TIM1);

#define SAMPLE_RATE 44100
#define BUFFER_SIZE 512
#define DELAY_BUFFER_SIZE 4096


uint8_t audioBuffer[BUFFER_SIZE];
uint8_t delayBuffer[DELAY_BUFFER_SIZE];
volatile uint16_t grainPosition = 0;
volatile uint16_t grainLength = 32 << 8;
volatile uint16_t grainSpeed = 1 << 8;
volatile uint8_t outputSample = 0;
volatile uint16_t delayIndex = 0;
volatile uint8_t feedbackGain = 50;


void timerHandler(void) {

    grainPosition += grainSpeed;

    if (grainPosition >= grainLength) grainPosition = (random(0, BUFFER_SIZE - (grainLength >> 8))) << 8;

    uint8_t index = grainPosition >> 8;
    outputSample = audioBuffer[index];

    uint8_t delayedSample = delayBuffer[delayIndex];
    uint16_t mixedSample = (uint16_t)-outputSample * (100 - feedbackGain) + (uint16_t)delayedSample * feedbackGain;
    delayBuffer[delayIndex] = (uint8_t)(mixedSample / 100);
    delayIndex = (delayIndex + 1) % DELAY_BUFFER_SIZE;

    outputSample = (outputSample >> 1) + (delayedSample >> 1);

	TIM2->CH4CVR = outputSample;

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

    for (int i = 0; i < BUFFER_SIZE; i++) {

        float t = (float)i / BUFFER_SIZE * 2 * PI;
        audioBuffer[i] = (uint8_t)(128.0f + 127.0f * sinf(t));

    }

}

void loop() {

    grainSpeed = (random(64, 1024)) << 6;
    grainLength = (random(16, 1024)) << 8;
    feedbackGain = random(0, 100);

    delay(random(1, 100));

}