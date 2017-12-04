#include <stdio.h>
#include <string.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*-------------------------------------------------------
 对公钥进行PEM格式化
-------------------------------------------------------*/
void PubKeyPEMFormat(char *pubkey)
{
	char format_pubkey[8192] = {0};
	char pub_tem[8192] = {0};
	char *pub_begin = "-----BEGIN PUBLIC KEY-----\n";
	char *pub_end = "-----END PUBLIC KEY-----\n";
	char *check = strstr(pubkey,pub_begin);
	if(check)
	{
		return;
	}
	else
	{
		int nPublicKeyLen = (int)strlen(pubkey);
		int index = 0,publength = 0;
		memcpy(format_pubkey,pub_begin,27);
		for(index = 0; index < nPublicKeyLen; index += 64)
		{			
			memcpy(pub_tem,pubkey+index,64);
			strcat(format_pubkey,pub_tem);
			publength = (int)strlen(format_pubkey);
			format_pubkey[publength] = '\n';
			memset(pub_tem, 0, sizeof(pub_tem));
		}

		strcat(format_pubkey,pub_end);
		memcpy(pubkey,format_pubkey,strlen(format_pubkey));
	}
}


/*-------------------------------------------------------
 对私钥进行PKCS#8非加密的PEM格式化
-------------------------------------------------------*/
void PrivKeyPEMFormat(char *privkey)
{
	char format_privkey[4096] = {0};
	char priv_tem[4096] = {0};
	char *priv_begin = "-----BEGIN PRIVATE KEY-----\n";
	char *priv_end = "-----END PRIVATE KEY-----\n";
	char *check = strstr(privkey, priv_begin); 
	if(check)
	{
		return;
	}
	else
	{
		int nPrivateKeyLen = (int)strlen(privkey);
		int index = 0,privlength = 0;
		memcpy(format_privkey,priv_begin,28);
		for(index = 0; index < nPrivateKeyLen; index += 64)
		{			
			memcpy(priv_tem,privkey+index,64);
			strcat(format_privkey,priv_tem);
			privlength = (int)strlen(format_privkey);
			format_privkey[privlength] = '\n';
			memset(priv_tem, 0, sizeof(priv_tem));
		}
		strcat(format_privkey,priv_end);
		memcpy(privkey,format_privkey,strlen(format_privkey));
	}
}


/*-------------------------------------------------------
 通过公钥长度获取加密长度
-------------------------------------------------------*/
int getEncryptLengthByPubKey(int pubKeyLen)
{
	int cryLen = 0;								/*加密长度*/
	switch (pubKeyLen)
	{
		case 134:	/*256*/
			cryLen = 256;
			break;
		case 178:	/*512*/
			cryLen = 512;
			break;
		case 272:	/*1024*/
			cryLen = 1024;
			break;
		case 451:	/*2048*/
			cryLen = 2048;
			break;
		case 796:	/*4096*/
			cryLen = 4096;
			break;
		default:
			break;
	}
	return cryLen;
}


/*-------------------------------------------------------
 根据私钥长度判断对应的加密长度，得出实际密文分段长度
-------------------------------------------------------*/
int getCipherRealLenByPriKey(int priKeyLen)
{
	/*相应加密长度对应的每段密文长度*/
	int SignleRealLen = 0;
	if(priKeyLen == 319 || priKeyLen == 323)			/*256*/
		SignleRealLen = 32;								
	else if(priKeyLen == 518 || priKeyLen == 522)		/*512*/
		SignleRealLen = 64;								
	else if(priKeyLen == 912 || priKeyLen == 916)		/*1024*/
		SignleRealLen = 128;								
	else if(priKeyLen == 1700 || priKeyLen == 1704)		/*2048*/
		SignleRealLen = 256;									
	else if(priKeyLen == 3268 || priKeyLen == 3272)		/*4096*/
		SignleRealLen = 512;									
	return SignleRealLen;
}


/*-------------------------------------------------------
 Base64编解码
-------------------------------------------------------*/
/*Base64编码*/
int Base64Encode(const char *encoded, int encodedLength, char *decoded)
{
    return EVP_EncodeBlock((unsigned char*)decoded, (const unsigned char*)encoded, encodedLength);
}

/*Base64解码*/
int Base64Decode(const char *encoded, int encodedLength, char *decoded)   
{      
    return EVP_DecodeBlock((unsigned char*)decoded, (const unsigned char*)encoded, encodedLength); 
}

