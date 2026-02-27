#pragma once
#include "Noise.h"

struct FSdfShape
{
	FSdfShape() {};
	virtual ~FSdfShape() {};
	
	virtual float Evaluate(const FVector& Point) const
	{
		return 0;
	}
};

struct FSdfSphere : FSdfShape
{
	FSdfSphere() {};
	virtual ~FSdfSphere() override {};
	
	FVector Center = FVector::ZeroVector;
	float Radius = 100.f;

	virtual float Evaluate(const FVector& Point) const override
	{
		// return (Point - Center).Length() - Radius + (20.f * FMath::Sin(Point.X * 0.1f));
		return (Point - Center).Length() - Radius;
	}
};

struct FSdfRockBase : FSdfShape
{

protected:
	FVector Center = FVector::ZeroVector;
	float Radius = 100.f;
	float Diameter = 200.f;

	float MaxWide = 1.4f;
	float MaxNarrow = 1.f;

public:
	FSdfRockBase() {};
	FSdfRockBase(const float InRadius) : Radius(InRadius)
	{
		Diameter = Radius * 2.f;
	};
	
	virtual ~FSdfRockBase() override {};
	

	virtual float Evaluate(const FVector& Point) const override
	{

		// Rocks should be wide at top, where z is close to +Radius
		// and narrow at bottom, where z is close to -Radius
		const float t = (Point.Z + Radius) / Diameter;
		const float ZScaleFactor = MaxNarrow + (MaxWide - MaxNarrow) * t;

		const float NoiseFreq = 0.5f;
		const float NoiseAmp = 4.f;

		float NoiseValue = FPerlinNoise(Point * NoiseFreq).GetValue();
		float Offset = FMath::Clamp(NoiseValue * NoiseAmp, -50*0.4f, 50*0.4f);
		
		const float Distance = (FVector(Point.X / ZScaleFactor, Point.Y / ZScaleFactor, Point.Z) - Center).Length() - (Radius + Offset);
		// const float Distance = (FVector(Point.X, Point.Y, Point.Z) - Center).Length() - (Radius + Offset);
		
		return Distance;
	}
};

struct FMeshData
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
