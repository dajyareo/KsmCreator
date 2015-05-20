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

#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

namespace KsmCreator
{
	//
	// Vertex holding a vertex ID and weight data
	//
	struct VertexBoneWeight
	{
		VertexBoneWeight()
		{}

		VertexBoneWeight(UINT vertexId, float weight)
			: VertexId(vertexId), Weight(weight)
		{}

		UINT VertexId;
		float Weight;
	};


	//
	// Vertex holding position, normal, and texture map info
	//
	struct VertexPositionNormalTexture
	{
		VertexPositionNormalTexture()
		{ }

		VertexPositionNormalTexture(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float u, float v) :
			Position(px, py, pz),
			Normal(nx, ny, nz),
			TextureCoordinate(u, v)
		{}

		VertexPositionNormalTexture(XMFLOAT3 const& position, XMFLOAT3 const& normal, XMFLOAT2 const& textureUv)
			: Position(position), Normal(normal), TextureCoordinate(textureUv)
		{}

		VertexPositionNormalTexture(XMVECTOR position, XMVECTOR normal, XMVECTOR textureUv)
		{
			XMStoreFloat3(&this->Position, position);
			XMStoreFloat3(&this->Normal, normal);
			XMStoreFloat2(&this->TextureCoordinate, textureUv);
		}

		XMFLOAT3 Position;
		XMFLOAT3 Normal;
		XMFLOAT2 TextureCoordinate;

		static const int InputElementCount = 3;
		static const D3D11_INPUT_ELEMENT_DESC InputElements[InputElementCount];
	};

	//
	// Vertex holding position, normal, and texture map info
	//
	struct VertexPositionNormalTextureBoneWeight
	{
		VertexPositionNormalTextureBoneWeight()
		{ }

		VertexPositionNormalTextureBoneWeight(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float u, float v) :
			Position(px, py, pz), Normal(nx, ny, nz), TextureCoordinate(u, v)
		{}

		VertexPositionNormalTextureBoneWeight(
			const XMFLOAT3& position,
			const XMFLOAT3& normal,
			const XMFLOAT2& textureUv,
			const XMFLOAT3& weights,
			byte index1, byte index2, byte index3, byte index4)
			: Position(position), Normal(normal), TextureCoordinate(textureUv), Weights(weights)
		{
			BoneIndices[0] = index1;
			BoneIndices[1] = index2;
			BoneIndices[2] = index3;
			BoneIndices[3] = index4;
		}

		VertexPositionNormalTextureBoneWeight(XMVECTOR position, XMVECTOR normal, XMVECTOR textureUv)
		{
			XMStoreFloat3(&this->Position, position);
			XMStoreFloat3(&this->Normal, normal);
			XMStoreFloat2(&this->TextureCoordinate, textureUv);
		}

		XMFLOAT3 Position;
		XMFLOAT3 Normal;
		XMFLOAT2 TextureCoordinate;
		XMFLOAT3 Weights;
		BYTE BoneIndices[4];

		static const int InputElementCount = 5;
		static D3D11_INPUT_ELEMENT_DESC InputElements[InputElementCount];
	};
}