#pragma once

#include "Gamebryo/NiCamera.hpp"
#include "Gamebryo/NiTPointerList.hpp"
#include "BSRenderedTexture.hpp"
#include "Gamebryo/NiNode.hpp"

class BSShaderAccumulator;
class ShadowSceneNode;
class ShadowSceneLight;

NiSmartPointer(BSCubeMapCamera);

class BSCubeMapCamera : public NiCamera {
public:
	virtual void Click(uint32_t auiFaceToRender);

	enum Face {
		FACE_POS_X	= 0,
		FACE_NEG_X	= 1,
		FACE_POS_Y	= 2,
		FACE_NEG_Y	= 3,
		FACE_POS_Z	= 4,
		FACE_NEG_Z	= 5,
		FACE_NUM	= 6,
	};

	uint32_t						uiMode;
	BSRenderedTexturePtr			spFaces[FACE_NUM];
	BSRenderedTexturePtr			spTexture;
	ShadowSceneLight*				pLight;
	NiNodePtr						spSceneNode;
	NiPointer<BSShaderAccumulator>	spAccumulator;
};

ASSERT_SIZE(BSCubeMapCamera, 0x140u)