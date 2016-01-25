// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/runner/register_local_aliases.h"

#include "mojo/shell/package_manager/package_manager_impl.h"

namespace mojo {
namespace runner {

void RegisterLocalAliases(shell::PackageManagerImpl* manager) {
  // TODO(erg): We should probably handle this differently; these could be
  // autogenerated from package manifests.
  manager->RegisterApplicationPackageAlias(
      GURL("mojo:clipboard"), GURL("mojo:core_services"), "Core");
  manager->RegisterApplicationPackageAlias(GURL("mojo:filesystem"),
                                           GURL("mojo:core_services"), "Files");
#if defined(OS_LINUX) && !defined(OS_ANDROID)
  manager->RegisterApplicationPackageAlias(GURL("mojo:font_service"),
                                           GURL("mojo:core_services"), "Files");
#endif
#if !defined(OS_ANDROID)
  // On Android, these are Java apps which are loaded in the shell process.
  manager->RegisterApplicationPackageAlias(GURL("mojo:mus"),
                                           GURL("mojo:core_services"),
                                           "Surfaces");
  manager->RegisterApplicationPackageAlias(
      GURL("mojo:network_service"), GURL("mojo:core_services"), "Network");
  manager->RegisterApplicationPackageAlias(
      GURL("mojo:resource_provider"), GURL("mojo:core_services"), "Files");
#endif

  manager->RegisterApplicationPackageAlias(
      GURL("mojo:web_view"), GURL("mojo:core_services"), "Core");
  manager->RegisterApplicationPackageAlias(
      GURL("mojo:tracing"), GURL("mojo:core_services"), "Core");
}

}  // namespace runner
}  // namespace mojo
