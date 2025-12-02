#include "Bethesda/BSMemory.hpp"
#include "Bethesda/BSShader.hpp"
#include "Bethesda/BSShaderManager.hpp"
#include "Bethesda/ImageSpaceManager.hpp"
#include "Bethesda/ImageSpaceEffect.hpp"
#include "Bethesda/ImageSpaceShader.hpp"
#include "Bethesda/ImageSpaceShaderParam.hpp"
#include "Bethesda/ShadowSceneNode.hpp"
#include "Gamebryo/NiCamera.hpp"
#include "Gamebryo/NiDX9Renderer.hpp"
#include "Misc/BSD3DTexture.hpp"
#include "Misc/CustomGameSetting.hpp"

#include "nvse/PluginAPI.h"
#include "simpleini/SimpleIni.h"

BS_ALLOCATORS

IDebugLog gLog("logs\\VanillaPlusAO.log");

// Constants.
static NVSEMessagingInterface*	pMsgInterface = nullptr;
static uint32_t					uiPluginHandle = 0;
static constexpr uint32_t		uiShaderLoaderVersion = 131;
static constexpr uint32_t		uiDepthResolveVersion = 120;

const char sSAO[] = "SAO";
const char sSAOBlur[] = "SAOBlur";
const char sSAOApply[] = "SAOApply";

// Statics.
typedef void (__cdecl pfn_AppendPostDepthEffect)(ImageSpaceEffect* apEffect);
static pfn_AppendPostDepthEffect* pAppendPostDepthEffectEnd = nullptr;

struct AOSettings {
	CustomGameSetting   bEnabled;
	CustomGameSetting	uiSamples;
	CustomGameSetting	uiTurns;
	CustomGameSetting	fRadius;
	CustomGameSetting	fBias;
	CustomGameSetting	fIntensity;
	CustomGameSetting	fEdgeSharpness;
	CustomGameSetting	fLuminanceThreshold;
	CustomGameSetting   fStrength;
	CustomGameSetting	fMinAmbient;
};

static AOSettings	kAOSettings[2];
static uint32_t		uiSAOWidth;
static uint32_t		uiSAOHeight;

static NiTexturePtr	spLinearizedDepthTex = nullptr;
static BSRenderedTexturePtr spAORT = nullptr;

static NiD3DPixelShaderPtr spShaderLinearizeDepth = nullptr;
static NiD3DPixelShaderPtr spShaderDownsampleDepth = nullptr;

static ImageSpaceShader* pShaderSAO = nullptr;
static ImageSpaceShader* pShaderSAOBlur = nullptr;
static ImageSpaceShader* pShaderSAOApply = nullptr;
static ImageSpaceEffect* pEffectSAO = nullptr;

template<const char* sName, uint32_t uiTextures>
class ImageSpaceShaderTemplate : public ImageSpaceShader {
private:
	static void* BSShader_vtable[85];
	static void* ImageSpaceEffect_vtable[8];
public:
	static ImageSpaceShaderTemplate<sName, uiTextures>* CreateObject() {
		ImageSpaceShaderTemplate<sName, uiTextures>* pShader = NiNew<ImageSpaceShaderTemplate<sName, uiTextures>>();

		// Call constructor for ImageSpaceShader and set vtable pointers.
#if GAME
		ThisCall(0xC04570, pShader);
#else
		ThisCall(0x977EE0, pShader);
#endif

		if (!BSShader_vtable[0]) [[unlikely]] {
			for (uint32_t i = 0; i < _countof(BSShader_vtable); i++) {
				BSShader_vtable[i] = ((void***)pShader)[0][i];
			}

			ReplaceVTableEntry(BSShader_vtable, 72, &ImageSpaceShaderTemplate<sName, uiTextures>::ReloadShadersEx);
		}
		((DWORD*)pShader)[0] = (DWORD)BSShader_vtable;

		if (!ImageSpaceEffect_vtable[0]) [[unlikely]] {
			for (uint32_t i = 0; i < _countof(ImageSpaceEffect_vtable); i++) {
				ImageSpaceEffect_vtable[i] = ((void***)pShader + 27)[0][i];
			}
		}
		((DWORD*)pShader + 27)[0] = (DWORD)ImageSpaceEffect_vtable;

		pShader->kTextures.SetSize(uiTextures);

		return pShader;
	}

	bool ReloadShadersEx() {
		LoadShadersEx(sName);
		return PresetStages();
	}
};

