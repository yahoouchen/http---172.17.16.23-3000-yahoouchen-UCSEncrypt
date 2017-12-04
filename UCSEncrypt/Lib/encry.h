#ifndef ENCRY_H
#define ENCRY_H

#if !defined(UCSENCRY_EXTERN)
#  if defined(__cplusplus)
#    define UCSENCRY_EXTERN extern "C"
#  else
#    define UCSENCRY_EXTERN extern
#  endif
#endif

/**
 * MD5摘要算法
 */
UCSENCRY_EXTERN
int get_md5(char *input, char *output);


/**
 * AES解密ECB模式
 */
UCSENCRY_EXTERN int decrypt_AES_ECB(unsigned char *ciphertext, int ciphertext_len, const char *key, unsigned char *plaintext);

/**
 * AES加密ECB模式
 */
UCSENCRY_EXTERN int encrypt_AES_ECB(unsigned char *plaintext, int plaintext_len, const char *key, unsigned char *ciphertext);

#endif
