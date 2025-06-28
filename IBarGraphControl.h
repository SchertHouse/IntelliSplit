#pragma once

//#include "IControl.h"        // Base class IControl
#include "IGraphics.h"       // IRECT, IGraphics, drawing methods
#include <array>             // std::array
#include <algorithm>         // std::clamp

template <size_t N_BAR>
class IBarGraphControl : public IControl
{
public:
  IBarGraphControl(const int& bounds, std::array<float, N_BAR>& levels)
    : IControl(bounds), mLevels(levels), mMarkerPos(0.0f)
  {
  }

  void Draw(IGraphics& g) override
  {
    constexpr int numBars = static_cast<int>(N_BAR);
    if(numBars == 0)
      return;

    float barWidth = mRECT.W() / static_cast<float>(numBars);
    float maxHeight = mRECT.H() - 20; // spazio per marker sotto

    // Disegna barre
    for (int i = 0; i < numBars; ++i)
    {
      float value = std::clamp(mLevels[i], 0.0f, 1.0f);
      float barHeight = value * maxHeight;
      float x = mRECT.L + i * barWidth;
      float y = mRECT.B - 20 - barHeight;

      IRECT barRect = IRECT(x, y, x + barWidth - 2.0f, mRECT.B - 20);

      g.FillRect(COLOR_GREEN, barRect);
    }

    // Disegna cerchio marker (posizione float)
    float markerRadius = 6.0f;
    // Clamp la posizione fra 0 e numBars-1
    float clampedPos = std::clamp(mMarkerPos, 0.0f, static_cast<float>(numBars - 1));
    // Calcola X in base alla posizione float
    float markerX = mRECT.L + clampedPos * barWidth + barWidth / 2.0f;
    float markerY = mRECT.B - 10.0f;

    g.FillEllipse(COLOR_RED, IRECT(markerX - markerRadius, markerY - markerRadius, markerX + markerRadius, markerY + markerRadius));
  }

  void SetMarkerPos(float pos)
  {
    float clampedPos = std::clamp(pos, 0.0f, static_cast<float>(N_BAR - 1));
    if (clampedPos != mMarkerPos)
    {
      mMarkerPos = clampedPos;
      SetDirty();
    }
  }

  float GetMarkerPos() const { return mMarkerPos; }

private:
  std::array<float, N_BAR>& mLevels;
  float mMarkerPos;
};
