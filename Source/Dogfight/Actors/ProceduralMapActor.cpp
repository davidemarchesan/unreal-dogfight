// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralMapActor.h"

#include "OnlineSubsystemTypes.h"
#include "Dogfight/Core/Procedural.h"

AProceduralMapActor::AProceduralMapActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

bool AProceduralMapActor::IsInsideBounds(const FVector& Point)
{
	return Point.X >= 0.f && Point.X <= MaxBoundaries && Point.Y >= 0.f && Point.Y <= MaxBoundaries;
}

void AProceduralMapActor::MarkPoint(const FVector& Point)
{
	DrawDebugSphere(
			GetWorld(),
			Point,
			10.f,
			10,
			FColor::Yellow,
			true);
}

void AProceduralMapActor::AddPointToActiveList(const FVector& Point)
{
	ActivePointsList.Add(Point);
}

void AProceduralMapActor::AddPointToGrid(const FVector& Point)
{
	const int32 Index = PointsList.Add(Point);
	
	const FGridCell GridCell = GetGridCell(Point);
	Grid[GridCell.X][GridCell.Y] = Index;
}

FGridCell AProceduralMapActor::GetGridCell(const FVector& Point)
{
	const int32 X = FMath::FloorToInt(Point.X / CellSize);
	const int32 Y = FMath::FloorToInt(Point.Y / CellSize);
	
	// UE_LOG(LogTemp, Warning, TEXT("%s =  %d , %d"), *Point.ToString(), X, Y);
	
	return FGridCell(X, Y);
}

void AProceduralMapActor::BeginPlay()
{
	Super::BeginPlay();

	GenerateMap();
}

void AProceduralMapActor::GenerateMap()
{
	// (Fast) Poisson disk sampling
	uint32 Seed = FProcedural::GenerateSeed(GetWorld()->GetTimeSeconds());
	FRandomStream RandomNumberGenerator(Seed);
	
	UE_LOG(LogTemp, Warning, TEXT("Radius: %f"), Radius);
	UE_LOG(LogTemp, Warning, TEXT("RadiusTwice: %f"), RadiusTwice);
	UE_LOG(LogTemp, Warning, TEXT("CellSize: %f"), CellSize);
	UE_LOG(LogTemp, Warning, TEXT("MaxBoundaries: %f"), MaxBoundaries);
	
	const int MaxCandidatesNum = 30; // k (number of attempts)

	// Initializing grid
	Grid.SetNum(GridSize);
	for (int i = 0; i < GridSize; i++)
	{
		Grid[i].Init(-1, GridSize);
	}
	
	DrawDebugBox(GetWorld(), FVector(MaxBoundaries * .5f, MaxBoundaries * .5f, 0.f), FVector(MaxBoundaries * .5f, MaxBoundaries * .5f, 0.f), FColor::Red, true);
	
	// First random point
	const FVector StartingPoint = FVector(
		RandomNumberGenerator.FRandRange(0, MaxBoundaries),
		RandomNumberGenerator.FRandRange(0, MaxBoundaries),
		0
		);
	
	MarkPoint(StartingPoint);
	AddPointToGrid(StartingPoint);
	AddPointToActiveList(StartingPoint);
	
	while (ActivePointsList.Num() > 0)
	{
		
		// Pick random point from list
		const int32 RandomIndex = FMath::RandRange(0, ActivePointsList.Num() - 1);
		const FVector ActivePoint = ActivePointsList[RandomIndex];
		
		for (int k = 0; k < MaxCandidatesNum; k++)
		{
			
			// Random angle of direction
			const float RandomAngle = FMath::DegreesToRadians(RandomNumberGenerator.FRandRange(0, 360.f));
			const FVector RandomDirection(
				FMath::Cos(RandomAngle),
				FMath::Sin(RandomAngle),
				0.f
				);
			
			const float RandomDistance = RandomNumberGenerator.FRandRange(Radius, RadiusTwice);
			const FVector PotentialCandidate = ActivePoint + (RandomDirection * RandomDistance);
			
			bool bCandidateIsValid = false;

			// UE_LOG(
			// 	LogTemp, 
			// 	Warning, 
			// 	TEXT("Potential candidate position: %s | Distance: %f | Bounds: %d | Greater than radius: %d"), 
			// 	*PotentialCandidate.ToString(), 
			// 	RandomDistance,
			// 	bIsInsideBounds,
			// 	RandomDistance > Radius
			// 	);
			
			if (IsInsideBounds(PotentialCandidate)) 
			{
				// Check neighbor cells
				const FGridCell GridCell = GetGridCell(PotentialCandidate);
				
				bool bNeighboursAreOkay = true;
				for (int i = -2; i <= 2; i++)
				{
					for (int j = -2; j <= 2; j++)
					{
						// Check out of bounds
						if ((GridCell.X + i) < 0 || (GridCell.X + i) >= GridSize || (GridCell.Y + j) < 0 || (GridCell.Y + j) >= GridSize) continue;
						
						const int32 NeighbourIndex = Grid[GridCell.X + i][GridCell.Y + j];
						if (NeighbourIndex == -1 || PointsList.IsValidIndex(NeighbourIndex) == false) continue;
						
						const FVector Neighbour = PointsList[NeighbourIndex];
						
						const float NeighbourDistance = FVector::DistSquared(PotentialCandidate, Neighbour);
						
						if (NeighbourDistance < (Radius * Radius))
						{
							bNeighboursAreOkay = false;
							break;
						}
						
					}
					
					if (bNeighboursAreOkay == false)
					{
						break;
					}
				}
				
				// Pass checks, candidate is valid
				if (bNeighboursAreOkay == true)
				{
					MarkPoint(PotentialCandidate);
					AddPointToGrid(PotentialCandidate);
					AddPointToActiveList(PotentialCandidate);
					
					break;
				}
			}
			
			// Remove it only if we didnt find any valid candidate within the K attempts
			if (k == (MaxCandidatesNum - 1))
			{
				ActivePointsList.RemoveAtSwap(RandomIndex);
			}
		
		}
		
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Created %d points"), PointsList.Num());
}

void AProceduralMapActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
