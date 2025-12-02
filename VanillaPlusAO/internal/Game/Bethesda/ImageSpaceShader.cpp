#include "ImageSpaceShader.hpp"

void ImageSpaceShader::LoadShadersEx(const char* apShaderName) {
	char cName[128];
	snprintf(cName, sizeof(cName), "VPAO\\IS%s.vso", apShaderName);
	spVertexShader = pCreateVertexShader(cName);
	
	snprintf(cName, sizeof(cName), "VPAO\\IS%s.pso", apShaderName);
	spPixelShader = pCreatePixelShader(cName);
}
