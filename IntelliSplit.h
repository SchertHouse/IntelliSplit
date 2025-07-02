#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include <bitset>
#include <unordered_set>
#include <optional>
#include <SmallSet.h>
#include <IBarGraphControl.h>
#include <thread>
#include <mutex>

#define N_KEY 128
#define N_FINGER 5
#define N_HAND_RANGE 12

constexpr float MAXF = 1;
constexpr float MINF = 0;
const int kNumPresets = 1;

enum EParams
{
	kNumParams = 5,
	kParamPSmooth = 1,
	kButtonGroup = 2,
	kOutputChannelSx = 3,
	kOutputChannelDx = 4,
	kBarGraphControl = 5
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
	IBarGraphControl<N_KEY>* barGraphControl;
	float computeSplit(const std::array<float, N_KEY>& keyboard, bool fromLeft);
	std::thread mWorkerThread;
	std::atomic<bool> mRunning = false;
	void OnUIOpen() override;
	void OnUIClose() override;
	std::array<float, N_KEY> keyboard = {};
	SmallSet<std::uint8_t, 0, N_KEY> noteOn;
	SmallSet<std::uint8_t, 0, N_KEY> noteOff;
	std::vector<uint8_t> left;
	std::vector<uint8_t> right;
	float splitMid = 0;

protected:
	bool IntelliSplit::ProcessingSplit(int note, const std::array<float, N_KEY>& keyboard);
	void IntelliSplit::Evolve(int note = -1);
	std::mutex mMutex;

	static float Smooth(float b, float newVal, float p) {
		return b * (1 - p) + newVal * p;
	}

	void EvolveKeyboard(float p, int note = -1) {

		if(note != -1)
			keyboard[note] = MAXF;

		for (auto it = noteOff.values().begin(); it != noteOff.values().end(); ) {
			if (keyboard[*it] <= 0)
				it = noteOff.remove(*it);
			else {
				keyboard[*it] = Smooth(keyboard[*it], MINF, p);
				++it;
			}
		}
	}
#endif
};
