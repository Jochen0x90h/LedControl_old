#pragma once

#include "util.h"
#include "types.h"


typedef struct {
	#ifndef NO_NAMES
	char FLASH * name;
	#endif
	uint8_t minValue;
	uint8_t maxValue;
	uint8_t step;
	uint8_t initValue;
} ParameterInfo;

// flag to add to step parameter to get wrap around when using digital potentiometer
const uint8_t WRAP = 0x80;


typedef void (*Init)(void * data, int ledCount);
typedef void (*Run)(void * data, int ledCount, uint8_t brightness, uint8_t * parameters);

typedef struct {
	#ifndef NO_NAMES
	char FLASH * name;
	#endif
	uint8_t parameterCount;
	ParameterInfo FLASH * parameterInfos;
	Init init;
	Run run;
} EffectInfo;

#ifdef NO_NAMES
	#define PARAMETER(name, min, max, step, init) {min, max, step, init}
	#define EFFECT(name) {sizeof(name##ParameterInfos) / sizeof(ParameterInfo), name##ParameterInfos,\
			(Init)name##Init, (Run)name##Run}
#else
	#define PARAMETER(name, min, max, step, init) {name, min, max, step, init}
	#define EFFECT(name) {#name, sizeof(name##ParameterInfos) / sizeof(ParameterInfo), name##ParameterInfos,\
			(Init)name##Init, (Run)name##Run}
#endif
