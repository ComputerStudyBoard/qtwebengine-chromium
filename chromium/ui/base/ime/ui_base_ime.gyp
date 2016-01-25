# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,
  },
  'targets': [
    {
      # GN version: //ui/base/ime
      'target_name': 'ui_base_ime',
      'type': '<(component)',
      'dependencies': [
        '../../../base/base.gyp:base',
        '../../../base/base.gyp:base_i18n',
        '../../../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '../../../skia/skia.gyp:skia',
        '../../../third_party/icu/icu.gyp:icui18n',
        '../../../third_party/icu/icu.gyp:icuuc',
        '../../../url/url.gyp:url_lib',
        '../../events/events.gyp:dom_keycode_converter',
        '../../events/events.gyp:events',
        '../../events/events.gyp:events_base',
        '../../gfx/gfx.gyp:gfx',
        '../../gfx/gfx.gyp:gfx_geometry',
        '../ui_base.gyp:ui_base',
      ],
      'defines': [
        'UI_BASE_IME_IMPLEMENTATION',
      ],
      'sources' : [
        'candidate_window.cc',
        'candidate_window.h',
        'chromeos/character_composer.cc',
        'chromeos/character_composer.h',
        'chromeos/component_extension_ime_manager.cc',
        'chromeos/component_extension_ime_manager.h',
        'chromeos/extension_ime_util.cc',
        'chromeos/extension_ime_util.h',
        'chromeos/fake_ime_keyboard.cc',
        'chromeos/fake_ime_keyboard.h',
        'chromeos/fake_input_method_delegate.cc',
        'chromeos/fake_input_method_delegate.h',
        'chromeos/ime_candidate_window_handler_interface.h',
        'chromeos/ime_keyboard.cc',
        'chromeos/ime_keyboard.h',
        'chromeos/ime_keyboard_ozone.cc',
        'chromeos/ime_keyboard_ozone.h',
        'chromeos/ime_keyboard_x11.cc',
        'chromeos/ime_keyboard_x11.h',
        'chromeos/ime_keymap.cc',
        'chromeos/ime_keymap.h',
        'chromeos/input_method_delegate.h',
        'chromeos/input_method_descriptor.cc',
        'chromeos/input_method_descriptor.h',
        'chromeos/input_method_manager.cc',
        'chromeos/input_method_manager.h',
        'chromeos/input_method_whitelist.cc',
        'chromeos/input_method_whitelist.h',
        'chromeos/mock_component_extension_ime_manager_delegate.cc',
        'chromeos/mock_component_extension_ime_manager_delegate.h',
        'chromeos/mock_ime_candidate_window_handler.cc',
        'chromeos/mock_ime_candidate_window_handler.h',
        'chromeos/mock_ime_engine_handler.cc',
        'chromeos/mock_ime_engine_handler.h',
        'chromeos/mock_ime_input_context_handler.cc',
        'chromeos/mock_ime_input_context_handler.h',
        'composition_text.cc',
        'composition_text.h',
        'composition_text_util_pango.cc',
        'composition_text_util_pango.h',
        'composition_underline.h',
        'ime_bridge.cc',
        'ime_bridge.h',
        'ime_engine_handler_interface.cc',
        'ime_engine_handler_interface.h',
        'ime_engine_observer.h',
        'ime_input_context_handler_interface.h',
        'infolist_entry.cc',
        'infolist_entry.h',
        'input_method.h',
        'input_method_auralinux.cc',
        'input_method_auralinux.h',
        'input_method_base.cc',
        'input_method_base.h',
        'input_method_chromeos.cc',
        'input_method_chromeos.h',
        'input_method_delegate.h',
        'input_method_factory.cc',
        'input_method_factory.h',
        'input_method_initializer.cc',
        'input_method_initializer.h',
        'input_method_mac.h',
        'input_method_mac.mm',
        'input_method_minimal.cc',
        'input_method_minimal.h',
        'input_method_observer.h',
        'input_method_win.cc',
        'input_method_win.h',
        'linux/fake_input_method_context.cc',
        'linux/fake_input_method_context.h',
        'linux/fake_input_method_context_factory.cc',
        'linux/fake_input_method_context_factory.h',
        'linux/linux_input_method_context.h',
        'linux/linux_input_method_context_factory.cc',
        'linux/linux_input_method_context_factory.h',
        'mock_input_method.cc',
        'mock_input_method.h',
        'remote_input_method_delegate_win.h',
        'remote_input_method_win.cc',
        'remote_input_method_win.h',
        'text_input_client.cc',
        'text_input_client.h',
        'text_input_type.h',
        'ui_base_ime_export.h',
        'win/imm32_manager.cc',
        'win/imm32_manager.h',
        'win/tsf_input_scope.cc',
        'win/tsf_input_scope.h',
      ],
      'conditions': [
        ['use_ozone==1', {
          'dependencies': [
            '../../events/ozone/events_ozone.gyp:events_ozone_layout',
            '../../ozone/ozone.gyp:ozone',
          ],
        }],
        ['use_pango==1', {
          'dependencies': [
            '../../../build/linux/system.gyp:pangocairo',
          ],
        }],
        ['OS=="win"', {
          # TODO(jschuh): C4267: http://crbug.com/167187 size_t -> int
          # C4324 is structure was padded due to __declspec(align()), which is
          # uninteresting.
          'msvs_disabled_warnings': [ 4267, 4324 ],
          'link_settings': {
            'libraries': [
              '-limm32.lib',
            ],
          },
        }],
        ['use_x11==1', {
          'dependencies': [
            '../../../build/linux/system.gyp:x11',
            '../../gfx/x/gfx_x11.gyp:gfx_x11',
          ],
        }],
        ['toolkit_views==0 and use_aura==0', {
          'sources!': [
            'input_method_factory.cc',
            'input_method_factory.h',
            'input_method_minimal.cc',
            'input_method_minimal.h',
          ],
        }],
        ['chromeos==1', {
          'dependencies': [
            '../../../chromeos/chromeos.gyp:chromeos',
          ],
        }],
        ['use_aura==0 or (desktop_linux==0 and use_ozone==0)', {
          'sources!': [
            'input_method_auralinux.cc',
            'input_method_auralinux.h',
            'linux/fake_input_method_context.cc',
            'linux/fake_input_method_context.h',
            'linux/fake_input_method_context_factory.cc',
            'linux/fake_input_method_context_factory.h',
            'linux/linux_input_method_context.h',
            'linux/linux_input_method_context_factory.cc',
            'linux/linux_input_method_context_factory.h',
          ],
        }],
        ['use_x11==0', {
          'sources!': [
            'composition_text_util_pango.cc',
            'composition_text_util_pango.h',
          ],
        }],
      ],
    },
  ],
}
