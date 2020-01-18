//
// Created by timothy on 1/11/20.
//

#ifndef BLEPROXIMITY_WAVEHANDLER_H
#define BLEPROXIMITY_WAVEHANDLER_H
#include <Arduino.h>

class WaveHandler {
    public: static uint8_t* createWavFile(uint8_t *audio_data, uint32_t audio_data_len, uint32_t sample_rate);
};


#endif //BLEPROXIMITY_WAVEHANDLER_H
