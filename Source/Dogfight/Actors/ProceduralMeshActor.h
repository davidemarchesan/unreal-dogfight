#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Dogfight/Core/SDF.h"
#include "ProceduralMeshActor.generated.h"

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

	FMeshData MeshData;
	
	FVector CalcNormal(const FSdfShape& SDF, const FVector& Vertex);
	FProcMeshTangent CalcTangent(const FVector& Normal);
	
	bool bGenerated = false;
	
	float Radius;
	float Height;
	float MaxWide;
	float MaxNarrow;
	
protected:
	virtual void BeginPlay() override;

public:
	
	UFUNCTION(CallInEditor, Category = "Generation")
	void GenerateMesh();
	
	UFUNCTION(CallInEditor, Category = "Generation")
	void SaveMesh();
};
