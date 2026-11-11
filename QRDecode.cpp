#include <cstdio>
#include <cstring>
#include <memory>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_MAX_DIMENSIONS 4096
#include "vendor/stb_image.h"
#include "vendor/quirc/lib/quirc.h"
int main(int argc, char **argv) {
    if (argc != 2) return 1;
    int w=0, h=0, channels=0;
    if (!stbi_info(argv[1], &w, &h, &channels) || w<=0 || h<=0 ||
        w>4096 || h>4096 || static_cast<long long>(w)*h>16000000) return 1;
    std::unique_ptr<unsigned char, decltype(&stbi_image_free)> pixels(
        stbi_load(argv[1], &w, &h, &channels, 1), stbi_image_free);
    if (!pixels) return 1;
    std::unique_ptr<quirc, decltype(&quirc_destroy)> scanner(quirc_new(), quirc_destroy);
    if (!scanner || quirc_resize(scanner.get(), w, h)<0) return 1;
    unsigned char *image = quirc_begin(scanner.get(), nullptr, nullptr);
    memcpy(image, pixels.get(), static_cast<size_t>(w)*h);
    quirc_end(scanner.get());
    for (int i=0; i<quirc_count(scanner.get()); ++i) {
        quirc_code code{}; quirc_data data{};
        quirc_extract(scanner.get(), i, &code);
        auto error=quirc_decode(&code, &data);
        if (error) { quirc_flip(&code); error=quirc_decode(&code, &data); }
        if (!error && data.payload_len>0) {
            return fwrite(data.payload, 1, data.payload_len, stdout)==static_cast<size_t>(data.payload_len) ? 0 : 1;
        }
    }
    return 1;
}
