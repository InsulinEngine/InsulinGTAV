#pragma once

// The manual build marker. module_start logs it and the debug panel shows it;
// they must agree, because this string is the project's only reliable proof of
// which .prx the console is actually running. Bump it on every deploy.
#define INSULIN_BUILD_TAG "vprev-6"
