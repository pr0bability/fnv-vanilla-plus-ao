#pragma once

#include "BSCompoundFrustum.hpp"
#include "BSCubeMapCamera.hpp"
#include "BSFogProperty.hpp"
#include "Gamebryo/NiNode.hpp"
#include "Gamebryo/NiTriBasedGeom.hpp"
#include "ShadowSceneLight.hpp"

class BSCullingProcess;
class NiDirectionalLight;

NiSmartPointer(ShadowSceneNode);

class ShadowSceneNode : public NiNode {
public:
	bool									bFrustumAddedToVis;
	float									fStoredFarClip;
	NiTPointerList<ShadowSceneLightPtr>		kLights;
	NiTPointerList<ShadowSceneLightPtr>		kShadowLights;
	NiTListIterator							kShadowLightsIter;
	NiTListIterator							kShadowLightActiveFence;
	NiTListIterator							kShadowLightFadingInFence;
	NiPointer<ShadowSceneLight>				spShadowLightActiveFenceObject;
	NiPointer<ShadowSceneLight>				spShadowLightFadingInFenceObject;
	ShadowSceneLight*						pSunLight;
	NiTPointerList<ShadowSceneLightPtr>		kLightQueueAdd;
	NiTPointerList<ShadowSceneLightPtr>		kLightQueueRemove;
	NiTPointerList<ShadowSceneLightPtr>		kLightQueueUpdate;
	NiTPointerList<NiAVObjectPtr>			kObjectQueueUpdate;
	NiTPointerList<NiAVObjectPtr>			kObjectQueueUpdateActiveOnly;
	uint8_t									ucSceneGraphIndex;
	NiPointer<BSRenderedTexture>			spWindowReflectionEnvMap;
	NiPointer<BSCubeMapCamera>				spCubeMapCamera;
	int32_t									iCurrentCubeMapFace;
	bool									bDisableLightUpdate;
	bool									bWireframe;
	BSFogPropertyPtr						spFog;
	BSCompoundFrustum						kCompoundFrustum;
	bool									bVisibleUnboundSpace;
	void*									pBSPortalGraph;
	NiPoint3								kLightingOffset;
	NiPoint3								kEyePosition;
	bool									bAllowLightRemoveQueues;

	NIRTTI_ADDRESS(0x11F9E80);

	static BSFogProperty* GetFogProperty(uint32_t aeType);

	void UpdateObjectLighting(NiAVObject* apObject, bool abIsMoving);

	void ClearLightsFromNode(NiAVObject* apObject);
};

ASSERT_SIZE(ShadowSceneNode, 0x200);