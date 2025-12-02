#pragma once

#include "BSRenderedTexture.hpp"
#include "BSTextureManager.hpp"
#include "Gamebryo/NiTexturingProperty.hpp"

class ImageSpaceTexture {
public:
	ImageSpaceTexture() : spTexture(0), iFilterMode(NiTexturingProperty::FILTER_NEAREST), iClampMode(NiTexturingProperty::CLAMP_S_CLAMP_T), byte0(false), bIsRenderedTexture(false), bIsBorrowed(false) {};
	ImageSpaceTexture(NiTexturingProperty::FilterMode aeFilter, NiTexturingProperty::ClampMode aeClamp) : spTexture(0), iFilterMode(aeFilter), iClampMode(aeClamp), byte0(false), bIsRenderedTexture(false), bIsBorrowed(false) {};
	~ImageSpaceTexture();

	bool							byte0;
	bool							bIsRenderedTexture;
	bool							bIsBorrowed;
	NiObjectPtr						spTexture;
	NiTexturingProperty::FilterMode	iFilterMode;
	NiTexturingProperty::ClampMode	iClampMode;

	static BSRenderedTexture* GetDepthBuffer();

	void				BorrowRenderedTexture(BSTextureManager::eTextureType aeType);
	BSRenderedTexture* GetRenderedTexture() const;
	NiTexture* GetTexture() const;
	void				SetTexture(BSRenderedTexture* apNewTexture);
	void				SetTexture(NiTexture* apNewTexture);
	void				ReturnRenderedTexture();
	void				ClearTexture();
};

ASSERT_SIZE(ImageSpaceTexture, 0x10);