template<const char* sName, uint32_t uiTextures>
void* ImageSpaceShaderTemplate<sName, uiTextures>::BSShader_vtable[85] = {};

template<const char* sName, uint32_t uiTextures>
void* ImageSpaceShaderTemplate<sName, uiTextures>::ImageSpaceEffect_vtable[8] = {};

template class ImageSpaceShaderTemplate<sSAO, 2>;
template class ImageSpaceShaderTemplate<sSAOBlur, 2>;
template class ImageSpaceShaderTemplate<sSAOApply, 3>;

using ImageSpaceShaderSAO = ImageSpaceShaderTemplate<sSAO, 2>;
using ImageSpaceShaderSAOBlur = ImageSpaceShaderTemplate<sSAOBlur, 2>;
using ImageSpaceShaderSAOApply = ImageSpaceShaderTemplate<sSAOApply, 3>;

class ImageSpaceEffectSAO : public ImageSpaceEffect {
private:
	static void* vtable[8];
public:
	ImageSpaceTexture kBuffers[2];
	D3DXMATRIX		  kProjMatrix;
	D3DXMATRIX		  kInvProjMatrix;

	static ImageSpaceEffectSAO* CreateObject() {
		ImageSpaceEffectSAO* pEffect = NiNew<ImageSpaceEffectSAO>();

		// Call constructor for ImageSpaceEffect and set vtable pointers.
#if GAME
		ThisCall(0xBA4080, pEffect);
#else
		ThisCall(0x938840, pEffect);
#endif

		if (!vtable[0]) [[unlikely]] {
			for (uint32_t i = 0; i < _countof(vtable); i++) {
				vtable[i] = ((void***)pEffect)[0][i];
			}

			ReplaceVTableEntry(vtable, 1, &ImageSpaceEffectSAO::RenderEx);
			ReplaceVTableEntry(vtable, 2, &ImageSpaceEffectSAO::SetupEx);
			ReplaceVTableEntry(vtable, 4, &ImageSpaceEffectSAO::BorrowTexturesEx);
			ReplaceVTableEntry(vtable, 5, &ImageSpaceEffectSAO::ReturnTexturesEx);
			ReplaceVTableEntry(vtable, 6, &ImageSpaceEffectSAO::IsActiveEx);
			ReplaceVTableEntry(vtable, 7, &ImageSpaceEffectSAO::UpdateParamsEx);

		}
		((DWORD*)pEffect)[0] = (DWORD)vtable;

		pEffect->kTextures.SetSize(4);

		return pEffect;
	}

	void RenderEx(NiTriShape* apScreenShape, NiDX9Renderer* apRenderer, ImageSpaceEffectParam* apParam, bool bEndFrame) {
		NiDX9Renderer* pRenderer = NiDX9Renderer::GetSingleton();

		SetProjMatrix(pRenderer->m_kD3DProj);

		PrepareDepth();

		ThisCall(0xBA3F20, this, apScreenShape, apRenderer, apParam, bEndFrame);
	}

	void SetupEx(ImageSpaceManager* pISManager, ImageSpaceEffectParam* apParam) {
		SetEffectCount(4);

		float fTexelWidth = 1.0f / uiSAOWidth;
		float fTexelHeight = 1.0f / uiSAOHeight;

		// Generate AO.
		ImageSpaceShaderParam* pSAOParam = ImageSpaceShaderParam::CreateObject();
		pSAOParam->ResizeConstantGroup(0, 5);
		pSAOParam->SetPixelConstants(0, fTexelWidth, fTexelHeight, uiSAOWidth, uiSAOHeight);
		SetEffect(0, pShaderSAO, pSAOParam, 3);
		SetEffectInputs(0, 2, NiTexturingProperty::FILTER_NEAREST_MIPNEAREST);

		// Blur AO.
		ImageSpaceShaderParam* pBlurVerticalParam = ImageSpaceShaderParam::CreateObject();
		pBlurVerticalParam->ResizeConstantGroup(0, 2);
		pBlurVerticalParam->SetPixelConstants(0, fTexelWidth, fTexelHeight, 0.0f, 1.0f);
		SetEffect(1, pShaderSAOBlur, pBlurVerticalParam, 3);
		SetEffectInputs(1, 3, NiTexturingProperty::FILTER_NEAREST);

		ImageSpaceShaderParam* pBlurHorizontalParam = ImageSpaceShaderParam::CreateObject();
		pBlurHorizontalParam->ResizeConstantGroup(0, 2);
		pBlurHorizontalParam->SetPixelConstants(0, fTexelWidth, fTexelHeight, 1.0f, 0.0f);
		SetEffect(2, pShaderSAOBlur, pBlurHorizontalParam, 3);
		SetEffectInputs(2, 3, NiTexturingProperty::FILTER_NEAREST);

		// Apply AO.
		ImageSpaceShaderParam* pApplyParam = ImageSpaceShaderParam::CreateObject();
		pApplyParam->ResizeConstantGroup(0, 3);
		SetEffect(3, pShaderSAOApply, pApplyParam, 0);
		SetEffectInputs(3, 1, 3, NiTexturingProperty::FILTER_NEAREST, NiTexturingProperty::FILTER_NEAREST);
	}

