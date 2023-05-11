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
/// パン
/// </summary>
/// <param name="pSourceVoice">ソースボイス</param>
/// <param name="pan">パンの値</param>
void SetStereoPan(IXAudio2SourceVoice* pSourceVoice, float pan)
{
	// パンの値
	pan = max(-1.0f, min(pan,1.0f));

	// パンの値に基づいて音声を分割する係数を計算
	float leftVolume = sqrtf(1.0f - (pan * 0.5f + 0.5f));
	float rigtVolume = sqrtf(1.0f + (pan * 0.5f - 0.5f));

	//パンの設定をソースボイスに適用
	XAUDIO2_VOICE_DETAILS details;
	pSourceVoice->GetVoiceDetails(&details);

	if (details.InputChannels == 2)
	{
		float matrix[] =
		{
			leftVolume,
			0.0f,
			0.0f,
			rigtVolume
		};

		pSourceVoice->SetOutputMatrix(nullptr, details.InputChannels, 2, matrix);
	}
}

int main()
{
	printf_s("パニングテスト\n");

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

	// パンの初期値
	SetStereoPan(pSourceVoice,0.0f);

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

	// パンの設定
	float pan = 0.0f;		// パンの値
	float speed = -0.1f;	// 左右に揺らすスピード(左から右へ)

	// 終了まで待機　
	while (true)
	{
		// 音を左右に揺らす
		pan += speed;
		if (pan >= 1.0f || pan <= -1.0f)
		{
			speed = -speed;
			pan += speed;
		}
		SetStereoPan(pSourceVoice, pan);

		// 現在の再生時間を求める
		XAUDIO2_VOICE_STATE state;
		pSourceVoice->GetState(&state);
		XAUDIO2_VOICE_DETAILS details;
		pSourceVoice->GetVoiceDetails(&details);
		double nowTime = static_cast<double>(state.SamplesPlayed / details.InputSampleRate);

		// どこまで再生するか
		double maxTime = 5.0;

		// maxTimeを過ぎたらフェードアウトする
		if (nowTime > maxTime)
		{
			FadeOut(pSourceVoice, 0.0f, 100);
		}

		// バッファー数が0になる、もしくはフェードが完了したらループ抜ける
		if (state.BuffersQueued == 0 || nowTime > maxTime)
		{
			break;
		}

		Sleep(10);
	}

	// XAudio2の終了・データの破棄
	pSourceVoice->DestroyVoice();
	pMasteringVoice->DestroyVoice();
	pXAudio2->Release();
	CoUninitialize();

	return 0;
}