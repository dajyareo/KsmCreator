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

#include "stdafx.h"
#include "KsmModel.h"
#include "../KsmCreatorLib/PathString.h"
#include "../KsmCreatorLib/AnimationData.h"

using namespace KsmCreator;
using namespace Assimp;

bool KsmModel::LoadFromDae(_In_ const wstring& path)
{
	PathString tmpPath(path.c_str());
	FileName = tmpPath.GetFileNameOnly();
	FileName.append(L".ksm");

	// Create the importer
	Assimp::Importer importer;

	_aiScene = importer.ReadFile(tmpPath.ToCStr(),
		aiProcess_JoinIdenticalVertices |		// join identical vertices/ optimize indexing
		aiProcess_ValidateDataStructure |		// perform a full validation of the loader's output
		aiProcess_ImproveCacheLocality |		// improve the cache locality of the output vertices
		aiProcess_RemoveRedundantMaterials |	// remove redundant materials
		aiProcess_GenUVCoords |					// convert spherical, cylindrical, box and planar mapping to proper UVs
		aiProcess_TransformUVCoords |			// pre-process UV transformations (scaling, translation ...)
		aiProcess_FindInstances |				// search for instanced meshes and remove them by references to one master
		aiProcess_LimitBoneWeights |			// limit bone weights to 4 per vertex
		aiProcess_OptimizeMeshes |				// join small meshes, if possible;
		aiProcess_GenSmoothNormals |			// generate smooth normal vectors if not existing
		aiProcess_SplitLargeMeshes |			// split large, unrenderable meshes into sub-meshes
		aiProcess_Triangulate |					// triangulate polygons with more than 3 edges
		aiProcess_ConvertToLeftHanded |			// convert everything to D3D left handed space
		aiProcess_SortByPType);					// make 'clean' meshes which consist of a single type of primitives

	// If the import failed, report it
	if (!_aiScene)
	{
		PathString err(importer.GetErrorString());
		wprintf_s(L"Failed to load model with error: %s", err.ToWCStr());
		return false;
	}

	//
	// Read in global stats
	NumMeshes		= _aiScene->mNumMeshes;
	NumMaterials	= _aiScene->mNumMaterials;
	NumTextures		= _aiScene->mNumTextures;
	NumAnimations	= _aiScene->mNumAnimations;
	HasAnimations	= NumAnimations > 0;

	//
	// Create the node tree
	RootNode = createNodeTree(_aiScene->mRootNode, nullptr);

	//
	// Extract animation data
	extractAnimations();

	//
	// Create Bounding Box
	createBoundingBox();

	return true;
}

