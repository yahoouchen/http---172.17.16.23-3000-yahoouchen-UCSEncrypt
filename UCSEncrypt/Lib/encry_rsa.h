
#ifndef ENCRY_RSA_H
#define ENCRY_RSA_H


#if !defined(ENCRY_RSA_EXTERN)
#  if defined(__cplusplus)
#    define ENCRY_RSA_EXTERN extern "C"
#  else
#    define ENCRY_RSA_EXTERN extern
#  endif
#endif
/*
 * 公钥加密
 */
ENCRY_RSA_EXTERN
int encryptByPublicKey(char *pubkey,char *in_plain,char *cipher);

/*
 * 公钥解密
 */
ENCRY_RSA_EXTERN
int decryptByPublicKey(char *pubkey,char *cipher,char *out_plain);

/*
 * 私钥加密
 */
ENCRY_RSA_EXTERN
int encryptByPrivateKey(char *privkey,char *in_plain,char *cipher);

#ifdef DEBUG
/*
 * 私钥解密
 */
ENCRY_RSA_EXTERN
int decryptByPrivateKey(char *privkey,char *cipher,char *out_plain);
#endif

#endif

