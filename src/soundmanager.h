#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <vector>
#include <map>
#include "utils.h"
#include "bass.h"

class SoundManager {

private:
	SoundManager();
	~SoundManager();

public:

	static SoundManager* instance;

	static SoundManager* getInstance() {
		if (instance == NULL) instance = new SoundManager();
		return instance;
	}

	void playSound(const std::string& name, bool loop);
	void stopSound(const std::string& name);
	void pauseSound();

	void loadSound();

	void setVolume(const std::string& name, float value);

	std::map<std::string, HCHANNEL> channels;
	std::map<std::string, HSAMPLE> samples;

};

#endif // !SOUNDMANAGER_H
