// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

WebInspector.OverridesUI = {}

/**
 * @constructor
 * @param {!Element} rotateButton
 * @param {?function(!WebInspector.EmulatedDevice, !WebInspector.EmulatedDevice.Mode)} callback
 */
WebInspector.DeviceSelect = function(rotateButton, callback)
{
    this._callback = callback;
    this._rotateButton = rotateButton;
    this.element = createElement("p");

    this._deviceSelectElement = this.element.createChild("select", "device-select");
    this._deviceSelectElement.addEventListener("change", this._deviceSelected.bind(this), false);

    var container = this.element.createChild("div", "mode-container");
    container.appendChild(this._rotateButton);
    this._rotateButton.addEventListener("click", this._rotateButtonClicked.bind(this), false);
    this._rotateButton.title = WebInspector.UIString("Change orientation");

    var modeSelectContainer = container.createChild("span", "mode-select");
    this._modeSelectElement = modeSelectContainer.createChild("select");
    this._modeSelectElement.addEventListener("change", this._modeSelected.bind(this), false);
    this._modeLabelElement = modeSelectContainer.createChild("label");
    this._modeLabelElement.addEventListener("click", this._rotateButtonClicked.bind(this), false);
    this._modeLabelElement.title = WebInspector.UIString("Change orientation");

    this._emulatedSettingChangedMuted = false;
    this._lastOrientation = null;

    WebInspector.overridesSupport.settings.emulateResolution.addChangeListener(this._emulatedSettingChanged, this);
    WebInspector.overridesSupport.settings.deviceWidth.addChangeListener(this._emulatedSettingChanged, this);
    WebInspector.overridesSupport.settings.deviceHeight.addChangeListener(this._emulatedSettingChanged, this);
    WebInspector.overridesSupport.settings.deviceScaleFactor.addChangeListener(this._emulatedSettingChanged, this);
    WebInspector.overridesSupport.settings.emulateMobile.addChangeListener(this._emulatedSettingChanged, this);
    WebInspector.overridesSupport.settings.emulateTouch.addChangeListener(this._emulatedSettingChanged, this);
    WebInspector.overridesSupport.settings.userAgent.addChangeListener(this._emulatedSettingChanged, this);

    WebInspector.emulatedDevicesList.addEventListener(WebInspector.EmulatedDevicesList.Events.CustomDevicesUpdated, this._deviceListChanged, this);
    WebInspector.emulatedDevicesList.addEventListener(WebInspector.EmulatedDevicesList.Events.StandardDevicesUpdated, this._deviceListChanged, this);
    this._deviceListChanged();
}

