#pragma once

namespace clessidra
{
// What a playhead does when the sand has run through its span: stops, turns
// the glass over by itself, or starts again from the top
enum class End
{
    Once,
    Turn,
    Loop
};

// A point through a span, 0..1, moved by a signed step per sample. Turned
// over, its steps run the other way.
class Playhead
{
  public:
    // From the start of the span, or from its end when moving backwards
    void Restart(bool backwards);
    // One sample. Once it has run out, it waits at the edge until a step the
    // other way: the glass turned by hand.
    void Advance(double step, End end);
    // The span changed: the point kept where it is, phase scaled by factor
    void Scale(double factor) { phase_ *= factor; }

    double Phase() const { return phase_; }
    bool   Ended() const { return ended_; }
    // 1, or -1 while turned over
    int Heading() const { return heading_; }
    // Of the span, what is left ahead in the last direction moved, 0..1
    double Left() const;

  private:
    double phase_   = 0.0;
    int    heading_ = 1;
    bool   forward_ = true; // the last direction moved
    bool   ended_   = false;
};

} // namespace clessidra
