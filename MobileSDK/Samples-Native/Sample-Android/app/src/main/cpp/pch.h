// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include <jni.h>
#include <cstdlib>
#include <cerrno>
#include <memory>
#include <cassert>
#include <ctime>
#include <unistd.h>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/resource.h>

#define UNREFERENCED_PARAMETER(P) (P)
#define ASSERT_MESSAGE(check_bool, failed_message) assert((check_bool) && (failed_message))

#include <android/log.h>
#include "SampleLog.h"

#include <xsapi-c/services_c.h>
#include <Xal/xal.h>
#include <Xal/xal_platform.h>