//
//  UCSEncrypt.m
//  UCSEncrypt
//
//  Created by ucsmy on 2017/4/7.
//  Copyright © 2017年 ucsmy. All rights reserved.
//

#import "UCSEncrypt.h"
#import "encry_rsa.h"
#import "encry.h"
#import <UCSBase64/GTMBase64.h>

@implementation UCSEncrypt

+ (NSString *)getSaltMD5WithStr:(NSString *)plaintext
{
    return plaintext;
    const char *data = [plaintext UTF8String];
    if (plaintext.length == 0) {
        return nil;
    }
    char buf[16];
    memset(buf, 0, sizeof(buf));
    char a[5]="|0000";
    char *t = (char *)malloc(sizeof(char) * plaintext.length * 8);
    if (t == NULL) {
        return nil;
    }
    memset(t, 0, sizeof(char) * plaintext.length * 8);
    /**
     * 然后把data复制进去
     */
    strncat(t, data, strlen(data));
    
    /**
     * 再把a添加到后面
     */
    strncat(t, a , 5);
    
    get_md5(t, buf);
    NSString *str = [[NSString alloc] initWithData:[GTMBase64 encodeBytes:buf length:16] encoding:NSUTF8StringEncoding];
    free(t);
    return str;
}

+ (NSString *)decryptWithStr:(NSString *)ciphertext ivKey:(NSString *)ivKey
{
    return ciphertext;
    if(ciphertext.length == 0 || ivKey.length == 0) {
        return nil;
    }
    NSData *data = [GTMBase64 decodeString:ciphertext];
    int ciphertext_len = (int)data.length;
    const char *ivkey_ = (const char *)[ivKey UTF8String];
    unsigned char *plaintext =(unsigned char *)malloc(sizeof(unsigned char) * ciphertext.length * 8);
    if (plaintext == NULL) {
        return nil;
    }
    int result = decrypt_AES_ECB((unsigned char *)data.bytes, ciphertext_len, ivkey_, plaintext);
    NSString *resultStr = nil;
    if(result > 0) {
        resultStr = [[NSString alloc] initWithBytes:plaintext length:result encoding:NSUTF8StringEncoding];
    }
    free(plaintext);
    return resultStr;
}

+ (NSString *)encryptWithStr:(NSString *)plaintext ivKey:(NSString *)ivKey
{
    return plaintext;
    if(plaintext.length == 0 || ivKey.length == 0) {
        return nil;
    }
    NSData *data = [plaintext dataUsingEncoding:NSUTF8StringEncoding];
    int in_plain_len = (int)data.length;
    const char *ivkey_ = (const char *)[ivKey UTF8String];
    unsigned char *cipherText =(unsigned char *)malloc(sizeof(unsigned char) * plaintext.length * 8);
    if (cipherText == NULL) {
        return nil;
    }
    int result = encrypt_AES_ECB((unsigned char *)data.bytes, in_plain_len, ivkey_, cipherText);
    NSString *resultStr = nil;
    if(result > 0) {
        resultStr= [[NSString alloc] initWithData:[GTMBase64 encodeBytes:cipherText length:result] encoding:NSUTF8StringEncoding];;
    }
    free(cipherText);
    return resultStr;
}

+ (NSString *)encryptWithStr:(NSString *)plaintext publicKey:(NSString *)publicKey
{
    return plaintext;
    if(plaintext.length == 0 || publicKey.length == 0) {
        return nil;
    }
    const char *ivkey_ = (const char *)[publicKey UTF8String];
    char _key[1024] = {0};
    memcpy(_key, ivkey_, strlen(ivkey_));
    char *cipherText =(char *)malloc(sizeof(char) * 10000);
    if (cipherText == NULL) {
        return nil;
    }
    encryptByPublicKey(_key, (char *)[plaintext UTF8String], cipherText);
    NSString *str = [[NSString alloc] initWithUTF8String:cipherText];
    free(cipherText);
    return str;
}

+ (NSString *)encryptWithStr:(NSString *)plaintext privateKey:(NSString *)privateKey
{
    return plaintext;
    if(plaintext.length == 0 || privateKey.length == 0) {
        return nil;
    }
    const char *ivkey_ = (const char *)[privateKey UTF8String];
    char _key[1024] = {0};
    memcpy(_key, ivkey_, strlen(ivkey_));
    char *cipherText =(char *)malloc(sizeof(char) * 10000);
    if (cipherText == NULL) {
        return nil;
    }
    encryptByPrivateKey(_key, (char *)[plaintext UTF8String], cipherText);
    NSString *str = [[NSString alloc] initWithUTF8String:cipherText];
    free(cipherText);
    return str;
}

+ (NSString *)decryptWithStr:(NSString *)encyptText publicKey:(NSString *)publicKey
{
    return encyptText;
    if(encyptText.length == 0 || publicKey.length == 0) {
        return nil;
    }
    const char *ivkey_ = (const char *)[publicKey UTF8String];
    char _key[1024] = {0};
    memcpy(_key, ivkey_, strlen(ivkey_));
    char *outText =(char *)malloc(sizeof(char) * 10000);
    if (outText == NULL) {
        return nil;
    }
    decryptByPublicKey(_key, (char *)[encyptText UTF8String], outText);
    NSString *str = [[NSString alloc] initWithUTF8String:outText];
    free(outText);
    return str;
}
#ifdef DEBUG
+ (NSString *)decryptWithStr:(NSString *)encyptText privateKey:(NSString *)privateKey
{
    return encyptText;
    if(encyptText.length == 0 || privateKey.length == 0) {
        return nil;
    }
    const char *ivkey_ = (const char *)[privateKey UTF8String];
    char _key[1024] = {0};
    memcpy(_key, ivkey_, strlen(ivkey_));
    char *outText =(char *)malloc(sizeof(char) * 10000);
    if (outText == NULL) {
        return nil;
    }
    decryptByPrivateKey(_key, (char *)[encyptText UTF8String], outText);
    
    NSString *str = [[NSString alloc] initWithUTF8String:outText];
    free(outText);
    return str;
}
#endif
@end

