#include "ProceduralMeshActor.h"

#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "StaticMeshAttributes.h"
#include "StaticMeshDescription.h"
#include "SWarningOrErrorBox.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Dogfight/Core/MarchingCubes.h"
#include "Dogfight/Core/Meshes.h"
#include "Dogfight/Core/Procedural.h"
#include "UObject/SavePackage.h"

AProceduralMeshActor::AProceduralMeshActor()
{
	PrimaryActorTick.bCanEverTick = true;

	if (USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene")))
	{
		RootComponent = SceneComponent;
	}

	ProcMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	if (ProcMeshComponent)
	{
		ProcMeshComponent->SetupAttachment(RootComponent);
	}
}

void AProceduralMeshActor::GenerateMesh()
{

	// Reset
	if (ProcMeshComponent == nullptr)
	{
		return;
	}

	ProcMeshComponent->ClearAllMeshSections();
	MeshData.Reset();

	// Generate
	uint32 Seed = FProcedural::GenerateSeed(GetWorld()->GetTimeSeconds());
	
	UE_LOG(LogTemp, Warning, TEXT("Generating mesh with seed %u"), Seed);

	FRandomStream RandomNumberGenerator(Seed);
	
	Radius = RandomNumberGenerator.FRandRange(100.f, 500.f);
	const float Range = Radius * 5;

	Height = RandomNumberGenerator.FRandRange(0.3f, 1.2f);
	MaxWide = RandomNumberGenerator.FRandRange(0.9f, 1.4f);
	MaxNarrow = RandomNumberGenerator.FRandRange(0.7f, 1.f);

	UE_LOG(LogTemp, Warning, TEXT("Radius %f \nRange %f \nHeight %f \nMaxWide %f \nMaxNarrow %f"), Radius, Range, Height, MaxWide, MaxNarrow);
	
	auto Sdf = FSdfRockBase(Radius,  Height, MaxWide, MaxNarrow);
	
	constexpr float Step = 50.f;
	constexpr float HalfStep = Step * 0.5f;

	// Internal diagonal of voxel (cube) + a little bit more space
	float MinDistance = FMath::Sqrt(3.f) * Step + 2.f;

	int32 Skipped = 0;

	for (float x = -Range; x <= Range; x += Step)
		for (float y = -Range; y <= Range; y += Step)
			for (float z = -Range; z <= Range; z += Step)
			{
				
				// Voxel's center
				FVector Point(x, y, z);
				const float VoxelDistance = Sdf.Evaluate(Point);

				// Is it worth to check 8 vertices? Is this voxel even near the surface?
				if (FMath::Abs(VoxelDistance) > MinDistance)
				{
					Skipped++;
					continue;
				}

				int VerticesInsideNum = 0;
				int VerticesByte[8] = {0, 0, 0, 0, 0, 0, 0, 0};
				uint8_t VerticesConfigIndex = 0;

				FVector EdgeVertex[12];

				// 8 vertices of voxel
				TArray<FVector> Vertices = {
					FVector(x - HalfStep, y - HalfStep, z - HalfStep),
					FVector(x - HalfStep, y + HalfStep, z - HalfStep),
					FVector(x - HalfStep, y + HalfStep, z + HalfStep),
					FVector(x - HalfStep, y - HalfStep, z + HalfStep),
					FVector(x + HalfStep, y - HalfStep, z - HalfStep),
					FVector(x + HalfStep, y + HalfStep, z - HalfStep),
					FVector(x + HalfStep, y + HalfStep, z + HalfStep),
					FVector(x + HalfStep, y - HalfStep, z + HalfStep),
				};

				for (int i = 0; i < Vertices.Num(); i++)
				{
					const float VertexDistance = Sdf.Evaluate(Vertices[i]);
					if (VertexDistance < 0.f)
					{
						VerticesInsideNum++;
						VerticesByte[i] = 1;
						VerticesConfigIndex |= (1 << i);
					}
					else
					{
						VerticesByte[i] = 0;
						VerticesConfigIndex |= (0 << i);
					}
				}

				if (VerticesInsideNum > 0 && VerticesInsideNum < 8)
				{
					for (int32 i = 0; i < 12; i++)
					{
						const int* EdgeIndices = FMarchingCubes::EdgeIndices[i];
						if (VerticesByte[EdgeIndices[0]] != VerticesByte[EdgeIndices[1]])
						{
							// Calculate interpolation between the two vertices
							// A + (A - B) * t = 0
							// => when t == 0
							// => when we are on the surface
							// t = A / (A - B)
							const float DistanceA = Sdf.Evaluate(Vertices[EdgeIndices[0]]);
							const float DistanceB = Sdf.Evaluate(Vertices[EdgeIndices[1]]);
							const float t = DistanceA / (DistanceA - DistanceB);

							// A + (A - B) * t
							FVector TriangleVertex = Vertices[EdgeIndices[0]] + (Vertices[EdgeIndices[1]] - Vertices[
								EdgeIndices[0]]) * t;

							// Cache interpolated point
							// This point will be part of the triangle(s)
							EdgeVertex[i] = TriangleVertex;
							
						}
					}

					// 256 possible fixed-configurations
					const int* Triangle = FMarchingCubes::TriTable[VerticesConfigIndex];
					for (int32 i = 0; i < 16; i++)
					{
						if (Triangle[i] == -1)
						{
							break;
						}
						
						const FVector Normal = CalcNormal(Sdf, EdgeVertex[Triangle[i]]);
						const FProcMeshTangent Tangent = CalcTangent(Normal);

						// todo: vertices deduplication
						const int32 VertexIndex = MeshData.AddVertex(
							EdgeVertex[Triangle[i]],
							Normal,
							FVector2D(0, 0),
							FColor::White,
							Tangent
						);
						MeshData.AddTriangle(VertexIndex);
					}
					
				}
			}

	UE_LOG(LogTemp, Warning, TEXT("Total vertices: %d"), MeshData.Vertices.Num());
	UE_LOG(LogTemp, Warning, TEXT("Total triangles: %d"), MeshData.Triangles.Num() / 3);
	UE_LOG(LogTemp, Warning, TEXT("Skipped: %d"), Skipped);

	ProcMeshComponent->CreateMeshSection(
		0,
		MeshData.Vertices,
		MeshData.Triangles,
		MeshData.Normals,
		MeshData.UV0,
		MeshData.VertexColors,
		MeshData.Tangents,
		true
	);
	
	bGenerated = true;
}

void AProceduralMeshActor::SaveMesh()
{
	
	if (bGenerated == false)
	{
		return;
	}
	
	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	FAssetRegistryModule& AssetRegistryModule= FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	
	// ID of the rock
	const FGuid Guid = FGuid::NewGuid();
	
	// Preparing static mesh and data asset packages
	const FString MeshPackageName = TEXT("/Game/Procedural/Rocks/StaticMeshes/SM_Rock_") + Guid.ToString();
	const FString MeshName = FPackageName::GetLongPackageAssetName(MeshPackageName);
	const FString MeshPackagePath = FPackageName::GetLongPackagePath(MeshPackageName);
	
	const FString DataPackageName = TEXT("/Game/Procedural/Rocks/Data/DataAssets/DA_Rock_") + Guid.ToString();
	const FString DataName = FPackageName::GetLongPackageAssetName(DataPackageName);
	const FString DataPackagePath = FPackageName::GetLongPackagePath(DataPackageName);
	
	UPackage* MeshPackage = CreatePackage(*MeshPackageName);
	MeshPackage->FullyLoad();
	
	UPackage* DataPackage = CreatePackage(*DataPackageName);
	DataPackage->FullyLoad();
	
	UStaticMesh* NewMeshAsset = NewObject<UStaticMesh>(MeshPackage, *MeshName, RF_Public | RF_Standalone);
	URockDataAsset* NewDataAsset = NewObject<URockDataAsset>(DataPackage, *DataName, RF_Public | RF_Standalone);
	// End preparing static mesh and data asset packages
	
	// Building static mesh
	UStaticMeshDescription* StaticMeshDesc = NewObject<UStaticMeshDescription>();
	FMeshDescription& MeshDesc = StaticMeshDesc->GetMeshDescription();
	
	FStaticMeshAttributes MeshAttr(MeshDesc);
	MeshAttr.Register();
	
	auto VertexPositions = MeshAttr.GetVertexPositions();
	auto Normals = MeshAttr.GetVertexInstanceNormals();
	auto UVs = MeshAttr.GetVertexInstanceUVs();
	
	// Vertices
	TArray<FVertexID> VertexIds;
	for (const FVector& Vertex : MeshData.Vertices)
	{
		const FVertexID ID = MeshDesc.CreateVertex();
		VertexPositions[ID] = FVector3f(Vertex);
		VertexIds.Add(ID);
	}
	
	// Triangles
	FPolygonGroupID PolyGroup = MeshDesc.CreatePolygonGroup(); 
	for (int32 i = 0; i < MeshData.Triangles.Num(); i += 3)
	{
		TArray<FVertexInstanceID> TriangleInstances;
		for (int32 j = 0; j < 3; ++j)
		{
			int32 DataIndex = MeshData.Triangles[i + j];
			FVertexInstanceID VIID = MeshDesc.CreateVertexInstance(VertexIds[DataIndex]);
			TriangleInstances.Add(VIID);

			// Map Normal and UV to the instance
			Normals[VIID] = FVector3f(MeshData.Normals[DataIndex]);
			UVs[VIID] = FVector2f(MeshData.UV0[DataIndex]);
		}
		MeshDesc.CreateTriangle(PolyGroup, TriangleInstances);
	}

	TArray<UStaticMeshDescription*> DescriptionContainer;
	DescriptionContainer.Add(StaticMeshDesc);
    
	NewMeshAsset->BuildFromStaticMeshDescriptions(DescriptionContainer);
	
	NewMeshAsset->PostEditChange(); 
	MeshPackage->SetDirtyFlag(true);
	// End building static mesh
	
	// Building data asset and row
	if (NewDataAsset && NewMeshAsset)
	{
		FBox Box = NewMeshAsset->GetBoundingBox();
		FVector Size = Box.GetSize();
		
		NewDataAsset->StaticMesh = NewMeshAsset;
		NewDataAsset->Width = Size.X;
		NewDataAsset->Height = Size.Z;
		
		FRockDataRow NewDataRow;
		NewDataRow.RockDataAsset = NewDataAsset;
		NewDataRow.Width = NewDataAsset->Width;
		NewDataRow.Height = NewDataAsset->Height;
		
		FString TablePath = TEXT("/Game/Procedural/Rocks/Data/DT_RockCatalog.DT_RockCatalog");
		UDataTable* Table = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *TablePath));

		if (Table)
		{
			Table->AddRow(*Guid.ToString(), NewDataRow);
		}
	}
	// End building data asset and row
	
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	SaveArgs.SaveFlags = SAVE_NoError;
	
	UPackage::SavePackage(
		MeshPackage, 
		NewMeshAsset, 
		*FPackageName::LongPackageNameToFilename(MeshPackageName, FPackageName::GetAssetPackageExtension()),
		SaveArgs
		);
	
	UPackage::SavePackage(
		DataPackage,
		NewDataAsset,
		*FPackageName::LongPackageNameToFilename(DataPackageName, FPackageName::GetAssetPackageExtension()),
		SaveArgs
		);
	
	AssetRegistry.AssetCreated(NewMeshAsset);
	AssetRegistry.AssetCreated(NewDataAsset);
	
	TArray<UObject*> Objects;
	Objects.Add(NewMeshAsset);
	Objects.Add(NewDataAsset);
	ContentBrowserModule.Get().SyncBrowserToAssets(Objects);
	
}

