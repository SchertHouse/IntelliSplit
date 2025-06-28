#include "IntelliSplit.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

IntelliSplit::IntelliSplit(const InstanceInfo& info)
	: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
	GetParam(kChannel)->InitInt("Canale SX", 1, 1, 16, "ch");

	GetParam(kParamPSmooth)->InitDouble("Knob 1", 0.0, 0.0, 1.0, 0.01);
	GetParam(kKnob2)->InitDouble("Knob 2", 0.0, 0.0, 1.0, 0.01);

	GetParam(kOutputChannel1)->InitInt("Out Channel 1", 1, 1, 16); // MIDI channels 1-16
	GetParam(kOutputChannel2)->InitInt("Out Channel 2", 2, 1, 16);

	GetParam(kButton1)->InitInt("Button 1", 0, 0, 2, "");
	GetParam(kButton2)->InitBool("Button 2", false);




#if IPLUG_DSP
	SetTailSize(4410000);
#endif

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
	mMakeGraphicsFunc = [&]() {
		return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
		};

	mLayoutFunc = [&](IGraphics* pGraphics) {

		pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
		pGraphics->AttachPanelBackground(COLOR_GRAY);

		const IRECT area = pGraphics->GetBounds().GetCentredInside(100);

		// Esempio: 3 opzioni (Low, Mid, High)
		pGraphics->AttachControl(new IVRadioButtonControl(
			area,
			kButton1,
			{ "Low", "Mid", "High" }
		));

		const IRECT bounds = pGraphics->GetBounds().GetCentredInside(50);
		pGraphics->AttachControl(new IVNumberBoxControl(bounds, kChannel, nullptr, "", DEFAULT_STYLE, true, 1, 1, 16, "%0.0f", false));

		const float knobSize = 60.f;

		auto actionFunc = [&](IControl* pCaller) {

			};

		pGraphics->AttachControl(new IVKnobControl(IRECT().MakeXYWH(0, 0, knobSize, knobSize), kParamPSmooth));
		pGraphics->AttachControl(new IVKnobControl(IRECT().MakeXYWH(30, 0, knobSize, knobSize), kKnob1));
		pGraphics->AttachControl(new IVKnobControl(IRECT().MakeXYWH(90, 0, knobSize, knobSize), kKnob2));
		};
#endif
}

#if IPLUG_DSP
void IntelliSplit::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
	mSampleCount += nFrames;
	int samplesPer100ms = static_cast<int>(GetSampleRate() * 0.1);

	if (mSampleCount >= samplesPer100ms)
	{
		mSampleCount -= samplesPer100ms;
		Evolve();
	}
}

