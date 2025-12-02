#pragma once

#include "Gamebryo/NiFogProperty.hpp"
#include "Gamebryo/NiPlane.hpp"

NiSmartPointer(BSFogProperty);

class BSFogProperty : public NiFogProperty {
public:
	BSFogProperty();
	~BSFogProperty();

	float	fStartDistance;
	float	fEndDistance;
	float	fStartWaterDistance;
	float	fEndWaterDistance;
	NiPlane kPlane;
	float	fFalloff;
	float	fHeight;
	NiColor kWaterColor;
	float	fPower;
};

ASSERT_SIZE(BSFogProperty, 0x64)