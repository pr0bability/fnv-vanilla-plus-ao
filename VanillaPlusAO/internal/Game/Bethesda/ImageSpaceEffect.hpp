#pragma once

#include "ImageSpaceTexture.hpp"
#include "ImageSpaceShaderParam.hpp"
#include "ImageSpaceEffectParam.hpp"
#include "Gamebryo/NiTexturingProperty.hpp"

class ImageSpaceManager;
class NiScreenGeometry;
class NiDX9Renderer;
class ImageSpaceShader;

class ImageSpaceEffect {
public:
	virtual ~ImageSpaceEffect();
	virtual void Render(NiGeometry* apScreenShape, NiDX9Renderer* apRenderer, ImageSpaceEffectParam* apParam, bool abEndFrame);
	virtual void Setup(ImageSpaceManager* pISManager, ImageSpaceEffectParam* apParam);
	virtual void Shutdown();
	virtual void BorrowTextures(ImageSpaceEffectParam* apParam);
	virtual void ReturnTextures();
	virtual bool IsActive() const;
	virtual bool UpdateParams(ImageSpaceEffectParam* apParam);

	struct EffectInput {
		int32_t							iTexIndex;
		NiTexturingProperty::FilterMode iFilterMode;
	};


	bool										bIsActive;
	bool										bParamsChanged;
	NiTPrimitiveArray<ImageSpaceEffect*>		kEffects;
	NiTPrimitiveArray<ImageSpaceEffectParam*>	kEffectParams;
	NiTPrimitiveArray<ImageSpaceTexture*>		kTextures;
	NiTPrimitiveArray<EffectInput*>				kEffectInputs;
	NiTPrimitiveArray<uint32_t>					kEffectOutputs;

	ImageSpaceTexture* GetTexture(uint32_t aiIndex) const;
	void SetTexture(uint32_t auiIndex, ImageSpaceTexture* apTex, NiTexturingProperty::FilterMode aiFilterMode);

	void ApplyEffectTextures(uint32_t auiIndex);

	void SetEffectCount(uint32_t auiCount);
	void SetEffect(uint32_t auiIndex, ImageSpaceEffect* apEffect, ImageSpaceShaderParam* apParam, uint32_t auiOutput);

	void SetEffectInputs(uint32_t auiIndex, int32_t aiTexIndex, NiTexturingProperty::FilterMode aiFilterMode);
	void SetEffectInputs(uint32_t auiIndex, int32_t aiTexIndex0, int32_t aiTexIndex1, NiTexturingProperty::FilterMode aiFilterMode0, NiTexturingProperty::FilterMode aiFilterMode1);
	void SetEffectInputs(uint32_t auiIndex, int32_t aiTexIndex0, int32_t aiTexIndex1, int32_t aiTexIndex2, NiTexturingProperty::FilterMode aiFilterMode0, NiTexturingProperty::FilterMode aiFilterMode1, NiTexturingProperty::FilterMode aiFilterMode2);
	void SetEffectInputs(uint32_t auiIndex, int32_t aiTexIndex0, int32_t aiTexIndex1, int32_t aiTexIndex2, int32_t aiTexIndex3, NiTexturingProperty::FilterMode aiFilterMode0, NiTexturingProperty::FilterMode aiFilterMode1, NiTexturingProperty::FilterMode aiFilterMode2, NiTexturingProperty::FilterMode aiFilterMode3);
	void SetEffectInputs(uint32_t auiIndex, int32_t aiTexIndex0, int32_t aiTexIndex1, int32_t aiTexIndex2, int32_t aiTexIndex3, int32_t aiTexIndex4, NiTexturingProperty::FilterMode aiFilterMode0, NiTexturingProperty::FilterMode aiFilterMode1, NiTexturingProperty::FilterMode aiFilterMode2, NiTexturingProperty::FilterMode aiFilterMode3, NiTexturingProperty::FilterMode aiFilterMode4);
};

ASSERT_SIZE(ImageSpaceEffect, 0x58)