WebInspector.DeviceSelect.prototype = {
    _deviceListChanged: function()
    {
        this._deviceSelectElement.removeChildren();

        var selectDeviceOption = new Option(WebInspector.UIString("<Select model>"), WebInspector.UIString("<Select model>"));
        selectDeviceOption.device = null;
        selectDeviceOption.lastSelectedIndex = 0;
        selectDeviceOption.disabled = true;
        this._deviceSelectElement.appendChild(selectDeviceOption);

        this._addDeviceGroup(WebInspector.UIString("Custom"), WebInspector.emulatedDevicesList.custom());
        this._addDeviceGroup(WebInspector.UIString("Devices"), WebInspector.emulatedDevicesList.standard());
        this._emulatedSettingChanged();
    },

    /**
     * @param {string} name
     * @param {!Array.<!WebInspector.EmulatedDevice>} devices
     */
    _addDeviceGroup: function(name, devices)
    {
        devices = devices.filter(function (d) { return d.show(); });
        if (!devices.length)
            return;
        devices.sort(WebInspector.EmulatedDevice.deviceComparator);
        var groupElement = this._deviceSelectElement.createChild("optgroup");
        groupElement.label = name;
        for (var i = 0; i < devices.length; ++i) {
            var option = new Option(devices[i].title, devices[i].title);
            option.device = devices[i];
            option.lastSelectedIndex = 0;
            groupElement.appendChild(option);
        }
    },

    _emulatedSettingChanged: function()
    {
        if (this._emulatedSettingChangedMuted)
            return;

        for (var i = 1; i < this._deviceSelectElement.options.length; ++i) {
            var option = this._deviceSelectElement.options[i];
            var device = /** @type {!WebInspector.EmulatedDevice} */ (option.device);
            for (var j = 0; j < device.modes.length; j++) {
                if (WebInspector.overridesSupport.isEmulatingDevice(device.modeToOverridesDevice(device.modes[j]))) {
                    this._select(device, device.modes[j]);
                    return;
                }
            }
        }

        this._select(null, null);
    },

    /**
     * @param {?WebInspector.EmulatedDevice} device
     * @param {?WebInspector.EmulatedDevice.Mode} mode
     */
    _select: function(device, mode)
    {
        for (var i = 0; i < this._deviceSelectElement.options.length; i++) {
            if (this._deviceSelectElement.options[i].device === device)
                this._deviceSelectElement.selectedIndex = i;
        }
        this._updateModeSelect();
        for (var i = 0; i < this._modeSelectElement.options.length; i++) {
            if (this._modeSelectElement.options[i].mode === mode)
                this._modeSelectElement.selectedIndex = i;
        }
        this._updateModeControls();
        this._saveLastSelectedIndex();
        if (this._callback) {
            var option = this._modeSelectElement.options[this._modeSelectElement.selectedIndex];
            this._callback(option.device, option.mode);
        }
    },

    _deviceSelected: function()
    {
        this._updateModeSelect();
        this._modeSelected();
    },

    _updateModeSelect: function()
    {
        this._modeSelectElement.removeChildren();
        var option = this._deviceSelectElement.options[this._deviceSelectElement.selectedIndex];
        var device = /** @type {!WebInspector.EmulatedDevice} */ (option.device);

        if (this._deviceSelectElement.selectedIndex === 0) {
            this._addMode(device, null, "");
        } else if (device.modes.length === 1) {
            this._addMode(device, device.modes[0], WebInspector.UIString("Default"));
        } else {
            this._addOrientation(device, WebInspector.EmulatedDevice.Vertical, WebInspector.UIString("Portrait"));
            this._addOrientation(device, WebInspector.EmulatedDevice.Horizontal, WebInspector.UIString("Landscape"));
        }
        this._updateRotateModes();

        var index = option.lastSelectedIndex;
        var modeOption = this._modeSelectElement.options[index];
        if (modeOption.rotateIndex != -1) {
            var rotateOption = this._modeSelectElement.options[modeOption.rotateIndex];
            if (rotateOption.mode && rotateOption.mode.orientation === this._lastOrientation)
                index = modeOption.rotateIndex;
        }
        this._modeSelectElement.selectedIndex = index;
        this._updateModeControls();
    },

    /**
     * @param {!WebInspector.EmulatedDevice} device
     * @param {string} orientation
     * @param {string} title
     */
    _addOrientation: function(device, orientation, title)
    {
        var modes = device.modesForOrientation(orientation);
        if (!modes.length)
            return;
        if (modes.length === 1) {
            this._addMode(device, modes[0], title);
        } else {
            for (var index = 0; index < modes.length; index++)
                this._addMode(device, modes[index], title + " \u2013 " + modes[index].title);
        }
    },

    /**
     * @param {!WebInspector.EmulatedDevice} device
     * @param {?WebInspector.EmulatedDevice.Mode} mode
     * @param {string} title
     */
    _addMode: function(device, mode, title)
    {
        var option = new Option(title, title);
        option.mode = mode;
        option.device = device;
        this._modeSelectElement.appendChild(option);
    },

    _updateRotateModes: function()
    {
        for (var i = 0; i < this._modeSelectElement.options.length; i++) {
            var modeI = this._modeSelectElement.options[i].mode;
            this._modeSelectElement.options[i].rotateIndex = -1;
            for (var j = 0; j < this._modeSelectElement.options.length; j++) {
                var modeJ = this._modeSelectElement.options[j].mode;
                if (modeI && modeJ && modeI.orientation !== modeJ.orientation && modeI.title === modeJ.title)
                    this._modeSelectElement.options[i].rotateIndex = j;
            }
        }
    },

    _updateModeControls: function()
    {
        this._modeLabelElement.textContent = this._modeSelectElement.options[this._modeSelectElement.selectedIndex].label;

        if (this._modeSelectElement.options.length <= 1) {
            this._modeSelectElement.classList.toggle("hidden", true);
            this._modeLabelElement.classList.toggle("hidden", true);
        } else {
            var showLabel = this._modeSelectElement.options.length === 2 && this._modeSelectElement.options[0].rotateIndex === 1;
            this._modeSelectElement.classList.toggle("hidden",  showLabel);
            this._modeLabelElement.classList.toggle("hidden", !showLabel);
        }

        this._rotateButton.classList.toggle("hidden", this._modeSelectElement.options[this._modeSelectElement.selectedIndex].rotateIndex === -1);
    },

    _modeSelected: function()
    {
        this._saveLastSelectedIndex();
        this._updateModeControls();
        var option = this._modeSelectElement.options[this._modeSelectElement.selectedIndex];
        if (this._callback)
            this._callback(option.device, option.mode);
        this._emulatedSettingChangedMuted = true;
        WebInspector.overridesSupport.emulateDevice(option.device.modeToOverridesDevice(option.mode));
        this._emulatedSettingChangedMuted = false;
    },

    _saveLastSelectedIndex: function()
    {
        this._deviceSelectElement.options[this._deviceSelectElement.selectedIndex].lastSelectedIndex = this._modeSelectElement.selectedIndex;

        var option = this._modeSelectElement.options[this._modeSelectElement.selectedIndex];
        if (option.mode && option.rotateIndex != -1)
            this._lastOrientation = option.mode.orientation;
    },

    _rotateButtonClicked: function()
    {
        this._modeSelectElement.selectedIndex = this._modeSelectElement.options[this._modeSelectElement.selectedIndex].rotateIndex;
        this._modeSelected();
    }
}
