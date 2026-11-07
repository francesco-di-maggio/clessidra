#pragma once

namespace clessidra
{
// What a playhead does when the sand has run through its span: stops, stops
// and turns the glass over (the core's Flip), or starts again from the top
enum class End
{
    Once,
    Flip,
    Loop
};

// A point through a span, 0..1, moved by a signed step per sample
class Playhead
{
  public:
    // From the start of the span, or from its end when moving backwards
    void Restart(bool backwards);
    // One sample. Once it has run out, it waits at the edge until a step the
    // other way.
    void Advance(double step, End end);
    // The span changed: the point kept where it is, phase scaled by factor
    void Scale(double factor) { phase_ *= factor; }

    double Phase() const { return phase_; }
    bool   Ended() const { return ended_; }
    // Round the end to the start, or the other way, in the last Advance()
    bool Wrapped() const { return wrapped_; }
    // Of the span, what is left ahead in the last direction moved, 0..1
    double Left() const;

  private:
    double phase_   = 0.0;
    bool   forward_ = true; // the last direction moved
    bool   ended_   = false;
    bool   wrapped_ = false;
};

} // namespace clessidra
