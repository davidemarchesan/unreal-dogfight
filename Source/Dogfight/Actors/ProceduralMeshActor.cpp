#include "ProceduralMeshActor.h"

#include "Dogfight/Core/MarchingCubes.h"

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
	UE_LOG(LogTemp, Warning, TEXT("Generating mesh..."));
	
	constexpr float Radius = 500.f;
	constexpr float Step = 50.f;
	constexpr float Range = 1200.f;
	constexpr float HalfStep = Step * 0.5f;

	// Internal diagonal of voxel (cube) + a little bit more space
	float MinDistance = FMath::Sqrt(3.f) * Step + 2.f;
	
	auto Sdf = FSdfRockBase(Radius);

	DrawDebugPoint(
		GetWorld(),
		GetActorLocation(),
		12.f,
		FColor::Yellow,
		true
	);

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

							DrawDebugPoint(
								GetWorld(),
								TriangleVertex,
								8.f,
								FColor::Yellow,
								true
							);
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

					// DrawDebugBox(
					// 	GetWorld(),
					// 	Point,
					// 	FVector(HalfStep),
					// 	FColor::Red,
					// 	true);
					// break;
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

	GenerateMesh();
}

void AProceduralMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
