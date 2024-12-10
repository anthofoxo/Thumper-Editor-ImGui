#pragma once

#include <utility>

namespace tcle {
    class Image final {
    public:
        constexpr Image() = default;
        Image(char const* aPath);
        ~Image() noexcept;

        Image(Image const&) = delete;
        Image& operator=(Image const&) = delete;

        inline Image(Image&& aOther) noexcept { *this = std::move(aOther); }
        Image& operator=(Image&& aOther) noexcept;

        inline int width() const { return mWidth; }
        inline int height() const { return mHeight; }
        inline unsigned char* pixels() const { return mPixels; }
        inline explicit operator bool() const { return mPixels; }
    private:
        int mWidth = 0, mHeight = 0;
        unsigned char* mPixels = nullptr;
    };
}