#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")

#include <memory>
#include <vector>

#include "WAVRoder/WAVRoder.h"

/// <summary>
/// フェードイン
/// </summary>
/// <param name="pSourceVoice">ソースボイス</param>
/// <param name="endVolume">音量設定</param>
/// <param name="fadeDuration">フェード時間</param>
void FadeIn(IXAudio2SourceVoice* pSourceVoice, float endVolume, UINT32 fadeDuration)
{
	// 最初は音量を0に設定
	pSourceVoice->SetVolume(0.0f); 

	// フェードイン時間を計算
	UINT32 fadeSteps = 100;
	UINT32 fadeInterval = fadeDuration / fadeSteps;

	// 音量を徐々に上げる
	for (UINT32 i = 0; i < fadeSteps; i++)
	{
		float volume = endVolume * (i + 1) / fadeSteps;
		pSourceVoice->SetVolume(volume);
		Sleep(fadeInterval);
	}

	// 最終的な音量に設定
	pSourceVoice->SetVolume(endVolume);
}

/// <summary>
/// フェードアウト
/// </summary>
/// <param name="pSourceVoice">ソースボイス</param>
/// <param name="endVolume">音量設定</param>
/// <param name="fadeDuration">フェード時間</param>
void FadeOut(IXAudio2SourceVoice* pSourceVoice, float endVolume, UINT32 fadeDuration)
{
	// 現在の音量を取得
	float startVolume;
	pSourceVoice->GetVolume(&startVolume);

	// フェードアウト時間を計算
	UINT32 fadeSteps = 100;
	UINT32 fadeInterval = fadeDuration / fadeSteps;

	// 音量を徐々を下げる
	for (UINT32 i = 0; i < fadeSteps; i++)
	{
		float volume = startVolume * (fadeSteps - i) / fadeSteps;
		pSourceVoice->SetVolume(volume);
		Sleep(fadeInterval);
	}

	// 最終的な音量に設定
	pSourceVoice->SetVolume(endVolume);
}

/// <summary>
/// ループ再生
/// </summary>
/// <param name="pSouceVoice">ソースボイス</param>
void LoopAudio(IXAudio2SourceVoice* pSouceVoice)
{
	while (true)
	{
		// 現在の再生時間を求める
		XAUDIO2_VOICE_STATE state;
		pSouceVoice->GetState(&state);
		XAUDIO2_VOICE_DETAILS details;
		pSouceVoice->GetVoiceDetails(&details);

		// バッファが再生終了したら再生をループさせる
		if (state.BuffersQueued == 0 && state.SamplesPlayed > 0)
		{
			// 音声の再生
			pSouceVoice->Start(0);
		}

		Sleep(10);
	}
}

int main()
{
	printf_s("ループテスト\n");

	// COMの初期化
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);		

	// XAudio2の初期化
	IXAudio2* pXAudio2{ nullptr };
	XAudio2Create(&pXAudio2, 0);

#ifdef _DEBUG
	// デバッグモード
	XAUDIO2_DEBUG_CONFIGURATION debug{ 0 };
	debug.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS;
	debug.BreakMask = XAUDIO2_LOG_ERRORS;
	pXAudio2->SetDebugConfiguration(&debug, 0);
#endif 
	// マスターボイスの作成
	IXAudio2MasteringVoice* pMasteringVoice{nullptr};
	pXAudio2->CreateMasteringVoice(&pMasteringVoice);

	// wavファイルの読み込み
	std::unique_ptr<uint8_t[]> waveFile{ nullptr };
	DirectX::WAVData waveData{ 0 };
	DirectX::LoadWAVAudioFromFileEx(L"Sound/SoundFile.wav", waveFile, waveData);

	// ソースボイスの作成
	IXAudio2SourceVoice* pSourceVoice{ nullptr };
	pXAudio2->CreateSourceVoice(&pSourceVoice, waveData.wfx);

	// 音声データのセット
	XAUDIO2_BUFFER buffer{ 0 };
	buffer.pAudioData = waveData.startAudio;
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.AudioBytes = waveData.audioBytes;
	pSourceVoice->SubmitSourceBuffer(&buffer);

	// 音声の再生
	pSourceVoice->Start(0);

	// 再生後にフェードイン
	FadeIn(pSourceVoice, 1.0f, 100);

	// ここまで完了したら文字列を表示
	printf_s("SoundFile.wavを再生中");

	LoopAudio(pSourceVoice);

	// XAudio2の終了・データの破棄
	pSourceVoice->DestroyVoice();
	pMasteringVoice->DestroyVoice();
	pXAudio2->Release();
	CoUninitialize();

	return 0;
}