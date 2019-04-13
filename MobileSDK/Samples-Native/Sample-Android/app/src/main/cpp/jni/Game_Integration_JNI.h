// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

JavaVM* Game_Integration_GetJavaVM();
JNIEnv* Game_Integration_GetJniEnv();
jobject Game_Integration_GetAppActivityIntance();
const std::string& Game_Integration_GetPath();