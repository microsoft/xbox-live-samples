//
// Prefix header for all source files of the 'iphone' target in the 'iphone' project
//

#ifdef __OBJC__
    #import <Foundation/Foundation.h>
    #import <UIKit/UIKit.h>
#endif

#ifdef __cplusplus
    // Useful macros.
    #define UNREFERENCED_PARAMETER(P) (P)
    #define ASSERT_MESSAGE(check_bool, failed_message) assert(check_bool && failed_message)

    // Defines the Windows SAL annotations used in the C++ code.
    // This workaround for non-Windows cross-platform support for SAL comes from here: https://github.com/nemequ/salieri
    #import "salieri.h"
    #define _Post_invalid_  // This annotation was added for SAL 2... not handled by the salieri.h workaround.

    // C++ standard libraries.
    #import <cstdlib>
    #import <cerrno>
    #import <memory>
    #import <cassert>
    #import <ctime>
    #import <unistd.h>
    #import <string>
    #import <thread>
    #import <mutex>
    #import <chrono>
    #import <iostream>
    #import <fstream>
    #import <sstream>
    #import <sys/resource.h>
    #import <vector>

    // Xbox Live iOS frameworks
    #import <Xal/xal.h>
    #import <Xal/xal_platform.h>
    #import <xsapi-c/services_c.h>

    // Logging.
    #import "ScreenLog.h"
#endif
