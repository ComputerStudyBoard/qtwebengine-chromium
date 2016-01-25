# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'includes': [
    'openh264.gypi',
    'openh264_args.gypi',
  ],
  # Settings shared by all openh264 targets.
  'target_defaults': {
    'variables': {
      'conditions': [
        ['OS!="win"', {
          # GCC/clang flags
          'openh264_cflags_add': [
            '-Wno-format',
            '-Wno-header-hygiene',
            '-Wno-unused-value',
          ],
          'openh264_cflags_remove': [
            '-Wall',
            '-Wheader-hygiene',
          ],
        },{
          # Windows uses 'msvs_disabled_warnings' instead, for MSVC flags.
          'openh264_cflags_add': [],
          'openh264_cflags_remove': [],
        }],
      ],
    },
    'cflags': [ '<@(openh264_cflags_add)' ],
    'cflags!': [ '<@(openh264_cflags_remove)' ],
    'xcode_settings': {
      'WARNING_CFLAGS': [ '<@(openh264_cflags_add)' ],
      'WARNING_CFLAGS!': [ '<@(openh264_cflags_remove)' ],
    },
    'msvs_disabled_warnings': [
      4324,  # structure was padded
      4245,  # signed/unsigned mismatch
      4701,  # uninitialized variable used
      4702,  # unreachable code
    ],

    # Platform-specific defines.
    'conditions': [
      ['OS=="android"', {
        'defines': [
          # Android NDK is necessary for its cpufeatures and this define is
          # what OpenH264 code uses to check if it should be used.
          'ANDROID_NDK',
        ],
      }],
    ],
  },
  'targets': [
    {
      'target_name': 'openh264_common',
      'type': 'static_library',
      'conditions': [
        ['OS=="android"', {
          'dependencies': [
            # Defines "android_get/setCpu..." functions. The original
            # OpenH264 build files replaces these using macros for
            # "wels_..." versions of the same functions. We do not have
            # access to these and use the <cpu-features.h> ones instead.
            '<(DEPTH)/build/android/ndk.gyp:cpu_features',
          ],
        }],
      ],
      'include_dirs+': [ '<@(openh264_common_includes)' ],
      'sources': [ '<@(openh264_common_sources)' ],
    },
    {
      'target_name': 'openh264_processing',
      'type': 'static_library',
      'dependencies': [
        'openh264_common',
      ],
      'include_dirs+': [ '<@(openh264_processing_includes)' ],
      'sources': [ '<@(openh264_processing_sources)' ],
    },
    {
      'target_name': 'openh264_encoder',
      'type': 'static_library',
      'dependencies': [
        'openh264_common',
        'openh264_processing',
      ],
      'include_dirs+': [ '<@(openh264_encoder_includes)' ],
      'sources': [ '<@(openh264_encoder_sources)' ],
    },
  ],
}
