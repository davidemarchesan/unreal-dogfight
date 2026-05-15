// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralMapActor.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Dogfight/Core/Meshes.h"
#include "Dogfight/Core/Procedural.h"

AProceduralMapActor::AProceduralMapActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	if (USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene")))
	{
		RootComponent = SceneComponent;
	}
}

bool AProceduralMapActor::IsInsideBounds(const FVector& Point)
{
	return Point.X >= 0.f && Point.X <= MaxBoundaries 
	&& Point.Y >= 0.f && Point.Y <= MaxBoundaries
	&& Point.Z >= 0.f && Point.Z <= MaxBoundaries;
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
	Grid[GridCell.X][GridCell.Y][GridCell.Z] = Index;
}

FGridCell AProceduralMapActor::GetGridCell(const FVector& Point)
{
	const int32 X = FMath::FloorToInt(Point.X / CellSize);
	const int32 Y = FMath::FloorToInt(Point.Y / CellSize);
	const int32 Z = FMath::FloorToInt(Point.Z / CellSize);
	
	// UE_LOG(LogTemp, Warning, TEXT("%s =  %d , %d"), *Point.ToString(), X, Y);
	
	return FGridCell(X, Y, Z);
}

void AProceduralMapActor::BeginPlay()
{
	Super::BeginPlay();
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
	for (int x = 0; x < GridSize; x++)
	{
		Grid[x].SetNum(GridSize);
		for (int y = 0; y < GridSize; y++)
		{
			Grid[x][y].Init(-1, GridSize);
		}
	}
	
	DrawDebugBox(GetWorld(), FVector(MaxBoundaries * .5f, MaxBoundaries * .5f, MaxBoundaries * .5f), FVector(MaxBoundaries * .5f, MaxBoundaries * .5f, MaxBoundaries * .5f), FColor::Red, true);
	
	// First random point
	const FVector StartingPoint = FVector(
		RandomNumberGenerator.FRandRange(0, MaxBoundaries),
		RandomNumberGenerator.FRandRange(0, MaxBoundaries),
		RandomNumberGenerator.FRandRange(0, MaxBoundaries)
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
			// const float RandomAngle = FMath::DegreesToRadians(RandomNumberGenerator.FRandRange(0, 360.f));
			const FVector RandomDirection = RandomNumberGenerator.GetUnitVector();
			
			const float RandomDistance = RandomNumberGenerator.FRandRange(Radius, RadiusTwice);
			const FVector PotentialCandidate = ActivePoint + (RandomDirection * RandomDistance);
			
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
				for (int x = -2; x <= 2; x++)
				{
					for (int y = -2; y <= 2; y++)
					{
						for (int z = -2; z <= 2; z++)
						{
							// Check out of bounds
							if ((GridCell.X + x) < 0 || (GridCell.X + x) >= GridSize || (GridCell.Y + y) < 0 || (GridCell.Y + y) >= GridSize || (GridCell.Z + z) < 0 || (GridCell.Z + z) >= GridSize) continue;
							
							const int32 NeighbourIndex = Grid[GridCell.X + x][GridCell.Y + y][GridCell.Z + z];
							if (NeighbourIndex == -1 || PointsList.IsValidIndex(NeighbourIndex) == false) continue;
							
							const FVector Neighbour = PointsList[NeighbourIndex];
							
							const float NeighbourDistance = FVector::DistSquared(PotentialCandidate, Neighbour);
							
							if (NeighbourDistance < (Radius * Radius))
							{
								bNeighboursAreOkay = false;
								break;
							}
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
	
	// Placing meshes
	FString TablePath = TEXT("/Game/Procedural/Rocks/Data/DT_RockCatalog.DT_RockCatalog");
	UDataTable* Table = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *TablePath));
	
	if (Table)
	{
		TArray<FRockDataRow*> Meshes;
		Table->GetAllRows("Map generation", Meshes);
		
		if (Meshes.IsEmpty() == false)
		{
			UE_LOG(LogTemp, Warning, TEXT("There are meshes"));
			
			// 1. Ensure we have a Root to attach to
			if (!GetRootComponent())
			{
				UE_LOG(LogTemp, Error, TEXT("NO ROOT COMPONENT FOUND! Spawning failed."));
				return;
			}
			
			// UHierarchicalInstancedStaticMeshComponent* TargetHISM = GetOrCreateHISM(Mesh);
			// If not, create a new HISM Component dynamically
			UHierarchicalInstancedStaticMeshComponent* NewHISM = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
    
			NewHISM->SetFlags(RF_Transactional);
			NewHISM->SetStaticMesh(Meshes[0]->RockDataAsset->StaticMesh);
			FBoxSphereBounds MeshBounds = NewHISM->GetStaticMesh()->GetBounds();

			UE_LOG(LogTemp, Warning,
				TEXT("Mesh bounds origin %s extent %s"),
				*MeshBounds.Origin.ToString(),
				*MeshBounds.BoxExtent.ToString());
			
			
			NewHISM->SetMobility(EComponentMobility::Movable); // Change to Movable if rocks drift
			UE_LOG(LogTemp, Warning, TEXT("HISM meshes %f %f"), Meshes[0]->RockDataAsset->Width, Meshes[0]->RockDataAsset->Height);
			UE_LOG(LogTemp, Warning, TEXT("Mesh Name: %s"), *Meshes[0]->RockDataAsset->StaticMesh->GetName());
			
			NewHISM->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			AddInstanceComponent(NewHISM);
			
			
			
			NewHISM->RegisterComponent();
			

			// Add to our library
			// HISMMap.Add(ForMesh, NewHISM);
			// return NewHISM;
    
			if (NewHISM)
			{
				FTransform InstanceTransform(FRotator::ZeroRotator, FVector::ZeroVector, FVector(1.f));
				int32 Index = NewHISM->AddInstance(InstanceTransform, true);
				NewHISM->UpdateBounds();
				// NewHISM->MarkRenderStateDirty();
				NewHISM->MarkRenderStateDirty();
				
				FBoxSphereBounds Bounds = NewHISM->Bounds;

				UE_LOG(LogTemp, Warning, TEXT("Bounds origin: %s extent: %s"),
					*Bounds.Origin.ToString(),
					*Bounds.BoxExtent.ToString());
				
				
				
				UE_LOG(LogTemp, Warning, TEXT("Created instance transform with index %d"), Index);
				UE_LOG(LogTemp, Warning, TEXT("HISM Registered: %s | Instance Index: %d"), NewHISM->IsRegistered() ? TEXT("TRUE") : TEXT("FALSE"), Index);
			}
		}
		
	}
	
}

void AProceduralMapActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
