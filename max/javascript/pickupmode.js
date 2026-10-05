// Max's Pickup mode, as documented:
// "The object will ignore incoming MIDI values until they cross over the
// current value of the object." (docs.cycling74.com/userguide/mapping)
//
// Inlet 0 (left)  = touch, the continuously-moving physical value
// Inlet 1 (right) = external, any non-physical value-setting event
//
// Crossing is detected by a sign change between consecutive physical
// values, so a jump past the target (49 -> 51) still counts.

inlets = 2;
outlets = 1;

var target = 0.0;
var last_phys = 0.0;
var have_last = 0;
var tracking = 1;

function msg_int(v) { msg_float(v); }

function msg_float(v) {
    if (inlet === 1) {
        target = v;
        have_last = 0;
        tracking = 0;
        outlet(0, v);
        return;
    }
    if (tracking) {
        target = v;
        last_phys = v;
        outlet(0, target);
        return;
    }
    var crossed = false;
    if (!have_last) {
        if (v === target) crossed = true;
    } else {
        var was_below = last_phys < target;
        var was_above = last_phys > target;
        var now_below = v < target;
        var now_above = v > target;
        if (v === target) crossed = true;
        else if (was_below && now_above) crossed = true;
        else if (was_above && now_below) crossed = true;
    }
    last_phys = v;
    have_last = 1;
    if (crossed) {
        tracking = 1;
        target = v;
        outlet(0, target);
    }
    // else: no output
}
