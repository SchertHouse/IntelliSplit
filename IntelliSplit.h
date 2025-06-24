#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include <bitset>
#define N_KEY 128
#define N_FINGER 5

const int kNumPresets = 1;
std::unordered_map<int /*channel*/, int /*note*/> channelToNoteMap;
inline uint8_t keyboard[N_KEY] = { 0 };
std::unordered_map<int /*note*/, int /*hand*/> hands;

enum EParams
{
	kParamPSmooth = 0,
	kNumParams,

	kOutputChannel1 = 0,
	kOutputChannel2 = 0,
	kButton1 = 0,
	kButton2 = 0,
	kKnob1 = 0,
	kKnob2 = 0
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
	bool ProcessingSplit(int channel);
	void IntelliSplit::Evolve(int note, bool onOff);

	using Func = uint8_t(*)(uint8_t, byte, double);
	static uint8_t Smooth(uint8_t b, byte newVal, double p) {
		return b * (1 - p) + newVal * p;

	}
	template<typename Func>
	void EvolveKeyboard(uint8_t* array, size_t size, Func f, int note, byte newVal, double p) {
		for (size_t i = 0; i < size; ++i) {
			array[i] = f(array[i], newVal, p);
		}
	}
#endif
};
