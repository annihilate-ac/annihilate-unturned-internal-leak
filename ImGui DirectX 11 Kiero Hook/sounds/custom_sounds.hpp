#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <cstdio>
#include <string>

#include <MMSystem.h>
#pragma comment(lib, "winmm.lib")

extern unsigned char welcome[];
extern unsigned char metallic[];
extern unsigned char cod[];
extern unsigned char bubble[];
extern unsigned char stapler[];
extern unsigned char bell[];
extern unsigned char kill1[];
extern unsigned char kill2[];
extern unsigned char kill3[];
extern unsigned char kill4[];
extern unsigned char kill5[];
extern unsigned char kill6[];
extern unsigned char kill7[];
extern unsigned char kill8[];
extern unsigned char kill9[];
extern unsigned char kill10[];
extern unsigned char kill11[];

__forceinline void setup_sounds(std::string base_path)
{

	std::string sounds_path_2 = std::string(base_path + std::string(("\\ann_sounds")));

	CreateDirectoryA(sounds_path_2.c_str(), NULL);

	FILE* file = nullptr;

	file = fopen((sounds_path_2 + "\\bell.wav").c_str(), "wb");
	fwrite(bell, sizeof(unsigned char), 42154, file);
	fclose(file);

	file = fopen((sounds_path_2 + "\\cod.wav").c_str(), "wb");
	fwrite(cod, sizeof(unsigned char), 11752, file);
	fclose(file);

	file = fopen((sounds_path_2 + "\\stapler.wav").c_str(), "wb");
	fwrite(stapler, sizeof(unsigned char), 105522, file);
	fclose(file);

}