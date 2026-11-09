#pragma once

namespace clessidra
{
// A point through a span, 0..1, moved by a signed step per sample, round it
class Playhead
{
  public:
    // From the start of the span, or from its end when moving backwards
    void Restart(bool backwards);
    // To a point of the span
    void Jump(double phase);
    // One sample, round the span at its ends
    void Advance(double step);

    double Phase() const { return phase_; }

  private:
    double phase_ = 0.0;
};

} // namespace clessidra
