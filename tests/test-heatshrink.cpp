#include <string>
#include <iostream>
extern "C" {
#include "heatshrink/heatshrink_encoder.h"
#include "heatshrink/heatshrink_decoder.h"
}

#if HEATSHRINK_DYNAMIC_ALLOC
#error HEATSHRINK_DYNAMIC_ALLOC must be false for static allocation test suite.
#endif

static size_t testEncode(
    char *output,
    size_t outputSize,
    const char *input,
    size_t inputSize
) {
    heatshrink_encoder hse;
    heatshrink_encoder_reset(&hse);

    size_t count = 0;
    uint32_t sunk = 0;
    size_t polled = 0;
    while (sunk < inputSize) {
        sunk += count;
        HSE_poll_res pres;
        do {
            pres = heatshrink_encoder_poll(&hse, (uint8_t *) output + polled, outputSize - polled, &count);
            polled += count;
        } while (pres == HSER_POLL_MORE);
        if (polled >= outputSize) {
            return 0;
        }
    }
    return polled;
}

int main() {
    std::string s("# heatshrink\n"
        "\n"
        "A data compression/decompression library for embedded/real-time systems.\n"
        "\n"
        "\n"
        "## Key Features:\n"
        "\n"
        "- **Low memory usage (as low as 50 bytes)**\n"
        "    It is useful for some cases with less than 50 bytes, and useful\n"
        "    for many general cases with < 300 bytes.\n"
        "- **Incremental, bounded CPU use**\n"
        "    You can chew on input data in arbitrarily tiny bites.\n"
        "    This is a useful property in hard real-time environments.\n"
        "- **Can use either static or dynamic memory allocation**\n"
        "    The library doesn't impose any constraints on memory management.\n"
        "- **ISC license**\n"
        "    You can use it freely, even for commercial purposes.");
    size_t compressedBufferSize = s.size();
    char *compressedBuffer = (char *) malloc(compressedBufferSize);
    memset(compressedBuffer, 0, compressedBufferSize);
    size_t compressedSize = testEncode(compressedBuffer, compressedBufferSize, s.c_str(), s.size());
    free(compressedBuffer);

    std::cout << s.size() << "->" << compressedSize << std::endl;
    // testDecode();
    return 0;
}