bool KsmModel::Save(_In_ const wstring& path)
{
	wstring outputFilePath = path;
	outputFilePath.append(L"\\");
	outputFilePath.append(FileName);

	// Dump vertex and index data to the output KSM file	
	ofstream ksmFile(outputFilePath.c_str(), ofstream::out | ofstream::binary);
	if (!ksmFile.is_open())
	{
		wprintf_s(L"Error: could not open file %s for write", outputFilePath.c_str());
		return false;
	}

	// Write Global Data
	ksmFile.write(reinterpret_cast<char*>(&NumMeshes), sizeof(unsigned int));
	ksmFile.write(reinterpret_cast<char*>(&NumMaterials), sizeof(unsigned int));
	ksmFile.write(reinterpret_cast<char*>(&NumTextures), sizeof(unsigned int));
	ksmFile.write(reinterpret_cast<char*>(&NumAnimations), sizeof(unsigned int));

	// Write the Node Tree
	RootNode->Save(ksmFile);

	// Write Animation Data
	if (HasAnimations)
	{
		for (size_t a = 0; a < NumAnimations; ++a)
		{
			unsigned int animationNameStrLen = AnimationClips[a].Name.length();
			ksmFile.write(reinterpret_cast<char*>(&animationNameStrLen), sizeof(unsigned int));
			ksmFile.write(reinterpret_cast<const char*>(AnimationClips[a].Name.c_str()), animationNameStrLen * sizeof(wchar_t));

			ksmFile.write(reinterpret_cast<char*>(&AnimationClips[a].Duration), sizeof(float));
			ksmFile.write(reinterpret_cast<char*>(&AnimationClips[a].TicksPerSecond), sizeof(float));

			unsigned int numBoneAnimations = AnimationClips[a].BoneAnimations.size();
			ksmFile.write(reinterpret_cast<char*>(&numBoneAnimations), sizeof(unsigned int));
			for (size_t ba = 0; ba < numBoneAnimations; ++ba)
			{
				unsigned int numKeyFrames = AnimationClips[a].BoneAnimations[ba].Keyframes.size();
				ksmFile.write(reinterpret_cast<char*>(&numKeyFrames), sizeof(unsigned int));

				for (size_t k = 0; k < numKeyFrames; ++k)
				{
					ksmFile.write(reinterpret_cast<char*>(&AnimationClips[a].BoneAnimations[ba].Keyframes[k].Translation), sizeof(XMFLOAT3));
					ksmFile.write(reinterpret_cast<char*>(&AnimationClips[a].BoneAnimations[ba].Keyframes[k].Scale), sizeof(XMFLOAT3));
					ksmFile.write(reinterpret_cast<char*>(&AnimationClips[a].BoneAnimations[ba].Keyframes[k].RotationQuat), sizeof(XMFLOAT4));
					ksmFile.write(reinterpret_cast<char*>(&AnimationClips[a].BoneAnimations[ba].Keyframes[k].TimePos), sizeof(float));
				}
			}

			unsigned int frameCount = AnimationClips[a].TotalFrames;
			ksmFile.write(reinterpret_cast<char*>(&frameCount), sizeof(unsigned int));
		}
	}

	//
	// Write model bounding box
	ksmFile.write(reinterpret_cast<char*>(&_boundingBox.Center), sizeof(XMFLOAT3));
	ksmFile.write(reinterpret_cast<char*>(&_boundingBox.Extents), sizeof(XMFLOAT3));

	ksmFile.close();

	return true;
}

shared_ptr<KsmModelNode> KsmModel::createNodeTree(
	_In_ aiNode* node,
	_In_ shared_ptr<KsmModelNode> parent)
{
	// create a KsmModelNode from the aiNode
	shared_ptr<KsmModelNode> ksmNode = make_shared<KsmModelNode>(node);
	ksmNode->Parent = parent;

	// store the node by name for easy reference and counting total nodes
	_aiNodeToKsmModelNode[node] = ksmNode;

	calculateGlobalTransform(ksmNode);

	// find the index of the animation track affecting this node, if any
	ksmNode->ChannelIndex = -1;
	bool channelFound = false;
	for (UINT i = 0; i < _aiScene->mNumAnimations; ++i)
	{
		if (channelFound)
		{
			break;
		}
		const aiAnimation* currentAnim = _aiScene->mAnimations[i];
		for (unsigned int c = 0; c < currentAnim->mNumChannels; c++)
		{
			if (currentAnim->mChannels[c]->mNodeName.data == ksmNode->NarrowName)
			{
				ksmNode->ChannelIndex = c;
				channelFound = true;
				break;
			}
		}
	}

	// Load the meshes that belong to this node
	for (UINT m = 0; m < node->mNumMeshes; ++m)
	{
		loadMeshData(ksmNode, _aiScene->mMeshes[node->mMeshes[m]]);
	}

	// continue for all child nodes and assign the created internal nodes as our children
	for (unsigned int c = 0; c < node->mNumChildren; c++)
	{
		shared_ptr<KsmModelNode> childNode = createNodeTree(node->mChildren[c], ksmNode);
		ksmNode->Children.push_back(childNode);
	}

	return ksmNode;
}