#pragma mark - 公钥加密私钥解密

/*-------------------------------------------------------
 利用公钥加密明文的过程
-------------------------------------------------------*/
int encryptByPublicKey(char *pubkey,char *in_plain,char *cipher)
{
	char plain[4096] = {0};			/*存放分段后的每一段明文*/
	char encrypted[4096] = {0};			/*存放每一段明文的解密结果*/
	char result[4096] = {0};			/*存放拼接后的密文*/
	char plain_rest[4096] = {0};		/*存放分段之后剩余部分的明文*/
	char encrypted_rest[4096] = {0};		/*存放对剩余部分明文的解密结果*/
	

	/*对公钥进行PEM格式化*/
	PubKeyPEMFormat(pubkey);
	
	/*根据公钥长度进行相关的计算*/
	int pubKeyLen = (int)strlen(pubkey);							/*计算公钥长度*/
	int CryLen = getEncryptLengthByPubKey(pubKeyLen);				/*通过公钥长度获取加密长度*/
	int maxPlain = CryLen / 8 - 11;							/*通过加密长度获取明文的最大加密长度*/
	int cipherLen = CryLen / 8;								/*通过加密长度获取密文的长度*/


	/*从字符串读取RSA公钥*/
	BIO *enc = NULL; 
	if ((enc = BIO_new_mem_buf(pubkey, -1)) == NULL)        
	{     
		//printf("BIO_new_mem_buf failed!\n");
        return -1;
	}


	/*解析公钥*/
	RSA *rsa_pub = NULL;
	rsa_pub = PEM_read_bio_RSA_PUBKEY(enc, &rsa_pub, NULL, NULL);
    
	if(rsa_pub == NULL)
	{
		//printf("Unable to read public key!\n");
        BIO_free_all(enc);
		return -1; 
	}


	/******************
	 分段循环加密过程
	******************/
	int label = 0,index = 0,index_rest = 0;
	int segment = (int)strlen(in_plain) / maxPlain;   /*分段数*/
	int rest = strlen(in_plain) % maxPlain;      /*余数*/

	/*明文长度大于最大加密长度且非整数倍*/
	if(strlen(in_plain) > maxPlain && rest != 0)
	{
		for(label = 0;label < segment; label++)
		{
			memset(plain,0,maxPlain);
			memset(encrypted,0,cipherLen);
			memcpy(plain, in_plain+index, maxPlain);		/*对明文进行分段*/
			plain[maxPlain] = '\0';
			int EncryptedLen = RSA_public_encrypt(maxPlain, (const unsigned char *)plain, (unsigned char*)encrypted, rsa_pub, RSA_PKCS1_PADDING);
			if(EncryptedLen == -1 )
			{
				//printf("Failed to encrypt!\n");
                BIO_free_all(enc);
                RSA_free(rsa_pub);
				return -1;
			} 
			
			/*对每一段定长密文进行拼接*/
			memcpy(result+label*cipherLen,encrypted,cipherLen);
			
			index += maxPlain;
		}
		
		/*对剩余部分明文进行加密*/
		index_rest = segment*maxPlain;
		memset(plain_rest,0,rest);
		memcpy(plain_rest, in_plain+index_rest, rest);		/*获取剩余部分明文*/
		plain_rest[rest] = '\0';
		memset(encrypted_rest,0,cipherLen);
		int EncryptedLen = RSA_public_encrypt(rest, (const unsigned char *)plain_rest, (unsigned char*)encrypted_rest, rsa_pub, RSA_PKCS1_PADDING);
		if(EncryptedLen == -1 )
		{
			//printf("Failed to encrypt!\n");
            BIO_free_all(enc);
            RSA_free(rsa_pub);
			return -1;
		}
		/*将剩余部分的密文拼接到整段密文中*/
		memcpy(result+label*cipherLen,encrypted_rest,cipherLen);
		
		/*对整段密文进行Base64编码*/
		Base64Encode(result, (label+1)*cipherLen, cipher);
	}

	/*明文长度等于最大加密长度的整数倍*/
	else if(strlen(in_plain) >= maxPlain && rest == 0)
	{
		for(label = 0;label < segment; label++)
		{
			memset(plain,0,maxPlain);
			memset(encrypted,0,cipherLen);
			memcpy(plain, in_plain+index, maxPlain);		/*对明文进行分段*/
			plain[maxPlain] = '\0';
			int EncryptedLen = RSA_public_encrypt(maxPlain, (const unsigned char *)plain, (unsigned char*)encrypted, rsa_pub, RSA_PKCS1_PADDING);
			if(EncryptedLen == -1 )
			{
				//printf("Failed to encrypt!\n");
                BIO_free_all(enc);
                RSA_free(rsa_pub);
				return -1;
			} 			
			/*拼接每段密文*/
			memcpy(result+label*cipherLen,encrypted,cipherLen);
		}
		/*对整段密文进行Base64编码*/
		Base64Encode(result, label*cipherLen, cipher);
	}

	/*明文长度小于最大加密长度*/
	else
	{
		int EncryptedLen = RSA_public_encrypt((int)strlen(in_plain), (const unsigned char *)in_plain, (unsigned char*)encrypted, rsa_pub, RSA_PKCS1_PADDING);
		if(EncryptedLen == -1 )
		{
			//printf("Failed to encrypt!\n");
            BIO_free_all(enc);
            RSA_free(rsa_pub);
			return -1;
		}
		/*对密文进行Base64编码*/
		Base64Encode(encrypted, cipherLen, cipher);
	}

	/*释放BIO内存和RSA结构体*/
	BIO_free_all(enc);
	RSA_free(rsa_pub);
	
	return 0;
}


