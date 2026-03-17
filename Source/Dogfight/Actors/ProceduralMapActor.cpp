// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralMapActor.h"

#include "Dogfight/Core/Procedural.h"

AProceduralMapActor::AProceduralMapActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AProceduralMapActor::BeginPlay()
{
	Super::BeginPlay();

	GenerateMap();
}

void AProceduralMapActor::GenerateMap()
{
	// Poisson disk sampling
	uint32 Seed = FProcedural::GenerateSeed(GetWorld()->GetTimeSeconds());
	FRandomStream RandomNumberGenerator(Seed);

	const float Radius = 400.f; // Min distance between two points
	const float TwoRadius = Radius * 2.f;
	const float CellSize = Radius / FMath::Sqrt(2.f); // Diagonal distance to avoid two points in the same cell

	UE_LOG(LogTemp, Warning, TEXT("Radius: %f"), Radius);
	UE_LOG(LogTemp, Warning, TEXT("TwoRadius: %f"), TwoRadius);
	UE_LOG(LogTemp, Warning, TEXT("CellSize: %f"), CellSize);
	
	const int CandidatesNum =3;

	// Initializing grid
	Grid.SetNum(GridSize);
	for (int i = 0; i < GridSize; i++)
	{
		Grid[i].SetNum(GridSize);
	}

	// First point
	FVector StartingPoint = FVector(CellSize * 5.f, CellSize * 5.f, 0);
	UE_LOG(LogTemp, Warning, TEXT("StartingPoint position: %s"), *StartingPoint.ToString());


	const int32 PointsListIndex = PointsList.Add(StartingPoint);
	Grid[0][0] = PointsListIndex;

	ActiveList.Add(StartingPoint);

	DrawDebugSphere(
			GetWorld(),
			StartingPoint,
			10.f,
			10,
			FColor::Yellow,
			true);

	for (int k = 0; k < CandidatesNum; k++)
	{

		// Random angle of direction
		const float Angle = FMath::DegreesToRadians(RandomNumberGenerator.FRandRange(0, 360.f));

		const FVector Direction(
			FMath::Cos(Angle),
			FMath::Sin(Angle),
			0.f
			);
		
		const float Distance = RandomNumberGenerator.FRandRange(Radius, TwoRadius);

		FVector Candidate = StartingPoint + (Direction * Distance);
		UE_LOG(LogTemp, Warning, TEXT("Candidate position: %s | Distance: %f"), *Candidate.ToString(), Distance);

		DrawDebugSphere(
			GetWorld(),
			Candidate,
			10.f,
			10,
			FColor::Red,
			true);

		// Get candidate cell x and y
		const int X = FMath::FloorToInt(Candidate.X / CellSize);
		const int Y = FMath::FloorToInt(Candidate.Y / CellSize);

		UE_LOG(LogTemp, Warning, TEXT("Candidate x and y: %d %d"), X, Y);
		
		// Check candidate neighbours distance
		DrawDebugBox(
			GetWorld(),
			(FVector(X, Y, 0.f) * CellSize) + CellSize * 0.5f,
			FVector(CellSize * 0.5f),
			FColor::Red,
			true
			);
		// If every neighbour distance >= Radius => ok
	}
}

void AProceduralMapActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
