#include "ShadowSceneNode.hpp"
#include "Bethesda/BSShaderManager.hpp"

BSFogProperty* ShadowSceneNode::GetFogProperty(uint32_t aeType) {
	return BSShaderManager::GetShadowSceneNode(static_cast<BSShaderManager::SceneGraphType>(aeType))->spFog;
}

// GAME - 0xB5D9F0
// GECK - 0x90C390
void ShadowSceneNode::UpdateObjectLighting(NiAVObject* apObject, bool abIsMoving) {
#ifdef GAME
	ThisCall(0xB5D9F0, this, apObject, abIsMoving);
#else
	ThisCall(0x90C390, this, apObject, abIsMoving);
#endif
}

// GAME - 0xB5B1C0
// GECK - 0x90AC70
void ShadowSceneNode::ClearLightsFromNode(NiAVObject* apObject) {
#ifdef GAME
	ThisCall(0xB5B1C0, this, apObject);
#else
	ThisCall(0x90AC70, this, apObject);
#endif
}
