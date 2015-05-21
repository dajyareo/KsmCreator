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

#pragma once;

#include <vector>
#include "..\KsmCreatorLib\LightHelper.h"
#include <DirectXCollision.h>
#include "VertexTypes.h"

using namespace std;
using namespace DirectX;

namespace KsmCreator
{
	struct KsmModelBone
	{
		wstring Name;

		XMFLOAT4X4 Offset;
	};

	// Holds mesh subset data
	struct KsmModelMesh
	{
		KsmModelMesh()
		{}

		void Save(_In_ ofstream& ksmFile)
		{
			ksmFile.write(reinterpret_cast<char*>(&NumBones), sizeof(unsigned int));
			ksmFile.write(reinterpret_cast<char*>(&FaceCount), sizeof(unsigned int));
			ksmFile.write(reinterpret_cast<char*>(&VertexCount), sizeof(unsigned int));
			ksmFile.write(reinterpret_cast<char*>(&IndexCount), sizeof(unsigned int));

			ksmFile.write(reinterpret_cast<char*>(&MeshMaterial.Diffuse), sizeof(XMFLOAT4));
			ksmFile.write(reinterpret_cast<char*>(&MeshMaterial.Ambient), sizeof(XMFLOAT4));
			ksmFile.write(reinterpret_cast<char*>(&MeshMaterial.Reflect), sizeof(XMFLOAT4));
			ksmFile.write(reinterpret_cast<char*>(&MeshMaterial.Specular), sizeof(XMFLOAT4));

			ksmFile.write(reinterpret_cast<char*>(&MaterialOpacity), sizeof(float));
			ksmFile.write(reinterpret_cast<char*>(&MaterialShininess), sizeof(float));
			ksmFile.write(reinterpret_cast<char*>(&SpecularStrength), sizeof(float));

			ksmFile.write(reinterpret_cast<char*>(&SaveIndicesAs16Bit), sizeof(bool));		

			ksmFile.write(reinterpret_cast<char*>(&Indices[0]), sizeof(UINT) * Indices.size());
		
			// One vertex vector will be full, the other empty
			bool containsAnimations = false;
			if (StaticVertices.size() > 0)
			{
				ksmFile.write(reinterpret_cast<char*>(&containsAnimations), sizeof(bool));
				ksmFile.write(reinterpret_cast<char*>(&StaticVertices[0]), sizeof(VertexPositionNormalTexture) * StaticVertices.size());
			}
			else
			{
				containsAnimations = true;
				ksmFile.write(reinterpret_cast<char*>(&containsAnimations), sizeof(bool));
				ksmFile.write(reinterpret_cast<char*>(&AnimatedVertices[0]), sizeof(VertexPositionNormalTextureBoneWeight) * AnimatedVertices.size());
			}

			unsigned int strLen = DiffuseTexturePath.length();
			ksmFile.write(reinterpret_cast<char*>(&strLen), sizeof(unsigned int));
			ksmFile.write(reinterpret_cast<const char*>(DiffuseTexturePath.c_str()), strLen * sizeof(wchar_t));

			strLen = NormalTexturePath.length();
			ksmFile.write(reinterpret_cast<char*>(&strLen), sizeof(unsigned int));
			ksmFile.write(reinterpret_cast<const char*>(NormalTexturePath.c_str()), strLen * sizeof(wchar_t));

			//
			// Write bones that affect this mesh
			for (auto& bone : Bones)
			{
				strLen = bone.Name.length();
				ksmFile.write(reinterpret_cast<char*>(&strLen), sizeof(unsigned int));
				ksmFile.write(reinterpret_cast<const char*>(bone.Name.c_str()), strLen * sizeof(wchar_t));

				ksmFile.write(reinterpret_cast<char*>(&bone.Offset), sizeof(XMFLOAT4X4));
			}

			//
			// Write mesh bounding box
			ksmFile.write(reinterpret_cast<char*>(&MeshBoundingBox.Center), sizeof(XMFLOAT3));
			ksmFile.write(reinterpret_cast<char*>(&MeshBoundingBox.Extents), sizeof(XMFLOAT3));
		}

		// The count of vertices that make up the mesh
		UINT VertexCount;

		// The count of faces that makes up the mesh
		UINT FaceCount;

		// The count of indices that makes up the mesh
		UINT IndexCount;

		// The diffuse texture for this mesh
		wstring DiffuseTexturePath;

		// The normal texture for this mesh
		wstring NormalTexturePath;

		// The material for this node
		Material MeshMaterial;

		// The names of the bone nodes that affect this mesh
		vector<KsmModelBone> Bones;

		// Animated vertex buffer
		vector<VertexPositionNormalTextureBoneWeight> AnimatedVertices;

		// Static vertex buffer
		vector<VertexPositionNormalTexture> StaticVertices;

		// Indices
		vector<UINT> Indices;

		// If there are less than 65536 indices we can save the indices as 16 bit instead of 32
		bool SaveIndicesAs16Bit;

		// opacity for the material
		float MaterialOpacity;

		// shininess for the material
		float MaterialShininess;

		// strength of the specular highlight
		float SpecularStrength;

		// The number of bones associated with this mesh
		UINT NumBones;

		// The bounding box surrounding the vertices in this mesh
		BoundingBox MeshBoundingBox;
	};
}