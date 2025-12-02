#include "BSShaderManager.hpp"

BSTextureManager* BSShaderManager::GetTextureManager() {
#ifdef GAME
	return *reinterpret_cast<BSTextureManager**>(0x11F91A8);
#else
	return *reinterpret_cast<BSTextureManager**>(0xF23BF8);
#endif
}

BSRenderedTexture* BSShaderManager::GetCurrentRenderTarget() {
	return pCurrentRenderTarget;
}

// GAME - 0x450B80
ShadowSceneNode* BSShaderManager::GetShadowSceneNode(uint32_t aeType) {
#ifdef GAME
	return reinterpret_cast<ShadowSceneNode**>(0x11F91C8)[aeType];
#else
	return reinterpret_cast<ShadowSceneNode**>(0xF23C18)[aeType];
#endif
}

BSShaderManager::SceneGraphType BSShaderManager::GetFogIndex() {
	return static_cast<SceneGraphType>(ucSceneGraph.Get());
};