/*-------------------------------------------------------
 利用私钥解密密文的过程
-------------------------------------------------------*/
#ifdef DEBUG
int decryptByPrivateKey(char *privkey,char *cipher,char *out_plain)
{
	char encrypted[4096] = {0};			/*存放解码后的整段密文*/
	char encrypted_result[4096] = {0};		/*存放分段后的每一段密文*/
	char decrypted[4096] = {0};			/*存放每一段密文的解密结果*/
	
	/*对私钥进行PKCS#8非加密的PEM格式化*/
	PrivKeyPEMFormat(privkey);

	/*根据私钥长度进行相关的计算*/
	int priKeyLen = (int)strlen(privkey);							/*私钥长度*/
	int CipherRealLen = getCipherRealLenByPriKey(priKeyLen);		/*通过私钥长度获取每段密文实际长度*/
	int plainLen = CipherRealLen - 11;

	/*从字符串读取RSA私钥*/
	BIO *dec = NULL;  
	if ((dec = BIO_new_mem_buf(privkey, -1)) == NULL)
	{     
		//printf("BIO_new_mem_buf failed!\n");
        return -1;
	}       
	
	/*解析私钥*/
	EVP_PKEY *pri = NULL;
	pri = PEM_read_bio_PrivateKey(dec, NULL, NULL, NULL);
	if(pri == NULL)
	{
		//printf("Unable to read private key!\n");
        BIO_free_all(dec);
		return -1;
	}
	
    RSA *rsa_pri = NULL;
	/*将EVP_PKEY结构体转换成RSA结构体*/
	rsa_pri = EVP_PKEY_get1_RSA(pri);

	/******************
	 分段循环解密过程
	******************/
	int CipherLen = (int)strlen(cipher);		/*Base64编码的密文长度*/
	int index = 0, label = 0, out_plainLen = 0;
	
	/*计算真实密文的段数*/
    if (CipherRealLen <= 0) {
        BIO_free_all(dec);
        RSA_free(rsa_pri);
        EVP_PKEY_free(pri);
        return -1;
    }
	int segment = CipherLen * 3 / 4 / CipherRealLen;
	
	memset(out_plain, 0 ,plainLen);
	
	/*对整段密文进行Base64解码*/
	Base64Decode(cipher, CipherLen, encrypted);
	
	/*将解码后的密文分段解密后合并*/
	while(label < segment)
	{
		memset(encrypted_result,0,CipherRealLen);
		memcpy(encrypted_result,encrypted+index,CipherRealLen);		/*对密文进行分段*/
		encrypted_result[CipherRealLen] = '\0';
		
		memset(decrypted, 0, plainLen);
		int DecryptedLen = RSA_private_decrypt(CipherRealLen, (const unsigned char *)encrypted_result, (unsigned char*)decrypted, rsa_pri, RSA_PKCS1_PADDING);
		if(DecryptedLen == -1)
		{
			//printf("Failed to decrypt!\n");
            BIO_free_all(dec);
            RSA_free(rsa_pri);
            EVP_PKEY_free(pri);
			return -1;
		}
		decrypted[DecryptedLen] = '\0';
		strcat(out_plain, decrypted);		/*将每一段的解密结果拼接到整段输出明文中*/
		out_plainLen += DecryptedLen;
		out_plain[out_plainLen] = '\0';
		index += CipherRealLen;
		label++;
	}

	/*释放BIO内存以及RSA和EVP_PKEY结构体*/
	BIO_free_all(dec);
	RSA_free(rsa_pri);
	EVP_PKEY_free(pri); 
	
	return 0;
}
#endif

