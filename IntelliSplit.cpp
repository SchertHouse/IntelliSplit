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

	GetParam(kParamPSmooth)->InitDouble("Smooth", 1.0, 0.0, 1.0, 0.01);

	GetParam(kOutputChannelSx)->InitInt("Out Channel 1", 1, 1, 16, "ch");
	GetParam(kOutputChannelDx)->InitInt("Out Channel 2", 2, 1, 16, "ch");

	GetParam(kButtonGroup)->InitInt("Channel Mode", 0, 0, 2);

	keyboard.fill(1.0f);
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
		const float buttonW = 40.0f;
		const float buttonH = 40.0f;

		// Manopola Smooth centrata in alto
		IRECT knobRect = bounds.GetCentredInside(knobSize, knobSize).GetTranslated(0, -bounds.H() / 2 + knobSize / 2 + padding);
		pGraphics->AttachControl(new IVKnobControl(knobRect, kParamPSmooth));

		// IVNumberBoxControl (Output Channel 1 e 2)
		float centerX = bounds.MW();
		float topY = knobRect.B + padding;

		IRECT ch1Rect = IRECT::MakeXYWH(centerX - numberBoxW - padding / 2, topY, numberBoxW, numberBoxH);
		IRECT ch2Rect = IRECT::MakeXYWH(centerX + padding / 2, topY, numberBoxW, numberBoxH);

		pGraphics->AttachControl(new IVNumberBoxControl(ch1Rect, kOutputChannelSx, nullptr, "", DEFAULT_STYLE, true, 1, 1, 16, "%0.0f", false));
		pGraphics->AttachControl(new IVNumberBoxControl(ch2Rect, kOutputChannelDx, nullptr, "", DEFAULT_STYLE, true, 1, 1, 16, "%0.0f", false));

		// Bottoni SX, Auto, DX
		float buttonsY = ch1Rect.B + padding;

		IRECT sxButtonRect = IRECT::MakeXYWH(ch1Rect.MW() - buttonW / 2, buttonsY, buttonW, buttonH);
		IRECT dxButtonRect = IRECT::MakeXYWH(ch2Rect.MW() - buttonW / 2, buttonsY, buttonW, buttonH);
		IRECT autoButtonRect = IRECT::MakeXYWH(centerX - buttonW / 2, buttonsY, buttonW, buttonH);

		auto autoButton = new IVButtonControl(autoButtonRect, nullptr, "Auto");
		auto dxButton = new IVButtonControl(dxButtonRect, nullptr, "DX");
		auto sxButton = new IVButtonControl(sxButtonRect, nullptr, "SX");

		autoButton->SetActionFunction([this, sxButton, dxButton](IControl* pCaller) {
			GetParam(kButtonGroup)->SetNormalized(0);
			sxButton->SetValue(0);
			dxButton->SetValue(0);
			sxButton->SetDirty(false);
			dxButton->SetDirty(false);
			});
		dxButton->SetActionFunction([this, autoButton, sxButton](IControl* pCaller) {
			GetParam(kButtonGroup)->SetNormalized(1);
			autoButton->SetValue(0);
			sxButton->SetValue(0);
			autoButton->SetDirty(false);
			sxButton->SetDirty(false);
			});
		sxButton->SetActionFunction([this, autoButton, dxButton](IControl* pCaller) {
			GetParam(kButtonGroup)->SetNormalized(0.5);
			for (std::uint8_t i = 0; i < N_KEY; ++i) {
				keyboard[i] = 1.0f; // Reset keyboard values
				noteOff.add(i);
			}
			autoButton->SetValue(0);
			dxButton->SetValue(0);
			autoButton->SetDirty(false);
			dxButton->SetDirty(false);
			});

		pGraphics->AttachControl(sxButton);
		pGraphics->AttachControl(autoButton);
		pGraphics->AttachControl(dxButton);

		barGraphControl = new IBarGraphControl<N_KEY>(bounds.GetPadded(-20).GetFromBottom(100), keyboard);
		pGraphics->AttachControl(barGraphControl, kBarGraphControl);

		splitBody = new Body(60);
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

	const int outCh1 = GetParam(kOutputChannelSx)->Int();
	const int outCh2 = GetParam(kOutputChannelDx)->Int();

	IMidiMsg outMsg;

	switch (msgType)
	{
	case IMidiMsg::kNoteOn:
	{
		noteOn.add(note);
		noteOff.remove(note);

		Evolve(note);
		const bool handOutR = ProcessingSplit(note, keyboard);
		const int outCh = handOutR ? outCh1 : outCh2;

		(handOutR ? right : left).push_back(note);

		outMsg.MakeNoteOnMsg(note, velocity, msg.mOffset, outCh);
		SendMidiMsg(outMsg);
		break;
	}
	case IMidiMsg::kNoteOff:
	{

		noteOn.remove(note);
		Utils::remove<uint8_t>(left, note);
		Utils::remove<uint8_t>(right, note);

		outMsg.MakeNoteOffMsg(note, msg.mOffset, Utils::contains<uint8_t>(left, note) ? outCh1 : outCh2);

		SendMidiMsg(outMsg);
		noteOff.add(note);
		break;
	}

	case IMidiMsg::kPolyAftertouch:
	{
		const int note = msg.NoteNumber();
		const int pressure = msg.PolyAfterTouch();

		outMsg.MakePolyATMsg(note, pressure, msg.mOffset, Utils::contains<uint8_t>(left, note) ? outCh1 : outCh2);
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
			outMsg.MakeControlChangeMsg(cc, val, Utils::contains<uint8_t>(left, note) ? outCh1 : outCh2, msg.mOffset);
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
			outMsg.MakePitchWheelMsg(bendVal, Utils::contains<uint8_t>(left, note) ? outCh1 : outCh2, msg.mOffset);
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
	lSplit = std::round(computeSplit(keyboard, true));
	rSplit = std::round(computeSplit(keyboard, false));
	float splitMid  = std::round(splitBody->getPosition());

	if (note >= splitMid && rSplit <= splitMid ) {
		lSplit = rSplit;
		splitBody->setPosition(rSplit);
	}

	if (note <= splitMid && lSplit >= splitMid) {
		rSplit = lSplit;
		splitBody->setPosition(lSplit);
	}
	
	float splitMidN = note > splitMid ? lSplit : rSplit;

	note < splitMid ? left.push_back(note) : right.push_back(note);

	return note < splitMid;
}


void IntelliSplit::Evolve(int note) {
	std::lock_guard<std::mutex> lock(mMutex);
	const float pSmooth = GetParam(kParamPSmooth)->Value();
	EvolveKeyboard(pSmooth, note);
	float position = splitBody->updateElasticPosition(lSplit, rSplit, pSmooth*10, UPDATE_FREQ/100.0f);
	SendControlValueFromDelegate(kBarGraphControl, position);
}

void IntelliSplit::OnUIOpen()
{
	if (!mRunning)
	{
		mRunning = true;

		mWorkerThread = std::thread([this]() {
			while (mRunning)
			{
				{
					Evolve();

					if (barGraphControl)
						barGraphControl->SetDirty();
				}

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
