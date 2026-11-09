#pragma once

namespace clessidra
{
// A value set by a knob shared between layers. After a layer change the knob
// moves the value from where it is, scaled so both reach the next stop
// together: the end, or for a centred value the centre when both are on the
// same side of it. No jump, no dead zone.
class Takeover
{
  public:
    Takeover() = default;
    explicit Takeover(float value, bool centred = false)
    : value_{value}, previous_{value}, centred_{centred}
    {
    }

    // The value as is, the knob from here
    void Engage(float knob) { previous_ = origin_ = knob; }
    // The value set outright, the knob from here
    void Set(float value, float knob)
    {
        value_ = value;
        Engage(knob);
    }
    void  Process(float knob);
    float Value() const { return value_; }

  private:
    // around the knob at Engage(): noise
    static constexpr float kDeadband = 0.02f;
    // past each stop, so a value at it still moves
    static constexpr float kMargin = 0.001f;

    // The knob from one reading to the next, split at the centre
    void Move(float from, float to);

    float value_    = 0.0f;
    float previous_ = 0.0f;
    float origin_   = -1.0f; // knob at Engage() until past the deadband
    bool  centred_  = false;
};

} // namespace clessidra
