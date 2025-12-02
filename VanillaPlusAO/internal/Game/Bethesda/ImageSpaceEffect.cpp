#include "ImageSpaceEffect.hpp"

ImageSpaceTexture* ImageSpaceEffect::GetTexture(uint32_t aiIndex) const {
	return kTextures.GetAt(aiIndex);
}

// GAME - 0xBA3CB0
// GECK - 0x9384A0
void ImageSpaceEffect::SetTexture(uint32_t auiIndex, ImageSpaceTexture* apTex, NiTexturingProperty::FilterMode aiFilterMode) {
	kTextures.SetAt(auiIndex, apTex);
	if (apTex)
		apTex->iFilterMode = aiFilterMode;
}

// GAME - 0xBA3E30
// GECK - 0x938620
void ImageSpaceEffect::ApplyEffectTextures(uint32_t auiIndex) {
#if GAME
	ThisCall(0xBA3E30, this, auiIndex);
#else
	ThisCall(0x938620, this, auiIndex);
#endif
}


void ImageSpaceEffect::SetEffectCount(uint32_t auiCount) {
	kEffects.SetSize(auiCount);
	kEffectParams.SetSize(auiCount);
	kEffectInputs.SetSize(auiCount);
	kEffectOutputs.SetSize(auiCount);
}

void ImageSpaceEffect::SetEffect(uint32_t auiIndex, ImageSpaceEffect* apEffect, ImageSpaceShaderParam* apParam, uint32_t auiOutput) {
	kEffects.SetAt(auiIndex, apEffect);
	kEffectParams.SetAt(auiIndex, apParam);
	kEffectOutputs.SetAt(auiIndex, auiOutput);
}

void ImageSpaceEffect::SetEffectInputs(uint32_t auiIndex, int32_t aiTexIndex, NiTexturingProperty::FilterMode aiFilterMode) {
	EffectInput* pEffectInput = new EffectInput;
	pEffectInput->iTexIndex = aiTexIndex;
	pEffectInput->iFilterMode = aiFilterMode;
	kEffectInputs.SetAt(auiIndex, pEffectInput);
}

void ImageSpaceEffect::SetEffectInputs(uint32_t auiIndex, int32_t aiTexIndex0, int32_t aiTexIndex1, NiTexturingProperty::FilterMode aiFilterMode0, NiTexturingProperty::FilterMode aiFilterMode1) {
	EffectInput* pEffectInput = new EffectInput[2];
	pEffectInput[0].iTexIndex = aiTexIndex0;
	pEffectInput[0].iFilterMode = aiFilterMode0;
	pEffectInput[1].iTexIndex = aiTexIndex1;
	pEffectInput[1].iFilterMode = aiFilterMode1;
	kEffectInputs.SetAt(auiIndex, pEffectInput);
}

void ImageSpaceEffect::SetEffectInputs(uint32_t auiIndex, int32_t aiTexIndex0, int32_t aiTexIndex1, int32_t aiTexIndex2, NiTexturingProperty::FilterMode aiFilterMode0, NiTexturingProperty::FilterMode aiFilterMode1, NiTexturingProperty::FilterMode aiFilterMode2) {
	EffectInput* pEffectInput = new EffectInput[3];
	pEffectInput[0].iTexIndex = aiTexIndex0;
	pEffectInput[0].iFilterMode = aiFilterMode0;
	pEffectInput[1].iTexIndex = aiTexIndex1;
	pEffectInput[1].iFilterMode = aiFilterMode1;
	pEffectInput[2].iTexIndex = aiTexIndex2;
	pEffectInput[2].iFilterMode = aiFilterMode2;
	kEffectInputs.SetAt(auiIndex, pEffectInput);
}

void ImageSpaceEffect::SetEffectInputs(uint32_t auiIndex, int32_t aiTexIndex0, int32_t aiTexIndex1, int32_t aiTexIndex2, int32_t aiTexIndex3, NiTexturingProperty::FilterMode aiFilterMode0, NiTexturingProperty::FilterMode aiFilterMode1, NiTexturingProperty::FilterMode aiFilterMode2, NiTexturingProperty::FilterMode aiFilterMode3) {
	EffectInput* pEffectInput = new EffectInput[4];
	pEffectInput[0].iTexIndex = aiTexIndex0;
	pEffectInput[0].iFilterMode = aiFilterMode0;
	pEffectInput[1].iTexIndex = aiTexIndex1;
	pEffectInput[1].iFilterMode = aiFilterMode1;
	pEffectInput[2].iTexIndex = aiTexIndex2;
	pEffectInput[2].iFilterMode = aiFilterMode2;
	pEffectInput[3].iTexIndex = aiTexIndex3;
	pEffectInput[3].iFilterMode = aiFilterMode3;
	kEffectInputs.SetAt(auiIndex, pEffectInput);
}

void ImageSpaceEffect::SetEffectInputs(uint32_t auiIndex, int32_t aiTexIndex0, int32_t aiTexIndex1, int32_t aiTexIndex2, int32_t aiTexIndex3, int32_t aiTexIndex4, NiTexturingProperty::FilterMode aiFilterMode0, NiTexturingProperty::FilterMode aiFilterMode1, NiTexturingProperty::FilterMode aiFilterMode2, NiTexturingProperty::FilterMode aiFilterMode3, NiTexturingProperty::FilterMode aiFilterMode4) {
	EffectInput* pEffectInput = new EffectInput[5];
	pEffectInput[0].iTexIndex = aiTexIndex0;
	pEffectInput[0].iFilterMode = aiFilterMode0;
	pEffectInput[1].iTexIndex = aiTexIndex1;
	pEffectInput[1].iFilterMode = aiFilterMode1;
	pEffectInput[2].iTexIndex = aiTexIndex2;
	pEffectInput[2].iFilterMode = aiFilterMode2;
	pEffectInput[3].iTexIndex = aiTexIndex3;
	pEffectInput[3].iFilterMode = aiFilterMode3;
	pEffectInput[4].iTexIndex = aiTexIndex4;
	pEffectInput[4].iFilterMode = aiFilterMode4;
	kEffectInputs.SetAt(auiIndex, pEffectInput);
}
