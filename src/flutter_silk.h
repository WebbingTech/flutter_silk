#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


#ifdef _WIN32
#define FFI_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FFI_PLUGIN_EXPORT
#endif


bool FFI_PLUGIN_EXPORT silkToPcm(unsigned char* silkData, int dataLen, int sampleRate, unsigned char** pcmData, unsigned long* pcmSize);

bool FFI_PLUGIN_EXPORT pcmToMp3(const unsigned char* pcmData, long pcmSize, int sampleRate, unsigned char** mp3Data, unsigned long* mp3Size);

bool FFI_PLUGIN_EXPORT silkToMp3(unsigned char* silkData, int dataLen, int sampleRate, unsigned char** mp3Data, unsigned long* mp3Size);
