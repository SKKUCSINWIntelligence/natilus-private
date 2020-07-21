#ifndef STRUCT_H
#define STRUCT_H

# define SEN_NUM 1000

namespace ns3 {

typedef struct State{
	uint32_t sampleRate[SEN_NUM] = {0}; // #/s
	double sampleValue[SEN_NUM] = {0};
	int sampleCar[1000] = {0}; // Car Cell
	uint64_t upInter[SEN_NUM] = {0}; // MilliSecond
	Time lastUpdateTime[SEN_NUM]; // MilliSecond
	uint32_t action[SEN_NUM] = {0}; // #/s 
}STATE;

}

#endif