void KsmModel::extractAnimations()
{
	for (size_t j = 0; j < _aiScene->mNumAnimations; j++)
	{
		const aiAnimation* sourceAnimation = _aiScene->mAnimations[j];
		AnimationClip animation;

		animation.TicksPerSecond = static_cast<float>(sourceAnimation->mTicksPerSecond);
		animation.Duration = static_cast<float>(sourceAnimation->mDuration);
		PathString name = sourceAnimation->mName.data;
		animation.Name = name.ToWCStr();

		for (UINT a = 0; a < sourceAnimation->mNumChannels; a++)
		{
			BoneAnimation boneAnimation;

			PathString tmp(sourceAnimation->mChannels[a]->mNodeName.data);
			boneAnimation.Name = tmp.ToWCStr();
			for (unsigned int i = 0; i < sourceAnimation->mChannels[a]->mNumPositionKeys; i++)
			{
				Keyframe keyFrame;
				keyFrame.Translation.x = sourceAnimation->mChannels[a]->mPositionKeys[i].mValue.x;
				keyFrame.Translation.y = sourceAnimation->mChannels[a]->mPositionKeys[i].mValue.y;
				keyFrame.Translation.z = sourceAnimation->mChannels[a]->mPositionKeys[i].mValue.z;

				keyFrame.RotationQuat.x = sourceAnimation->mChannels[a]->mRotationKeys[i].mValue.x;
				keyFrame.RotationQuat.y = sourceAnimation->mChannels[a]->mRotationKeys[i].mValue.y;
				keyFrame.RotationQuat.z = sourceAnimation->mChannels[a]->mRotationKeys[i].mValue.z;
				keyFrame.RotationQuat.w = sourceAnimation->mChannels[a]->mRotationKeys[i].mValue.w;

				keyFrame.Scale.x = sourceAnimation->mChannels[a]->mScalingKeys[i].mValue.x;
				keyFrame.Scale.y = sourceAnimation->mChannels[a]->mScalingKeys[i].mValue.y;
				keyFrame.Scale.z = sourceAnimation->mChannels[a]->mScalingKeys[i].mValue.z;

				keyFrame.TimePos = static_cast<float>(sourceAnimation->mChannels[a]->mPositionKeys[i].mTime);

				boneAnimation.Keyframes.push_back(keyFrame);
			}
			animation.BoneAnimations.push_back(boneAnimation);
		}
		AnimationClips.push_back(animation);
	}
}