	void BorrowTexturesEx(ImageSpaceEffectParam* apParam) {
		kBuffers[0].SetTexture(spLinearizedDepthTex);
		kBuffers[1].SetTexture(spAORT);

		kTextures.SetAt(2, &kBuffers[0]);
		kTextures.SetAt(3, &kBuffers[1]);
	}

	void ReturnTexturesEx() {
		kTextures.GetAt(2)->ClearTexture();
		kTextures.GetAt(3)->ClearTexture();

		kTextures.SetAt(2, nullptr);
		kTextures.SetAt(3, nullptr);
	}

	bool IsActiveEx() {
		const AOSettings& kSettings = kAOSettings[BSShaderManager::bIsInInterior];
		return kSettings.bEnabled.Bool();
	}

	bool UpdateParamsEx(ImageSpaceEffectParam* apParam) {
		const AOSettings& kSettings = kAOSettings[BSShaderManager::bIsInInterior];

		NiCamera* pSceneCamera = BSShaderManager::pCamera;
		float fNear = pSceneCamera->m_kViewFrustum.m_fNear;
		float fFar = pSceneCamera->m_kViewFrustum.m_fFar;
		float fFmN = fFar - fNear;
		float fNtF = fNear * fFar;

		// SAO constants.
		float fRadiusScalingFactor = 2.0f / kProjMatrix._11 * (1.0f / uiSAOWidth);
		float fRadius = kSettings.fRadius.Float() * dM2NI;
		float fBias = kSettings.fBias.Float() * dM2NI;
		float fIntensityNormalized = kSettings.fIntensity.Float() * dM2NI / pow(fRadius, 6.0f);

		ImageSpaceShaderParam* pSAOParam = static_cast<ImageSpaceShaderParam*>(kEffectParams.GetAt(0));
		pSAOParam->SetPixelConstants(1, fNear, fFar, fFmN, fNtF);
		pSAOParam->SetPixelConstants(2, kSettings.uiSamples.UInt(), kSettings.uiTurns.UInt(), 0.0f, 0.0f);
		pSAOParam->SetPixelConstants(3, fRadiusScalingFactor, fRadius, fBias, fIntensityNormalized);
		pSAOParam->SetPixelConstants(4, 2.0f / kProjMatrix._11, -2.0f / kProjMatrix._22, -1.0f / kProjMatrix._11, 1.0f / kProjMatrix._22);

		// Blur constants.
		ImageSpaceShaderParam* pBlurVerticalParam = static_cast<ImageSpaceShaderParam*>(kEffectParams.GetAt(1));
		pBlurVerticalParam->SetPixelConstants(1, kSettings.fEdgeSharpness.Float(), 0.0f, 0.0f, 0.0f);

		ImageSpaceShaderParam* pBlurHorizontalParam = static_cast<ImageSpaceShaderParam*>(kEffectParams.GetAt(2));
		pBlurHorizontalParam->SetPixelConstants(1, kSettings.fEdgeSharpness.Float(), 0.0f, 0.0f, 0.0f);

		// Apply constants.
		float fFogDistNear = 0.0f;
		float fFogDistFar = 500000.0f;
		float fFogPower = 0.0f;

		BSFogProperty* pFogProp = ShadowSceneNode::GetFogProperty(BSShaderManager::GetFogIndex());

		if (pFogProp) {
			fFogDistNear = pFogProp->fEndDistance - pFogProp->fStartDistance;
			fFogDistFar = pFogProp->fEndDistance;
			fFogPower = pFogProp->fPower;
		}

		ImageSpaceShaderParam* pApplyParam = static_cast<ImageSpaceShaderParam*>(kEffectParams.GetAt(3));
		pApplyParam->SetPixelConstants(0, fNear, fFar, fFmN, fNtF);
		pApplyParam->SetPixelConstants(1, kSettings.fStrength.Float(), kSettings.fLuminanceThreshold.Float(), kSettings.fMinAmbient.Float(), 0.0f);
		pApplyParam->SetPixelConstants(2, fFogDistNear, fFogDistFar, fFogPower, 0.0f);

		return true;
	}

