#pragma once

#include <array>
#include <cstddef>
#include "hw/controls.h"

namespace clessidra
{
// Knobs with more functions, one per layer, while a pad is held. A knob takes
// over a value only once it reaches it (pick-up), so nothing jumps when the
// layer changes.
class Layers
{
  public:
    static constexpr size_t kNumLayers = 3; // the knobs' own, and two held
    // knob within this of a value, or across it: picked up
    static constexpr float kPickUp = 0.02f;

    // knob gets a function on layer (1 or more), starting at value
    void Add(size_t layer, size_t knob, float value);
    // Every control pass: the knobs as read, and the layer held, 0 for none
    void Process(const std::array<float, Controls::kNumKnobs>& knobs,
                 size_t                                        layer);

    float Value(size_t layer, size_t knob) const
    {
        return knobs_[knob].value[layer];
    }
    // For UIs with one control per function: sets a value directly
    void Set(size_t layer, size_t knob, float value)
    {
        knobs_[knob].value[layer] = value;
    }
    // The value last changed by a knob, while a layer is held
    bool  Turned() const { return turned_ < Controls::kNumKnobs; }
    float TurnedValue() const { return knobs_[turned_].value[layer_]; }

  private:
    struct Knob
    {
        std::array<float, kNumLayers> value{};
        std::array<bool, kNumLayers>  used{true};  // the knob's own always
        float                         last = 0.0f; // reading at the last pass
        bool caught = true; // the knob drives its value on the active layer
    };

    std::array<Knob, Controls::kNumKnobs> knobs_{};
    size_t                                layer_  = 0;
    bool                                  ready_  = false;
    size_t                                turned_ = Controls::kNumKnobs;
};

} // namespace clessidra
