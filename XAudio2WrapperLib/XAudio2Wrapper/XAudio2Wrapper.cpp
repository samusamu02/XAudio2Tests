#include "XAudio2Wrapper.h"

XAudio2Wrapper::XAudio2Wrapper() :
    xAudio2_(nullptr),masteringVoice_(nullptr),sourceVoice_(nullptr)
{
}

XAudio2Wrapper::~XAudio2Wrapper()
{
    Delete();
}

bool XAudio2Wrapper::Init(void)
{
    HRESULT hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr))
    {
        return false;
    }

    hr = xAudio2_->CreateMasteringVoice(&masteringVoice_);
    if (FAILED(hr))
    {
        return false;
    }

    return true;
}

void XAudio2Wrapper::Delete(void)
{
    DestroySouceVoice();

    if (masteringVoice_)
    {
        masteringVoice_->DestroyVoice();
        masteringVoice_ = nullptr;
    }

    if (xAudio2_)
    {
        xAudio2_->Release();
        xAudio2_ = nullptr;
    }
}

bool XAudio2Wrapper::LoadWav(const char* filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file)
    {
        return false;
    }

    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (fileSize < sizeof(Wav))
    {
        return false;
    }

    file.read(reinterpret_cast<char*>(&wav_), sizeof(wav_));

    if (strncmp(wav_.riff, "RIFF", 4) != 0 || strncmp(wav_.wave, "WAV", 4) != 0 ||
        strncmp(wav_.fmt, "fmt ", 4) != 0 || strncmp(wav_.data, "data", 4) != 0)
    {
        return false;
    }

    audioData_.resize(wav_.dataSize);
    file.read(reinterpret_cast<char*>(audioData_.data()), wav_.dataSize);

    file.close();

    return true;
}

bool XAudio2Wrapper::CreateSouceVoice(void)
{
    WAVEFORMATEX waveFomat{};
    waveFomat.wFormatTag = WAVE_FORMAT_PCM;
    waveFomat.nChannels = wav_.numChannels;
    waveFomat.nSamplesPerSec = wav_.sampleRate;
    waveFomat.wBitsPerSample = wav_.bitsPerSample;
    waveFomat.nBlockAlign = wav_.blockAlign;
    waveFomat.nAvgBytesPerSec = wav_.byteRate;
    waveFomat.cbSize = 0;

    HRESULT hr = xAudio2_->CreateSourceVoice(&sourceVoice_, &waveFomat);
    if (FAILED(hr))
    {
        return false;
    }

    return true;
}

void XAudio2Wrapper::DestroySouceVoice(void)
{
    if (sourceVoice_)
    {
        sourceVoice_->Stop();
        sourceVoice_->DestroyVoice();
        sourceVoice_ = nullptr;
    }
}

void XAudio2Wrapper::Play(void)
{
    if (sourceVoice_)
    {
        XAUDIO2_BUFFER buffer{};
        buffer.AudioBytes = static_cast<UINT32>(audioData_.size());
        buffer.pAudioData = audioData_.data();
        buffer.Flags = XAUDIO2_END_OF_STREAM;

        sourceVoice_->SubmitSourceBuffer(&buffer, nullptr);
        sourceVoice_->Start(0);
    }
}

void XAudio2Wrapper::Stop(void)
{
    if (sourceVoice_)
    {
        sourceVoice_->Stop();
        sourceVoice_->FlushSourceBuffers();
    }
}