	void PrepareDepth() {
		BSD3DTexture* pHardwareDepth = static_cast<BSD3DTexture*>(ImageSpaceManager::GetDepthTexture());
		BSD3DTexture* pLinearizedDepth = static_cast<BSD3DTexture*>(spLinearizedDepthTex.m_pObject);

		IDirect3DTexture9* pLinearizedDepthTex = static_cast<IDirect3DTexture9*>(pLinearizedDepth->GetDX9RendererData()->GetD3DTexture());
		IDirect3DSurface9* pLinearizedDepthSurf;
		pLinearizedDepthTex->GetSurfaceLevel(0, &pLinearizedDepthSurf);

		uint32_t uiWidth = uiSAOWidth;
		uint32_t uiHeight = uiSAOHeight;

		NiDX9Renderer* pRenderer = NiDX9Renderer::GetSingleton();
		IDirect3DDevice9* pDevice = pRenderer->GetD3DDevice();

		IDirect3DSurface9* pOriginalRT = nullptr;
		pDevice->GetRenderTarget(0, &pOriginalRT);

		// Linearize depth first.
		pDevice->SetPixelShader(spShaderLinearizeDepth->GetShaderHandle());

		DWORD dwOrigCull, dwOrigAlphaEnable, dwOrigZEnable, dwOrigZWriteEnable, dwOrigFVF;
		pDevice->GetRenderState(D3DRS_CULLMODE, &dwOrigCull);
		pDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwOrigAlphaEnable);
		pDevice->GetRenderState(D3DRS_ZENABLE, &dwOrigZEnable);
		pDevice->GetRenderState(D3DRS_ZWRITEENABLE, &dwOrigZWriteEnable);

		pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, D3DZB_FALSE);
		pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
		pDevice->SetRenderState(D3DRS_ZWRITEENABLE, D3DZB_FALSE);

		pDevice->SetRenderTarget(0, pLinearizedDepthSurf);

		pDevice->SetTexture(0, pHardwareDepth->GetDX9RendererData()->GetD3DTexture());
		pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);

		NiCamera* pSceneCamera = BSShaderManager::pCamera;
		float fNear = pSceneCamera->m_kViewFrustum.m_fNear;
		float fFar = pSceneCamera->m_kViewFrustum.m_fFar;
		float fFmN = fFar - fNear;
		float fNtF = fNear * fFar;

		D3DXVECTOR4 kDepthConstants(fNear, fFar, fFmN, fNtF);

		pDevice->SetPixelShaderConstantF(0, (float*)&kDepthConstants, 1);

		struct Vertex {
			float x, y, z, rhw;
			float u, v;
		};

		Vertex vertices[] = {
			{-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f},
			{uiWidth - 0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f},
			{-0.5f, uiHeight - 0.5f, 0.0f, 1.0f, 0.0f, 1.0f},
			{uiWidth - 0.5f, uiHeight - 0.5f, 0.0f, 1.0f, 1.0f, 1.0f}
		};

		pDevice->GetFVF(&dwOrigFVF);
		pDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);

		pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(Vertex));

		// Render mip chain.
		pDevice->SetPixelShader(spShaderDownsampleDepth->GetShaderHandle());

		D3DXVECTOR4 kScreenData;

		pDevice->SetTexture(0, pLinearizedDepthTex);
		pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);

		for (uint32_t i = 1; i < pLinearizedDepthTex->GetLevelCount(); i++) {
			kScreenData.x = 1.0f / uiWidth;
			kScreenData.y = 1.0f / uiHeight;
			kScreenData.z = i - 1;

			uiWidth /= 2;
			uiHeight /= 2;

			pLinearizedDepthSurf->Release();
			pLinearizedDepthTex->GetSurfaceLevel(i, &pLinearizedDepthSurf);

			pDevice->SetRenderTarget(0, pLinearizedDepthSurf);

			pDevice->SetPixelShaderConstantF(0, (float*)&kScreenData, 1);

			vertices[1].x = uiWidth - 0.5f;
			vertices[2].y = uiHeight - 0.5f;
			vertices[3].x = uiWidth - 0.5f;
			vertices[3].y = uiHeight - 0.5f;

			pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(Vertex));
		}

		// Restore original render target
		pDevice->SetRenderState(D3DRS_CULLMODE, dwOrigCull);
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, dwOrigAlphaEnable);
		pDevice->SetRenderState(D3DRS_ZENABLE, dwOrigZEnable);
		pDevice->SetRenderState(D3DRS_ZWRITEENABLE, dwOrigZWriteEnable);
		pDevice->SetFVF(dwOrigFVF);
		pDevice->SetRenderTarget(0, pOriginalRT);
		pLinearizedDepthSurf->Release();

		if (pOriginalRT) {
			pOriginalRT->Release();
		}
	}

	void SetProjMatrix(const D3DXMATRIX& arMatrix) {
		kProjMatrix = arMatrix;
		D3DXMatrixInverse(&kInvProjMatrix, nullptr, &kProjMatrix);
	}
};

