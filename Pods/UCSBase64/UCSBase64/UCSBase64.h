//
//  UCSBase64.h
//  UCSSDK
//
//  Created by HuiYang on 17/3/2.
//  Copyright © 2017年 HuiYang. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface UCSBase64 : NSObject

+ (NSString *)ucsEncodeBase64String:(NSString *)input;
+ (NSString *)ucsDecodeBase64String:(NSString *)input;

+ (NSString *)ucsStringByEncodingData:(NSData *)data;

+ (NSData *)ucsDecodeData:(NSData *)data;
+ (NSData *)ucsDecodeString:(NSString *)string;

@end
