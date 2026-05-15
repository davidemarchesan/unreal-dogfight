#pragma once

#include "Procedural.generated.h"

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

USTRUCT()
struct FGridCell
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY()
	int32 X = 0;
	
	UPROPERTY()
	int32 Y = 0;
	
	UPROPERTY()
	int32 Z = 0;
	
public:
	
	FGridCell() = default;
	
	FGridCell(const int32 InX, const int32 InY, const int32 InZ)
		: X(InX), Y(InY), Z(InZ)
	{
		
	}
};
