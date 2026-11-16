#include "RadiophoneDSP.h"
#include <ctime>
#include <iostream>
#include <xmmintrin.h>
int main()
{
    _mm_setcsr (_mm_getcsr() | 0x8040);
    for (int scenario = 0; scenario < 5; ++scenario)
    {
        rph::Engine e; e.prepare (48000); rph::Params p;
        p.mode = scenario == 3 ? rph::kRadio : rph::kFunk;
        p.key = scenario == 0 ? rph::kKeyUp : rph::kKeyDown;
        rph::Samples s; s.rate = 48000; s.tune.assign (96000, 0.01f); e.setSamples (s);
        float data[256] {}; e.process (data, 256, p);
        volatile float checksum = 0;
        const auto start = std::clock();
        for (int block = 0; block < 10000; ++block)
        {
            if (scenario == 4) p.mode = block % 3;
            if (scenario == 3 && block % 300 == 0) ++p.tuneCount;
            for (int i = 0; i < 256; ++i) data[i] = scenario < 2 ? 0.0f : 0.1f * std::sin ((float) (block * 256 + i) * 0.05f);
            e.process (data, 256, p); checksum += data[100];
        }
        const auto ms = 1000.0 * (double) (std::clock() - start) / CLOCKS_PER_SEC;
        std::cout << scenario << ',' << ms << ',' << checksum << '\n';
    }
}
