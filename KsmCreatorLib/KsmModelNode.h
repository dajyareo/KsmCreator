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

#include "KsmModelMesh.h"
#include <scene.h>

using namespace std;

namespace KsmCreator
{
	// Holds mesh subset data
	struct KsmModelNode
	{
		KsmModelNode(_In_ aiNode* node)
		{
			//
			// Convert the name to a wide string
			NarrowName = node->mName.data;
			std::wstringstream ws;
			ws << NarrowName.c_str();
			Name = ws.str();

			//
			// Convert LocalTransform from aiMatrix to XMFLOAT4X4
			DirectX::XMMATRIX* tm = reinterpret_cast<DirectX::XMMATRIX*>(&node->mTransformation);
			DirectX::XMMATRIX om(*tm);
			XMStoreFloat4x4(&LocalTransform, om);
		}

		void Save(_In_ ofstream& ksmFile)
		{
			unsigned int nameStrLen = Name.length();
			ksmFile.write(reinterpret_cast<char*>(&nameStrLen), sizeof(unsigned int));
			ksmFile.write(reinterpret_cast<const char*>(Name.c_str()), nameStrLen * sizeof(wchar_t));

			ksmFile.write(reinterpret_cast<char*>(&LocalTransform), sizeof(XMFLOAT4X4));
			
			ksmFile.write(reinterpret_cast<char*>(&ChannelIndex), sizeof(size_t));

			UINT meshCount = static_cast<UINT>(Meshs.size());
			ksmFile.write(reinterpret_cast<char*>(&meshCount), sizeof(UINT));

			for (auto& mesh : Meshs)
			{
				mesh.Save(ksmFile);
			}

			UINT childCount = static_cast<UINT>(Children.size());
			ksmFile.write(reinterpret_cast<char*>(&childCount), sizeof(UINT));

			for (auto& child : Children)
			{
				child->Save(ksmFile);
			}
		}

		// Holds the starting vertex for each mesh subset
		vector<KsmModelMesh> Meshs;

		// The child nodes to this one
		std::vector<shared_ptr<KsmModelNode>> Children;

		// The parent node to this one
		shared_ptr<KsmModelNode> Parent;

		// Most recently calculated local transform
		XMFLOAT4X4 LocalTransform;

		// Most recently calculated global transform. This includes the local bone transform if applicable
		XMFLOAT4X4 GlobalTransform;

		// Index in the current animation's channel array. -1 if not animated.
		size_t ChannelIndex;

		// The name of this node
		wstring Name;

		// The name stored as a narrow string for comparing to assimp objects
		string NarrowName;
	};

}