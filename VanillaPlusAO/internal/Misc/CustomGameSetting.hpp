#pragma once

class CustomGameSetting {
public:
    union Info {
        const char* str;
        int32_t			i;
        uint32_t		u;
        float			f;
        bool			b;
        int8_t			c;
        uint8_t			h;
        DWORD			r;
    };

    CustomGameSetting() {
        memset(this, 0, sizeof(CustomGameSetting));
    }

    ~CustomGameSetting() {
    }

    void*       __vftable;
    Info        uValue;
    const char* pKey;

    void Initialize(const char* apName, uint32_t value) {
        ThisCall(0x40C150, this, apName, value);
    }

    void Initialize(const char* apName, float value) {
        ThisCall(0x40C150, this, apName, value);
    }

    void Initialize(const char* apName, bool value) {
        ThisCall(0x40C150, this, apName, value);
    }

    uint32_t UInt() const {
        return uValue.u;
    }

    float Float() const {
        return uValue.f;
    }

    bool Bool() const {
        return uValue.b;
    }
};
