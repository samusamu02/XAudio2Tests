#pragma once
#include <xaudio2.h>
#include <fstream>
#include <vector>

// wavÇéÊÇËàµÇ§ÇΩÇﬂÇÃç\ë¢ëÃ
#pragma pack(push,1)
struct Wav
{
	char riff[4];
	uint32_t fileSize;
	char wave[4];
	char fmt[4];
	uint32_t fmtSize;
	uint16_t audioFormat;
	uint16_t numChannels;
	uint32_t sampleRate;
	uint32_t byteRate;
	uint16_t blockAlign;
	uint16_t bitsPerSample;
	char data[4];
	uint32_t dataSize;
};
#pragma pack(pop)

class XAudio2Wrapper
{
public:
	XAudio2Wrapper();
	~XAudio2Wrapper();

	bool Init(void);
	void Delete(void);

	bool LoadWav(const char* filePath);
	void Play(void);
	void Stop(void);
private:
	IXAudio2* xAudio2_;
	IXAudio2MasteringVoice* masteringVoice_;
	IXAudio2SourceVoice* sourceVoice_;
	Wav wav_;
	std::vector<uint8_t> audioData_;

	bool CreateSouceVoice(void);
	void DestroySouceVoice(void);
};

