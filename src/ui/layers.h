#pragma once

#include <array>
#include <cstddef>
#include "hw/controls.h"

namespace clessidra
{
// Knobs with a second function while a pad is held. A knob takes over a value
// only once it reaches it (pick-up), so nothing jumps when the layer changes.
class Layers
{
  public:
    // knob within this of a value, or across it: picked up
    static constexpr float kPickUp = 0.02f;

    // knob gets a second function, starting at value
    void Add(size_t knob, float value);
    // Every control pass: the knobs as read, and the second layer held
    void Process(const std::array<float, Controls::kNumKnobs>& knobs,
                 bool                                          second);

    float First(size_t knob) const { return knobs_[knob].first; }
    float Second(size_t knob) const { return knobs_[knob].second; }
    // For UIs with one control per function: sets a second value directly
    void SetSecond(size_t knob, float value) { knobs_[knob].second = value; }
    // The second value last changed by a knob, while the layer is held
    bool  Turned() const { return turned_ < Controls::kNumKnobs; }
    float TurnedValue() const { return knobs_[turned_].second; }

  private:
    struct Knob
    {
        float first   = 0.0f;
        float second  = 0.0f;
        float last    = 0.0f; // reading at the last pass
        bool  layered = false;
        bool  caught  = true; // the knob drives the value of the active layer
    };

    std::array<Knob, Controls::kNumKnobs> knobs_{};
    bool                                  second_ = false;
    bool                                  ready_  = false;
    size_t                                turned_ = Controls::kNumKnobs;
};

} // namespace clessidra
