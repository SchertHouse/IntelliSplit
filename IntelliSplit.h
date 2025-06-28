#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include <bitset>
#include <unordered_set>
#include <optional>
#include<SmallSet.h>
#define N_KEY 128
#define N_FINGER 5

const int kNumPresets = 1;
std::array<float, N_KEY> keyboard = {};
SmallSet<std::uint8_t, 0, N_KEY> noteOn;
SmallSet<std::uint8_t, 0, N_KEY> noteOff;

enum EParams
{
	kNumParams = 9,
	kParamPSmooth = 0,
	kOutputChannel1 = 1,
	kOutputChannel2 = 2,
	kButton1 = 3,
	kButton2 = 4,
	kKnob1 = 5,
	kKnob2 = 6,
	kChannel = 8
};

enum EControlTags
{
	kNumCtrlTags
};

using namespace iplug;
using namespace igraphics;

class IntelliSplit final : public Plugin
{
public:
	IntelliSplit(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
public:
	void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
	void ProcessMidiMsg(const IMidiMsg& msg) override;

private:
	int mSampleCount;

protected:
	bool ProcessingSplit(int msgType, int note);
	void IntelliSplit::Evolve(int note);

	static float Smooth(float b, float newVal, float p) {
		return b * (1 - p) + newVal * p;
	}

	void EvolveKeyboard(float p, int note = -1) {

		if(note != -1)
			keyboard[note] = std::numeric_limits<float>::max();

		for (auto it = noteOff.values().begin(); it != noteOff.values().end(); ) {
			if (keyboard[*it] <= 0)
				it = noteOff.remove(*it);
			else {		
				keyboard[*it] = Smooth(keyboard[*it], std::numeric_limits<float>::min(), p);
				++it;
			}
		}
	}
#endif
};
