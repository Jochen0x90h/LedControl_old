#pragma once

#include "Types.h"


struct ParameterInfo {
	const char * name;
	uint8_t minValue;
	uint8_t maxValue;
	uint8_t step;
	uint8_t initValue;
};
#define PARAMETER(name, min, max, step, init) {name, min, max, step, init}

// flag to add to step parameter to get wrap around when using digital potentiometer
const uint8_t WRAP = 0x80;


class Effect {
public:
	virtual ~Effect() {};
	virtual void run(int ledCount, uint8_t brightness, uint8_t * parameters) = 0;
};

using Constructor = Effect * (*)(void * data, int ledCount);

struct EffectInfo {
	const char * name;
	uint8_t parameterCount;
	const ParameterInfo * parameterInfos;
	Constructor construct;
};

template <typename E>
Effect * construct(void * data, int ledCount) {
	// https://stackoverflow.com/questions/765459/how-can-you-do-c-when-your-embedded-compiler-doesnt-have-operator-new-or-stl
	return new (data) E(ledCount);
}

#define EFFECT(name) {#name, sizeof(name##ParameterInfos) / sizeof(ParameterInfo), name##ParameterInfos,\
		(Constructor)&construct<name>}
