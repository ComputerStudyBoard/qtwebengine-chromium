// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>

#include "base/pickle.h"
#include "build/build_config.h"
#include "content/common/cursors/webcursor.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/WebKit/public/platform/WebCursorInfo.h"

#if defined(OS_WIN)
#include <windows.h>
#endif

using blink::WebCursorInfo;

namespace content {

TEST(WebCursorTest, OKCursorSerialization) {
  WebCursor custom_cursor;
  // This is a valid custom cursor.
  base::Pickle ok_custom_pickle;
  // Type and hotspots.
  ok_custom_pickle.WriteInt(WebCursorInfo::TypeCustom);
  ok_custom_pickle.WriteInt(0);
  ok_custom_pickle.WriteInt(0);
  // X & Y
  ok_custom_pickle.WriteInt(1);
  ok_custom_pickle.WriteInt(1);
  // Scale
  ok_custom_pickle.WriteFloat(1.0);
  // Data len including enough data for a 1x1 image.
  ok_custom_pickle.WriteInt(4);
  ok_custom_pickle.WriteUInt32(0);
  // Custom Windows message.
  ok_custom_pickle.WriteUInt32(0);
  base::PickleIterator iter(ok_custom_pickle);
  EXPECT_TRUE(custom_cursor.Deserialize(&iter));
}

TEST(WebCursorTest, BrokenCursorSerialization) {
  WebCursor custom_cursor;
  // This custom cursor has not been send with enough data.
  base::Pickle short_custom_pickle;
  // Type and hotspots.
  short_custom_pickle.WriteInt(WebCursorInfo::TypeCustom);
  short_custom_pickle.WriteInt(0);
  short_custom_pickle.WriteInt(0);
  // X & Y
  short_custom_pickle.WriteInt(1);
  short_custom_pickle.WriteInt(1);
  // Scale
  short_custom_pickle.WriteFloat(1.0);
  // Data len not including enough data for a 1x1 image.
  short_custom_pickle.WriteInt(3);
  short_custom_pickle.WriteUInt32(0);
  base::PickleIterator iter(short_custom_pickle);
  EXPECT_FALSE(custom_cursor.Deserialize(&iter));

  // This custom cursor has enough data but is too big.
  base::Pickle large_custom_pickle;
  // Type and hotspots.
  large_custom_pickle.WriteInt(WebCursorInfo::TypeCustom);
  large_custom_pickle.WriteInt(0);
  large_custom_pickle.WriteInt(0);
  // X & Y
  static const int kTooBigSize = 4096 + 1;
  large_custom_pickle.WriteInt(kTooBigSize);
  large_custom_pickle.WriteInt(1);
  // Scale
  large_custom_pickle.WriteFloat(1.0);
  // Data len including enough data for a 4097x1 image.
  large_custom_pickle.WriteInt(kTooBigSize * 4);
  for (int i = 0; i < kTooBigSize; ++i)
    large_custom_pickle.WriteUInt32(0);
  iter = base::PickleIterator(large_custom_pickle);
  EXPECT_FALSE(custom_cursor.Deserialize(&iter));

  // This custom cursor uses negative lengths.
  base::Pickle neg_custom_pickle;
  // Type and hotspots.
  neg_custom_pickle.WriteInt(WebCursorInfo::TypeCustom);
  neg_custom_pickle.WriteInt(0);
  neg_custom_pickle.WriteInt(0);
  // X & Y
  neg_custom_pickle.WriteInt(-1);
  neg_custom_pickle.WriteInt(-1);
  // Scale
  neg_custom_pickle.WriteFloat(1.0);
  // Data len including enough data for a 1x1 image.
  neg_custom_pickle.WriteInt(4);
  neg_custom_pickle.WriteUInt32(0);
  // Custom Windows message.
  neg_custom_pickle.WriteUInt32(0);
  iter = base::PickleIterator(neg_custom_pickle);
  EXPECT_FALSE(custom_cursor.Deserialize(&iter));

  // This custom cursor uses zero scale.
  base::Pickle scale_zero_custom_pickle;
  // Type and hotspots.
  scale_zero_custom_pickle.WriteInt(WebCursorInfo::TypeCustom);
  scale_zero_custom_pickle.WriteInt(0);
  scale_zero_custom_pickle.WriteInt(0);
  // X & Y
  scale_zero_custom_pickle.WriteInt(1);
  scale_zero_custom_pickle.WriteInt(1);
  // Scale
  scale_zero_custom_pickle.WriteFloat(0);
  // Data len including enough data for a 1x1 image.
  scale_zero_custom_pickle.WriteInt(4);
  scale_zero_custom_pickle.WriteUInt32(0);
  // Custom Windows message.
  scale_zero_custom_pickle.WriteUInt32(0);
  iter = base::PickleIterator(scale_zero_custom_pickle);
  EXPECT_FALSE(custom_cursor.Deserialize(&iter));

  // This custom cursor uses tiny scale.
  base::Pickle scale_tiny_custom_pickle;
  // Type and hotspots.
  scale_tiny_custom_pickle.WriteInt(WebCursorInfo::TypeCustom);
  scale_tiny_custom_pickle.WriteInt(0);
  scale_tiny_custom_pickle.WriteInt(0);
  // X & Y
  scale_tiny_custom_pickle.WriteInt(1);
  scale_tiny_custom_pickle.WriteInt(1);
  // Scale
  scale_tiny_custom_pickle.WriteFloat(0.001f);
  // Data len including enough data for a 1x1 image.
  scale_tiny_custom_pickle.WriteInt(4);
  scale_tiny_custom_pickle.WriteUInt32(0);
  // Custom Windows message.
  scale_tiny_custom_pickle.WriteUInt32(0);
  iter = base::PickleIterator(scale_tiny_custom_pickle);
  EXPECT_FALSE(custom_cursor.Deserialize(&iter));
}

TEST(WebCursorTest, ClampHotspot) {
  WebCursor custom_cursor;
  // This is a valid custom cursor.
  base::Pickle ok_custom_pickle;
  // Type and hotspots.
  ok_custom_pickle.WriteInt(WebCursorInfo::TypeCustom);
  // Hotspot is invalid --- outside the bounds of the image.
  ok_custom_pickle.WriteInt(5);
  ok_custom_pickle.WriteInt(5);
  // X & Y
  ok_custom_pickle.WriteInt(2);
  ok_custom_pickle.WriteInt(2);
  // Scale
  ok_custom_pickle.WriteFloat(1.0);
  // Data len including enough data for a 2x2 image.
  ok_custom_pickle.WriteInt(4 * 4);
  for (size_t i = 0; i < 4; i++)
    ok_custom_pickle.WriteUInt32(0);
  // Custom Windows message.
  ok_custom_pickle.WriteUInt32(0);
  base::PickleIterator iter(ok_custom_pickle);
  EXPECT_TRUE(custom_cursor.Deserialize(&iter));

  // Convert to WebCursorInfo, make sure the hotspot got clamped.
  WebCursor::CursorInfo info;
  custom_cursor.GetCursorInfo(&info);
  EXPECT_EQ(gfx::Point(1, 1), info.hotspot);

  // Set hotspot to an invalid point again, pipe back through WebCursor,
  // and make sure the hotspot got clamped again.
  info.hotspot = gfx::Point(-1, -1);
  custom_cursor.InitFromCursorInfo(info);
  custom_cursor.GetCursorInfo(&info);
  EXPECT_EQ(gfx::Point(0, 0), info.hotspot);
}

TEST(WebCursorTest, EmptyImage) {
  WebCursor custom_cursor;
  base::Pickle broken_cursor_pickle;
  broken_cursor_pickle.WriteInt(WebCursorInfo::TypeCustom);
  // Hotspot is at origin
  broken_cursor_pickle.WriteInt(0);
  broken_cursor_pickle.WriteInt(0);
  // X & Y are empty
  broken_cursor_pickle.WriteInt(0);
  broken_cursor_pickle.WriteInt(0);
  // Scale
  broken_cursor_pickle.WriteFloat(1.0);
  // No data for the image since the size is 0.
  broken_cursor_pickle.WriteInt(0);
  // Custom Windows message.
  broken_cursor_pickle.WriteInt(0);

  // Make sure we can read this on all platforms; it is technicaally a valid
  // cursor.
  base::PickleIterator iter(broken_cursor_pickle);
  EXPECT_TRUE(custom_cursor.Deserialize(&iter));
}

TEST(WebCursorTest, Scale2) {
  WebCursor custom_cursor;
  // This is a valid custom cursor.
  base::Pickle ok_custom_pickle;
  // Type and hotspots.
  ok_custom_pickle.WriteInt(WebCursorInfo::TypeCustom);
  ok_custom_pickle.WriteInt(0);
  ok_custom_pickle.WriteInt(0);
  // X & Y
  ok_custom_pickle.WriteInt(1);
  ok_custom_pickle.WriteInt(1);
  // Scale - 2 image pixels per UI pixel.
  ok_custom_pickle.WriteFloat(2.0);
  // Data len including enough data for a 1x1 image.
  ok_custom_pickle.WriteInt(4);
  ok_custom_pickle.WriteUInt32(0);
  // Custom Windows message.
  ok_custom_pickle.WriteUInt32(0);
  base::PickleIterator iter(ok_custom_pickle);
  EXPECT_TRUE(custom_cursor.Deserialize(&iter));
}

TEST(WebCursorTest, AlphaConversion) {
  SkBitmap bitmap;
  SkPMColor testColor = SkPreMultiplyARGB(10, 255, 255, 255);
  bitmap.allocN32Pixels(1,1);
  SkAutoLockPixels bitmap_lock(bitmap);
  *bitmap.getAddr32(0, 0) = testColor;
  WebCursor::CursorInfo cursor_info;
  cursor_info.type = WebCursorInfo::TypeCustom;
  cursor_info.custom_image = bitmap;
  cursor_info.image_scale_factor = 1;
  WebCursor custom_cursor;

  // This round trip will convert the cursor to unpremultiplied form.
  custom_cursor.InitFromCursorInfo(cursor_info);
  custom_cursor.GetCursorInfo(&cursor_info);
  {
    SkAutoLockPixels lock(cursor_info.custom_image);
    EXPECT_EQ(kUnpremul_SkAlphaType, cursor_info.custom_image.alphaType());
    EXPECT_EQ(testColor,
              SkPreMultiplyColor(*cursor_info.custom_image.getAddr32(0,0)));
  }

  // Second round trip should not do any conversion because data is already
  // unpremultiplied.
  custom_cursor.InitFromCursorInfo(cursor_info);
  custom_cursor.GetCursorInfo(&cursor_info);
  {
    SkAutoLockPixels lock(cursor_info.custom_image);
    EXPECT_EQ(kUnpremul_SkAlphaType, cursor_info.custom_image.alphaType());
    EXPECT_EQ(testColor,
              SkPreMultiplyColor(*cursor_info.custom_image.getAddr32(0,0)));
  }

#if defined(OS_MACOSX)
  // On MacOS, test roundtrip through NSCursor conversion.
  WebCursor custom_cursor_copy;
  custom_cursor_copy.InitFromNSCursor(custom_cursor.GetNativeCursor());
  custom_cursor_copy.GetCursorInfo(&cursor_info);
  {
    SkAutoLockPixels lock(cursor_info.custom_image);
    EXPECT_EQ(kUnpremul_SkAlphaType, cursor_info.custom_image.alphaType());
    EXPECT_EQ(testColor,
              SkPreMultiplyColor(*cursor_info.custom_image.getAddr32(0,0)));
  }
#endif
}

#if defined(USE_AURA)
TEST(WebCursorTest, CursorScaleFactor) {
  gfx::Display display;
  display.set_device_scale_factor(80.2f);

  WebCursor::CursorInfo info;
  info.image_scale_factor = 2.0f;

  WebCursor cursor;
  cursor.InitFromCursorInfo(info);
  cursor.SetDisplayInfo(display);

  EXPECT_EQ(40.1f, cursor.GetCursorScaleFactor());
}
#endif

#if defined(OS_WIN)
void ScaleCursor(float scaleFactor, int hotspotX, int hotspotY) {
  gfx::Display display;
  display.set_device_scale_factor(scaleFactor);

  WebCursor::CursorInfo info;
  info.type = WebCursorInfo::TypeCustom;
  info.hotspot = gfx::Point(hotspotX, hotspotY);

  info.custom_image = SkBitmap();
  info.custom_image.allocN32Pixels(10, 10);
  info.custom_image.eraseColor(0);

  WebCursor cursor;
  cursor.SetDisplayInfo(display);
  cursor.InitFromCursorInfo(info);

  HCURSOR windowsCursorHandle = cursor.GetPlatformCursor();
  EXPECT_NE(nullptr, windowsCursorHandle);
  ICONINFO windowsIconInfo;
  EXPECT_TRUE(GetIconInfo(windowsCursorHandle, &windowsIconInfo));
  EXPECT_FALSE(windowsIconInfo.fIcon);
  EXPECT_EQ(static_cast<DWORD>(scaleFactor * hotspotX),
            windowsIconInfo.xHotspot);
  EXPECT_EQ(static_cast<DWORD>(scaleFactor * hotspotY),
            windowsIconInfo.yHotspot);
}

TEST(WebCursorTest, WindowsCursorScaledAtHiDpi) {
  ScaleCursor(2.0f, 4, 6);
  ScaleCursor(1.5f, 2, 8);
  ScaleCursor(1.25f, 3, 7);
}
#endif

}  // namespace content
