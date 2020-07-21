#include "function.h"

namespace ns3{

uint32_t Byte2Bit (uint32_t byte)
{
	return byte*8;
}

uint32_t KByte2Bit (uint32_t kbyte)
{
	return kbyte*8*1024;
}

uint32_t MByte2Bit (uint32_t mbyte)
{
	return KByte2Bit (mbyte*1024);
}

double Bit2Mbps (uint32_t bit)
{
	return (double) bit / 1000000;
}

}
