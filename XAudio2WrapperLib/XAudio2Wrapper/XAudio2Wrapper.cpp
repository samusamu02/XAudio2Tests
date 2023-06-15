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

    return true;
}

void XAudio2Wrapper::Delete(void)
{
}

bool XAudio2Wrapper::LoadWav(const char* filePath)
{
    return false;
}

void XAudio2Wrapper::Play(void)
{
}

void XAudio2Wrapper::Stop(void)
{
}

bool XAudio2Wrapper::CreateSouceVoice(void)
{
    return false;
}

void XAudio2Wrapper::DestroySouceVoice(void)
{
}
