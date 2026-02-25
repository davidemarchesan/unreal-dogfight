#include "ProceduralMeshActor.h"

#include "Dogfight/Core/MarchingCubes.h"
#include "Generators/MarchingCubes.h"

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

	FSphereSDF SphereSdf = FSphereSDF();

	constexpr float Step = 30.f;
	constexpr int32 Range = 300;
	constexpr int32 HalfStep = Step * 0.5f;

	DrawDebugPoint(
		GetWorld(),
		GetActorLocation(),
		12.f,
		FColor::Yellow,
		true
	);

	for (float x = -Range; x <= Range; x += Step)
		for (float y = -Range; y <= Range; y += Step)
	for (float z = -Range; z <= Range; z += Step)
	{
		// float x = 0; // test
		// float y = 0; // test
		FVector Point(x, y, z);

		int N = 0;
		int bit[8] = {0, 0, 0, 0, 0, 0, 0, 0};
		uint8_t Index = 0;

		FVector EdgeVertex[12];

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
			const float VertexDistance = SphereSdf.Evaluate(Vertices[i]);
			if (VertexDistance < 0.f)
			{
				N++;
				bit[i] = 1;
			}
			else
			{
				bit[i] = 0;
			}
		}

		for (int i = 0; i < 8; i++)
		{
			Index |= (bit[i] << i);
			// Index |= (bit[i] << (7 - i));
		}

		if (N > 0 && N < 8)
		{
			UE_LOG(LogTemp, Warning, TEXT("Voxel at %s with vertices inside: %d"), *Point.ToString(), N);
			UE_LOG(LogTemp, Warning, TEXT("Vertices:"));
			for (auto& Vertex : Vertices)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s with distance %f"), *Vertex.ToString(), SphereSdf.Evaluate(Vertex));
			}
			UE_LOG(LogTemp, Warning, TEXT("Bit vertices:"));
			for (int i = 0; i < 8; i++)
			{
				UE_LOG(LogTemp, Warning, TEXT("%d"), bit[i]);
			}

			for (int32 i = 0; i < 12; i++)
			{
				const int* Edge = FMarchingCubes::EdgeIndices[i];
				if (bit[Edge[0]] != bit[Edge[1]])
				{
					UE_LOG(LogTemp, Warning, TEXT("Edge: %d - %d"), Edge[0], Edge[1]);


					const float DistanceA = SphereSdf.Evaluate(Vertices[Edge[0]]);
					const float DistanceB = SphereSdf.Evaluate(Vertices[Edge[1]]);
					const float T = DistanceA / (DistanceA - DistanceB);

					FVector TriangleVertex = Vertices[Edge[0]] + (Vertices[Edge[1]] - Vertices[Edge[0]]) * T;
					EdgeVertex[i] = TriangleVertex;
					UE_LOG(LogTemp, Warning, TEXT("Triangle vertex at %s with T %f"), *TriangleVertex.ToString(), T);
					DrawDebugPoint(
						GetWorld(),
						TriangleVertex,
						8.f,
						FColor::Yellow,
						true
					);
				}
			}

			UE_LOG(LogTemp, Warning, TEXT("Triangle index %d"), Index);
			const int* Triangle = FMarchingCubes::TriTable[Index];
			
			for (int32 i = 0; i < 16; i++)
			{
				if (Triangle[i] == -1)
				{
					break;
				}

				UE_LOG(LogTemp, Warning, TEXT("Triangle %d -> %s"), Triangle[i], *EdgeVertex[Triangle[i]].ToString());

				const int32 VertexIndex = MeshData.AddVertex(
					EdgeVertex[Triangle[i]],
					FVector::UpVector,
					FVector2D(0, 0),
					FColor::White,
					FProcMeshTangent()
				);
				MeshData.AddTriangle(VertexIndex);
			}

			DrawDebugBox(
				GetWorld(),
				Point,
				FVector(HalfStep),
				FColor::Red,
				true);
			// break;
		}

		// if (FMath::Abs(Distance) <= 10.f)
		// if (Distance < 0.f)
		// {
		// 	DrawDebugPoint(
		// 		GetWorld(),
		// 		GetActorLocation() + Point,
		// 		8.f,
		// 		FColor::Red,
		// 		true
		// 		);
		// }
	}

	// for (int32 x = -200; x <= 200; x+=50)
	// {
	// 	FVector Point(x, 0, 0);
	// 	float Distance = SphereSdf.Evaluate(Point);
	//
	// 	UE_LOG(LogTemp, Warning, TEXT("x=%d -> %f"), x, Distance);
	// }

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

void AProceduralMeshActor::AddQuad(
	const FVector& V1,
	const FVector& V2,
	const FVector& V3,
	const FVector& V4,
	const FVector& Normal
)
{
	const int32 V1Index = MeshData.AddVertex(V1, Normal, FVector2D(0, 0), FColor::White, FProcMeshTangent());
	const int32 V2Index = MeshData.AddVertex(V2, Normal, FVector2D(0, 0), FColor::White, FProcMeshTangent());
	const int32 V3Index = MeshData.AddVertex(V3, Normal, FVector2D(0, 0), FColor::White, FProcMeshTangent());
	const int32 V4Index = MeshData.AddVertex(V4, Normal, FVector2D(0, 0), FColor::White, FProcMeshTangent());

	MeshData.AddTriangle(V1Index, V2Index, V3Index);
	MeshData.AddTriangle(V1Index, V3Index, V4Index);
}

void AProceduralMeshActor::BeginPlay()
{
	Super::BeginPlay();

	GenerateMesh();

	// Bottom
	AddQuad(
		FVector(100.f, 100.f, 0),
		FVector(0, 100.f, 0),
		FVector(0, 0, 0),
		FVector(100.f, 0, 0),
		FVector(0, 0, -1.f)
	);

	// Top
	AddQuad(
		FVector(0, 0, 100.f),
		FVector(0, 100.f, 100.f),
		FVector(100.f, 100.f, 100.f),
		FVector(100.f, 0, 100.f),
		FVector(0, 0, 1.f)
	);

	// Left
	AddQuad(
		FVector(0, 0, 100.f),
		FVector(0, 0, 0),
		FVector(100.f, 0, 0),
		FVector(100.f, 0, 100.f),
		FVector(0, 1.f, 0)
	);
}

void AProceduralMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
