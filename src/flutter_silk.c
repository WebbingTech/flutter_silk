/***********************************************************************
Copyright (c) 2006-2012, Skype Limited. All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, (subject to the limitations in the disclaimer below)
are permitted provided that the following conditions are met:
- Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
- Neither the name of Skype Limited, nor the names of specific
contributors, may be used to endorse or promote products derived from
this software without specific prior written permission.
NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED
BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
CONTRIBUTORS ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************************/


/*****************************/
/* Silk decoder test program */
/*****************************/
#include "flutter_silk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>



#define _CRT_SECURE_NO_WARNINGS

#include "SKP_Silk_SDK_API.h"
#include "SKP_Silk_SigProc_FIX.h"
#include "lame.h"


#include <stdlib.h>
#include <string.h>

/* Define codec specific settings */
#define MAX_INPUT_FRAMES        5
#define MAX_FRAME_LENGTH        480
#define FRAME_LENGTH_MS         20
#define MAX_API_FS_KHZ          48
#define MAX_LBRR_DELAY          2
#define MAX_BYTES_PER_FRAME     250 // Equals peak bitrate of 100 kbps

#ifdef _SYSTEM_IS_BIG_ENDIAN
/* Function to convert a little endian int16 to a */
/* big endian int16 or vica verca                 */
void swap_endian(
  SKP_int16       vec[],              /*  I/O array of */
  SKP_int         len                 /*  I   length      */
)
{
  SKP_int i;
  SKP_int16 tmp;
  SKP_uint8* p1, * p2;

  for (i = 0; i < len; i++) {
    tmp = vec[i];
    p1 = (SKP_uint8*)&vec[i]; p2 = (SKP_uint8*)&tmp;
    p1[0] = p2[1]; p1[1] = p2[0];
  }
}
#endif

bool FFI_PLUGIN_EXPORT silkToPcm(unsigned char* silkData, int dataLen, int sampleRate, unsigned char** pcmData, unsigned long* pcmSize)
{
    SKP_uint8 payload[MAX_BYTES_PER_FRAME * MAX_INPUT_FRAMES * (MAX_LBRR_DELAY + 1)];
    SKP_uint8* payloadEnd = NULL, * payloadToDec = NULL;
    SKP_int16 nBytesPerPacket[MAX_LBRR_DELAY + 1];
    SKP_int16 out[((FRAME_LENGTH_MS * MAX_API_FS_KHZ) << 1) * MAX_INPUT_FRAMES], * outPtr;
    SKP_int32 remainPackets = 0;
    SKP_int16 len, nBytes, totalLen = 0;
    SKP_int32 decSizeBytes, result;
    unsigned char* psRead = silkData;
    void* psDec = NULL;

    SKP_SILK_SDK_DecControlStruct DecControl;

    /* Check Silk header */
    if (strncmp((char*)psRead, "\x02#!SILK_V3", 0x0A) != 0)
        return false;

    psRead += 0x0A;

    /* Create decoder */
    result = SKP_Silk_SDK_Get_Decoder_Size(&decSizeBytes);
    if (result) return false;

    /* Reset decoder */
    psDec = malloc(decSizeBytes);
    result = SKP_Silk_SDK_InitDecoder(psDec);
    if (result) return false;

    payloadEnd = payload;
    DecControl.framesPerPacket = 1;
    DecControl.API_sampleRate = sampleRate;

    /* Simulate the jitter buffer holding MAX_FEC_DELAY packets */
    {
        for (int i = 0; i < MAX_LBRR_DELAY; i++) {

            /* Read payload size */
            nBytes = *(SKP_int16*)psRead;
            psRead += sizeof(SKP_int16);

#ifdef _SYSTEM_IS_BIG_ENDIAN
            swap_endian(&nBytes, 1);
#endif

            /* Read payload */
            memcpy(payloadEnd, (SKP_uint8*)psRead, nBytes);
            psRead += sizeof(SKP_uint8) * nBytes;

            nBytesPerPacket[i] = nBytes;
            payloadEnd += nBytes;
        }

        nBytesPerPacket[MAX_LBRR_DELAY] = 0;
    }

    while (1) {

        if (remainPackets == 0) {

            /* Read payload size */
            nBytes = *(SKP_int16*)psRead;
            psRead += sizeof(SKP_int16);

#ifdef _SYSTEM_IS_BIG_ENDIAN
            swap_endian(&nBytes, 1);
#endif

            if (nBytes < 0 || psRead - silkData >= dataLen) {
                remainPackets = MAX_LBRR_DELAY;
                goto decode;
            }

            /* Read payload */
            memcpy(payloadEnd, (SKP_uint8*)psRead, nBytes);
            psRead += sizeof(SKP_uint8) * nBytes;

        }
        else if (--remainPackets <= 0) break;

    decode:
        if (nBytesPerPacket[0] != 0) {
            nBytes = nBytesPerPacket[0];
            payloadToDec = payload;
        }

        outPtr = out;
        totalLen = 0;
        int frames = 0;

        /* Decode all frames in the packet */
        do {
            /* Decode 20 ms */
            SKP_Silk_SDK_Decode(psDec, &DecControl, 0, payloadToDec, nBytes, outPtr, &len);

            frames++;
            outPtr += len;
            totalLen += len;

            if (frames > MAX_INPUT_FRAMES) {
                /* Hack for corrupt stream that could generate too many frames */
                outPtr = out;
                totalLen = 0;
                frames = 0;
            }

            /* Until last 20 ms frame of packet has been decoded */
        } while (DecControl.moreInternalDecoderFrames);

        /* Write output to pcmData */
#ifdef _SYSTEM_IS_BIG_ENDIAN
        swap_endian(out, totalLen);
#endif
        *pcmData = (unsigned char*)realloc(*pcmData, sizeof(SKP_int16) * (*pcmSize + sizeof(SKP_int16) * totalLen));
        memcpy(*pcmData + *pcmSize, out, sizeof(SKP_int16) * totalLen);
        *pcmSize += sizeof(SKP_int16) * totalLen;

        /* Update buffer */
        SKP_int16 totBytes = 0;
        for (int i = 0; i < MAX_LBRR_DELAY; i++) {
            totBytes += nBytesPerPacket[i + 1];
        }

        /* Check if the received totBytes is valid */
        if (totBytes < 0 || totBytes > sizeof(payload))
        {
            return false;
        }

        SKP_memmove(payload, &payload[nBytesPerPacket[0]], totBytes * sizeof(SKP_uint8));
        payloadEnd -= nBytesPerPacket[0];
        SKP_memmove(nBytesPerPacket, &nBytesPerPacket[1], MAX_LBRR_DELAY * sizeof(SKP_int16));
    }

    free(psDec);
    return true;
}

