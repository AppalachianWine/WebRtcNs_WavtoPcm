#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//ref:https://github.com/mackron/dr_libs/blob/master/dr_wav.h
#include "noise_sub_params.h"

#define DR_WAV_IMPLEMENTATION

#include "timing.h"
#include "dr_wav.h"
#include "noise_suppression.h"

#ifndef nullptr
#define nullptr 0
#endif

#ifndef MIN
#define MIN(A, B)        ((A) < (B) ? (A) : (B))
#endif

//写wav文件
void wavWrite_int16(char *filename, int16_t *buffer, size_t sampleRate, size_t totalSampleCount) {
    drwav_data_format format = {};
    format.container = drwav_container_riff;     // <-- drwav_container_riff = normal WAV files, drwav_container_w64 = Sony Wave64.
    format.format = DR_WAVE_FORMAT_PCM;          // <-- Any of the DR_WAVE_FORMAT_* codes.
    format.channels = 1;
    format.sampleRate = (drwav_uint32) sampleRate;
    format.bitsPerSample = 16;
    drwav *pWav = drwav_open_file_write(filename, &format);
    if (pWav) {
        drwav_uint64 samplesWritten = drwav_write(pWav, totalSampleCount, buffer);
        drwav_uninit(pWav);
        if (samplesWritten != totalSampleCount) {
            fprintf(stderr, "ERROR\n");
            exit(1);
        }
    }
	free(pWav);//
}

//读取wav文件
int16_t *wavRead_int16(char *filename, uint32_t *sampleRate, uint64_t *totalSampleCount) {
    unsigned int channels;
    int16_t *buffer = drwav_open_and_read_file_s16(filename, &channels, sampleRate, totalSampleCount);
    if (buffer == nullptr) {
        printf("ERROR.");
    }
    return buffer;
}

//分割路径函数
void splitpath(const char *path, char *drv, char *dir, char *name, char *ext) {
    const char *end;
    const char *p;
    const char *s;
    if (path[0] && path[1] == ':') {
        if (drv) {
            *drv++ = *path++;
            *drv++ = *path++;
            *drv = '\0';
        }
    } else if (drv)
        *drv = '\0';
    for (end = path; *end && *end != ':';)
        end++;
    for (p = end; p > path && *--p != '\\' && *p != '/';)
        if (*p == '.') {
            end = p;
            break;
        }
    if (ext)
        for (s = end; (*ext = *s++);)
            ext++;
    for (p = end; p > path;)
        if (*--p == '\\' || *p == '/') {
            p++;
            break;
        }
    if (name) {
        for (s = p; s < end;)
            *name++ = *s++;
        *name = '\0';
    }
    if (dir) {
        for (s = path; s < p;)
            *dir++ = *s++;
        *dir = '\0';
    }
}

enum nsLevel {
    kLow,
    kModerate,
    kHigh,
    kVeryHigh
};


int nsProcess(int16_t *buffer, uint32_t sampleRate, uint64_t inSampleCount,NsHandle *nsHandle,int16_t status ) {//,  enum nsLevel level
    if (buffer == nullptr) return -1;
    if (inSampleCount == 0) return -1;
    size_t samples = MIN(160, sampleRate / 100);
    if (samples == 0) return -1;
    uint32_t num_bands = 1;
    int16_t *input = buffer;
    //size_t nTotal = (inSampleCount / samples);

	
    if (status != 0) {
        printf("WebRtcNs_Init fail\n");
        return -1;
    }
    
    if (status != 0) {
        printf("WebRtcNs_set_policy fail\n");
        return -1;
    }
	int16_t *nsIn[1] = {input};   //ns input[band][data]
	int16_t *nsOut[1] = {input};  //ns output[band][data]
	WebRtcNs_Analyze(nsHandle, nsIn[0]);
	WebRtcNs_Process(nsHandle, (const int16_t *const *) nsIn, num_bands, nsOut);

    return 1;
}

void noise_suppression(int16_t *buffer1, uint32_t sampleRate, uint64_t inSampleCount, NsHandle *nsHandle,int16_t status){

		//if (buffer1 != nullptr) {
		double startTime = now();
		nsProcess(buffer1, sampleRate, inSampleCount, nsHandle,status);
		double time_interval = calcElapsed(startTime, now());
		printf("time interval: %d ms\n ", (int) (time_interval * 1000));
		

}

int main(int argc, char *argv[]) {
    printf("Start WebRtc Noise Suppression\n");
    if (argc < 2)
        return -1;
	
	FILE *fp1;
	FILE *fp;
	
	NsHandle *nsHandle = WebRtcNs_Create();
	int status = WebRtcNs_Init(nsHandle, sampleRate);
	
	fp1=fopen(argv[1],"rb+");
	memset(buffer1, 0, sizeof(buffer1));
	fread (&buffer1, 2, 22, fp1);
	char name[128] = {0};
	sprintf(name, argv[2]);
	fp=fopen(name, "wb");

	status = WebRtcNs_set_policy(nsHandle, kModerate);
	
	while(!feof(fp1)){
		fread (&buffer1, 2,frames,fp1);
		noise_suppression( buffer1,  sampleRate,  inSampleCount, nsHandle, status);
		fwrite(buffer1, 1, frames*2, fp);
	}	
	
	WebRtcNs_Free(nsHandle);
	
	free(fp1);
	free(fp);
		
    printf("press any key to exit. \n");
    getchar();
    return 0;
}
