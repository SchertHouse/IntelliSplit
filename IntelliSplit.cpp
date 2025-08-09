#include "IntelliSplit.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "Utils.h"
#include "IBarGraphControl.h"
#include <iostream>

IntelliSplit::IntelliSplit(const InstanceInfo& info)
	: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
	mRunning = false;
	for (std::uint8_t i = 0; i < N_KEY; ++i) {
		noteOff.add(i);
	} 

	GetParam(kParamPSmooth)->InitDouble("Smooth", 250, 0, 5000, 1, "ms");
	GetParam(kTimeReset)->InitDouble("Reset", 1000, 0.0, 10000, 1, "ms");

	GetParam(kOutputChannelSx)->InitInt("Out Channel 1", 1, 1, 16);
	GetParam(kOutputChannelDx)->InitInt("Out Channel 2", 2, 1, 16);
	GetParam(kSplit)->InitInt("Split Note Number", DEFAULT_SPLIT, 0, N_KEY-1);

	GetParam(kButtonGroup)->InitInt("Channel Mode", 0, -1, 1);

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

		const IRECT bounds = pGraphics->GetBounds();
		const float padding = 10.0f;
		const float knobSize = 60.0f;
		const float numberBoxW = 50.0f;
		const float numberBoxH = 25.0f;
		const float buttonW = numberBoxW;
		const float buttonH = numberBoxH;

		// Manopola Smooth centrata in alto
		IRECT knobRect = bounds.GetCentredInside(knobSize, knobSize).GetTranslated(-knobSize / 2 - padding/2, -bounds.H() / 2 + knobSize / 2 + padding);
		IRECT knobRect2 = bounds.GetCentredInside(knobSize, knobSize).GetTranslated(knobSize / 2 + padding / 2, -bounds.H() / 2 + knobSize / 2 + padding);
		pGraphics->AttachControl(new IVKnobControl(knobRect, kParamPSmooth));
		pGraphics->AttachControl(new IVKnobControl(knobRect2, kTimeReset));

		// IVNumberBoxControl (Output Channel 1 e 2)
		float centerX = bounds.MW();
		float topY = knobRect.B + padding;

		IRECT ch1Rect = IRECT::MakeXYWH(centerX - numberBoxW * 1.5f - padding, topY, numberBoxW, numberBoxH);
		IRECT splRect = IRECT::MakeXYWH(centerX - numberBoxW * .5f, topY, numberBoxW, numberBoxH);
		IRECT ch2Rect = IRECT::MakeXYWH(centerX + numberBoxW * .5f + padding, topY, numberBoxW, numberBoxH);

		pGraphics->AttachControl(new IVNumberBoxControl(ch1Rect, kOutputChannelSx, nullptr, "", DEFAULT_STYLE, true, 1, 1, 16, "%0.0f", false));
		pGraphics->AttachControl(new IVNumberBoxControl(splRect, kSplit, nullptr, "", DEFAULT_STYLE, true, DEFAULT_SPLIT, 0, N_KEY - 1, "%0.0f", false));
		pGraphics->AttachControl(new IVNumberBoxControl(ch2Rect, kOutputChannelDx, nullptr, "", DEFAULT_STYLE, true, 2, 1, 16, "%0.0f", false));
		

		// Bottoni SX, Auto, DX
		float buttonsY = splRect.B + padding;

		IRECT sxButtonRect = IRECT::MakeXYWH(ch1Rect.L, buttonsY, ch1Rect.W(), ch1Rect.H());
		IRECT autoButtonRect = IRECT::MakeXYWH(splRect.L, buttonsY, splRect.W(), splRect.H());
		IRECT dxButtonRect = IRECT::MakeXYWH(ch2Rect.L, buttonsY, ch2Rect.W(), ch2Rect.H());
		
		auto dxButton = new IVButtonControl(dxButtonRect, nullptr, "DX");
		auto autoButton = new IVButtonControl(autoButtonRect, nullptr, "Auto");
		auto sxButton = new IVButtonControl(sxButtonRect, nullptr, "SX");

		autoButton->SetActionFunction([this, sxButton, dxButton](IControl* pCaller) {
			GetParam(kButtonGroup)->SetNormalized(B_GROUP_AUTO);
			pCaller->SetValue(1);
			sxButton->SetValue(0);
			dxButton->SetValue(0);
			sxButton->SetDirty(false);
			dxButton->SetDirty(false);
			});
		dxButton->SetActionFunction([this, autoButton, sxButton](IControl* pCaller) {
			GetParam(kButtonGroup)->SetNormalized(B_GROUP_DX);
			pCaller->SetValue(1);
			autoButton->SetValue(0);
			sxButton->SetValue(0);
			autoButton->SetDirty(false);
			sxButton->SetDirty(false);
			});
		sxButton->SetActionFunction([this, autoButton, dxButton](IControl* pCaller) {
			GetParam(kButtonGroup)->SetNormalized(B_GROUP_SX);
			pCaller->SetValue(1);
			autoButton->SetValue(0);
			dxButton->SetValue(0);
			autoButton->SetDirty(false);
			dxButton->SetDirty(false);
			});

		if (GetParam(kButtonGroup)->GetNormalized() == B_GROUP_AUTO) {
			autoButton->GetActionFunction()(autoButton);
		}
		else if (GetParam(kButtonGroup)->GetNormalized() == B_GROUP_SX) {
			sxButton->GetActionFunction()(sxButton);
		}
		else {
			dxButton->GetActionFunction()(dxButton);
		}

		pGraphics->AttachControl(sxButton);
		pGraphics->AttachControl(autoButton);
		pGraphics->AttachControl(dxButton);

		barGraphControl = new IBarGraphControl<N_KEY>(IRECT::MakeXYWH(bounds.L, autoButtonRect.B + padding, bounds.W(), bounds.H() - autoButtonRect.B - padding), keyboard);
		pGraphics->AttachControl(barGraphControl, kBarGraphControl);
		};
