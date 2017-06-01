#include "utils.h"
#include <iostream>
#include <vector>
#include "soundmanager.h"
#include "framework.h"
#include "game.h"

SoundManager* SoundManager::instance = NULL;

SoundManager::SoundManager()
{
	//Inicializamos BASS  (id_del_device, muestras por segundo, ...)
	BASS_Init(-1, 44100, BASS_DEVICE_DEFAULT, 0, NULL);
}

SoundManager::~SoundManager()
{
	
}

void SoundManager::playSound(const std::string& name, bool loop)
{
	std::cout << "playing sound.. " << name << std::endl;

	std::string sound = "data/sounds/" + name + ".wav";

	if (name == "shot")
	{
		HSAMPLE hSample = BASS_SampleLoad(false, sound.c_str(), 0L, 0, 1, loop ? BASS_SAMPLE_LOOP : 0);
		HCHANNEL hSampleChannel = BASS_SampleGetChannel(hSample, false);
		BASS_ChannelPlay(hSampleChannel, loop);
		return;
	}

	auto it = samples.find(name);
	if (it != samples.end())
	{
		HCHANNEL hSampleChannel;
		if (channels[name] == -1)
		{
			hSampleChannel = BASS_SampleGetChannel(samples[name], false);
			channels[name] = hSampleChannel;
		}
		else
		{
			hSampleChannel = channels[name];
		}
		BASS_ChannelPlay(hSampleChannel, loop);
		return;
	}

	HSAMPLE hSample = BASS_SampleLoad(false, sound.c_str(), 0L, 0, 1, loop ? BASS_SAMPLE_LOOP : 0);
	HCHANNEL hSampleChannel = BASS_SampleGetChannel(hSample, false);
	BASS_ChannelPlay(hSampleChannel, loop);

	samples[name] = hSample;
	channels[name] = hSampleChannel;
}

void SoundManager::stopSound(const std::string& name)
{
	std::cout << "stopping sound.. " << name << std::endl;

	auto it = samples.find(name);
	if (it != samples.end())
	{
		HCHANNEL hSampleChannel = channels[name];
		BASS_ChannelStop(hSampleChannel);
		channels[name] = -1;
		return;
	}

	std::cout << "could not find sample.. " << name << std::endl;
}

void SoundManager::setVolume(const std::string& name, float value)
{
	std::cout << "changing volume to .. " << name << std::endl;

	auto it = samples.find(name);
	if (it != samples.end())
	{
		HCHANNEL hSampleChannel = channels[name];
		BASS_ChannelSetAttribute(hSampleChannel, BASS_ATTRIB_VOL, value);
		return;
	}

	std::cout << "could not find sample.. " << name << std::endl;
}