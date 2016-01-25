// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "modules/app_banner/AppBannerCallbacks.h"

namespace blink {

AppBannerCallbacks::AppBannerCallbacks(UserChoiceProperty* userChoice)
    : m_userChoice(userChoice)
{
}

void AppBannerCallbacks::onSuccess(const WebAppBannerPromptResult& result)
{
    m_userChoice->resolve(AppBannerPromptResult::create(result.platform, result.outcome));
}

} // namespace blink
