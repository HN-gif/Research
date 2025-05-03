#include <iostream>
#include <portaudio.h>
#include <soundtouch/SoundTouch.h>

using namespace soundtouch;

// compile:
/*
g++ AAF.cpp -o AAF -I"C:\msys64\ucrt64\include" -L"C:\msys64\ucrt64\lib" -lsoundtouch -lportaudio
*/

// ── 設定 ──
constexpr int SAMPLE_RATE       = 44100;
constexpr int CHANNELS          = 1;
constexpr int FRAMES_PER_BUFFER = 512;
constexpr float PITCH_SEMITONES = 3.0f;

struct UserData {
    SoundTouch* soundTouch;
};

// PortAudio コールバック
static int paCallback(const void* inputBuffer, void* outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo* /*timeInfo*/,
                      PaStreamCallbackFlags /*statusFlags*/,
                      void* userData) {
    auto* data = static_cast<UserData*>(userData);
    auto* st   = data->soundTouch;
    const float* in  = static_cast<const float*>(inputBuffer);
    float* out       = static_cast<float*>(outputBuffer);

    if (inputBuffer) {
        // SoundTouch にサンプルを渡す
        st->putSamples(in, framesPerBuffer);
        // 加工後サンプルを受け取る
        unsigned int received = st->receiveSamples(out, framesPerBuffer);
        // 足りない分をゼロ埋め
        for (unsigned int i = received; i < framesPerBuffer; ++i) {
            out[i] = 0.0f;
        }
    } else {
        // 入力バッファ欠損時は無音出力
        for (unsigned int i = 0; i < framesPerBuffer; ++i) {
            out[i] = 0.0f;
        }
    }
    return paContinue;
}

int main() {
    PaError err;

    // PortAudio 初期化
    err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio init error: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }

    // SoundTouch オブジェクト設定
    SoundTouch st;
    st.setSampleRate(SAMPLE_RATE);
    st.setChannels(CHANNELS);
    st.setPitchSemiTones(PITCH_SEMITONES);
    UserData data{&st};

    // ストリームオープン
    PaStream* stream = nullptr;
    err = Pa_OpenDefaultStream(&stream,
                               CHANNELS, CHANNELS,
                               paFloat32,
                               SAMPLE_RATE,
                               FRAMES_PER_BUFFER,
                               paCallback,
                               &data);
    if (err != paNoError) {
        std::cerr << "Failed to open stream: " << Pa_GetErrorText(err) << std::endl;
        Pa_Terminate();
        return 1;
    }

    // ストリーム開始
    err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cerr << "Failed to start stream: " << Pa_GetErrorText(err) << std::endl;
        Pa_CloseStream(stream);
        Pa_Terminate();
        return 1;
    }

    std::cout << "Realtime pitch-shifted. Press Ctrl+C to exit" << std::endl;
    // アクティブ状態を監視
    while ((err = Pa_IsStreamActive(stream)) == 1) {
        Pa_Sleep(100);
    }

    // 後片付け
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    return 0;
}