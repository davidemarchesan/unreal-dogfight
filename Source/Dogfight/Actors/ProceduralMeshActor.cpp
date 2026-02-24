#include "ProceduralMeshActor.h"
#include "ProceduralMeshComponent.h"

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

void AProceduralMeshActor::BeginPlay()
{
	Super::BeginPlay();

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FProcMeshTangent> Tangents;
	TArray<FColor> VertexColors;

	Vertices.Add(FVector(0, 0, 0));
	Vertices.Add(FVector(0, 100.f, 0));
	Vertices.Add(FVector(100.f, 100.f, 0));
	Vertices.Add(FVector(100.f, 0, 0));
	
	Triangles = {0, 2, 1, 0, 3, 2};

	Normals.Init(FVector(0, 0, -1), 4);

	UVs = {
		FVector2D(0, 0),
		FVector2D(1, 0),
		FVector2D(0, 1),
		FVector2D(0, 1),
	};

	VertexColors.Init(FColor::White, 4);

	Tangents.Init(FProcMeshTangent(), 4);

	ProcMeshComponent->CreateMeshSection(
		0,
		Vertices,
		Triangles,
		Normals,
		UVs,
		VertexColors,
		Tangents,
		true
	);

	// Vertices
	// Triangles
	// Normals
	// UV
	// Tangents
	// Vertex Colors (optional)
	
}

void AProceduralMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

