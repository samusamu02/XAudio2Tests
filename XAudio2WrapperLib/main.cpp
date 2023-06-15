#include <iostream>
#include "XAudio2Wrapper/XAudio2Wrapper.h"

int main()
{
	XAudio2Wrapper xaudio2Wrapper;

	if (!xaudio2Wrapper.LoadWav("Sound/SoundFile.wav"))
	{
		xaudio2Wrapper.Delete();
		return 1;
	}


	if (!xaudio2Wrapper.Init())
	{
		xaudio2Wrapper.Delete();
		return 1;
	}

	xaudio2Wrapper.Play();
	Sleep(10000);
	xaudio2Wrapper.Stop();

	return 0;
}
