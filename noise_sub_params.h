#include <stdio.h>
#include "noise_suppression.h"


uint32_t sampleRate =16000;

//int frames = 160;
#define frames 160
uint64_t inSampleCount = frames;
int16_t buffer1[frames];

//NsHandle *nsHandle = WebRtcNs_Create();
//int status = WebRtcNs_Init(nsHandle, sampleRate);
