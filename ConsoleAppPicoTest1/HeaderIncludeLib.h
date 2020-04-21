#include <stdio.h>

 /* Headers for Windows */
#ifdef _WIN32
#include "windows.h"
#include <conio.h>
#include "ps5000aApi.h"
#else
#include <sys/types.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#include <libps5000a-1.1/ps5000aApi.h>
#ifndef PICO_STATUS
#include <libps5000a-1.1/PicoStatus.h>
#endif

#define Sleep(a) usleep(1000*a)
#define scanf_s scanf
#define fscanf_s fscanf
#define memcpy_s(a,b,c,d) memcpy(a,c,d)

typedef enum enBOOL { FALSE, TRUE } BOOL;

/* A function to detect a keyboard press on Linux */
int32_t _getch()
{
	struct termios oldt, newt;
	int32_t ch;
	int32_t bytesWaiting;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	setbuf(stdin, NULL);
	do {
		ioctl(STDIN_FILENO, FIONREAD, &bytesWaiting);
		if (bytesWaiting)
			getchar();
	} while (bytesWaiting);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return ch;
}

int32_t _kbhit()
{
	struct termios oldt, newt;
	int32_t bytesWaiting;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	setbuf(stdin, NULL);
	ioctl(STDIN_FILENO, FIONREAD, &bytesWaiting);

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return bytesWaiting;
}

int32_t fopen_s(FILE** a, const int8_t* b, const int8_t* c)
{
	FILE* fp = fopen(b, c);
	*a = fp;
	return (fp > 0) ? 0 : -1;
}

/* A function to get a single character on Linux */
#define max(a,b) ((a) > (b) ? a : b)
#define min(a,b) ((a) < (b) ? a : b)
#endif



#define BUFFER_SIZE 	1024

#define QUAD_SCOPE		4
#define DUAL_SCOPE		2

#define MAX_PICO_DEVICES 64
#define TIMED_LOOP_STEP 500

typedef struct
{
	int16_t DCcoupled;
	int16_t range;
	int16_t enabled;
	float analogueOffset;
}CHANNEL_SETTINGS;

typedef enum
{
	MODEL_NONE = 0,
	MODEL_PS5242A = 0xA242,
	MODEL_PS5242B = 0xB242,
	MODEL_PS5243A = 0xA243,
	MODEL_PS5243B = 0xB243,
	MODEL_PS5244A = 0xA244,
	MODEL_PS5244B = 0xB244,
	MODEL_PS5442A = 0xA442,
	MODEL_PS5442B = 0xB442,
	MODEL_PS5443A = 0xA443,
	MODEL_PS5443B = 0xB443,
	MODEL_PS5444A = 0xA444,
	MODEL_PS5444B = 0xB444
} MODEL_TYPE;

typedef enum
{
	SIGGEN_NONE = 0,
	SIGGEN_FUNCTGEN = 1,
	SIGGEN_AWG = 2
} SIGGEN_TYPE;

typedef struct tPwq
{
	PS5000A_CONDITION* pwqConditions;
	int16_t nPwqConditions;
	PS5000A_DIRECTION* pwqDirections;
	int16_t nPwqDirections;
	uint32_t lower;
	uint32_t upper;
	PS5000A_PULSE_WIDTH_TYPE type;
}PWQ;

typedef struct
{
	int16_t handle;
	MODEL_TYPE				model;
	int8_t						modelString[8];
	int8_t						serial[10];
	int16_t						complete;
	int16_t						openStatus;
	int16_t						openProgress;
	PS5000A_RANGE			firstRange;
	PS5000A_RANGE			lastRange;
	int16_t						channelCount;
	int16_t						maxADCValue;
	SIGGEN_TYPE				sigGen;
	int16_t						hasHardwareETS;
	uint16_t					awgBufferSize;
	CHANNEL_SETTINGS	channelSettings[PS5000A_MAX_CHANNELS];
	PS5000A_DEVICE_RESOLUTION	resolution;
	int16_t						digitalPortCount;
}UNIT;





//int8_t blockFile[20] = "block.txt";
//int8_t streamFile[20] = "stream.txt";

typedef struct tBufferInfo
{
	UNIT* unit;
	int16_t** driverBuffers;
	int16_t** appBuffers;

} BUFFER_INFO;