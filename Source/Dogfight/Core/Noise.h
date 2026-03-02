#pragma once

struct FPerlinNoise
{

	FVector Point;

	FPerlinNoise(const FVector& InPoint)
		: Point(InPoint)
	{
	};

	~FPerlinNoise()
	{
	};

	const FVector Gradients[12] = {
		FVector(1,1,0),
		FVector(-1,1,0),
		FVector(1,-1,0),
		FVector(-1,-1,0),
		FVector(1,0,1),
		FVector(-1,0,1),
		FVector(1,0,-1),
		FVector(-1,0,-1),
		FVector(0,1,1),
		FVector(0,-1,1),
		FVector(0,1,-1),
		FVector(0,-1,-1)
	};

	float Fade(float Value) const
	{
		return Value * Value * Value * (Value * (Value * 6 - 15) + 10);
	}

	int Hash(int X, int Y, int Z) const
	{
		uint32 Hash = X * 374761393 + Y * 668265263 + Z * 2147483647;
		Hash = (Hash ^ (Hash >> 13)) * 1274126177;
		return Hash % 12;
	}

	FVector GetGradient(const int X0, const int Y0, const int Z0) const
	{
		int H = Hash(X0, Y0, Z0);
		return Gradients[H];
	}

	float GradientDot(const FVector& Distance, const int X0, const int Y0, const int Z0)
	{
		const FVector Gradient = GetGradient(X0, Y0, Z0);
		return FVector::DotProduct(Gradient, Distance);
	}

	float Lerp(const float A, const float B, const float T)
	{
		return A + (B - A) * T;
	}
	
	float GetValue()
	{

		// Get cube internal corners
		const int X0 = FMath::FloorToInt(Point.X);
		const int Y0 = FMath::FloorToInt(Point.Y);
		const int Z0 = FMath::FloorToInt(Point.Z);

		const int X1 = X0 + 1;
		const int Y1 = Y0 + 1;
		const int Z1 = Z0 + 1;

		// Get local float vector
		const float LocalX = Point.X - X0;
		const float LocalY = Point.Y - Y0;
		const float LocalZ = Point.Z - Z0;

		const float U = Fade(LocalX);
		const float V = Fade(LocalY);
		const float W = Fade(LocalZ);
		
		// Dot product for each corner
		// Dot product between Distance and Gradient
		// Distance = Point - Corner
		// Gradient = Gradients[Hash] => Hash from Corner XYZ
		float Dot000 = GradientDot(FVector(LocalX, LocalY    , LocalZ    ), X0, Y0, Z0);
		float Dot010 = GradientDot(FVector(LocalX, LocalY - 1, LocalZ    ), X0, Y1, Z0);
		float Dot001 = GradientDot(FVector(LocalX, LocalY    , LocalZ - 1), X0, Y0, Z1);
		float Dot011 = GradientDot(FVector(LocalX, LocalY - 1, LocalZ - 1), X0, Y1, Z1);

		float Dot100 = GradientDot(FVector(LocalX - 1, LocalY    , LocalZ    ), X1, Y0, Z0);
		float Dot110 = GradientDot(FVector(LocalX - 1, LocalY - 1, LocalZ    ), X1, Y1, Z0);
		float Dot101 = GradientDot(FVector(LocalX - 1, LocalY    , LocalZ - 1), X1, Y0, Z1);
		float Dot111 = GradientDot(FVector(LocalX - 1, LocalY - 1, LocalZ - 1), X1, Y1, Z1);

		// Lerp everything
		float LerpX00 = Lerp(Dot000, Dot100, U);
		float LerpX10 = Lerp(Dot010, Dot110, U);
		float LerpX01 = Lerp(Dot001, Dot101, U);
		float LerpX11 = Lerp(Dot011, Dot111, U);
		
		float LerpXY0 = Lerp(LerpX00, LerpX10, V);
		float LerpXY1 = Lerp(LerpX01, LerpX11, V);

		float LerpXYZ = Lerp(LerpXY0, LerpXY1, W);

		return LerpXYZ;
	}
	
};
