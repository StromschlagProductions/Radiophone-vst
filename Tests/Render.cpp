// Render deterministic voice-only comparison data; floats, native endian, 3 x 48000 samples.
#include "RadiophoneDSP.h"
#include <fstream>
#include <xmmintrin.h>
int main (int argc, char** argv)
{
    if (argc != 2) return 1;
    _mm_setcsr (_mm_getcsr() | 0x8040);
    std::ofstream file (argv[1], std::ios::binary);
    for (int mode = 0; mode < 3; ++mode)
    {
        rph::Engine e; e.prepare (48000);
        rph::Params p; p.mode = mode; p.key = rph::kKeyDown; p.sfxDb = -80; p.interference = 0;
        std::vector<float> data (48000);
        for (int i = 0; i < 48000; ++i)
            data[(size_t) i] = (float) (0.08 * std::sin (i * 0.057) + 0.03 * std::sin (i * 0.213));
        e.process (data.data(), (int) data.size(), p);
        file.write (reinterpret_cast<const char*> (data.data()), (std::streamsize) (data.size() * sizeof (float)));
    }
    return file ? 0 : 1;
}
