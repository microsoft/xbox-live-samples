// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

@interface TextViewLog : NSObject

+ (TextViewLog *)shared;

- (void)setTextView:(UITextView *)textView;

- (void)log:(int)level format:(NSString *)text, ...;

@end
