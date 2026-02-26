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
	
protected:
	virtual void BeginPlay() override;

public:
	
	void GenerateMesh();
};
