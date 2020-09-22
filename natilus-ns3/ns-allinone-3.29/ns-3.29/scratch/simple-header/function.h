#ifndef FUNCTION_H
#define FUNCTION_H

#include "ns3/core-module.h"

#include <iostream>
#include <string>
#include <list>
#include <queue>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <utility>
#include <chrono>
#include <iomanip>

#define PI 3.14159265358979323846

namespace ns3{

typedef struct Data
{
	// App Header
	uint32_t dataSize;
	Time genTime;
	uint32_t serId;
	uint32_t cellId;

	/* Paylaod */
	// Sensor case
	uint32_t sampleRate;
	double sampleValue;
	bool *carCell;

	// Sink case
	uint32_t action;
}DATA;

uint32_t Byte2Bit (uint32_t byte);
uint32_t KByte2Bit (uint32_t kbyte);
uint32_t MByte2Bit (uint32_t mbyte);
double Bit2Mbps (uint64_t bit);


/************
* Template
************/

// Array Initialization
template <typename T>
void Arr1Ini (T *arr, uint32_t n, T value)
{
	for (uint32_t i=0; i<n; i++)
		arr[i] = value;
}

template <typename T>
void Arr2Ini (T **arr, uint32_t n1, uint32_t n2, T value)
{
	for (uint32_t i=0; i<n1; i++)
	{
		for (uint32_t j=0; j<n2; j++)
			arr[i][j] = value;
	}
}

// Array Create
template <typename T>
T* Arr1Create (uint32_t n, T value)
{
	T *arr = new T[n];

	for (uint32_t i=0; i<n;i++)
		arr[i] = value;

	return arr;
}

template <typename T>
T** Arr2Create (uint32_t n1, uint32_t n2, T value)
{
	T **arr = new T*[n1];
	for (uint32_t i=0; i<n1; i++)
	{
		arr[i] = new T[n2];
	}

	for (uint32_t i=0; i<n1; i++)
	{
		for (uint32_t j=0; j<n2; j++)
		{
			arr[i][j] = value;
		}
	}

	return arr;
}

// Array Delete
template <typename T>
void Arr2Delete (T **arr, uint32_t n)
{
	for (uint32_t i=0; i<n; i++)
		delete[] arr[i];
	delete[] arr;
}

/*
// Array VInitialize
template <typename T>
void Arr1Init (T *arr, uint32_t n, T value)
{
	for (ui
}
*/


// Print State
template <typename T>
void PrintState (T *arr, uint32_t n)
{
	std::cout.setf(std::ios::right);
	std::cout.precision(4); 
	for (uint32_t i=0; i<n; i++)
	{
		uint32_t unit = sqrt (n);
		std::cout << std::setw(6) << arr[i] << "\t";

		if ( (i+1) % unit == 0)
			printf("\n");
	}
	printf("\n");
}

template <typename T>
void PrintState (uint32_t serId, std::string str, T *arr, uint32_t n)
{
	std::cout << "\n[State]::Ser:"<< serId << ":" << str << std::endl;
	std::cout.setf(std::ios::right);
	std::cout.precision(4); 
	for (uint32_t i=0; i<n; i++)
	{
		uint32_t unit = sqrt (n);
		std::cout << std::setw(6) << arr[i] <<"\t";

		if ( (i+1) % unit == 0)
			printf("\n");
	}
	printf("\n");
}

}

#endif
