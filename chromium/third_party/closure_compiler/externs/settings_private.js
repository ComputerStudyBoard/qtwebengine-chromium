// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file was generated by:
//   ./tools/json_schema_compiler/compiler.py.
// NOTE: The format of types has changed. 'FooType' is now
//   'chrome.settingsPrivate.FooType'.
// Please run the closure compiler before committing changes.
// See https://code.google.com/p/chromium/wiki/ClosureCompilation.

/** @fileoverview Externs generated from namespace: settingsPrivate */

/**
 * @const
 */
chrome.settingsPrivate = {};

/**
 * @enum {string}
 * @see https://developer.chrome.com/extensions/settingsPrivate#type-PrefType
 */
chrome.settingsPrivate.PrefType = {
  BOOLEAN: 'BOOLEAN',
  NUMBER: 'NUMBER',
  STRING: 'STRING',
  URL: 'URL',
  LIST: 'LIST',
};

/**
 * @enum {string}
 * @see https://developer.chrome.com/extensions/settingsPrivate#type-PolicySource
 */
chrome.settingsPrivate.PolicySource = {
  DEVICE_POLICY: 'DEVICE_POLICY',
  USER_POLICY: 'USER_POLICY',
  OWNER: 'OWNER',
  PRIMARY_USER: 'PRIMARY_USER',
  EXTENSION: 'EXTENSION',
};

/**
 * @enum {string}
 * @see https://developer.chrome.com/extensions/settingsPrivate#type-PolicyEnforcement
 */
chrome.settingsPrivate.PolicyEnforcement = {
  ENFORCED: 'ENFORCED',
  RECOMMENDED: 'RECOMMENDED',
};

/**
 * @typedef {{
 *   key: string,
 *   type: !chrome.settingsPrivate.PrefType,
 *   value: *,
 *   policySource: (!chrome.settingsPrivate.PolicySource|undefined),
 *   policySourceName: (string|undefined),
 *   policyEnforcement: (!chrome.settingsPrivate.PolicyEnforcement|undefined),
 *   recommendedValue: (*|undefined),
 *   extensionId: (string|undefined),
 *   readOnly: (boolean|undefined)
 * }}
 * @see https://developer.chrome.com/extensions/settingsPrivate#type-PrefObject
 */
chrome.settingsPrivate.PrefObject;

/**
 * Sets a settings value.
 * @param {string} name The name of the pref.
 * @param {*} value The new value of the pref.
 * @param {string} pageId The user metrics identifier or null.
 * @param {function(boolean):void} callback The callback for whether the pref
 *     was set or not.
 * @see https://developer.chrome.com/extensions/settingsPrivate#method-setPref
 */
chrome.settingsPrivate.setPref = function(name, value, pageId, callback) {};

/**
 * Gets an array of all the prefs.
 * @param {function(!Array<!chrome.settingsPrivate.PrefObject>):void} callback
 * @see https://developer.chrome.com/extensions/settingsPrivate#method-getAllPrefs
 */
chrome.settingsPrivate.getAllPrefs = function(callback) {};

/**
 * Gets the value of a specific pref.
 * @param {string} name
 * @param {function(!chrome.settingsPrivate.PrefObject):void} callback
 * @see https://developer.chrome.com/extensions/settingsPrivate#method-getPref
 */
chrome.settingsPrivate.getPref = function(name, callback) {};

/**
 * Gets the page zoom factor as an integer percentage.
 * @param {function(number):void} callback
 * @see https://developer.chrome.com/extensions/settingsPrivate#method-getDefaultZoomPercent
 */
chrome.settingsPrivate.getDefaultZoomPercent = function(callback) {};

/**
 * Sets the page zoom factor from a zoom percentage.
 * @param {number} percent
 * @param {function(boolean):void=} callback
 * @see https://developer.chrome.com/extensions/settingsPrivate#method-setDefaultZoomPercent
 */
chrome.settingsPrivate.setDefaultZoomPercent = function(percent, callback) {};

/**
 * <p>Fired when a set of prefs has changed.</p><p>|prefs| The prefs that
 * changed.</p>
 * @type {!ChromeEvent}
 * @see https://developer.chrome.com/extensions/settingsPrivate#event-onPrefsChanged
 */
chrome.settingsPrivate.onPrefsChanged;