void* ImageSpaceEffectSAO::vtable[8] = {};

void LoadSettings() {
	CSimpleIniA kIni;
	kIni.SetUnicode();
	SI_Error rc = kIni.LoadFile("Data\\NVSE\\Plugins\\VanillaPlusAO.ini");
	if (rc < 0) {
		_MESSAGE("No INI, using default values.");
	};

	kAOSettings[0].bEnabled.Initialize("bAOExtEnabled", static_cast<bool>(kIni.GetBoolValue("Exterior", "bEnabled", true)));
	kAOSettings[0].uiSamples.Initialize("uAOExtSamples", static_cast<uint32_t>(kIni.GetLongValue("Exterior", "uSamples", 9)));
	kAOSettings[0].uiTurns.Initialize("uAOExtTurns", static_cast<uint32_t>(kIni.GetLongValue("Exterior", "uTurns", 7)));
	kAOSettings[0].fRadius.Initialize("fAOExtRadius", static_cast<float>(kIni.GetDoubleValue("Exterior", "fRadius", 1.5)));
	kAOSettings[0].fBias.Initialize("fAOExtBias", static_cast<float>(kIni.GetDoubleValue("Exterior", "fBias", 0.02)));
	kAOSettings[0].fIntensity.Initialize("fAOExtIntensity", static_cast<float>(kIni.GetDoubleValue("Exterior", "fIntensity", 1.0)));
	kAOSettings[0].fEdgeSharpness.Initialize("fAOExtEdgeSharpness", static_cast<float>(kIni.GetDoubleValue("Exterior", "fEdgeSharpness", 2.0)));
	kAOSettings[0].fLuminanceThreshold.Initialize("fAOExtLuminanceThreshold", static_cast<float>(kIni.GetDoubleValue("Exterior", "fLuminanceThreshold", 1.0)));
	kAOSettings[0].fStrength.Initialize("fAOExtStrength", static_cast<float>(kIni.GetDoubleValue("Exterior", "fStrength", 1.5)));
	kAOSettings[0].fMinAmbient.Initialize("fAOExtMinAmbient", static_cast<float>(kIni.GetDoubleValue("Exterior", "fMinAmbient", 0.1)));

	kAOSettings[1].bEnabled.Initialize("bAOIntEnabled", static_cast<bool>(kIni.GetBoolValue("Interior", "bEnabled", true)));
	kAOSettings[1].uiSamples.Initialize("uAOIntSamples", static_cast<uint32_t>(kIni.GetLongValue("Interior", "uSamples", 9)));
	kAOSettings[1].uiTurns.Initialize("uAOIntTurns", static_cast<uint32_t>(kIni.GetLongValue("Interior", "uTurns", 7)));
	kAOSettings[1].fRadius.Initialize("fAOIntRadius", static_cast<float>(kIni.GetDoubleValue("Interior", "fRadius", 1.5)));
	kAOSettings[1].fBias.Initialize("fAOIntBias", static_cast<float>(kIni.GetDoubleValue("Interior", "fBias", 0.02)));
	kAOSettings[1].fIntensity.Initialize("fAOIntIntensity", static_cast<float>(kIni.GetDoubleValue("Interior", "fIntensity", 1.0)));
	kAOSettings[1].fEdgeSharpness.Initialize("fAOIntEdgeSharpness", static_cast<float>(kIni.GetDoubleValue("Interior", "fEdgeSharpness", 1.0)));
	kAOSettings[1].fLuminanceThreshold.Initialize("fAOIntLuminanceThreshold", static_cast<float>(kIni.GetDoubleValue("Interior", "fLuminanceThreshold", 1.0)));
	kAOSettings[1].fStrength.Initialize("fAOIntStrength", static_cast<float>(kIni.GetDoubleValue("Interior", "fStrength", 1.5)));
	kAOSettings[1].fMinAmbient.Initialize("fAOIntMinAmbient", static_cast<float>(kIni.GetDoubleValue("Interior", "fMinAmbient", 0.1)));
}

