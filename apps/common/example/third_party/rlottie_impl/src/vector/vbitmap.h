/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd. All rights reserved.

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef VBITMAP_H
#define VBITMAP_H

#include "vrect.h"
#include <vsharedptr.h>

V_BEGIN_NAMESPACE

class VBitmap
{
public:
    enum class Format : uint8_t {
        Invalid,
        Alpha8,
        ARGB32,
        ARGB32_Premultiplied
    };

    VBitmap() = default;
    VBitmap(size_t w, size_t h, VBitmap::Format format);
    VBitmap(uint8_t *data, size_t w, size_t h, size_t bytesPerLine,
            VBitmap::Format format);
    void reset(uint8_t *data, size_t w, size_t h, size_t stride,
               VBitmap::Format format);
    void reset(size_t w, size_t h, VBitmap::Format format = Format::ARGB32_Premultiplied);
    size_t          stride() const;
    size_t          width() const;
    size_t          height() const;
    size_t          depth() const;
    VBitmap::Format format() const;
    bool            valid() const;
    uint8_t        *data();
    uint8_t        *data() const;
    VRect           rect() const;
    VSize           size() const;
    void            fill(uint32_t pixel);
    void    updateLuma();
private:
    struct Impl {
        std::unique_ptr<uint8_t[]> mOwnData{nullptr};
        uint8_t                   *mRoData{nullptr};
        uint32_t                   mWidth{0};
        uint32_t                   mHeight{0};
        uint32_t                   mStride{0};
        uint8_t                    mDepth{0};
        VBitmap::Format mFormat{VBitmap::Format::Invalid};

        explicit Impl(size_t width, size_t height, VBitmap::Format format)
        {
            reset(width, height, format);
        }
        explicit Impl(uint8_t *data, size_t w, size_t h, size_t bytesPerLine,
                      VBitmap::Format format)
        {
            reset(data, w, h, bytesPerLine, format);
        }
        VRect   rect() const
        {
            return VRect(0, 0, mWidth, mHeight);
        }
        VSize   size() const
        {
            return VSize(mWidth, mHeight);
        }
        size_t  stride() const
        {
            return mStride;
        }
        size_t  width() const
        {
            return mWidth;
        }
        size_t  height() const
        {
            return mHeight;
        }
        uint8_t        *data()
        {
            return mRoData ? mRoData : mOwnData.get();
        }
        VBitmap::Format format() const
        {
            return mFormat;
        }
        void reset(uint8_t *, size_t, size_t, size_t, VBitmap::Format);
        void reset(size_t, size_t, VBitmap::Format);
        static uint8_t depth(VBitmap::Format format);
        void fill(uint32_t);
        void updateLuma();
    };

    rc_ptr<Impl> mImpl;
};

V_END_NAMESPACE

#endif  // VBITMAP_H
