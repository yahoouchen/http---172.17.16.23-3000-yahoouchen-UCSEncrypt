#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <openssl/md5.h>
#include <openssl/des.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <openssl/aes.h>


int get_md5(char *input, char *output)
{
    MD5_CTX ctx = { 0 };
    MD5_Init(&ctx);
    MD5_Update(&ctx, input, strlen(input));
    MD5_Final((unsigned char *)output, &ctx);
    return 0;
}

/**
 * 处理异常
 */
int handleErrors(void) {
	return 0;
}
    

/**
 * AES加密ECB模式
 */
int encrypt_AES_ECB(unsigned char *plaintext, int plaintext_len, const char *key, unsigned char *ciphertext) //, int olen)
{
	if (strlen(key) == 0) {
		return 0;
	}

	unsigned char aes_keybuf[34];

	if (strlen(key) < 32) {

		strncpy((char*) aes_keybuf, key, sizeof(aes_keybuf));

		int beginIndex = (int)strlen(key);

		for( ; beginIndex <= 32 ;beginIndex++){
			aes_keybuf[beginIndex] = '0';
		}

	}else{
		strncpy((char*) aes_keybuf, key, 32);
	}

	unsigned char *iv = NULL;

	EVP_CIPHER_CTX *ctx;

	int len;

	int ciphertext_len;

	/* Create and initialise the context */
	if (!(ctx = EVP_CIPHER_CTX_new()))
		handleErrors();


	/* Initialise the encryption operation. IMPORTANT - ensure you use a key
	 * and IV size appropriate for your cipher
	 * In this example we are using 256 bit AES (i.e. a 256 bit key). The
	 * IV size for *most* modes is the same as the block size. For AES this
	 * is 128 bits */
	if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, aes_keybuf, iv))
		handleErrors();


	EVP_CIPHER_CTX_set_padding(ctx,5);

	/* Provide the message to be encrypted, and obtain the encrypted output.
	 * EVP_EncryptUpdate can be called multiple times if necessary
	 */
	if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
		handleErrors();

	ciphertext_len = len;

	/* Finalise the encryption. Further ciphertext bytes may be written at
	 * this stage.
	 */
	if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
		handleErrors();
	ciphertext_len += len;

	/* Clean up */
	EVP_CIPHER_CTX_free(ctx);

	return ciphertext_len;
}

/**
 * AES解密ECB模式
 */
int decrypt_AES_ECB(unsigned char *ciphertext, int ciphertext_len, const char *key, unsigned char *plaintext)
{
	if (strlen(key) == 0) {
		return 0;
	}

	unsigned char aes_keybuf[34];

	if (strlen(key) < 32) {

		strncpy((char*) aes_keybuf, key, sizeof(aes_keybuf));

		int beginIndex = (int)strlen(key);

		for( ; beginIndex <= 32 ;beginIndex++){
			aes_keybuf[beginIndex] = '0';
		}

	}else{
		strncpy((char*) aes_keybuf, key, 32);
	}


	EVP_CIPHER_CTX *ctx;

	unsigned char *iv = NULL;

	int len;

	int plaintext_len;

	/* Create and initialise the context */
	if (!(ctx = EVP_CIPHER_CTX_new()))
		handleErrors();

	/* Initialise the decryption operation. IMPORTANT - ensure you use a key
	 * and IV size appropriate for your cipher
	 * In this example we are using 256 bit AES (i.e. a 256 bit key). The
	 * IV size for *most* modes is the same as the block size. For AES this
	 * is 128 bits */
	if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, aes_keybuf, iv))
		handleErrors();

	EVP_CIPHER_CTX_set_padding(ctx,5);

	/* Provide the message to be decrypted, and obtain the plaintext output.
	 * EVP_DecryptUpdate can be called multiple times if necessary
	 */
	if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
		handleErrors();
	plaintext_len = len;

	/* Finalise the decryption. Further plaintext bytes may be written at
	 * this stage.
	 */
	if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
		handleErrors();
	plaintext_len += len;

	/* Clean up */
	EVP_CIPHER_CTX_free(ctx);

	return plaintext_len;
}


