// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MeshEditorController.generated.h"

/**
 * 
 */
UCLASS()
class DOGFIGHT_API AMeshEditorController : public APlayerController
{
	GENERATED_BODY()

public:

	// Console command
	UFUNCTION(Exec)
	void GenerateMesh();
	
};
