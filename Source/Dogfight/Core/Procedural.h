#pragma once

class FProcedural
{
public:
	static uint32 GenerateSeed(double TimeSeconds)
	{
		const int Seconds = FMath::FloorToInt(TimeSeconds);
		const int Rand = FMath::Rand();

		uint32 Combined = Seconds ^ Rand;

		// MurmurHash
		Combined = (Combined ^ (Combined >> 16)) * 0x85ebca6b;
		Combined = (Combined ^ (Combined >> 13)) * 0xc2b2ae35;
		Combined ^= (Combined >> 16);
	
		return Combined;
	}
};
