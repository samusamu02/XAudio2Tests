#include "XAudio2_Wrapper.h"

XAudio2_Wrapper::XAudio2_Wrapper()
{
    CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
    XAudio2Create(&xAudio2_);
    xAudio2_->CreateMasteringVoice(&masteringVoice_);
}

XAudio2_Wrapper::~XAudio2_Wrapper()
{
    for (IXAudio2SourceVoice* sourceVoice : sourceVoices_)
    {
        sourceVoice->Stop();
        sourceVoice->DestroyVoice();
    }
    soundData_.clear();

    masteringVoice_->DestroyVoice();
    xAudio2_->Release();
    CoUninitialize();
}

int XAudio2_Wrapper::LoadFile(const std::string& filePath)
{
    FILE* file;
    errno_t err = fopen_s(&file, filePath.c_str(), "rb");

    // �G���[����
    if (err != 0 || !file)
    {
        return -1;
    }

    // wav�t�@�C���w�b�_�̓ǂݍ���
    WAVEFORMATEX waveFormat;
    DWORD dataSize;
    fseek(file, 20, SEEK_SET);
    fread(&waveFormat, sizeof(WAVEFORMATEX), 1, file);
    fseek(file, 40, SEEK_SET);
    fread(&dataSize, sizeof(DWORD), 1, file);

    // �T�E���h�f�[�^�̓ǂݍ���
    BYTE* soundData = new BYTE[dataSize];
    fread(soundData, dataSize, 1, file);
    fclose(file);

    // �\�[�X�{�C�X�̍쐬
    IXAudio2SourceVoice* sourceVoice;
    xAudio2_->CreateSourceVoice(&sourceVoice, &waveFormat, 0, XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, nullptr, nullptr);
    sourceVoices_.push_back(sourceVoice);

    // �o�b�t�@�쐬
    XAUDIO2_BUFFER buffer = { 0 };
    buffer.AudioBytes = dataSize;
    buffer.pAudioData = soundData;
    sourceVoice->SubmitSourceBuffer(&buffer);

    soundData_.push_back(soundData);

    return static_cast<int>(sourceVoices_.size() - 1);
}

void XAudio2_Wrapper::PlaySoundData(int soundFile)
{
    if (soundFile >= 0 && soundFile < sourceVoices_.size())
    {
        sourceVoices_[soundFile]->Start();
    }
}

void XAudio2_Wrapper::StopSoundData(int soundFile)
{
    if (soundFile >= 0 && soundFile < sourceVoices_.size())
    {
        sourceVoices_[soundFile]->Stop();
    }
}

void XAudio2_Wrapper::DeleteSoundData(int soundFile)
{
    if (soundFile >= 0 && soundFile < sourceVoices_.size())
    {
        sourceVoices_[soundFile]->Stop();
        sourceVoices_[soundFile]->DestroyVoice();
        sourceVoices_.erase(sourceVoices_.begin() + soundFile);

        BYTE* soundData = soundData_[soundFile];
        soundData_.erase(soundData_.begin() + soundFile);
        delete[] soundData;
    }
}