#endif
}

#if IPLUG_DSP
void IntelliSplit::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{

}

void IntelliSplit::ProcessMidiMsg(const IMidiMsg& msg)
{
	const int msgType = msg.StatusMsg();
	const int note = msg.NoteNumber();

	const int channel = msg.Channel();
	const int velocity = msg.Velocity();

	const int outCh1 = GetParam(kOutputChannelSx)->Int()-1;
	const int outCh2 = GetParam(kOutputChannelDx)->Int()-1;

	IMidiMsg outMsg;

	switch (msgType)
	{
	case IMidiMsg::kNoteOn:
	{
		noteOn.add(note);
		channelNote[channel] = note;
		noteOff.remove(note);

		Evolve(note);
		const bool handOutR = ProcessingSplit(note, keyboard);
		const int outCh = handOutR ? outCh1 : outCh2;

		outMsg.MakeNoteOnMsg(note, velocity, msg.mOffset, outCh);
		SendMidiMsg(outMsg);
		break;
	}
	case IMidiMsg::kNoteOff:
	{
		boolean leftHand = left.contains(note);
		outMsg.MakeNoteOffMsg(note, msg.mOffset, leftHand ? outCh1 : outCh2);
		SendMidiMsg(outMsg);

		noteOff.add(note);
		noteOn.remove(note);
		millis = Utils::GetCurrentTimeMilliseconds() + GetParam(kTimeReset)->Value();

		if (leftHand)
			left.remove_value(note);
		else
			right.remove_value(note);
		break;
	}

	case IMidiMsg::kPolyAftertouch:
	{
		const int note = msg.NoteNumber();
		const int pressure = msg.PolyAfterTouch();

		outMsg.MakePolyATMsg(note, pressure, msg.mOffset, left.contains(note) ? outCh1 : outCh2);
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
			outMsg.MakeControlChangeMsg(cc, val, left.contains(channelNote[channel]) ? outCh1 : outCh2, msg.mOffset);
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
			outMsg.MakePitchWheelMsg(bendVal, left.contains(channelNote[channel]) ? outCh1 : outCh2, msg.mOffset);
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
	int borderKey = fromLeft ? N_KEY : 0;

	for (int i = 0; ptr != end; ++i, ptr += step) {
		const float val = *ptr;

		if (val != 0) {
			const int key = fromLeft ? i : N_KEY - 1 - i;

			if (val == MAXF) {
				borderKey = key;
				break;
			}

			weightedSum += val * key;
			weightTotal += val;
		}
	}

	if (weightTotal > 0.0f) {
		return borderKey + step * (N_HAND_RANGE - std::abs((weightedSum / weightTotal) - borderKey) * (weightTotal >= 1 ? 1 : weightTotal));
	}
	else {
		return borderKey + step * N_HAND_RANGE;
	}
}

bool IntelliSplit::ProcessingSplit(int note, const std::array<float, N_KEY>& keyboard) {
	lSplit = computeSplit(keyboard, true);
	rSplit = computeSplit(keyboard, false);
	int splitType = GetParam(kButtonGroup)->Value();

	if (note == splitMid)
		splitMid -= splitType;

	if (note > splitMid && rSplit <= splitMid) {
		splitMid = rSplit;
	}
	else if (note < splitMid && lSplit >= splitMid) {
		splitMid = lSplit;
	}

	if(!reset) {
		if(rSplit - lSplit >= 0)
			splitMid = lSplit + (rSplit - lSplit) / 2;
		else if (splitType == 0) {
			splitMid = lastPlay ? lSplit : rSplit;
		}
		else{
			splitMid = splitType < 0 ? lSplit : rSplit;
		}
	}

	reset = false;

	note < splitMid ? left.push_back(note) : right.push_back(note);
	SendControlValueFromDelegate(kBarGraphControl, splitMid);

	return lastPlay = note < splitMid;
}


void IntelliSplit::Evolve(int note) {
	std::lock_guard<std::mutex> lock(mMutex);
	const float pSmooth = Utils::ComputeSmoothingP(UPDATE_FREQ, GetParam(kParamPSmooth)->Value());
	EvolveKeyboard(pSmooth, note);
}

void IntelliSplit::OnUIOpen()
{
	if (!mRunning)
	{
		mRunning = true;

		mWorkerThread = std::thread([this]() {
			while (mRunning)
			{
				Evolve();

				if (noteOn.size() == 0 && millis <= Utils::GetCurrentTimeMilliseconds()) {
					splitMid = GetParam(kSplit)->Value();
					SendControlValueFromDelegate(kBarGraphControl, splitMid);
					reset = true;
				}

				if (barGraphControl)
					barGraphControl->SetDirty();

				std::this_thread::sleep_for(std::chrono::milliseconds(UPDATE_FREQ));
			}
			});
	}
}

void IntelliSplit::OnUIClose()
{
	mRunning = false;
	if (mWorkerThread.joinable())
		mWorkerThread.join();
}
#endif
