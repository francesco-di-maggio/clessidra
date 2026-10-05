// Glide-then-lock: moves toward the physical value, then switches to
// 1:1 tracking once the gap is below threshold.
//
// Inlet 0 (left)  = touch, the continuously-moving physical value
// Inlet 1 (right) = external, any non-physical value-setting event
//
// rate      = fraction of the remaining gap closed per touch update (0-1)
// max_step  = largest change allowed per update, caps big jumps
// threshold = gap size below which it locks to direct 1:1 tracking

inlets = 2;
outlets = 1;

var rate = 0.15;
var max_step = 5.0;
var threshold = 0.5;

var current = 0.0;
var tracking = 1;

function msg_int(v) { msg_float(v); }

function msg_float(v) {
    if (inlet === 1) {
        current = v;
        tracking = 0;
        outlet(0, current);
        return;
    }
    if (tracking) {
        current = v;
        outlet(0, current);
        return;
    }
    var delta = (v - current) * rate;
    if (Math.abs(delta) > max_step) {
        delta = (delta > 0) ? max_step : -max_step;
    }
    current += delta;
    if (Math.abs(v - current) < threshold) {
        tracking = 1;
        current = v;
    }
    outlet(0, current);
}
