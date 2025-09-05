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



