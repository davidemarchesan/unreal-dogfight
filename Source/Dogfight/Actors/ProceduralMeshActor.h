#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

protected:
	virtual void BeginPlay() override;
};
