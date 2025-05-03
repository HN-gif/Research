import time
import numpy as np
import pyaudio
from pysoundtouch.pysoundtouch import SoundTouch

# ── 設定 ──
RATE       = 44100        # サンプリングレート(Hz)
CHANNELS   = 1            # モノラル
BUFFER     = 512          # フレーム数(小さいほどレイテンシ低)
PITCH_SEMI = 3.0          # 変化量(半音): 正で高く，負で低く

# SoundTouch オブジェクト初期化
st = SoundTouch(channels=CHANNELS, sample_rate=RATE)
st.set_pitch_semitones(PITCH_SEMI)

# PyAudio ストリームコールバック
def callback(in_data, frame_count, time_info, status):
    # 入力バッファを numpy 配列へ
    samples_in = np.frombuffer(in_data, dtype=np.int16)
    # SoundTouch に入力
    st.put_samples(samples_in)
    # 加工後サンプルを取得
    out = st.receive_samples(frame_count)
    # numpy→バイト列
    return (out.astype(np.int16).tobytes(), pyaudio.paContinue)

# PyAudio ストリーム立ち上げ
p = pyaudio.PyAudio()
stream = p.open(format=pyaudio.paInt16,
                channels=CHANNELS,
                rate=RATE,
                input=True,
                output=True,
                frames_per_buffer=BUFFER,
                stream_callback=callback)
stream.start_stream()

print("リアルタイムピッチシフト開始。Ctrl+C で停止。")
try:
    while stream.is_active():
        time.sleep(0.1)
except KeyboardInterrupt:
    pass
finally:
    stream.stop_stream()
    stream.close()
    p.terminate()
    print("終了しました。")
