#pragma once

#include "BSRenderedTexture.hpp"
#include "BSSimpleArray.hpp"
#include "Gamebryo/NiLight.hpp"
#include "Gamebryo/NiCamera.hpp"
#include "Gamebryo/NiFrustumPlanes.hpp"

class BSShaderAccumulator;
class NiTriBasedGeom;
class BSMultiBoundRoom;
class BSPortal;
class BSPortalGraph;
class BSPortalSharedNode;
class BSCullingProcess;

NiSmartPointer(ShadowSceneLight);

class ShadowSceneLight : public NiRefObject {
public:
	uint32_t									unk008;
	float										fLuminance;
	D3DXMATRIX									kViewProjMatrix;
	D3DXMATRIX									kViewMatrix;
	D3DXMATRIX									kProjMatrix;
	float										fLODDimmer;
	float										fFade;
	float										fFadeTarget;
	float										fFadePercent;
	NiTPointerList<NiPointer<NiTriBasedGeom>>	kGeometryList;
	bool										bCastsShadow;
	bool										byte0ED;
	NiAVObjectPtr								spObject0F0;
	bool										bPointLight;
	bool										bAmbientLight;
	NiLightPtr									spLight;
	bool										bDynamicLight;
	NiPoint3									kDynamicPosition;
	BSRenderedTexturePtr						spShadowRenderTarget;
	uint16_t									usFrustumCull;
	NiAVObjectPtr								spObject114;
	bool										bRenderShadows;
	float										fFOV;
	float										fFalloffExponent;
	bool										bShowDebugTexture;
	NiPointer<NiAVObject>						spShadowCaster;
	NiTPointerList<NiObjectPtr>					kList12C;
	NiTListItem<NiPointer<NiTriBasedGeom>>*		pGeomListFence;
	NiPointer<NiTriBasedGeom>					spFenceObject;
	NiPointer<NiCamera>							spShadowMapCamera;
	NiFrustumPlanes								kFrustumPlanes;
	float										fClipPlanes[24];
	bool										bSetClipPlanes;
	NiPointer<BSShaderAccumulator>				spShadowAccum;
	BSSimpleArray<BSMultiBoundRoom*>			kMultiBoundRooms;
	BSSimpleArray<BSPortal*>					kPortals;
	BSSimpleArray<BSPortalSharedNode*>			kPortalSharedNodes;
	BSPortalGraph*								pPortalGraph;
	uint32_t									unk244;
	uint32_t									unk248;
	uint32_t									unk24C;

	NiLight* GetLight() const;
};

ASSERT_SIZE(ShadowSceneLight, 0x250);