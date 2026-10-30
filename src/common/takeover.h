#pragma once

namespace clessidra
{
// A value set by a knob shared between layers. After a layer change the knob
// moves the value from where it is, scaled so both reach the end stops
// together: no jump, no dead zone.
class Takeover
{
  public:
    explicit Takeover(float value = 0.0f) : value_{value}, previous_{value} {}

    void  Engage(float knob) { previous_ = origin_ = knob; }
    void  Process(float knob);
    float Value() const { return value_; }

  private:
    // around the knob at Engage(): noise
    static constexpr float kDeadband = 0.02f;

    float value_;
    float previous_;
    float origin_ = -1.0f; // knob at Engage() until it moves past the deadband
};

} // namespace clessidra
