#include "ImageSpaceShaderParam.hpp"
#include "BSMemory.hpp"

// GAME - 0xB8A9E0
// GECK - 0x939B20
ImageSpaceShaderParam* ImageSpaceShaderParam::CreateObject() {
#ifdef GAME
	return BSMemory::create<ImageSpaceShaderParam, 0xB8A9E0>();
#else
	return BSMemory::create<ImageSpaceShaderParam, 0x939B20>()
#endif
}

// GAME - 0xB8A7E0
// GECK - 0x939920
void ImageSpaceShaderParam::ClearConstantGroups() {
#if GAME
	ThisCall(0xB8A7E0, this);
#else
	ThisCall(0x939920, this);
#endif
}

// GAME - 0xB8AB10
// GECK - 0x939C50
void ImageSpaceShaderParam::ResizeConstantGroup(uint32_t auiVertexGroupSize, uint32_t auiPixelGroupSize) {
#if GAME
	ThisCall(0xB8AB10, this, auiVertexGroupSize, auiPixelGroupSize);
#else
	ThisCall(0x939C50, this, auiVertexGroupSize, auiPixelGroupSize);
#endif
}

// GAME - 0xB8A760
// GECK - 0x9398A0
void ImageSpaceShaderParam::SetVertexConstants(uint32_t auiIndex, float afVal1, float afVal2, float afVal3, float afVal4) {
#if GAME
	ThisCall(0xB8A760, this, auiIndex, afVal1, afVal2, afVal3, afVal4);
#else
	ThisCall(0x9398A0, this, auiIndex, afVal1, afVal2, afVal3, afVal4);
#endif
}

// GAME - 0xB8A790
// GECK - 0x9398D0
void ImageSpaceShaderParam::SetPixelConstants(uint32_t auiIndex, float afVal1, float afVal2, float afVal3, float afVal4) {
#if GAME
	ThisCall(0xB8A790, this, auiIndex, afVal1, afVal2, afVal3, afVal4);
#else
	ThisCall(0x9398D0, this, auiIndex, afVal1, afVal2, afVal3, afVal4);
#endif
}

// GAME - 0xB8A6E0
// GECK - 0x939820
void ImageSpaceShaderParam::SetTextureOffsets(uint32_t auiTexture, uint32_t auiWidth, uint32_t auiHeight) {
	if (auiTexture > 2)
		return;

	float fOffsetX = 0.f;
	float fOffsetY = 0.f;

	if (!auiTexture || bHasOffset[auiTexture]) {
		fOffsetX = 0.5f / auiWidth;
		fOffsetY = 0.5f / auiHeight;
	}

	SetVertexConstants(auiTexture, fOffsetX, fOffsetY, 0.f, 0.f);
}


void ImageSpaceShaderParam::CopyVertexConstants(uint32_t auiIndex, float* apData, uint32_t auiSize) {
	memcpy(&pfVertexConstantGroup[4 * auiIndex], apData, auiSize);
}

void ImageSpaceShaderParam::CopyPixelConstants(uint32_t auiIndex, float* apData, uint32_t auiSize) {
	memcpy(&pfPixelConstantGroup[4 * auiIndex], apData, auiSize);
}

void ImageSpaceShaderParam::SetVertexMatrix(uint32_t auiIndex, const D3DXMATRIX& arMatrix) {
	CopyVertexConstants(auiIndex, (float*)&arMatrix, sizeof(D3DMATRIX));
}

void ImageSpaceShaderParam::SetPixelMatrix(uint32_t auiIndex, const D3DXMATRIX& arMatrix) {
	CopyPixelConstants(auiIndex, (float*)&arMatrix, sizeof(D3DMATRIX));
}
