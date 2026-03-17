#pragma once
#include "Noise.h"

struct FSdfShape
{
	FSdfShape()
	{
	};

	virtual ~FSdfShape()
	{
	};

	virtual float Evaluate(const FVector& Point) const
	{
		return 0;
	}
};

struct FSdfSphere : FSdfShape
{
	FSdfSphere()
	{
	};

	virtual ~FSdfSphere() override
	{
	};

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

	float Height = 1.f;

	float MaxWide = 1.4f;
	float MaxNarrow = 0.3f;

public:
	FSdfRockBase()
	{
	};

	FSdfRockBase(const float InRadius) : Radius(InRadius)
	{
		Diameter = Radius * 2.f;
	};

	FSdfRockBase(const float InRadius, const float InHeight, const float InMaxWide, const float InMaxNarrow)
		: Radius(InRadius),
		Height(InHeight),
		MaxWide(InMaxWide),
		MaxNarrow(InMaxNarrow)
	{}

	virtual ~FSdfRockBase() override
	{
	};

	float FractalNoise(const FVector& Point) const
	{
		// https://thebookofshaders.com/13/
		const int Octaves = 10;
		const float Lacunarity = 2.f;
		const float Gain = 0.5f;

		// Initial values
		float Frequency = 0.1f;
		float Amplitude = 1.f;

		float Value = 0.f;

		for (int i = 0; i < Octaves; i++)
		{
			Value += Amplitude * FPerlinNoise(Point * Frequency).GetValue();
			Frequency *= Lacunarity;
			Amplitude *= Gain;
		}

		return Value;
	}

	virtual float Evaluate(const FVector& Point) const override
	{
		// Rocks should be wide at top, where z is close to +Radius
		// and narrow at bottom, where z is close to -Radius
		const float t = (Point.Z * Height + Radius) / Diameter;
		const float ZScaleFactor = MaxNarrow + (MaxWide - MaxNarrow) * t;

		const FVector P(
			Point.X / ZScaleFactor,
			Point.Y / ZScaleFactor,
			Point.Z * Height
		);
		
		// Perlin Noise + Fractal Brownian Motion
		float NoiseOffset = FractalNoise(P);

		const float Distance = (P - Center).Length() - (Radius + NoiseOffset);

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

	void Reset()
	{
		Vertices.Reset();
		Triangles.Reset();
		Normals.Reset();
		UV0.Reset();
		Tangents.Reset();
		VertexColors.Reset();
	}
};