void KsmModel::loadMaterial(
	_In_ KsmModelMesh* ksmMesh,
	_In_ const aiMesh* sourceMesh)
{
	// extract all properties from the ASSIMP material structure
	const aiMaterial* sourceMaterial = _aiScene->mMaterials[sourceMesh->mMaterialIndex];

	//
	// DIFFUSE COLOR
	//
	aiColor4D materialColor;
	if (AI_SUCCESS == aiGetMaterialColor(sourceMaterial, AI_MATKEY_COLOR_DIFFUSE, &materialColor))
	{
		ksmMesh->MeshMaterial.Diffuse.x = materialColor.r;
		ksmMesh->MeshMaterial.Diffuse.y = materialColor.g;
		ksmMesh->MeshMaterial.Diffuse.z = materialColor.b;
		ksmMesh->MeshMaterial.Diffuse.w = materialColor.a;
	}
	else
	{
		ksmMesh->MeshMaterial.Diffuse.x = 1.0f;
		ksmMesh->MeshMaterial.Diffuse.y = 1.0f;
		ksmMesh->MeshMaterial.Diffuse.z = 1.0f;
		ksmMesh->MeshMaterial.Diffuse.w = 1.0f;
	}
	//
	// SPECULAR COLOR
	//
	if (AI_SUCCESS == aiGetMaterialColor(sourceMaterial, AI_MATKEY_COLOR_SPECULAR, &materialColor))
	{
		ksmMesh->MeshMaterial.Specular.x = materialColor.r;
		ksmMesh->MeshMaterial.Specular.y = materialColor.g;
		ksmMesh->MeshMaterial.Specular.z = materialColor.b;
		ksmMesh->MeshMaterial.Specular.w = materialColor.a;
	}
	else
	{
		ksmMesh->MeshMaterial.Specular.x = 1.0f;
		ksmMesh->MeshMaterial.Specular.y = 1.0f;
		ksmMesh->MeshMaterial.Specular.z = 1.0f;
		ksmMesh->MeshMaterial.Specular.w = 1.0f;
	}

	//
	// AMBIENT COLOR
	//
	if (AI_SUCCESS == aiGetMaterialColor(sourceMaterial, AI_MATKEY_COLOR_AMBIENT, &materialColor))
	{
		ksmMesh->MeshMaterial.Ambient.x = materialColor.r;
		ksmMesh->MeshMaterial.Ambient.y = materialColor.g;
		ksmMesh->MeshMaterial.Ambient.z = materialColor.b;
		ksmMesh->MeshMaterial.Ambient.w = materialColor.a;
	}
	else
	{
		ksmMesh->MeshMaterial.Ambient.x = 1.0f;
		ksmMesh->MeshMaterial.Ambient.y = 1.0f;
		ksmMesh->MeshMaterial.Ambient.z = 1.0f;
		ksmMesh->MeshMaterial.Ambient.w = 1.0f;
	}

	//
	// EMISSIVE COLOR
	//
	if (AI_SUCCESS == aiGetMaterialColor(sourceMaterial, AI_MATKEY_COLOR_EMISSIVE, &materialColor))
	{
		ksmMesh->MeshMaterial.Reflect.x = materialColor.r;
		ksmMesh->MeshMaterial.Reflect.y = materialColor.g;
		ksmMesh->MeshMaterial.Reflect.z = materialColor.b;
		ksmMesh->MeshMaterial.Reflect.w = materialColor.a;
	}
	else
	{
		ksmMesh->MeshMaterial.Reflect.x = 1.0f;
		ksmMesh->MeshMaterial.Reflect.y = 1.0f;
		ksmMesh->MeshMaterial.Reflect.z = 1.0f;
		ksmMesh->MeshMaterial.Reflect.w = 1.0f;
	}

	//
	// Opacity
	//
	if (AI_SUCCESS != aiGetMaterialFloat(sourceMaterial, AI_MATKEY_OPACITY, &ksmMesh->MaterialOpacity))
	{
		ksmMesh->MaterialOpacity = 1.0f;
	}

	//
	// Shininess
	//
	if (AI_SUCCESS != aiGetMaterialFloat(sourceMaterial, AI_MATKEY_SHININESS, &ksmMesh->MaterialShininess))
	{
		// assume 15 as default shininess
		ksmMesh->MaterialShininess = 15.0f;
	}

	//
	// Shininess strength
	//
	if (AI_SUCCESS != aiGetMaterialFloat(sourceMaterial, AI_MATKEY_SHININESS_STRENGTH, &ksmMesh->SpecularStrength))
	{
		// assume 1.0 as default shininess strength
		ksmMesh->SpecularStrength = 1.0f;
	}

	aiString szPath;
	aiTextureMapMode mapU(aiTextureMapMode_Wrap), mapV(aiTextureMapMode_Wrap);

	//bool bib = false;
	if (sourceMesh->mTextureCoords[0])
	{
		//
		// Diffuse texture
		//
		if (AI_SUCCESS == aiGetMaterialString(sourceMaterial, AI_MATKEY_TEXTURE_DIFFUSE(0), &szPath))
		{
			// strip off any paths and get the filename only
			PathString texturePath(szPath.C_Str());
			string filenameOnly = texturePath.GetFullFileNameN();

			// TODO: Replace this with a relative path that makes sense for where you store model textures
			string tmppath("Engine\\Assets\\Textures\\Models\\");
			tmppath.append(filenameOnly);
			PathString path(tmppath.c_str());

			ksmMesh->DiffuseTexturePath = path.ToWCStr();
		}

		//
		// Normal texture
		//
		if (AI_SUCCESS == aiGetMaterialString(sourceMaterial, AI_MATKEY_TEXTURE_NORMALS(0), &szPath))
		{
			// strip off any paths and get the filename only
			PathString texturePath(szPath.C_Str());
			string filenameOnly = texturePath.GetFullFileNameN();

			// TODO: Replace this with a relative path that makes sense for where you store model textures
			string tmppath("Engine\\Assets\\Textures\\Models\\");
			tmppath.append(filenameOnly);
			PathString path(tmppath.c_str());

			ksmMesh->NormalTexturePath = path.ToWCStr();
		}
	}
}

