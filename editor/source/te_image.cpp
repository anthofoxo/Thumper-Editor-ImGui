#include "te_image.hpp"

#include <stb_image.h>

namespace tcle {
    Image::Image(char const* aPath) {
        mPixels = stbi_load(aPath, &mWidth, &mHeight, nullptr, 4);
    }

    Image::~Image() noexcept {
        if (mPixels) {
            stbi_image_free(mPixels);
        }
    }

    Image& Image::operator=(Image&& other) noexcept {
        std::swap(mWidth, other.mWidth);
        std::swap(mHeight, other.mHeight);
        std::swap(mPixels, other.mPixels);
        return *this;
    }
}