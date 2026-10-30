#pragma once

namespace clessidra
{
// Moves a phase through a span, 0..1, and decides what happens at its ends.
class Playhead
{
  public:
    enum class End
    {
        OneShot,  // stops at the end, until it is restarted or flipped
        PingPong, // turns at each end
        Loop      // wraps around
    };

    // From the start of the span, in the direction of the speed
    void Restart();
    // After a freeze: the phase may have moved away from the end
    void Resume() { stopped_ = false; }

    // One sample, within low..high of the span. inc: phase per sample, negative
    // = backward. margin: kept clear at each end so the last grains stay
    // inside. Not for Loop.
    double Step(double phase,
                double inc,
                End    end,
                double low,
                double high,
                double margin);

    bool IsStopped() const { return stopped_; }
    bool IsBackward(double inc) const { return inc * direction_ < 0.0; }

  private:
    double direction_ = 1.0; // -1 on the way back in PingPong
    bool   stopped_   = false;
    bool   at_top_    = false; // stopped at the end of the span, not the start
};

} // namespace clessidra