#pragma mark - 私钥加密公钥解密
/*-------------------------------------------------------
 利用私钥加密密文的过程
 -------------------------------------------------------*/
int encryptByPrivateKey(char *privkey, char *in_plain, char *cipher)
{
    int rsa_private_len = 0;
    /*对私钥进行PKCS#8非加密的PEM格式化*/
    PrivKeyPEMFormat((char *)privkey);
    
    /*从字符串读取RSA私钥*/
    BIO *dec = NULL;
    if ((dec = BIO_new_mem_buf(privkey, -1)) == NULL)
    {
        //printf("BIO_new_mem_buf failed!\n");
        return -1;
    }
    
    /*解析私钥*/
    EVP_PKEY *pri = NULL;
    pri = PEM_read_bio_PrivateKey(dec, NULL, NULL, NULL);
    if(pri == NULL)
    {
        //printf("Unable to read private key!\n");
        BIO_free_all(dec);
        return -1;
    }
    
    /*将EVP_PKEY结构体转换成RSA结构体*/
    RSA *rsa_privateKey = NULL;
    rsa_privateKey = EVP_PKEY_get1_RSA(pri);
    if (rsa_privateKey == NULL) {
        BIO_free_all(dec);
        EVP_PKEY_free(pri);
        return -1;
    }
    
    rsa_private_len = RSA_size(rsa_privateKey);
    //printf("RSA private length: %d\n", rsa_private_len);
    
    // 11 bytes is overhead required for encryption
    int chunk_length = rsa_private_len - 11;
    // plain text length
    int plain_char_len = (int)strlen(in_plain);
    // calculate the number of chunks
    int num_of_chunks = (int)(strlen(in_plain) / chunk_length) + 1;
    
    int total_cipher_length = 0;
    
    // the output size is (total number of chunks) x (the key length)
    int encrypted_size = (num_of_chunks * rsa_private_len);
    unsigned char *cipher_data = malloc(encrypted_size + 1);
    if (cipher_data == NULL) {
        BIO_free_all(dec);
        RSA_free(rsa_privateKey);
        EVP_PKEY_free(pri);
        return -1;
    }
    
    for (int i = 0; i < plain_char_len; i += chunk_length) {
        // get the remaining character count from the plain text
        int remaining_char_count = plain_char_len - i;
        
        // this len is the number of characters to encrypt, thus take the minimum between the chunk count & the remaining characters
        // this must less than rsa_private_len - 11
        
        int len = remaining_char_count < chunk_length ? remaining_char_count: chunk_length;
        unsigned char *plain_chunk = malloc(len + 1);
        // take out chunk of plain text
        memcpy(&plain_chunk[0], &in_plain[i], len);
        
        //printf("Plain chunk: %s\n", plain_chunk);
        
        unsigned char *result_chunk = malloc(rsa_private_len + 1);
        
        int result_length = RSA_private_encrypt(len, plain_chunk, result_chunk, rsa_privateKey, RSA_PKCS1_PADDING);
        //printf("Encrypted Result chunk: %s\nEncrypted Chunk length: %d\n", result_chunk, result_length);
        
        
        if (result_length == -1) {
            BIO_free_all(dec);
            RSA_free(rsa_privateKey);
            EVP_PKEY_free(pri);
            free(plain_chunk);
            free(result_chunk);
            free(cipher_data);
            return -1;
        }
        
        free(plain_chunk);
        
        memcpy(&cipher_data[total_cipher_length], &result_chunk[0], result_length);
        
        total_cipher_length += result_length;
        
        free(result_chunk);
    }
    //printf("Total cipher length: %d\n", total_cipher_length);
    
    BIO_free_all(dec);
    RSA_free(rsa_privateKey);
    EVP_PKEY_free(pri);
    
    char *encrypted = malloc(4096);
    if (encrypted == NULL) {
        free(cipher_data);
        return -1;
    }
    memset(encrypted, 0, 0);
    /*对密文进行Base64编码*/
    Base64Encode((char *)cipher_data, encrypted_size, encrypted);
    
    //printf("Final result: %s\n Final result length: %zu\n", encrypted, total_len);
    
    free(cipher_data);
    strcpy(cipher, encrypted);
    free(encrypted);
    return 0;
}

