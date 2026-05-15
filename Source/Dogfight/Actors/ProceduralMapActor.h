// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Dogfight/Core/Procedural.h"
#include "GameFramework/Actor.h"
#include "ProceduralMapActor.generated.h"

class UInstancedStaticMeshComponent;

UCLASS()
class DOGFIGHT_API AProceduralMapActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AProceduralMapActor();
	virtual void Tick(float DeltaTime) override;

private:

	const int GridSize = 10;
	TArray<TArray<TArray<int32>>> Grid;
	
	float Radius = 10000.f;								// Min distance between two points
	float RadiusTwice = Radius * 2.f;
	float CellSize = Radius / FMath::Sqrt(3.f);	// Diagonal distance to avoid two points in the same cell
	float MaxBoundaries = CellSize * GridSize;

	TArray<FVector> PointsList;
	TArray<FVector> ActivePointsList;
	
	bool IsInsideBounds(const FVector& Point);
	void MarkPoint(const FVector& Point);
	void AddPointToActiveList(const FVector& Point);
	void AddPointToGrid(const FVector& Point);
	
	FGridCell GetGridCell(const FVector& Point);

protected:
	virtual void BeginPlay() override;

public:

	UFUNCTION(CallInEditor, Category = "Generation")
	void GenerateMap();
};
