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

const uint8_t sineLUT[BUFFER_SIZE] = {

    128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
    144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
    160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
    176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
    192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
    208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
    224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
    240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,
    255,254,253,252,251,250,249,248,247,246,245,244,243,242,241,240,
    239,238,237,236,235,234,233,232,231,230,229,228,227,226,225,224,
    223,222,221,220,219,218,217,216,215,214,213,212,211,210,209,208,
    207,206,205,204,203,202,201,200,199,198,197,196,195,194,193,192,
    191,190,189,188,187,186,185,184,183,182,181,180,179,178,177,176,
    175,174,173,172,171,170,169,168,167,166,165,164,163,162,161,160,
    159,158,157,156,155,154,153,152,151,150,149,148,147,146,145,144,
    143,142,141,140,139,138,137,136,135,134,133,132,131,130,129,128,
    127,126,125,124,123,122,121,120,119,118,117,116,115,114,113,112,
    111,110,109,108,107,106,105,104,103,102,101,100, 99, 98, 97, 96,
     95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80,
     79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64,
     63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
    47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
    31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
    15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0,
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
    96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,
    112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127

};

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

    for (int i = 0; i < BUFFER_SIZE; i++) audioBuffer[i] = sineLUT[i];

}

void loop() {

    grainSpeed = (random(64, 1024)) << 6;
    grainLength = (random(16, 1024)) << 8;
    feedbackGain = random(0, 100);

    delay(random(1, 100));

}