void InitializeTextures() {
	NiDX9Renderer* pRenderer = NiDX9Renderer::GetSingleton();

	if (BSShaderManager::bLetterBox) {
		uiSAOWidth = BSShaderManager::iLetterboxWidth;
		uiSAOHeight = BSShaderManager::iLetterboxHeight;
	}
	else {
		uiSAOWidth = pRenderer->GetScreenWidth();
		uiSAOHeight = pRenderer->GetScreenHeight();
	}

	IDirect3DDevice9* pDevice = NiDX9Renderer::GetSingleton()->GetD3DDevice();
	IDirect3DTexture9* pD3DTexture = nullptr;
	pDevice->CreateTexture(uiSAOWidth, uiSAOHeight, 6, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT, &pD3DTexture, nullptr);

	spLinearizedDepthTex = BSD3DTexture::CreateObject(pD3DTexture);

	NiTexture::FormatPrefs kTextureFormat(NiTexture::FormatPrefs::DOUBLE_COLOR_32, NiTexture::FormatPrefs::ALPHA_DEFAULT, NiTexture::FormatPrefs::NO);
	BSRenderedTexture::bUseCustomFormat = true;
	BSRenderedTexture::bIsRenderTarget = true;
	BSRenderedTexture::eFormat = D3DFMT_G16R16F;
	spAORT = BSRenderedTexture::CreateTexture("AO", uiSAOWidth, uiSAOHeight, kTextureFormat, Ni2DBuffer::MULTISAMPLE_NONE, false, nullptr, 0, 0);
}

void InitializeShaders() {
	pShaderSAO = ImageSpaceShaderSAO::CreateObject();
	pShaderSAOBlur = ImageSpaceShaderSAOBlur::CreateObject();
	pShaderSAOApply = ImageSpaceShaderSAOApply::CreateObject();
	pEffectSAO = ImageSpaceEffectSAO::CreateObject();

	ImageSpaceManager* pISManager = ImageSpaceManager::GetSingleton();

	spShaderLinearizeDepth = BSShader::pCreatePixelShader("VPAO\\ISLINEARIZEDEPTH.pso");
	spShaderDownsampleDepth = BSShader::pCreatePixelShader("VPAO\\ISDOWNDEPTH.pso");

	pShaderSAO->ReloadShaders();
	pShaderSAOBlur->ReloadShaders();
	pShaderSAOApply->ReloadShaders();
	pEffectSAO->Setup(pISManager, nullptr);
}

bool TextureResetCallback(bool abBeforeReset, void* pvData) {
	if (abBeforeReset) {
		spLinearizedDepthTex = nullptr;
	}
	else {
		NiDX9Renderer* pRenderer = NiDX9Renderer::GetSingleton();
		if (!pRenderer || !pRenderer->GetD3DDevice()) {
			_MESSAGE("Device not available during reset callback");
			return false;
		}

		IDirect3DDevice9* pDevice = NiDX9Renderer::GetSingleton()->GetD3DDevice();
		IDirect3DTexture9* pD3DTexture = nullptr;
		pDevice->CreateTexture(uiSAOWidth, uiSAOHeight, 6, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT, &pD3DTexture, nullptr);

		spLinearizedDepthTex = BSD3DTexture::CreateObject(pD3DTexture);
	}

	return true;
}

void MessageHandler(NVSEMessagingInterface::Message* msg) {
	switch (msg->type) {
	case NVSEMessagingInterface::kMessage_DeferredInit:
		InitializeTextures();
		InitializeShaders();
		pAppendPostDepthEffectEnd(pEffectSAO);

		NiDX9Renderer::GetSingleton()->AddResetNotificationFunc(TextureResetCallback, nullptr);

		break;
	}
}

