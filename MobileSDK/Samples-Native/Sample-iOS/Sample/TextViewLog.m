// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import <UIKit/UIKit.h>
#import "TextViewLog.h"
#import "ScreenLog.h"

@interface TextViewLog ()

@property (nonatomic, retain) UITextView *logTextView;
@property (nonatomic, retain) NSMutableAttributedString *logContent;

@end

@implementation TextViewLog

+ (TextViewLog *)shared {
    static TextViewLog *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[TextViewLog alloc] init];
    });
    return sharedInstance;
}

- (void)setTextView:(UITextView *)textView {
    self.logTextView = textView;
}

- (void)log:(int)level format:(NSString *)text, ... {
    if (self.logContent == nil) {
        self.logContent = [[NSMutableAttributedString alloc] initWithString:@""];
    }
    
    va_list args;
    va_start(args, text);
    NSString *argsText = [[NSString alloc] initWithFormat:text arguments:args];
    va_end(args);

    NSLog(@"<ScreenLog>[%@] %@", [self levelText:level], argsText);
    
    if (self.logTextView != nil) {
        dispatch_async(dispatch_get_main_queue(), ^{
            // UI updates must be done on the main thread.
            NSString *logLineEnded = [argsText stringByAppendingString:@"\n"];
            NSMutableAttributedString *attributedText = [[NSMutableAttributedString alloc] initWithString:logLineEnded];

            // Append the new log message to the stored attributed string, and setup a color attribute for it.
            NSUInteger logStart = self.logContent.length;
            UIColor *logColor = [self levelColor:level];
            
            [self.logContent appendAttributedString:attributedText];
            
            NSRange logRange = NSMakeRange(logStart, logLineEnded.length - 1);
            [self.logContent addAttribute:NSForegroundColorAttributeName value:logColor range:logRange];

            self.logTextView.attributedText = self.logContent;
            
            if (self.logTextView.attributedText.length > 0 ) {
                NSRange bottom = NSMakeRange(self.logTextView.attributedText.length - 1, 1);
                [self.logTextView scrollRangeToVisible:bottom];
            }
        });
    }
}

- (UIColor *)levelColor:(int)level {
    switch (level) {
        case LL_INFO:       return [UIColor whiteColor];
        case LL_DEBUG:      return [UIColor greenColor];
        case LL_ERROR:      return [UIColor redColor];
        case LL_WARNING:    return [UIColor orangeColor];
        case LL_TRACE:      return [UIColor cyanColor];
        default:            return [UIColor yellowColor];
    }
}

- (NSString *)levelText:(int)level {
    switch (level) {
        case LL_INFO:       return @"INFO";
        case LL_DEBUG:      return @"DEBUG";
        case LL_ERROR:      return @"ERROR";
        case LL_WARNING:    return @"WARNING";
        case LL_TRACE:      return @"TRACE";
        default:            return @"FATAL!";
    }
}

@end
