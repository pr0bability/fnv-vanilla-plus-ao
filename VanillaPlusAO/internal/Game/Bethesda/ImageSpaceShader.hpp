#pragma once

#include "BSShader.hpp"
#include "ImageSpaceEffect.hpp"
#include "ImageSpaceShaderParam.hpp"

class ImageSpaceShader : public BSShader, public ImageSpaceEffect {
public:
	virtual void LoadShaders(const char* apVertexShaderPath, const char* apPixelShaderPath, const char* apShaderName, const char* apVSTarget, const char* apPSTarget);
	virtual void SetupShaderDeclaration();
	virtual void Func_81();
	virtual void SetParameters(ImageSpaceShaderParam* apParam);
	virtual bool PresetStages();
	virtual void Func_84();

	NiD3DVertexShaderPtr	spVertexShader;
	NiD3DPixelShaderPtr		spPixelShader;
	ImageSpaceShaderParam*	pParameters;

	void LoadShadersEx(const char* apShaderName);
};

ASSERT_SIZE(ImageSpaceShader, 0xD0);