bool FFI_PLUGIN_EXPORT
pcmToMp3(const unsigned char* pcmData, long pcmSize, int sampleRate, unsigned char** mp3Data, unsigned long* mp3Size)
{
    assert(pcmData != NULL && pcmSize > 0 && mp3Data != NULL && mp3Size != NULL);



    const int MP3_SIZE = 4096;
    const int num_of_channels = 1;

    lame_global_flags *gfp = NULL;
    gfp = lame_init();
    if (NULL == gfp)
    {
        return false;
    }

    lame_set_in_samplerate(gfp, sampleRate);
    lame_set_preset(gfp, 56);
    lame_set_mode(gfp, MONO);
    // RG is enabled by default
    lame_set_findReplayGain(gfp, 1);
    // lame_set_quality(gfp, 4);
    //Setting Channels
    lame_set_num_channels(gfp, num_of_channels);

    unsigned long fsize = (unsigned long) (pcmSize / ( 2 * num_of_channels));
    lame_set_num_samples(gfp, fsize);

    int samples_to_read = lame_get_framesize(gfp);
    int samples_of_channel = 576;
    samples_to_read = samples_of_channel * num_of_channels ;    //
    // int framesize = samples_to_read;
    // std::assert(framesize <= 1152);
    // int bytes_per_sample = sizeof(short int);

    lame_set_out_samplerate(gfp, sampleRate);

    if(lame_init_params(gfp) == -1)
    {
        //lame initialization failed
        lame_close(gfp);
        return false;
    }

    unsigned char mp3_buffer[MP3_SIZE];
    int count = (int)((pcmSize + samples_to_read * 2 - 1) / (samples_to_read * 2));

    *mp3Data = (unsigned char*)malloc(count * MP3_SIZE);
    if (*mp3Data == NULL)
    {
        lame_close(gfp);
        return false;
    }

    unsigned char* mp3_ptr = *mp3Data;
    *mp3Size = 0;

    for (int idx = 0; idx < count; ++idx)
    {
        int offset = idx * samples_to_read * 2;
        int samples_to_encode = (offset + samples_to_read * 2 > pcmSize) ? (pcmSize - offset) : samples_to_read * 2;

        int write = lame_encode_buffer(gfp, (short int*)(pcmData + offset), NULL, samples_to_encode /2 , mp3_buffer, MP3_SIZE);
        memcpy(mp3_ptr, mp3_buffer, write);
        mp3_ptr += write;
        *mp3Size += write;
    }

    lame_close(gfp);

    return true;
}

bool FFI_PLUGIN_EXPORT silkToMp3(unsigned char* silkData, int dataLen, int sampleRate, unsigned char** mp3Data, unsigned long* mp3Size)
{
    unsigned char* pcmData = NULL;
    unsigned long pcmSize = 0;
    unsigned char* tempMp3Data = NULL;
    unsigned long tempMp3Size = 0;

    // Convert SILK to PCM
    if (!silkToPcm(silkData, dataLen, sampleRate, &pcmData, &pcmSize)) {
        return false;
    }

    // Convert PCM to MP3
    if (!pcmToMp3(pcmData, pcmSize, sampleRate, &tempMp3Data, &tempMp3Size)) {
        free(pcmData);
        return false;
    }

    // Clean up temporary memory
    free(pcmData);

    // Set output parameters
    *mp3Data = tempMp3Data;
    *mp3Size = tempMp3Size;

    return true;
}
