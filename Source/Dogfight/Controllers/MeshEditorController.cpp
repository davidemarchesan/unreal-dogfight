// Fill out your copyright notice in the Description page of Project Settings.


#include "MeshEditorController.h"

#include "EngineUtils.h"
#include "Dogfight/Actors/ProceduralMeshActor.h"

void AMeshEditorController::GenerateMesh()
{
	for (TActorIterator<AProceduralMeshActor> It(GetWorld()); It; ++It)
	{
		It->GenerateMesh();
	}
}
