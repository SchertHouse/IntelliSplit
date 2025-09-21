#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include <bitset>
#include <unordered_set>
#include <optional>
#include <SmallSet.h>
#include <IBarGraphControl.h>
#include <thread>
#include <mutex>
#include "Utils.h"
#include "ThreadSafeVector.h"

#define N_KEY 128
#define N_FINGER 5
#define N_HAND_RANGE 12
#define UPDATE_FREQ (1000 / PLUG_FPS)
#define DEFAULT_SPLIT (127 / 2)
#define B_GROUP_AUTO 0.5f
#define B_GROUP_SX 0.0f
#define B_GROUP_DX 1.0f
#define N_CH 16
#define DEFAULT_MIN 24
#define DEFAULT_MAX 108
#define INIT_CH_1 1
#define INIT_CH_2 2
#define FADETH 3

constexpr float MAXF = 1;
constexpr float MINF = 0;
const int kNumPresets = 1;

enum EParams
{
	kParamPSmooth = 1,
	kButtonGroup = 2,
	kOutputChannelSx = 3,
	kOutputChannelDx = 4,
	kBarGraphControl = 5,
	kSplit = 6,
	kTimeReset= 7,
	kOutputTrasSx = 8,
	kOutputTrasDx = 9,
	kOutputMin = 10,
	kOutputMax = 11,
	kParamRange = 12,
	kNumParams
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
	uint8_t channelNote[N_CH];
	ThreadSafeVector<uint8_t> left, leftD;
	ThreadSafeVector<uint8_t> right, rightD;
	float lSplit = 0, rSplit = 0, splitMid = 0;
	int64_t delayTime = 0;
	bool lastLeftPlay = false;
	bool reset = false;
	int oldCh1 = INIT_CH_1, oldCh2 = INIT_CH_2;
	int64_t mCurrentSample = 0;
	int64_t mNextEvolveSample = 0;
	bool fromUI = false;

protected:
	int IntelliSplit::ProcessingSplit(const IMidiMsg& msg, const std::array<float, N_KEY>& keyboard, bool noteEvent = true);
	void IntelliSplit::Evolve(int note = -1);
	int IntelliSplit::computeTrans(int note, int param);
	std::mutex mMutex;

	void EvolveKeyboard(float p, int note = -1) {

		if(note != -1)
			keyboard[note] = MAXF;

		noteOff.remove_if_modify(
			[&](std::uint8_t note) { return keyboard[note] <= 0; },
			[&](std::uint8_t note) { keyboard[note] = Utils::Smooth(keyboard[note], MINF, p); if (keyboard[note] < 0.5f) { leftD.remove_value(note); rightD.remove_value(note); }
	}
		);
	}
#endif
};
