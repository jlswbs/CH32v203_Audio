// PT3 chiptune player - pin PA3 audio output //

#include <HardwareTimer.h>
#include "ay_emu.h"
#include "acidgoa.h"

HardwareTimer Timer(TIM1);

#define SAMPLE_RATE 44100
#define AY_CLOCK    1750000
#define FRAME_RATE  50

int interruptCnt;

struct PT3_Channel_Parameters
{
  unsigned short Address_In_Pattern, OrnamentPointer, SamplePointer, Ton;
  unsigned char Loop_Ornament_Position, Ornament_Length, Position_In_Ornament, Loop_Sample_Position, Sample_Length, Position_In_Sample, Volume, Number_Of_Notes_To_Skip, Note, Slide_To_Note, Amplitude;
  bool Envelope_Enabled, Enabled, SimpleGliss;
  short Current_Amplitude_Sliding, Current_Noise_Sliding, Current_Envelope_Sliding, Ton_Slide_Count, Current_OnOff, OnOff_Delay, OffOn_Delay, Ton_Slide_Delay, Current_Ton_Sliding, Ton_Accumulator, Ton_Slide_Step, Ton_Delta;
  signed char Note_Skip_Counter;
};

struct PT3_Parameters
{
  unsigned char Env_Base_lo;
  unsigned char Env_Base_hi;
  short Cur_Env_Slide, Env_Slide_Add;
  signed char Cur_Env_Delay, Env_Delay;
  unsigned char Noise_Base, Delay, AddToNoise, DelayCounter, CurrentPosition;
  int Version;
};

struct PT3_SongInfo
{
  PT3_Parameters PT3;
  PT3_Channel_Parameters PT3_A, PT3_B, PT3_C;
};

struct AYSongInfo
{
  unsigned char* module;
  unsigned char* module1;
  int module_len;
  PT3_SongInfo data;
  PT3_SongInfo data1;
  bool is_ts;

  AYChipStruct chip0;
};

struct AYSongInfo AYInfo;

void ay_resetay(AYSongInfo* info, int chipnum)
{
  ay_init(&info->chip0);
}

void ay_writeay(AYSongInfo* info, int reg, int val, int chipnum)
{
  ay_out(&info->chip0, reg, val);
}

#include "PT3Play.h"

void timerHandler(void) {

  uint32_t out;

  if (interruptCnt++ >= (SAMPLE_RATE / FRAME_RATE)) {

    PT3_Play_Chip(AYInfo, 0);
    interruptCnt = 0;
  
  }

  ay_tick(&AYInfo.chip0, (AY_CLOCK / SAMPLE_RATE / 8));
  out = AYInfo.chip0.out[0] + AYInfo.chip0.out[2] + AYInfo.chip0.out[1];
  
	TIM2->CH4CVR = out >> 9;

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

  memset(&AYInfo, 0, sizeof(AYInfo));

  ay_init(&AYInfo.chip0);

  AYInfo.module = music_data;
  AYInfo.module_len = music_data_size;

  PT3_Init(AYInfo);

}

void loop() {

}