#pragma once

#include <rapidjson/rapidjson.h>

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshActor.generated.h"

struct FSphereSDF
{
	FVector Center = FVector::ZeroVector;
	float Radius = 100.f;
	
	float Evaluate(const FVector& Point) const
	{
		// return (Point - Center).Length() - Radius + (20.f * FMath::Sin(Point.X * 0.1f));
		return (Point - Center).Length() - Radius;
	}
};

struct FMeshBuilder
{
public:
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FProcMeshTangent> Tangents;
	TArray<FColor> VertexColors;

public:
	int32 AddVertex(
		const FVector& Vertex,
		const FVector& Normal,
		const FVector2D& UV,
		const FColor& VertexColor,
		const FProcMeshTangent& Tangent
	)
	{
		const int32 VertexIndex = Vertices.Add(Vertex);
		Normals.Add(Normal);
		UV0.Add(UV);
		VertexColors.Add(VertexColor);
		Tangents.Add(Tangent);

		return VertexIndex;
	}

	void AddTriangle(const int32 A)
	{
		Triangles.Add(A);
	}
	
	void AddTriangle(const int32 A, const int32 B, const int32 C)
	{
		Triangles.Add(A);
		Triangles.Add(B);
		Triangles.Add(C);
	}
};

class UProceduralMeshComponent;

UCLASS()
class DOGFIGHT_API AProceduralMeshActor : public AActor
{
	GENERATED_BODY()

public:
	AProceduralMeshActor();
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleDefaultsOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UProceduralMeshComponent> ProcMeshComponent;

	FMeshBuilder MeshData;
	
	void AddQuad(const FVector& V1, const FVector& V2, const FVector& V3, const FVector& V4, const FVector& Normal);
	
protected:
	virtual void BeginPlay() override;

public:
	
	void GenerateMesh();
};
