#pragma once

#include "Meshes.generated.h"

UENUM(BlueprintType)
enum class ERockSizeCategory : uint8
{
	Small,
	Medium,
	Large
};

UCLASS()
class DOGFIGHT_API URockDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere)
	UStaticMesh* StaticMesh;
	
	UPROPERTY(EditAnywhere)
	float Width;
	
	UPROPERTY(EditAnywhere)
	float Height;
	
};

USTRUCT(BlueprintType)
struct FRockDataRow : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<URockDataAsset> RockDataAsset;
	
	UPROPERTY(EditAnywhere)
	float Width;
	
	UPROPERTY(EditAnywhere)
	float Height;
	
};