FVector AProceduralMeshActor::CalcNormal(const FSdfShape& SDF, const FVector& Vertex)
{
	const float Eps = 0.1f;

	const float dx = SDF.Evaluate(Vertex + FVector(Eps, 0, 0)) - SDF.Evaluate(Vertex - FVector(Eps, 0, 0));
	const float dy = SDF.Evaluate(Vertex + FVector(0, Eps, 0)) - SDF.Evaluate(Vertex - FVector(0, Eps, 0));
	const float dz = SDF.Evaluate(Vertex + FVector(0, 0, Eps)) - SDF.Evaluate(Vertex - FVector(0, 0, Eps));

	FVector Normal(dx, dy, dz);
	Normal.Normalize();

	return Normal;
}

FProcMeshTangent AProceduralMeshActor::CalcTangent(const FVector& Normal)
{
	FVector Tangent;

	if (FMath::Abs(Normal.Z) < 0.999f)
	{
		Tangent = FVector::CrossProduct(FVector::UpVector, Normal).GetSafeNormal();
	}
	else
	{
		Tangent = FVector::CrossProduct(FVector::RightVector, Normal).GetSafeNormal();
	}

	return FProcMeshTangent(Tangent, false);
}

void AProceduralMeshActor::BeginPlay()
{
	Super::BeginPlay();
}

void AProceduralMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
