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
    #include "salieri.h"
    #define _Post_invalid_  // This annotation was added for SAL 2... not handled by the salieri.h workaround.

    // C++ standard libraries.
    #include <cstdlib>
    #include <cerrno>
    #include <memory>
    #include <cassert>
    #include <ctime>
    #include <unistd.h>
    #include <string>
    #include <thread>
    #include <mutex>
    #include <chrono>
    #include <iostream>
    #include <fstream>
    #include <sys/resource.h>

    // Cocos2D libraries.
    #include "cocos2d.h"
    #include "ui/CocosGUI.h"

    // Xbox Live iOS frameworks
    #include <Xal/xal.h>
    #include <Xal/xal_platform.h>
    #include <xsapi-c/services_c.h>

    // Other headers.
    #include "ScreenLog.h"
#endif
