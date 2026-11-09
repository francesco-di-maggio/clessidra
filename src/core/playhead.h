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
// over at an end (End::Turn), its steps run the other way.
class Playhead
{
  public:
    // From the start of the span, or from its end when moving backwards,
    // upright
    void Restart(bool backwards);
    // To a point of the span, sand in the glass again
    void Jump(double phase);
    // One sample. Once it has run out, it waits at the edge until a step the
    // other way.
    void Advance(double step, End end);
    // The span changed: the same point, at its new phase
    void Move(double phase) { phase_ = phase; }

    double Phase() const { return phase_; }
    bool   Ended() const { return ended_; }
    // 1 upright, -1 turned over
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
