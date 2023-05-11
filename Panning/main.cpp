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

/// <summary>
/// �p��
/// </summary>
/// <param name="pSourceVoice">�\�[�X�{�C�X</param>
/// <param name="pan">�p���̒l</param>
void SetStereoPan(IXAudio2SourceVoice* pSourceVoice, float pan)
{
	// �p���̒l
	pan = max(-1.0f, min(pan,1.0f));

	// �p���̒l�Ɋ�Â��ĉ����𕪊�����W�����v�Z
	float leftVolume = sqrtf(1.0f - (pan * 0.5f + 0.5f));
	float rigtVolume = sqrtf(1.0f + (pan * 0.5f - 0.5f));

	//�p���̐ݒ���\�[�X�{�C�X�ɓK�p
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
	printf_s("�p�j���O�e�X�g\n");

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

	// �p���̏����l
	SetStereoPan(pSourceVoice,0.0f);

	// �����f�[�^�̃Z�b�g
	XAUDIO2_BUFFER buffer{ 0 };
	buffer.pAudioData = waveData.startAudio;
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.AudioBytes = waveData.audioBytes;
	pSourceVoice->SubmitSourceBuffer(&buffer);

	// �����̍Đ�
	pSourceVoice->Start(0);

	// �Đ���Ƀt�F�[�h�C��
	FadeIn(pSourceVoice, 1.0f, 100);

	// �����܂Ŋ��������當�����\��
	printf_s("SoundFile.wav���Đ���");

	// �p���̐ݒ�
	float pan = 0.0f;		// �p���̒l
	float speed = -0.1f;	// ���E�ɗh�炷�X�s�[�h(������E��)

	// �I���܂őҋ@�@
	while (true)
	{
		// �������E�ɗh�炷
		pan += speed;
		if (pan >= 1.0f || pan <= -1.0f)
		{
			speed = -speed;
			pan += speed;
		}
		SetStereoPan(pSourceVoice, pan);

		// ���݂̍Đ����Ԃ����߂�
		XAUDIO2_VOICE_STATE state;
		pSourceVoice->GetState(&state);
		XAUDIO2_VOICE_DETAILS details;
		pSourceVoice->GetVoiceDetails(&details);
		double nowTime = static_cast<double>(state.SamplesPlayed / details.InputSampleRate);

		// �ǂ��܂ōĐ����邩
		double maxTime = 5.0;

		// maxTime���߂�����t�F�[�h�A�E�g����
		if (nowTime > maxTime)
		{
			FadeOut(pSourceVoice, 0.0f, 100);
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