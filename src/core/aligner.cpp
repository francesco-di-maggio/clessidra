#include "aligner.h"

#include <algorithm>
#include <cmath>

namespace clessidra
{
double Aligner::Align(const Buffer& b,
                      double        from,
                      float         from_rate,
                      double        target,
                      float         rate,
                      size_t        reach)
{
    const size_t lags = std::min(reach, kReach) / kStride;
    if(lags == 0 || b.Length() == 0)
        return target;

    uint32_t     source[kWords];
    uint32_t     candidates[kTargetWords];
    const double step = double(kStride) * double(rate);
    Signs(b, from, double(kStride) * double(from_rate), kWords, source);
    Signs(b, target - double(lags) * step, step, kTargetWords, candidates);

    // From the centre outward: on a tie the nearest stays
    size_t   best       = lags;
    uint32_t best_score = 0;
    for(size_t n = 0; n <= 2 * lags; n++)
    {
        const size_t lag   = n % 2 ? lags + (n + 1) / 2 : lags - n / 2;
        const size_t word  = lag >> 5;
        const size_t bit   = lag & 31;
        uint32_t     score = 0;
        for(size_t i = 0; i < kWords; i++)
        {
            uint32_t bits = candidates[word + i] << bit;
            if(bit)
                bits |= candidates[word + i + 1] >> (32 - bit);
            score += uint32_t(__builtin_popcount(~(source[i] ^ bits)));
        }
        if(score > best_score)
        {
            best_score = score;
            best       = lag;
        }
    }
    return target + (double(best) - double(lags)) * step;
}

void Aligner::Signs(const Buffer& b,
                    double        start,
                    double        step,
                    size_t        words,
                    uint32_t*     out)
{
    const float* mem = b.Samples();
    const double len = double(b.Length());
    double       pos = start - len * std::floor(start / len);
    for(size_t w = 0; w < words; w++)
    {
        uint32_t bits = 0;
        for(size_t k = 0; k < 32; k++)
        {
            const size_t i = std::min(size_t(pos), b.Length() - 1);
            bits           = (bits << 1) | (mem[i] > 0.0f ? 1u : 0u);
            pos += step;
            if(pos >= len)
                pos -= len;
            else if(pos < 0.0)
                pos += len;
        }
        out[w] = bits;
    }
}

} // namespace clessidra
