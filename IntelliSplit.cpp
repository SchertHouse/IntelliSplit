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
		pGraphics->AttachControl(new IVNumberBoxControl(bounds, kChannel,nullptr,"",DEFAULT_STYLE,true,1,1,16,"%0.0f", false));

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
		
		for (auto i = 0; i < N_KEY; i++) {
			Evolve(i, hands.count(i) > 0);
		}
	}
}

void IntelliSplit::ProcessMidiMsg(const IMidiMsg& msg)
{
	const int outCh1 = GetParam(kOutputChannel1)->Int();
	const int outCh2 = GetParam(kOutputChannel2)->Int();

	const int msgType = msg.StatusMsg();
	const int channel = msg.Channel();
	const int note = msg.NoteNumber();
	const int velocity = msg.Velocity();
	IMidiMsg outMsg;

	const int outCh = ProcessingSplit(channel) ? outCh1 : outCh2;

	switch (msgType)
	{
	case IMidiMsg::kNoteOn:
	{
		channelToNoteMap[channel] = msg.NoteNumber();

		hands.insert({ note, ProcessingSplit(channel)});

		outMsg.MakeNoteOnMsg(note, velocity, msg.mOffset, outCh);

		SendMidiMsg(outMsg);
		break;
	}
	case IMidiMsg::kNoteOff:
	{

		channelToNoteMap.erase(channel);
		outMsg.MakeNoteOffMsg(note, msg.mOffset, outCh);

		SendMidiMsg(outMsg);
		break;
	}

	case IMidiMsg::kPolyAftertouch:
	{
		const int note = msg.NoteNumber();
		const int pressure = msg.PolyAfterTouch();

		outMsg.MakePolyATMsg(note, pressure, msg.mOffset, outCh);
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
			outMsg.MakeControlChangeMsg(cc, val, outCh, msg.mOffset);
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
			outMsg.MakePitchWheelMsg(bendVal, outCh, msg.mOffset);
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

bool IntelliSplit::ProcessingSplit(int channel) {
	// Check if the channel is being processed
	return channelToNoteMap.find(channel) != channelToNoteMap.end();
}


void IntelliSplit::Evolve(int note, bool onOff) {
	byte newVal = onOff ? std::numeric_limits<uint8_t>::max() : 0;
	const double pSmooth = GetParam(kParamPSmooth)->Value() / 100.;
	EvolveKeyboard(keyboard, N_KEY, Smooth, note, newVal, pSmooth);
}
#endif
