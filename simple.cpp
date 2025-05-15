#include <iostream>
#include <cstring>
#include <portaudio.h>

// compile with:
// g++ simple.cpp -o simple -I"C:\msys64\ucrt64\include" -L"C:\msys64\ucrt64\lib" -lportaudio


// ── 設定 ──
constexpr int SAMPLE_RATE       = 44100;
constexpr int CHANNELS          = 1;
constexpr int FRAMES_PER_BUFFER = 512;

struct UserData { /* 今回は特にデータは不要 */ };

// PortAudio コールバック：入力をそのまま出力
static int paCallback(const void* inputBuffer, void* outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo* /*timeInfo*/,
                      PaStreamCallbackFlags /*statusFlags*/,
                      void* /*userData*/) {
    const float* in  = static_cast<const float*>(inputBuffer);
    float* out       = static_cast<float*>(outputBuffer);

    if (in) {
        // 入力バッファをそのまま出力バッファへコピー
        std::memcpy(out, in, framesPerBuffer * CHANNELS * sizeof(float));
    } else {
        // 入力欠損時には無音出力
        std::memset(out, 0, framesPerBuffer * CHANNELS * sizeof(float));
    }
    return paContinue;
}

int main() {
    PaError err;

    // PortAudio の初期化
    err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio init error: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }

    // ストリームを開く
    PaStream* stream = nullptr;
    err = Pa_OpenDefaultStream(&stream,
                               CHANNELS,    // 入力チャネル数
                               CHANNELS,    // 出力チャネル数
                               paFloat32,   // サンプルフォーマット
                               SAMPLE_RATE,
                               FRAMES_PER_BUFFER,
                               paCallback,
                               nullptr);    // UserData は不要なので nullptr
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

    std::cout << "Realtime pass-through (echo). Press Ctrl+C to exit." << std::endl;
    // ストリームがアクティブな間はスリープ
    while ((err = Pa_IsStreamActive(stream)) == 1) {
        Pa_Sleep(100);
    }

    // 後片付け
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    return 0;
}
