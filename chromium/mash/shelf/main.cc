// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mash/shelf/shelf_application.h"
#include "mojo/public/c/system/main.h"
#include "mojo/shell/public/cpp/application_runner.h"

MojoResult MojoMain(MojoHandle shell_handle) {
  mojo::ApplicationRunner runner(new mash::shelf::ShelfApplication);
  return runner.Run(shell_handle);
}
