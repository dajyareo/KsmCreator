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

#include <map>

namespace KsmCreator
{
	///<summary>
	/// A Keyframe defines the bone transformation at an instant in time.
	///</summary>
	struct Keyframe
	{
		Keyframe();
		~Keyframe();

		float TimePos;
		DirectX::XMFLOAT3 Translation;
		DirectX::XMFLOAT3 Scale;
		DirectX::XMFLOAT4 RotationQuat;
	};

	///<summary>
	/// A BoneAnimation is defined by a list of keyframes.  For time
	/// values in between two key frames, we interpolate between the
	/// two nearest keyframes that bound the time.  
	///
	/// We assume an animation always has two keyframes.
	///</summary>
	struct BoneAnimation
	{
		float GetStartTime() const;
		float GetEndTime() const;

		void Interpolate(
			_In_ float t,
			_Out_ DirectX::XMFLOAT4X4& M,
			_In_opt_ bool interpolate = true) const;

		std::vector<Keyframe> Keyframes;
		std::wstring Name;
	};

	///<summary>
	/// Examples of AnimationClips are "Walk", "Run", "Attack", "Defend".
	/// An AnimationClip requires a BoneAnimation for every bone to form
	/// the animation clip.    
	///</summary>
	struct AnimationClip
	{
		float GetTimeStepAtFrame(_In_ UINT frame) const;

		void Interpolate(
			_In_ float t,
			_Out_ std::vector<DirectX::XMFLOAT4X4>& boneTransforms,
			_In_opt_ bool interpolate = true) const;

		std::vector<BoneAnimation> BoneAnimations;
		float TicksPerSecond;
		float Duration;
		std::wstring Name;
		UINT TotalFrames;
	};

	class AnimationData
	{
	public:

		UINT BoneCount() const;

		inline static void Transpose(_In_ DirectX::XMFLOAT4X4& m)
		{
			DirectX::XMMATRIX M = XMLoadFloat4x4(&m);

			M = XMMatrixTranspose(M);

			XMStoreFloat4x4(&m, M);
		}

		bool ContainsData() const
		{
			return _animations.size() > 0;
		}

		//float GetClipStartTime(_In_ const std::wstring& clipName) const;
		//float GetClipEndTime(_In_ const std::wstring& clipName) const;
		float GetTimeStepAtFrame(
			//_In_ const std::wstring& clipName, 
			_In_ UINT frame) const;

		//void Set(
		//	_In_ std::vector<int>& boneHierarchy, 
		//	_In_ std::vector<DirectX::XMFLOAT4X4>& boneOffsets,
		//	_In_ std::map<std::wstring, AnimationClip>& animations);

		void AddAnimationClip(_In_ AnimationClip animationClip)
		{
			animationClip.TotalFrames = static_cast<UINT>(animationClip.Duration * 30.0f);
			_animations[animationClip.Name] = animationClip;
			_clipName = animationClip.Name;
		}

		void SetCurrentClip(_In_ std::wstring clipName)
		{
			_clipName = clipName;
		}

		// TODO: cache the result if there was a chance
		// that you were calling this several times with the same clipName at 
		// the same timePos.
		void GetFinalTransforms(
			//_In_ const std::wstring& clipName, 
			_In_ float timePos, 
			_In_ std::vector<DirectX::XMFLOAT4X4>& localTransforms,
			_In_opt_ bool interpolate = true) const;

	private:

		std::map<std::wstring, AnimationClip> _animations;

		// The name for the combined clip (engine currently supports only 1 clip that contains all animations, but it
		// retains a lot of code to allow for changing this in the future)
		std::wstring _clipName;
	};

}