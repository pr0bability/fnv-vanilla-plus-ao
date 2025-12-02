#include "ImageSpaceTexture.hpp"
#include "BSShaderManager.hpp"
#include "Gamebryo/NiDX9Renderer.hpp"

// GAME - 0xBA3840
// GECK - 0x9380A0
void ImageSpaceTexture::BorrowRenderedTexture(BSTextureManager::eTextureType aeType) {
	spTexture = BSShaderManager::GetTextureManager()->BorrowRenderedTexture(NiDX9Renderer::GetSingleton(), aeType);
	bIsRenderedTexture = true;
	bIsBorrowed = true;
}

// GAME - 0xBA3770
// GECK - 0x937FD0
BSRenderedTexture* ImageSpaceTexture::GetRenderedTexture() const {
	return bIsRenderedTexture ? static_cast<BSRenderedTexture*>(spTexture.m_pObject) : nullptr;
}

// GAME - 0xB4F530
// GECK - 0x8F8510
BSRenderedTexture* ImageSpaceTexture::GetDepthBuffer() {
#if GAME
	return CdeclCall<BSRenderedTexture*>(0xB4F530);
#else
	return CdeclCall<BSRenderedTexture*>(0x8F8510);
#endif
}

// GAME - 0xBA3780
// GECK - 0x937FE0
NiTexture* ImageSpaceTexture::GetTexture() const {
	if (bIsRenderedTexture)
		return static_cast<BSRenderedTexture*>(spTexture.m_pObject)->GetTexture(0);
	else
		return static_cast<NiTexture*>(spTexture.m_pObject);
}

// GAME - 0xBA37A0
// GECK - 0x938000
void ImageSpaceTexture::SetTexture(BSRenderedTexture* apNewTexture) {
	spTexture = apNewTexture;
	bIsRenderedTexture = true;
}

// GAME - 0xBA37F0
// GECK - 0x938050
void ImageSpaceTexture::SetTexture(NiTexture* apNewTexture) {
	spTexture = apNewTexture;
	bIsRenderedTexture = false;
}

// GAME - 0xBA39A0
// GECK - 0x938200
void ImageSpaceTexture::ReturnRenderedTexture() {
#if GAME
	ThisCall(0xBA39A0, this);
#else
	ThisCall(0x938200, this);
#endif
}

void ImageSpaceTexture::ClearTexture() {
	spTexture = nullptr;
	bIsRenderedTexture = false;
	iFilterMode = NiTexturingProperty::FILTER_NEAREST;
	iClampMode = NiTexturingProperty::CLAMP_S_CLAMP_T;
}
