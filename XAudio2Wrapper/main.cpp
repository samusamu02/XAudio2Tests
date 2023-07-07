#include "XAudio2_Wrapper/XAudio2_Wrapper.h"

int main()
{
	XAudio2_Wrapper audioWrapper;

	int soundFile = audioWrapper.LoadFile("Sound/SoundFile.wav");
	if (soundFile != -1)
	{
		audioWrapper.PlaySoundData(soundFile);

		//audioWrapper.StopSoundData(soundFile);
		//audioWrapper.DeleteSoundData(soundFile);
	}

	return 0;
}