//////////////////////////////////////////////////////////////////////////
// (c) 2015 Overclocked Games LLC
// KSMCREATOR PUBLIC LICENSE
//
// KSMCREATOR PUBLIC LICENSE TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
//
// Do whatever you like with the original work, just don't be a dick.
//
// Being a dick includes - but is not limited to - the following instances:
//
// 1a. Outright copyright infringement - Don't just copy this and change the name.
// 1b. Selling the unmodified original with no work done what-so-ever, that's REALLY being a dick.
// 1c. Modifying the original work to contain hidden harmful content. That would make you a PROPER dick.
//
// If you become rich through modifications, related works/services, or supporting the original work, share the 
// love. Only a dick would make loads off this work and not buy the original works creator(s) a pint.
//
// Code is provided with no warranty. Using somebody else's code and bitching when it goes wrong makes you a    
// DONKEY dick. Fix the problem yourself. A non-dick would submit the fix back.
//////////////////////////////////////////////////////////////////////////

#pragma once

// Note: Make sure structure alignment agrees with HLSL structure padding rules. 
//   Elements are packed into 4D vectors with the restriction that an element
//   cannot straddle a 4D vector boundary.

struct DirectionalLight
{
	DirectionalLight() { ZeroMemory(this, sizeof(this)); }

	DirectX::XMFLOAT4 Ambient;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular;
	DirectX::XMFLOAT3 Direction;
	float Pad; // Pad the last float so we can set an array of lights if we wanted.
};

struct PointLight
{
	PointLight() { ZeroMemory(this, sizeof(this)); }

	DirectX::XMFLOAT4 Ambient;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular;

	// Packed into 4D vector: (Position, Range)
	DirectX::XMFLOAT3 Position;
	float Range;

	// Packed into 4D vector: (A0, A1, A2, Pad)
	DirectX::XMFLOAT3 Att;
	float Pad; // Pad the last float so we can set an array of lights if we wanted.
};

struct SpotLight
{
	SpotLight() { ZeroMemory(this, sizeof(this)); }

	DirectX::XMFLOAT4 Ambient;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular;

	// Packed into 4D vector: (Position, Range)
	DirectX::XMFLOAT3 Position;
	float Range;

	// Packed into 4D vector: (Direction, Spot)
	DirectX::XMFLOAT3 Direction;
	float Spot;

	// Packed into 4D vector: (Att, Pad)
	DirectX::XMFLOAT3 Att;
	float Pad; // Pad the last float so we can set an array of lights if we wanted.
};

struct Material
{
	Material() 
		: Ambient(0, 0, 0, 0), Diffuse(0, 0, 0, 0), Specular(0, 0, 0, 0), Reflect(0, 0, 0, 0)
	{}

	DirectX::XMFLOAT4 Ambient;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular;
	DirectX::XMFLOAT4 Reflect;
};