/*-------------------------------------------------------
 利用公钥解密明文的过程
 -------------------------------------------------------*/
int decryptByPublicKey(char *pubkey,char *cipher,char *out_plain)
{
    
    /*对公钥进行PEM格式化*/
    PubKeyPEMFormat((char *)pubkey);
    
    /*从字符串读取RSA公钥*/
    BIO *enc = NULL;
    if ((enc = BIO_new_mem_buf(pubkey, -1)) == NULL) {
        //printf("BIO_new_mem_buf failed!\n");
        return -1;
    }
    
    /*解析公钥*/
    RSA *rsa_publicKey = NULL;
    rsa_publicKey = PEM_read_bio_RSA_PUBKEY(enc, &rsa_publicKey, NULL, NULL);
    
    if(rsa_publicKey == NULL) {
        BIO_free_all(enc);
        //printf("Unable to read public key!\n");
        return -1;
    }
    
    int rsa_public_len = RSA_size(rsa_publicKey);
    //printf("RSA public length: %d\n", rsa_public_len);
    
    size_t cipher_len = (int)strlen(cipher);
    size_t crypt_len = cipher_len / 4 * 3;
    if (cipher[cipher_len - 1] == '=') crypt_len--;
    if (cipher[cipher_len - 2] == '=') crypt_len--;
    
    /*对整段密文进行Base64解码*/
    char * crypt = malloc(crypt_len * 2);
    if (crypt == NULL) {
        BIO_free_all(enc);
        RSA_free(rsa_publicKey);
        return -1;
    }

    memset(crypt, 0, 0);
    Base64Decode(cipher, (int)cipher_len, crypt);
    //printf("Decoded cipher: %s\nCrypt length: %ld\n", crypt, crypt_len);
    
    // If no static, it will cause "address of stack memory associated with local variable ...", which mean the variable will released from memory after the end of this function
    char *plain_char = malloc(cipher_len * 2);
    if (plain_char == NULL) {
        BIO_free_all(enc);
        RSA_free(rsa_publicKey);
        free(crypt);
        return -1;
    }
    // initialize
    memset(plain_char, 0, 0);
    
    for (int i = 0; i < crypt_len; i += rsa_public_len) {
        unsigned char *crypt_chunk = malloc(rsa_public_len + 1);
        memcpy(&crypt_chunk[0], &crypt[i], rsa_public_len);
        
        //printf("Crypt chunk: %s\n", crypt_chunk);
        
        unsigned char *result_chunk = malloc(crypt_len + 1);
        int result_length = RSA_public_decrypt(rsa_public_len, crypt_chunk, result_chunk, rsa_publicKey, RSA_PKCS1_PADDING);
        // chunk length should be the size of publickey (in bytes) minus 11 (overhead during encryption)
        //printf("Result chunk: %s\nChunk length: %d\n", result_chunk, result_length);
        
        free(crypt_chunk);
        
        // this is to omit the dummy character behind
        // i.e. Result chunk: ABC-1234567-201308101427371250-abcdefghijklmnopqrstuv\240Z
        //      Chunk length: 53
        //      New chunk: ABC-1234567-201308101427371250-abcdefghijklmnopqrstuv
        //
        // by copying the chunk to a temporary variable with an extra length (i.e. in this case is 54)
        // and then set the last character of temporary variable to NULL
        char tmp_result[result_length + 1];
        memcpy(tmp_result, result_chunk, result_length);
        tmp_result[result_length] = '\0';
        //printf("New chunk: %s\n", tmp_result);
        
        free(result_chunk);
        
        if (result_length == -1) {
            BIO_free_all(enc);
            RSA_free(rsa_publicKey);
            free(plain_char);
            free(crypt);
            return -1;
        }
        
        strcat(plain_char, tmp_result);
    }
    
    //printf("Final result: %s\n", plain_char);
    strcpy(out_plain, plain_char);
    free(plain_char);
    free(crypt);
    BIO_free_all(enc);
    RSA_free(rsa_publicKey);
    return 0;
}
