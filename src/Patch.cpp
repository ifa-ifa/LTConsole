#include <Windows.h>
#include "Patch.h"
#include "LunarTear++.h"



void PatchInfiniteJump() {

	void* addr = (void*)(LunarTear::Get().Game().GetProcessBaseAddress() + 0x6A007A);
	DWORD oldProtect;
	VirtualProtect(addr, 2, PAGE_EXECUTE_READWRITE, &oldProtect);
	unsigned char patch[2] = { 0x90, 0x90 };
	memcpy(addr, patch, sizeof(patch));
	VirtualProtect(addr, 2, oldProtect, &oldProtect);
}
void UnpatchInfiniteJump() {

	void* addr = (void*)(LunarTear::Get().Game().GetProcessBaseAddress() + 0x6A007A);
	DWORD oldProtect;
	VirtualProtect(addr, 2, PAGE_EXECUTE_READWRITE, &oldProtect);
	unsigned char patch[2] = { 0x84, 0xC0 };
	memcpy(addr, patch, sizeof(patch));
	VirtualProtect(addr, 2, oldProtect, &oldProtect);
}


class Patch {
public:
    uintptr_t offset;          
    std::vector<uint8_t> patchBytes;
    std::vector<uint8_t> originalBytes;

    Patch(uintptr_t off, std::vector<uint8_t> patch)
        : offset(off), patchBytes(std::move(patch)) {
    }

    void Apply(uintptr_t base) {
        void* addr = reinterpret_cast<void*>(base + offset);
        DWORD oldProtect;

        if (originalBytes.empty()) {
            originalBytes.resize(patchBytes.size());
            memcpy(originalBytes.data(), addr, patchBytes.size());
        }

        VirtualProtect(addr, patchBytes.size(), PAGE_EXECUTE_READWRITE, &oldProtect);
        memcpy(addr, patchBytes.data(), patchBytes.size());
        VirtualProtect(addr, patchBytes.size(), oldProtect, &oldProtect);
    }

    void Revert(uintptr_t base) {
        if (originalBytes.empty()) return;

        void* addr = reinterpret_cast<void*>(base + offset);
        DWORD oldProtect;

        VirtualProtect(addr, originalBytes.size(), PAGE_EXECUTE_READWRITE, &oldProtect);
        memcpy(addr, originalBytes.data(), originalBytes.size());
        VirtualProtect(addr, originalBytes.size(), oldProtect, &oldProtect);
    }
};


std::vector<Patch> noMovePatches = {

    {0xe48f0, {0xc3, 0x90}},
    {0x6742f0, {0xc3, 0x90, 0x90, 0x90, 0x90}},
    {0xdbf10, {0xc3, 0x90}},
    {0x669670, {0xc3, 0x90}},
    {0x068d680, {0xc3, 0x90, 0x90}}

};


void PatchNoMove() {
    
    for (auto& patch : noMovePatches) {
        patch.Apply(LunarTear::Get().Game().GetProcessBaseAddress());
    }
}


void UnpatchNoMove() {

    for (auto& patch : noMovePatches) {
        patch.Revert(LunarTear::Get().Game().GetProcessBaseAddress());
    }
}