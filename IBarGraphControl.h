#pragma once

#include <array>
#include <algorithm>

template <size_t N_BAR>
class IBarGraphControl : public IControl
{
public:
  IBarGraphControl(const iplug::igraphics::IRECT& bounds, std::array<float, N_BAR>& levels)
    : IControl(bounds), mLevels(levels), mMarkerPos(0.0f)
  {
  }

  void Draw(iplug::igraphics::IGraphics& g) override
  {
    constexpr int numBars = static_cast<int>(N_BAR);
    if(numBars == 0)
      return;

    float barWidth = mRECT.W() / static_cast<float>(numBars);
    float markerheight = 20;
    float maxHeight = mRECT.H() - markerheight; // spazio per marker sotto

    // Disegna barre
    for (int i = 0; i < numBars; ++i)
    {
      float value = std::clamp(mLevels[i], 0.0f, 1.0f);
      float barHeight = value * maxHeight;
      float d = 0;
      float x = mRECT.L + i * barWidth;
      float y = mRECT.B - markerheight - barHeight;

      IRECT barRect = IRECT(x, y, x + barWidth - d, mRECT.B - markerheight);

      g.FillRect(COLOR_GREEN, barRect);
    }

    // Disegna cerchio marker (posizione float)
    float markerRadius = 6.0f;
    // Clamp la posizione fra 0 e numBars-1
    float clampedPos = std::clamp(mMarkerPos, 0.0f, static_cast<float>(numBars - 1));
    // Calcola X in base alla posizione float
    float markerX = mRECT.L + clampedPos * barWidth + barWidth / 2.0f;
    float markerY = mRECT.B - markerheight/2;

    float halfBase = markerRadius / 2.0f;
    float height = std::sqrt(markerRadius * markerRadius - halfBase * halfBase);
    float x1 = markerX;
    float y1 = markerY - height;
    float x2 = markerX - halfBase;
    float y2 = markerY + height / 2.0f;
    float x3 = markerX + halfBase;
    float y3 = markerY + height / 2.0f;

    g.FillTriangle(COLOR_RED, x1, y1, x2, y2, x3, y3);
  }

  void SetValueFromDelegate(double value, int valIdx = 0) override
  {
      SetMarkerPos(value);
  }

  void SetMarkerPos(float pos)
  {
    float clampedPos = std::clamp(pos, 0.0f, static_cast<float>(N_BAR - 1));
    if (clampedPos != this->mMarkerPos)
    {
        this->mMarkerPos = clampedPos;
        SetDirty();
    }
  }

  float GetMarkerPos() const { return this->mMarkerPos; }

private:
  std::array<float, N_BAR>& mLevels;
  float mMarkerPos;
};
