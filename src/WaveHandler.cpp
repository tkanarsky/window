//
// Created by timothy on 1/11/20.
//
// Wave file formatter, given an array of 8-bit unsigned audio samples.
// Big thanks to http://soundfile.sapp.org/doc/WaveFormat/.

#include "WaveHandler.h"
#define BYTESIZE 8

uint8_t* WaveHandler::createWavFile(uint8_t *audio_data, uint32_t audio_data_len, uint32_t sample_rate) {
    audio_data[0] = 'R';
    audio_data[1] = 'I';
    audio_data[2] = 'F';
    audio_data[3] = 'F';
    Serial.println("Wrote RIFF header!");
    uint32_t chunk_size = audio_data_len - 44 + 36;
    Serial.print("Chunk size: ");
//    Serial.println(chunk_size);
    for (int i = 0; i < 4; i++) {
//        Serial.print("Chunk size byte: ");
//        Serial.print(i);
//        Serial.print(" ");
//        Serial.println(((chunk_size & (255u << (i * 8u))) >> (i * 8u)));
        *(audio_data + 4 + i) = ((chunk_size & (255u << (i * 8u))) >> (i * 8u));
    }
    audio_data[8] = 'W';
    audio_data[9] = 'A';
    audio_data[10] = 'V';
    audio_data[11] = 'E';
    audio_data[12] = 'f';
    audio_data[13] = 'm';
    audio_data[14] = 't';
    audio_data[15] = ' ';
    Serial.print("Wrote WAVEfmt!");

    audio_data[16] = 0x10; // Next 4 bytes indicate format chunk size
    audio_data[17] = 0x00;
    audio_data[18] = 0x00;
    audio_data[19] = 0x00;
    audio_data[20] = 0x01; // Next 2 bytes indicate PCM format
    audio_data[21] = 0x00;
    audio_data[22] = 0x01; // Next 2 bytes indicate single-channel audio
    audio_data[23] = 0x00;

    for (int i = 0; i < 4; i++) { // Writes sample rate
        Serial.print("Sample rate byte: ");
//        Serial.print(i);
//        Serial.print(" ");
//        Serial.println(((sample_rate & (255u << i * (8u))) >> (i * 8u)));
        *(audio_data + 24 + i) = ((sample_rate & (255u << (i * 8u))) >> (i * 8u));
    }

    uint32_t byte_rate = sample_rate; // 1 byte per sample

    for (int i = 0; i < 4; i++) { // Writes byte rate
        Serial.print("Byte rate byte: ");
//        Serial.print(i);
//        Serial.print(" ");
//        Serial.println(((byte_rate & (255u << (i * 8u))) >> (i * 8u)));
        *(audio_data + 28 + i) = ((byte_rate & (255u << (i * 8u))) >> (i * 8u));
    }

    audio_data[32] = 0x01; // 1 byte per sample
    audio_data[33] = 0x00;

    audio_data[34] = 0x08; // 8-bit audio
    audio_data[35] = 0x00;

    audio_data[36] = 'd';
    audio_data[37] = 'a';
    audio_data[38] = 't';
    audio_data[39] = 'a';

    for (int i = 0; i < 4; i++) { // Writes audio data length
        Serial.print("Audio length byte: ");
//        Serial.print(i);
//        Serial.print(" ");
//        Serial.println(((audio_data_len & (255u << (i * 8u))) >> (i * 8u)));
        *(audio_data + 40 + i) = (((audio_data_len - 44) & (255u << (i * 8u))) >> (i * 8u));
    }
    Serial.println("asdf");
    // Since we already have the sample data in indices 44 to (44 + audio_data_len), return the pointer
    return audio_data;
}