void KsmModel::calculateGlobalTransform(_In_ shared_ptr<KsmModelNode> ksmModelNode)
{
	// Concatenate all parent transforms to get the global transform for this node
	ksmModelNode->GlobalTransform = ksmModelNode->LocalTransform;
	shared_ptr<KsmModelNode> parent = ksmModelNode->Parent;

	while (parent)
	{
		XMMATRIX lt = XMLoadFloat4x4(&parent->LocalTransform);
		XMMATRIX gt = XMLoadFloat4x4(&ksmModelNode->GlobalTransform);
		XMMATRIX result = lt * gt;
		XMStoreFloat4x4(&ksmModelNode->GlobalTransform, result);

		parent = parent->Parent;
	}
}

void KsmModel::loadMeshData(
	_In_ shared_ptr<KsmModelNode> ownerNode,
	_In_ const aiMesh* mesh)
{
	KsmModelMesh ksmMesh;

	// Read global data
	ksmMesh.NumBones = mesh->mNumBones;
	ksmMesh.FaceCount = mesh->mNumFaces;
	ksmMesh.VertexCount = mesh->mNumVertices;

	// create the material for the mesh
	loadMaterial(&ksmMesh, mesh);

	unsigned int nidx;
	switch (mesh->mPrimitiveTypes) {
	case aiPrimitiveType_POINT:
		nidx = 1; break;
	case aiPrimitiveType_LINE:
		nidx = 2; break;
	case aiPrimitiveType_TRIANGLE:
		nidx = 3; break;
	default:
		nidx = 3;
	};

	// check whether we can use 16 bit indices
	ksmMesh.SaveIndicesAs16Bit = true;
	if (mesh->mNumFaces * 3 >= 65536)
	{
		ksmMesh.SaveIndicesAs16Bit = false;
	}

	// now fill the index buffer
	for (unsigned int x = 0; x < mesh->mNumFaces; ++x)
	{
		for (unsigned int a = 0; a < nidx; ++a)
		{			
			ksmMesh.Indices.push_back(mesh->mFaces[x].mIndices[a]);
		}
	}

	ksmMesh.IndexCount = static_cast<UINT>(ksmMesh.Indices.size());

	// collect weights on all vertices
	vector<vector<aiVertexWeight> > weightsPerVertex(mesh->mNumVertices);
	for (unsigned int a = 0; a < mesh->mNumBones; a++)
	{
		const aiBone* bone = mesh->mBones[a];
		for (unsigned int b = 0; b < bone->mNumWeights; b++)
		{
			weightsPerVertex[bone->mWeights[b].mVertexId].push_back(aiVertexWeight(a, bone->mWeights[b].mWeight));
		}
	}

	// now fill the vertex buffer
	if (HasAnimations)
	{
		ksmMesh.AnimatedVertices.resize(mesh->mNumVertices);
	}
	else
	{
		ksmMesh.StaticVertices.resize(mesh->mNumVertices);
	}
	
	for (unsigned int x = 0; x < mesh->mNumVertices; ++x)
	{
		XMFLOAT3 position;
		XMFLOAT3 normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT2 texture = XMFLOAT2(0.5f, 0.5f);
		unsigned char boneIndices[4] = { 0, 0, 0, 0 };
		float boneWeights[4] = { 0, 0, 0, 0 };

		position.x = mesh->mVertices[x].x;
		position.y = mesh->mVertices[x].y;
		position.z = mesh->mVertices[x].z;

		if (NULL != mesh->mNormals)
		{
			normal.x = mesh->mNormals[x].x;
			normal.y = mesh->mNormals[x].y;
			normal.z = mesh->mNormals[x].z;
		}

		if (mesh->HasTextureCoords(0))
		{
			texture.x = mesh->mTextureCoords[0][x].x;
			texture.y = mesh->mTextureCoords[0][x].y;
		}

		// Bone indices and weights
		if (mesh->HasBones())	
		{			
			assert(weightsPerVertex[x].size() <= 4);
			for (unsigned int a = 0; a < weightsPerVertex[x].size(); a++)
			{
				boneIndices[a] = weightsPerVertex[x][a].mVertexId;
				boneWeights[a] = weightsPerVertex[x][a].mWeight;
			}
		}

		if (HasAnimations)
		{
			ksmMesh.AnimatedVertices[x] = 
				VertexPositionNormalTextureBoneWeight(
				position,
				normal, 
				texture, 
				XMFLOAT3(boneWeights[0], boneWeights[1], boneWeights[2]), 
				boneIndices[0], boneIndices[1], boneIndices[2], boneIndices[3]);
		}
		else
		{
			ksmMesh.StaticVertices[x] = VertexPositionNormalTexture(position, normal, texture);
		}
	}

	//
	// Load Bones
	for (UINT b = 0; b < mesh->mNumBones; ++b)
	{
		KsmModelBone ksmBone;
		aiBone* bone = mesh->mBones[b];

		string narrow = bone->mName.data;
		wstringstream ws;
		ws << narrow.c_str();
		ksmBone.Name = ws.str();

		XMMATRIX* tm = reinterpret_cast<XMMATRIX*>(&bone->mOffsetMatrix);
		XMMATRIX om(*tm);
		XMStoreFloat4x4(&ksmBone.Offset, om);

		ksmMesh.Bones.push_back(ksmBone);
	}
	
	ownerNode->Meshs.push_back(ksmMesh);
}

