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

	GetParam(kOutputChannelSx)->InitInt("Out Channel 1", 1, 1, N_CH);
	GetParam(kOutputChannelDx)->InitInt("Out Channel 2", 2, 1, N_CH);

	GetParam(kOutputTrasSx)->InitInt("Trans. Ch 1", 0, -N_KEY, N_KEY);
	GetParam(kOutputTrasDx)->InitInt("Trans. Ch 1", 0, -N_KEY, N_KEY);

	GetParam(kOutputMin)->InitInt("Min", DEFAULT_MIN, 0, N_KEY-1);
	GetParam(kOutputMax)->InitInt("Max", DEFAULT_MAX, 0, N_KEY-1);

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
		const float centerX = bounds.MW();
		const float ratioW = 3;
		const float ratioH = 20;
		const float paddingW = bounds.W() / ((4 * ratioW) + 5);
		const float paddingH = bounds.H() / ((5 * ratioH) + 6);

		const float columnW = paddingW * ratioW;
		const float rowH = paddingH * ratioH;
		const float numberBoxW = paddingW * ratioW;
		const float numberBoxH = rowH/1.4f;
		const float knobSize = columnW > rowH ? rowH : columnW;
		const float buttonW = columnW;
		const float buttonH = rowH /2.5f;

		// Manopola Smooth centrata in alto
		
		IRECT SmoothRect = IRECT::MakeXYWH(centerX - knobSize - paddingW/2, paddingH, knobSize, knobSize);
		IRECT minRect = SmoothRect.GetCentredInside(numberBoxW, numberBoxH).GetTranslated(- knobSize - paddingW, 0);
		
		IRECT resetRect = SmoothRect.GetTranslated(paddingW + knobSize, 0);
		IRECT maxRect = resetRect.GetCentredInside(numberBoxW, numberBoxH).GetTranslated(paddingW + knobSize, 0);

		pGraphics->AttachControl(new IVKnobControl(SmoothRect, kParamPSmooth));
		pGraphics->AttachControl(new IVKnobControl(resetRect, kTimeReset));
		pGraphics->AttachControl(new IVNumberBoxControl(minRect, kOutputMin, nullptr, "Min", DEFAULT_STYLE, true, 1, 1, N_CH, "%0.0f", false));
		pGraphics->AttachControl(new IVNumberBoxControl(maxRect, kOutputMax, nullptr, "Max", DEFAULT_STYLE, true, 1, 1, N_CH, "%0.0f", false));

		// IVNumberBoxControl (Output Channel 1 e 2)
		float topY = paddingH*2 + rowH + rowH /2 - numberBoxH/2;

		IRECT ch1Rect = IRECT::MakeXYWH(centerX - numberBoxW * 1.5f - paddingW, topY, numberBoxW, numberBoxH);
		IRECT splRect = IRECT::MakeXYWH(centerX - numberBoxW * .5f, topY, numberBoxW, numberBoxH);
		IRECT ch2Rect = IRECT::MakeXYWH(centerX + numberBoxW * .5f + paddingW, topY, numberBoxW, numberBoxH);

		pGraphics->AttachControl(new IVNumberBoxControl(ch1Rect, kOutputChannelSx, nullptr, "Ch1", DEFAULT_STYLE, true, 1, 1, N_CH, "%0.0f", false));		
		pGraphics->AttachControl(new IVNumberBoxControl(splRect, kSplit, nullptr, "Init.Split", DEFAULT_STYLE, true, DEFAULT_SPLIT, 0, N_KEY - 1, "%0.0f", false));		
		pGraphics->AttachControl(new IVNumberBoxControl(ch2Rect, kOutputChannelDx, nullptr, "Ch2", DEFAULT_STYLE, true, 2, 1, N_CH, "%0.0f", false));
		

		//Transponse
		float transY = paddingH * 3 + rowH * 2 + rowH / 2 - numberBoxH / 2;
		IRECT Tr1Rect = IRECT::MakeXYWH(ch1Rect.L, transY, numberBoxW, numberBoxH);
		IRECT Tr2Rect = IRECT::MakeXYWH(ch2Rect.L, transY, numberBoxW, numberBoxH);
		pGraphics->AttachControl(new IVNumberBoxControl(Tr1Rect, kOutputTrasSx, nullptr, "Trans.Ch1", DEFAULT_STYLE, true, 24, 1, N_KEY, "%0.0f", false));
		pGraphics->AttachControl(new IVNumberBoxControl(Tr2Rect, kOutputTrasDx, nullptr, "Trans.Ch2", DEFAULT_STYLE, true, 108, 1, N_KEY, "%0.0f", false));
		

		// Bottoni SX, Auto, DX
		float buttonsY = paddingH * 4 + rowH * 3 + rowH / 2 - buttonH / 2;

		IRECT sxButtonRect = IRECT::MakeXYWH(ch1Rect.L, buttonsY, buttonW, buttonH);
		IRECT autoButtonRect = IRECT::MakeXYWH(splRect.L, buttonsY, buttonW, buttonH);
		IRECT dxButtonRect = IRECT::MakeXYWH(ch2Rect.L, buttonsY, buttonW, buttonH);
		
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

		float graphY = dxButtonRect.B + paddingH;
		barGraphControl = new IBarGraphControl<N_KEY>(IRECT::MakeXYWH(bounds.L, graphY, bounds.W(), bounds.H() - graphY), keyboard);
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
	int note = msg.NoteNumber();

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
		const int noteTrans = handOutR ? computeTrans(note, GetParam(kOutputTrasSx)->Int()) : computeTrans(note, GetParam(kOutputTrasDx)->Int());

		outMsg.MakeNoteOnMsg(noteTrans, velocity, msg.mOffset, outCh);
		SendMidiMsg(outMsg);
		break;
	}
	case IMidiMsg::kNoteOff:
	{
		boolean leftHand = left.contains(note);
		const int noteTrans = leftHand ? computeTrans(note, GetParam(kOutputTrasSx)->Int()) : computeTrans(note, GetParam(kOutputTrasDx)->Int());
		outMsg.MakeNoteOffMsg(noteTrans, msg.mOffset, leftHand ? outCh1 : outCh2);
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
	} else if(!reset) {
		if(rSplit - lSplit >= 0)
			splitMid = lSplit + (rSplit - lSplit) / 2;
		else if (splitType == 0) {
			splitMid = lastPlay ? lSplit : rSplit;
		}
		else{
			splitMid = splitType < 0 ? lSplit : rSplit;
		}
	}

	if (splitMid < GetParam(kOutputMin)->Int()) {
		splitMid = GetParam(kOutputMin)->Int();
	}
	else if (splitMid > GetParam(kOutputMax)->Int()){
		splitMid = GetParam(kOutputMax)->Int();
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

int IntelliSplit::computeTrans(int note, int param) {
	note += param;

	// Evita di uscire dal range MIDI 0–127
	if (note > 127 || note < 0) return -1;

	return note;
}

void IntelliSplit::OnUIClose()
{
	mRunning = false;
	if (mWorkerThread.joinable())
		mWorkerThread.join();
}
#endif
