// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_WM_CORE_FOCUS_CONTROLLER_H_
#define UI_WM_CORE_FOCUS_CONTROLLER_H_

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "base/observer_list.h"
#include "base/scoped_observer.h"
#include "ui/aura/client/focus_client.h"
#include "ui/aura/window_observer.h"
#include "ui/events/event_handler.h"
#include "ui/wm/public/activation_change_observer.h"
#include "ui/wm/public/activation_client.h"
#include "ui/wm/wm_export.h"

namespace wm {

class FocusRules;

// FocusController handles focus and activation changes for an environment
// encompassing one or more RootWindows. Within an environment there can be
// only one focused and one active window at a time. When focus or activation
// changes notifications are sent using the
// aura::client::Focus/ActivationChangeObserver interfaces.
// Changes to focus and activation can be from three sources. The source can be
// determined by the ActivationReason parameter in
// ActivationChangeObserver::OnWindowActivated(...).
// . ActivationReason::ACTIVATION_CLIENT: The Aura Client API (implemented here
//   in aura::client::ActivationClient). (The FocusController must be set as the
//   ActivationClient implementation for all RootWindows).
// . ActivationReason::INPUT_EVENT: Input events (implemented here in
//   ui::EventHandler). (The FocusController must be registered as a pre-target
//   handler for the applicable environment owner, either a RootWindow or
//   another type).
// . ActivationReason::WINDOW_DISPOSITION_CHANGED: Window disposition changes
//   (implemented here in aura::WindowObserver). (The FocusController registers
//   itself as an observer of the active and focused windows).
class WM_EXPORT FocusController : public aura::client::ActivationClient,
                                  public aura::client::FocusClient,
                                  public ui::EventHandler,
                                  public aura::WindowObserver {
 public:
  // |rules| cannot be NULL.
  explicit FocusController(FocusRules* rules);
  ~FocusController() override;

 private:
  // Overridden from aura::client::ActivationClient:
  void AddObserver(aura::client::ActivationChangeObserver* observer) override;
  void RemoveObserver(
      aura::client::ActivationChangeObserver* observer) override;
  void ActivateWindow(aura::Window* window) override;
  void DeactivateWindow(aura::Window* window) override;
  aura::Window* GetActiveWindow() override;
  aura::Window* GetActivatableWindow(aura::Window* window) override;
  aura::Window* GetToplevelWindow(aura::Window* window) override;
  bool CanActivateWindow(aura::Window* window) const override;

  // Overridden from aura::client::FocusClient:
  void AddObserver(aura::client::FocusChangeObserver* observer) override;
  void RemoveObserver(aura::client::FocusChangeObserver* observer) override;
  void FocusWindow(aura::Window* window) override;
  void ResetFocusWithinActiveWindow(aura::Window* window) override;
  aura::Window* GetFocusedWindow() override;

  // Overridden from ui::EventHandler:
  void OnKeyEvent(ui::KeyEvent* event) override;
  void OnMouseEvent(ui::MouseEvent* event) override;
  void OnScrollEvent(ui::ScrollEvent* event) override;
  void OnTouchEvent(ui::TouchEvent* event) override;
  void OnGestureEvent(ui::GestureEvent* event) override;

  // Overridden from aura::WindowObserver:
  void OnWindowVisibilityChanged(aura::Window* window, bool visible) override;
  void OnWindowDestroying(aura::Window* window) override;
  void OnWindowHierarchyChanging(const HierarchyChangeParams& params) override;
  void OnWindowHierarchyChanged(const HierarchyChangeParams& params) override;

  // Internal implementation that coordinates window focus and activation
  // changes.
  void FocusAndActivateWindow(
      aura::client::ActivationChangeObserver::ActivationReason reason,
      aura::Window* window);

  // Internal implementation that sets the focused window, fires events etc.
  // This function must be called with a valid focusable window.
  void SetFocusedWindow(aura::Window* window);

  // Internal implementation that sets the active window, fires events etc.
  // This function must be called with a valid |activatable_window|.
  // |requested_window| refers to the window that was passed in to an external
  // request (e.g. FocusWindow or ActivateWindow). It may be NULL, e.g. if
  // SetActiveWindow was not called by an external request. |activatable_window|
  // refers to the actual window to be activated, which may be different.
  void SetActiveWindow(
      aura::client::ActivationChangeObserver::ActivationReason reason,
      aura::Window* requested_window,
      aura::Window* activatable_window);

  // Stack the |active_window_| on top of the window stack. This function is
  // called when activating a window or re-activating the current active window.
  void StackActiveWindow();

  // Called when a window's disposition changed such that it and its hierarchy
  // are no longer focusable/activatable. |next| is a valid window that is used
  // as a starting point for finding a window to focus next based on rules.
  void WindowLostFocusFromDispositionChange(aura::Window* window,
                                            aura::Window* next);

  // Called when an attempt is made to focus or activate a window via an input
  // event targeted at that window. Rules determine the best focusable window
  // for the input window.
  void WindowFocusedFromInputEvent(aura::Window* window);

  aura::Window* active_window_;
  aura::Window* focused_window_;

  bool updating_focus_;
  bool updating_activation_;

  scoped_ptr<FocusRules> rules_;

  base::ObserverList<aura::client::ActivationChangeObserver>
      activation_observers_;
  base::ObserverList<aura::client::FocusChangeObserver> focus_observers_;

  ScopedObserver<aura::Window, aura::WindowObserver> observer_manager_;

  DISALLOW_COPY_AND_ASSIGN(FocusController);
};

}  // namespace wm

#endif  // UI_WM_CORE_FOCUS_CONTROLLER_H_