void KsmModel::createBoundingBox()
{
	_boundingBox.Extents = XMFLOAT3(0, 0, 0);

	for (const auto& nodePair : _aiNodeToKsmModelNode)
	{
		XMMATRIX nodeTransform = XMLoadFloat4x4(&nodePair.second->GlobalTransform);
		nodeTransform = XMMatrixTranspose(nodeTransform);

		for (auto& mesh : nodePair.second->Meshs)
		{
			createMeshBoundingBox(mesh);
			mesh.MeshBoundingBox.Transform(mesh.MeshBoundingBox, nodeTransform);

			mesh.MeshBoundingBox.CreateMerged(_boundingBox, mesh.MeshBoundingBox, _boundingBox);
		}
	}
}

void KsmModel::createMeshBoundingBox(_In_ KsmModelMesh& mesh)
{
	vector<XMFLOAT3> positionVertices;

	if (HasAnimations)
	{
		for (const auto& vertex : mesh.AnimatedVertices)
		{
			positionVertices.push_back(vertex.Position);
		}
	}
	else
	{
		for (const auto& vertex : mesh.StaticVertices)
		{
			positionVertices.push_back(vertex.Position);
		}
	}

	XMFLOAT3 vMinf3(FLT_MAX, FLT_MAX, FLT_MAX);
	XMFLOAT3 vMaxf3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	XMVECTOR vMin = XMLoadFloat3(&vMinf3);
	XMVECTOR vMax = XMLoadFloat3(&vMaxf3);

	for (const auto& vertex : positionVertices)
	{
		XMVECTOR P = XMLoadFloat3(&vertex);
		vMin = XMVectorMin(vMin, P);
		vMax = XMVectorMax(vMax, P);
	}

	XMStoreFloat3(&mesh.MeshBoundingBox.Center, 0.5f*(vMin + vMax));
	XMStoreFloat3(&mesh.MeshBoundingBox.Extents, 0.5f*(vMax - vMin));
}
