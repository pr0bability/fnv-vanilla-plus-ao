#pragma once

#include "Gamebryo/NiFrustumPlanes.hpp"
#include "Gamebryo/NiBound.hpp"
#include "BSSimpleArray.hpp"

class NiAVObject;
class BSOcclusionPlane;
class BSPortal;

class BSCompoundFrustum {
public:
	BSCompoundFrustum();
	~BSCompoundFrustum();

	struct FunctionOp {
		int32_t op = 0;
		int32_t jp = 0;
		int32_t jf = 0;
	};

	bool							bPrethreaded;
	BSSimpleArray<NiFrustumPlanes>	kPlanes;
	BSSimpleArray<FunctionOp>		kFunctionOperators;
	uint32_t						uiFreePlane;
	uint32_t						uiFreeOp;
	uint32_t						uiFirstOp;
	NiFrustumPlanes					kViewFrustum;
	NiPoint3						kViewPosition;
	bool							bSkipViewFrustum;
};

ASSERT_SIZE(BSCompoundFrustum, 0xA4);