void IntelliSplit::ProcessMidiMsg(const IMidiMsg& msg)
{
	const int msgType = msg.StatusMsg();
	const int note = msg.NoteNumber();

	const int channel = msg.Channel();
	const int velocity = msg.Velocity();

	const int outCh1 = GetParam(kOutputChannel1)->Int();
	const int outCh2 = GetParam(kOutputChannel2)->Int();

	IMidiMsg outMsg;

	switch (msgType)
	{
	case IMidiMsg::kNoteOn:
	{
		noteOn.add(note);
		noteOff.remove(note);

		Evolve(note);
		const bool handOutR = ProcessingSplit(msgType, note);
		const int outCh = handOutR ? outCh1 : outCh2;

		(handOutR ? right : left).insert(note);

		outMsg.MakeNoteOnMsg(note, velocity, msg.mOffset, outCh);		
		SendMidiMsg(outMsg);
		break;
	}
	case IMidiMsg::kNoteOff:
	{

		noteOn.remove(note);

		outMsg.MakeNoteOffMsg(note, msg.mOffset, left.count(note) ? outCh1 : outCh2);

		SendMidiMsg(outMsg);
		noteOff.add(note);
		break;
	}

	case IMidiMsg::kPolyAftertouch:
	{
		const int note = msg.NoteNumber();
		const int pressure = msg.PolyAfterTouch();

		outMsg.MakePolyATMsg(note, pressure, msg.mOffset, left.count(note) ? outCh1 : outCh2);
		SendMidiMsg(outMsg);
		break;
	}

	case IMidiMsg::kControlChange:
	{
		IMidiMsg::EControlChangeMsg cc = msg.ControlChangeIdx();
		const double val = msg.ControlChange(cc);

		if (channel == 0) {
			outMsg.MakeControlChangeMsg(cc, val, outCh1, msg.mOffset);
			IMidiMsg outMsg2;
			outMsg2.MakeControlChangeMsg(cc, val, outCh2, msg.mOffset);
			SendMidiMsg(outMsg);
			SendMidiMsg(outMsg2);
		}
		else {
			outMsg.MakeControlChangeMsg(cc, val, left.count(note) ? outCh1 : outCh2, msg.mOffset);
			SendMidiMsg(outMsg);
		}
		break;
	}

	case IMidiMsg::kPitchWheel:
	{
		const int bendVal = msg.PitchWheel();

		if (channel == 0) {
			outMsg.MakePitchWheelMsg(bendVal, outCh1, msg.mOffset);
			IMidiMsg outMsg2;
			outMsg2.MakePitchWheelMsg(bendVal, outCh2, msg.mOffset);
			SendMidiMsg(outMsg);
			SendMidiMsg(outMsg2);
		}
		else {
			outMsg.MakePitchWheelMsg(bendVal, left.count(note) ? outCh1 : outCh2, msg.mOffset);
			SendMidiMsg(outMsg);
		}
		break;
	}

	case IMidiMsg::kChannelAftertouch:
	{
		outMsg.MakeChannelATMsg(msg.ChannelAfterTouch(), msg.mOffset, outCh1);
		IMidiMsg outMsg2;
		outMsg2.MakeChannelATMsg(msg.ChannelAfterTouch(), msg.mOffset, outCh2);
		SendMidiMsg(outMsg);
		SendMidiMsg(outMsg2);
		break;
	}

	case IMidiMsg::kProgramChange:
	{
		outMsg.MakeProgramChange(msg.Program(), outCh1, msg.mOffset);
		IMidiMsg outMsg2;
		outMsg2.MakeProgramChange(msg.Program(), outCh2, msg.mOffset);
		SendMidiMsg(outMsg);
		SendMidiMsg(outMsg2);
		break;
	}
	default:
		break;
	}
}

float IntelliSplit::computeSplit(const std::array<float, N_KEY>& keyboard, bool fromLeft) {
	const float* ptr = fromLeft ? keyboard.data() : keyboard.data() + N_KEY - 1;
	const float* end = fromLeft ? keyboard.data() + N_KEY : keyboard.data() - 1;
	const int step = fromLeft ? 1 : -1;

	float weightedSum = 0.0f;
	float weightTotal = 0.0f;
	int countOn = 0;
	int borderKey = fromLeft ? N_KEY : 0;

	for (int i = 0; ptr != end && countOn < N_FINGER; ++i, ptr += step) {
		const float val = *ptr;
		const int key = fromLeft ? i : N_KEY - 1 - i;

		if (val == MAXF) {
			borderKey = key;
			countOn++;
			break;
		}

		weightedSum += val * key;
		weightTotal += val;
	}

	if (weightTotal > 0.0f) {
		float avgPos = weightedSum / weightTotal;
		float delta = std::abs(avgPos - static_cast<float>(borderKey));
		return static_cast<float>(borderKey) + static_cast<float>(step) * (N_HAND_RANGE - delta);
	}
	else {
		return static_cast<float>(borderKey) + static_cast<float>(step) * N_HAND_RANGE;
	}
}

bool IntelliSplit::ProcessingSplit(int note, const std::array<float, N_KEY>& keyboard) {
	float lSplit = computeSplit(keyboard, true);
	float rSplit = computeSplit(keyboard, false);
	float splitMid = (lSplit + rSplit) / 2.0f;

	return note < splitMid;
}


void IntelliSplit::Evolve(int note = -1) {
	const double pSmooth = GetParam(kParamPSmooth)->Value() / 100.;
	EvolveKeyboard(pSmooth, note);
}
#endif
