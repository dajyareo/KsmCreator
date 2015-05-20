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
#include "stdafx.h"
#include "KsmModelNode.h"
#include "KsmModelMesh.h"
#include <DirectXCollision.h>
#include "AnimationData.h"

using namespace Assimp;

namespace KsmCreator
{
	//
	// The class the wraps reading and writing the KSM model format (Kamishibai Model)
	// 
	class KsmModel
	{
	public:

		KsmModel()
			: NumMeshes(0), NumMaterials(0), NumTextures(0), NumAnimations(0), HasAnimations(false), 
			_aiScene(nullptr)
		{}

		~KsmModel()
		{}

		KsmModel(const KsmModel& other) = delete;
		KsmModel& operator=(const KsmModel& rhs) = delete;

		//
		// Load model data from a DAE (Collada) file
		//
		bool LoadFromDae(_In_ const std::wstring& path);

		//
		// Save to a KSM file
		//
		bool Save(_In_ const std::wstring& path);

		// The number of meshes this model contains
		UINT NumMeshes;

		// The number of materials this model contains
		UINT NumMaterials;

		// The number of textures this model contains
		UINT NumTextures;

		// The number of animations this model contains
		UINT NumAnimations;

		// Indicates if this model contains animations or is static
		bool HasAnimations;

		// The root node of this model
		shared_ptr<KsmModelNode> RootNode;

		// The animation data for this model
		//Engine::AnimationData ModelAnimationData;

		// The animation clips for this model
		vector<AnimationClip> AnimationClips;

		// The filename to use when saving the file
		wstring FileName;

		// The bounding box that encapsulates this model in model space
		BoundingBox _boundingBox;

	private:

		//
		// Parses all nodes and creates the tree
		//
		shared_ptr<KsmModelNode> createNodeTree(
			_In_ aiNode* node,
			_In_ shared_ptr<KsmModelNode> parent);

		//
		// Extracts all animation data
		//
		void extractAnimations();

		//
		// Creates the bounding box surrounding all model space vertices
		//
		void createBoundingBox();

		//
		// Creates the bounding box surrounding the given mesh
		//
		void createMeshBoundingBox(_In_ KsmModelMesh& mesh);

		//
		// Loads all vertices and indices
		//
		void loadMeshData(
			_In_ shared_ptr<KsmModelNode> ownerNode,
			_In_ const aiMesh* mesh);

		//
		// Loads all the material data from the source aiMesh into the given KsmModelMesh
		//
		void loadMaterial(
			_In_ KsmModelMesh* ksmMesh,
			_In_ const aiMesh* sourceMesh);

		//
		// Calculates the global transform of the given node which is done by concatenating
		// all parent local transforms
		//
		void calculateGlobalTransform(_In_ shared_ptr<KsmModelNode> ksmModelNode);

	private:

		// Pointer to the assimp scene object
		const aiScene* _aiScene;

		// All aiNode objects in the assimp scene and their KsmModelNode counterparts
		map<const aiNode*, shared_ptr<KsmModelNode>> _aiNodeToKsmModelNode;
	};
}
