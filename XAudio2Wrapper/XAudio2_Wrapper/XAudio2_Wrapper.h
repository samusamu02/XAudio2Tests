#pragma once
#include <xaudio2.h>
#include <vector>
#include <string>

class XAudio2_Wrapper
{
public:
	XAudio2_Wrapper();
	~XAudio2_Wrapper();

	int LoadFile(const std::string& filePath);
	void PlaySoundData(int soundFile);
	void StopSoundData(int soundFile);
	void DeleteSoundData(int soundFile);

private:
	IXAudio2* xAudio2_;
	IXAudio2MasteringVoice* masteringVoice_;
	std::vector<IXAudio2SourceVoice*> sourceVoices_;
	std::vector<BYTE*> soundData_;
};

