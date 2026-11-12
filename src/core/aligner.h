#pragma once

#include <cstddef>
#include <cstdint>
#include "buffer.h"

namespace clessidra
{
// Where a grain starts so that it continues the grain before in phase, as
// Clouds' WSOLA player does (clouds/dsp/wsola_sample_player.h,
// correlator.cc): the sign bits of what the grain before plays next, against
// those around the new grain's start, the start moved to the best match.
// Overlapping grains then add up instead of combing.
class Aligner
{
  public:
    // How far a start may move either way, samples of output: 5.3 ms, a
    // period of 94 Hz
    static constexpr size_t kReach = 256;

    // The start, within reach of target, read at rate, best continuing what
    // from, read at from_rate, plays. reach: at most kReach. The match nearest
    // target wins a tie.
    static double Align(const Buffer& b,
                        double        from,
                        float         from_rate,
                        double        target,
                        float         rate,
                        size_t        reach);

  private:
    // Every kStride samples, kWords words of 32: 512 samples compared
    static constexpr size_t kStride      = 2;
    static constexpr size_t kWords       = 8;
    static constexpr size_t kLags        = 2 * kReach / kStride;
    static constexpr size_t kTargetWords = kWords + kLags / 32 + 1;

    // Bit 31 of word 0 first: each sample's sign, positive set
    static void Signs(const Buffer& b,
                      double        start,
                      double        step,
                      size_t        words,
                      uint32_t*     out);
};

} // namespace clessidra
