// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMapActor.generated.h"

UCLASS()
class DOGFIGHT_API AProceduralMapActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AProceduralMapActor();
	virtual void Tick(float DeltaTime) override;

private:

	const int GridSize = 10;
	TArray<TArray<int32>> Grid;

	TArray<FVector> PointsList;
	TArray<FVector> ActiveList;

protected:
	virtual void BeginPlay() override;

public:

	void GenerateMap();
};
