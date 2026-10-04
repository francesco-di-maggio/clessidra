// Max's official "Scale" mode, reimplemented from the documented mechanics:
// "Incoming MIDI values will scale the parameter value between its current
// value and the minimum or maximum value, until the object reaches its
// minimum or maximum." (docs.cycling74.com/userguide/mapping)
//
// Inlet 0 (left)  = touch, the continuously-moving physical value
// Inlet 1 (right) = external, any non-physical value-setting event
//
// Needs the real range of both sides - set these to match your setup:
var phys_min = 0.0;
var phys_max = 127.0;
var param_min = 0.0;
var param_max = 127.0;

inlets = 2;
outlets = 1;

var target = 0.0;
var phys_start = 0.0;
var last_phys = 0.0;
var tracking = 1;

function scale_direct(v) {
    return param_min + (v - phys_min) / (phys_max - phys_min) * (param_max - param_min);
}

function msg_int(v) { msg_float(v); }

function msg_float(v) {
    if (inlet === 1) {
        phys_start = last_phys;
        target = v;
        tracking = 0;
        outlet(0, v);
        return;
    }
    last_phys = v;
    if (tracking) {
        outlet(0, scale_direct(v));
        return;
    }
    if (v >= phys_max || v <= phys_min) {
        tracking = 1;
        outlet(0, scale_direct(v));
        return;
    }
    var output;
    if (v > phys_start) {
        var span = phys_max - phys_start;
        var progress = (span !== 0) ? (v - phys_start) / span : 1;
        output = target + progress * (param_max - target);
    } else if (v < phys_start) {
        var span2 = phys_start - phys_min;
        var progress2 = (span2 !== 0) ? (phys_start - v) / span2 : 1;
        output = target + progress2 * (param_min - target);
    } else {
        output = target;
    }
    outlet(0, output);
}
