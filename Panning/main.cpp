#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")

#include <memory>
#include <vector>

#include "WAVRoder/WAVRoder.h"

/// <summary>
/// �t�F�[�h�C��
/// </summary>
/// <param name="pSourceVoice">�\�[�X�{�C�X</param>
/// <param name="endVolume">���ʐݒ�</param>
/// <param name="fadeDuration">�t�F�[�h����</param>
void FadeIn(IXAudio2SourceVoice* pSourceVoice, float endVolume, UINT32 fadeDuration)
{
	// �ŏ��͉��ʂ�0�ɐݒ�
	pSourceVoice->SetVolume(0.0f); 

	// �t�F�[�h�C�����Ԃ��v�Z
	UINT32 fadeSteps = 100;
	UINT32 fadeInterval = fadeDuration / fadeSteps;

	// ���ʂ����X�ɏグ��
	for (UINT32 i = 0; i < fadeSteps; i++)
	{
		float volume = endVolume * (i + 1) / fadeSteps;
		pSourceVoice->SetVolume(volume);
		Sleep(fadeInterval);
	}

	// �ŏI�I�ȉ��ʂɐݒ�
	pSourceVoice->SetVolume(endVolume);
}

/// <summary>
/// �t�F�[�h�A�E�g
/// </summary>
/// <param name="pSourceVoice">�\�[�X�{�C�X</param>
/// <param name="endVolume">���ʐݒ�</param>
/// <param name="fadeDuration">�t�F�[�h����</param>
void FadeOut(IXAudio2SourceVoice* pSourceVoice, float endVolume, UINT32 fadeDuration)
{
	// ���݂̉��ʂ��擾
	float startVolume;
	pSourceVoice->GetVolume(&startVolume);

	// �t�F�[�h�A�E�g���Ԃ��v�Z
	UINT32 fadeSteps = 100;
	UINT32 fadeInterval = fadeDuration / fadeSteps;

	// ���ʂ����X��������
	for (UINT32 i = 0; i < fadeSteps; i++)
	{
		float volume = startVolume * (fadeSteps - i) / fadeSteps;
		pSourceVoice->SetVolume(volume);
		Sleep(fadeInterval);
	}

	// �ŏI�I�ȉ��ʂɐݒ�
	pSourceVoice->SetVolume(endVolume);
}

int main()
{
	printf_s("�t�F�[�h�e�X�g\n");

	// COM�̏�����
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);		

	// XAudio2�̏�����
	IXAudio2* pXAudio2{ nullptr };
	XAudio2Create(&pXAudio2, 0);

#ifdef _DEBUG
	// �f�o�b�O���[�h
	XAUDIO2_DEBUG_CONFIGURATION debug{ 0 };
	debug.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS;
	debug.BreakMask = XAUDIO2_LOG_ERRORS;
	pXAudio2->SetDebugConfiguration(&debug, 0);
#endif 
	// �}�X�^�[�{�C�X�̍쐬
	IXAudio2MasteringVoice* pMasteringVoice{nullptr};
	pXAudio2->CreateMasteringVoice(&pMasteringVoice);

	// wav�t�@�C���̓ǂݍ���
	std::unique_ptr<uint8_t[]> waveFile{ nullptr };
	DirectX::WAVData waveData{ 0 };
	DirectX::LoadWAVAudioFromFileEx(L"Sound/SoundFile.wav", waveFile, waveData);

	// �\�[�X�{�C�X�̍쐬
	IXAudio2SourceVoice* pSourceVoice{ nullptr };
	pXAudio2->CreateSourceVoice(&pSourceVoice, waveData.wfx);

	// �����f�[�^�̃Z�b�g
	XAUDIO2_BUFFER buffer{ 0 };
	buffer.pAudioData = waveData.startAudio;
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.AudioBytes = waveData.audioBytes;
	pSourceVoice->SubmitSourceBuffer(&buffer);

	// �����̍Đ�
	pSourceVoice->Start(0);

	// �Đ���Ƀt�F�[�h�C��
	FadeIn(pSourceVoice, 1.0, 100);

	// �����܂Ŋ��������當�����\��
	printf_s("SoundFile.wav���Đ���");

	// �I���܂őҋ@�@
	while (true)
	{
		XAUDIO2_VOICE_STATE state;
		pSourceVoice->GetState(&state);

		XAUDIO2_VOICE_DETAILS details;
		pSourceVoice->GetVoiceDetails(&details);

		// ���݂̍Đ����Ԃ����߂�
		double nowTime = static_cast<double>(state.SamplesPlayed / details.InputSampleRate);

		// �ǂ��܂ōĐ����邩
		double maxTime = 9.0;

		// maxTime���߂�����t�F�[�h�A�E�g����
		if (nowTime > maxTime)
		{
			FadeOut(pSourceVoice, 0.0, 100);
		}

		// �o�b�t�@�[����0�ɂȂ�A�������̓t�F�[�h�����������烋�[�v������
		if (state.BuffersQueued == 0 || nowTime > maxTime)
		{
			break;
		}

		Sleep(10);
	}

	// XAudio2�̏I���E�f�[�^�̔j��
	pSourceVoice->DestroyVoice();
	pMasteringVoice->DestroyVoice();
	pXAudio2->Release();
	CoUninitialize();

	return 0;
}