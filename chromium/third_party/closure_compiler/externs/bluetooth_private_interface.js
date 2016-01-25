// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file was generated by:
//   ./tools/json_schema_compiler/compiler.py.

/** @fileoverview Interface for bluetoothPrivate that can be overriden. */

assertNotReached('Interface file for Closure Compiler should not be executed.');

/** @interface */
function BluetoothPrivate() {}

BluetoothPrivate.prototype = {
  /**
   * Changes the state of the Bluetooth adapter.
   * @param {!chrome.bluetoothPrivate.NewAdapterState} adapterState
   * @param {function():void=} callback
   * @see https://developer.chrome.com/extensions/bluetoothPrivate#method-setAdapterState
   */
  setAdapterState: assertNotReached,

  /**
   * @param {!chrome.bluetoothPrivate.SetPairingResponseOptions} options
   * @param {function():void=} callback
   * @see https://developer.chrome.com/extensions/bluetoothPrivate#method-setPairingResponse
   */
  setPairingResponse: assertNotReached,

  /**
   * Tears down all connections to the given device.
   * @param {string} deviceAddress
   * @param {function():void=} callback
   * @see https://developer.chrome.com/extensions/bluetoothPrivate#method-disconnectAll
   */
  disconnectAll: assertNotReached,

  /**
   * Forgets the given device.
   * @param {string} deviceAddress
   * @param {function():void=} callback
   * @see https://developer.chrome.com/extensions/bluetoothPrivate#method-forgetDevice
   */
  forgetDevice: assertNotReached,

  /**
   * Set or clear discovery filter.
   * @param {!chrome.bluetoothPrivate.DiscoveryFilter} discoveryFilter
   * @param {function():void=} callback
   * @see https://developer.chrome.com/extensions/bluetoothPrivate#method-setDiscoveryFilter
   */
  setDiscoveryFilter: assertNotReached,

  /**
   * Connects to the given device. This will only throw an error if the device
   * address is invalid or the device is already connected. Otherwise this will
   * succeed and invoke |callback| with ConnectResultType.
   * @param {string} deviceAddress
   * @param {function(!chrome.bluetoothPrivate.ConnectResultType):void=}
   *     callback
   * @see https://developer.chrome.com/extensions/bluetoothPrivate#method-connect
   */
  connect: assertNotReached,

  /**
   * Pairs the given device.
   * @param {string} deviceAddress
   * @param {function():void=} callback
   * @see https://developer.chrome.com/extensions/bluetoothPrivate#method-pair
   */
  pair: assertNotReached,

  /**
   * Fired when a pairing event occurs.
   * @type {!ChromeEvent}
   * @see https://developer.chrome.com/extensions/bluetoothPrivate#event-onPairing
   */
  onPairing: new ChromeEvent(),
};