void ShaderLoaderMessageHandler(NVSEMessagingInterface::Message* apMessage) {
	switch (apMessage->type) {
	case 0:
		spShaderLinearizeDepth = BSShader::pCreatePixelShader("VPAO\\ISLINEARIZEDEPTH.pso");
		spShaderDownsampleDepth = BSShader::pCreatePixelShader("VPAO\\ISDOWNDEPTH.pso");
		pShaderSAO->ReloadShaders();
		pShaderSAOBlur->ReloadShaders();
		pShaderSAOApply->ReloadShaders();
		break;
	}
}

EXTERN_DLL_EXPORT bool NVSEPlugin_Query(const NVSEInterface* nvse, PluginInfo* info) {
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "Vanilla Plus AO";
	info->version = 100;

	return !nvse->isEditor;
}

EXTERN_DLL_EXPORT bool NVSEPlugin_Load(NVSEInterface* nvse) {
	HMODULE hShaderLoader = GetModuleHandle("Fallout Shader Loader.dll");
	HMODULE hLODFlickerFix = GetModuleHandle("LODFlickerFix.dll");
	HMODULE hDepthResolve = GetModuleHandle("DepthResolve.dll");

	if (!hShaderLoader) {
		MessageBox(NULL, "Fallout Shader Loader not found.\nVanilla Plus AO cannot be used without it, please install it.", "Vanilla Plus AO", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}

	if (!hLODFlickerFix) {
		MessageBox(NULL, "LOD Flicker Fix not found.\nDepth Resolve cannot be used without it, please install it.", "Vanilla Plus AO", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}

	if (!hDepthResolve) {
		MessageBox(NULL, "Depth Resolve not found.\nVanilla Plus AO cannot be used without it, please install it.", "Vanilla Plus AO", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}

	pMsgInterface = (NVSEMessagingInterface*)nvse->QueryInterface(kInterface_Messaging);
	uiPluginHandle = nvse->GetPluginHandle();
	pMsgInterface->RegisterListener(uiPluginHandle, "NVSE", MessageHandler);
	pMsgInterface->RegisterListener(uiPluginHandle, "Shader Loader", ShaderLoaderMessageHandler);

	auto pQuery = (_NVSEPlugin_Query)GetProcAddress(hShaderLoader, "NVSEPlugin_Query");
	PluginInfo kInfo = {};
	pQuery(nvse, &kInfo);
	if (kInfo.version < uiShaderLoaderVersion) {
		char cBuffer[192];
		sprintf_s(cBuffer, "Fallout Shader Loader is outdated.\nPlease update it to use Vanilla Plus AO!\nCurrent version: %i\nMinimum required version: %i", kInfo.version, uiShaderLoaderVersion);
		MessageBox(NULL, cBuffer, "Vanilla Plus AO", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}

	BSShader::pCreateVertexShader = (pfn_CreateVertexShader*)GetProcAddress(hShaderLoader, "CreateVertexShader");
	BSShader::pCreatePixelShader = (pfn_CreatePixelShader*)GetProcAddress(hShaderLoader, "CreatePixelShader");

	if (!BSShader::pCreatePixelShader || !BSShader::pCreateVertexShader) {
		MessageBox(NULL, "Failed to load shader loader functions.", "Vanilla Plus AO", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}

	pQuery = (_NVSEPlugin_Query)GetProcAddress(hDepthResolve, "NVSEPlugin_Query");
	pQuery(nvse, &kInfo);
	if (kInfo.version < uiDepthResolveVersion) {
		char cBuffer[192];
		sprintf_s(cBuffer, "Depth Resolve is outdated.\nPlease update it to use Vanilla Plus AO!\nCurrent version: %i\nMinimum required version: %i", kInfo.version, uiDepthResolveVersion);
		MessageBox(NULL, cBuffer, "Vanilla Plus AO", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}

	pAppendPostDepthEffectEnd = (pfn_AppendPostDepthEffect*)GetProcAddress(hDepthResolve, "AppendPostDepthEffect");

	if (!pAppendPostDepthEffectEnd) {
		MessageBox(NULL, "Failed to load Depth Resolve functions.", "Vanilla Plus AO", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}

	LoadSettings();

	return true;
}

BOOL WINAPI DllMain(
	HANDLE  hDllHandle,
	DWORD   dwReason,
	LPVOID  lpreserved
)
{
	return TRUE;
}