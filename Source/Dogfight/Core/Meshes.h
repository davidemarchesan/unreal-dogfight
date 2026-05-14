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
	ERockSizeCategory SizeCategory;
	
	UPROPERTY(EditAnywhere)
	float Radius;
	
	UPROPERTY(EditAnywhere)
	float Height;
	
	UPROPERTY(EditAnywhere)
	float MaxWide;
	
	UPROPERTY(EditAnywhere)
	float MaxNarrow;
};