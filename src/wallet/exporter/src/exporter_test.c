#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// test functions
int runTestCases(void);
int testDecryption(const unsigned char*, const unsigned char*, const int, const unsigned char*, const unsigned char*, const unsigned char*, unsigned char*);
int bytesToKey(const unsigned char*, const unsigned char*, const int, unsigned char*, unsigned char*);
int decrypt(const unsigned char*, const unsigned char*, const unsigned char*, int, unsigned char*);
int hash(const unsigned char*, const int, unsigned char*); 
int doubleHash(const unsigned char*, const int, unsigned char*); 
int deserializeArray(const unsigned char*, const unsigned int, unsigned char*, const unsigned int, const unsigned int, const unsigned int);

int main() {
  int res = runTestCases();
}

int runTestCases() {
  printf("\n");
  int res = 0;
  int testCaseLength = 4;
  char *testStrings[] = { "ffffffffffffffff", "fffefdfcfbfaf9f8", "0000000000000000", "0123456789abcdef" }; 
  char specialString[] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef };
  int len = 8;
  unsigned char in[len];
  unsigned char start[] = { 0xff, 0xff, 0x00 };
  unsigned char out[len*2+1];
  int ret;
  for(int i = 0; i < testCaseLength; i++) {
    printf("Running test case %d...\n", i + 1);
    int dec = 0;
    for(int j = 0; j < len; j++) {
      if (i == 3) {
        in[j] = specialString[j];
      } else {
        in[j] = start[i] - dec;
        if (i == 1) {
          dec++;
        } else {
          dec = 0;
        }
      }
    }
    ret = deserializeArray((const unsigned char*)in, 0, out, 8, 1, 1);
    if (ret != len) {
      fprintf(stderr, "test case failed, length of returned string was: %d but the length was supposed to be: %d\n", ret, len);
      res = 1;
    } else {
      printf("length test passed!\n");
    }
    if (strcmp((const char*)out, testStrings[i]) != 0) {
      fprintf(stderr, "test case failed, out string: %s, does not equal expected string of: %s\n", out, testStrings[i]);
      res = 1;
    } else {
      printf("string matching test passed\n");
    }
    printf("\n");
  }
  const unsigned char der[] = {
    0x30, 0x81, 0xd3, 0x02, 0x01, 0x01, 0x04, 0x20, 
    0x6e, 0x54, 0xff, 0x7a, 0x5d, 0x09, 0x55, 0x3c, 
    0x7a, 0x00, 0x13, 0x0c, 0x25, 0x87, 0xc0, 0x96, 
    0x33, 0x6a, 0x63, 0x68, 0xff, 0x83, 0x62, 0x99, 
    0xe2, 0x07, 0x2c, 0x72, 0x5d, 0x04, 0xff, 0x73, 
    0xa0, 0x81, 0x85, 0x30, 0x81, 0x82, 0x02, 0x01, 
    0x01, 0x30, 0x2c, 0x06, 0x07, 0x2a, 0x86, 0x48, 
    0xce, 0x3d, 0x01, 0x01, 0x02, 0x21, 0x00, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xfe, 0xff, 0xff, 0xfc, 0x2f, 0x30, 
    0x06, 0x04, 0x01, 0x00, 0x04, 0x01, 0x07, 0x04, 
    0x21, 0x02, 0x79, 0xbe, 0x66, 0x7e, 0xf9, 0xdc, 
    0xbb, 0xac, 0x55, 0xa0, 0x62, 0x95, 0xce, 0x87, 
    0x0b, 0x07, 0x02, 0x9b, 0xfc, 0xdb, 0x2d, 0xce, 
    0x28, 0xd9, 0x59, 0xf2, 0x81, 0x5b, 0x16, 0xf8, 
    0x17, 0x98, 0x02, 0x21, 0x00, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xfe, 0xba, 0xae, 0xdc, 
    0xe6, 0xaf, 0x48, 0xa0, 0x3b, 0xbf, 0xd2, 0x5e, 
    0x8c, 0xd0, 0x36, 0x41, 0x41, 0x02, 0x01, 0x01, 
    0xa1, 0x24, 0x03, 0x22, 0x00, 0x03, 0xfc, 0x20, 
    0xf6, 0xee, 0x4e, 0x02, 0x7b, 0x96, 0x2a, 0x4e, 
    0xb7, 0x1f, 0x85, 0xf3, 0x2a, 0x60, 0xb2, 0xca, 
    0x25, 0xdd, 0x22, 0x74, 0x28, 0xbd, 0xc5, 0xa4, 
    0x36, 0xa1, 0x92, 0x40, 0xff, 0x74
  };
  const unsigned char sl[] = { 0x0B, 0xE3, 0xFB, 0x75, 0xEC, 0x80, 0x5A, 0xBD };
  const unsigned char ct[] = { 
                  0x32, 0xE2, 0xD5, 0x76, 0xAA, 0x78, 0xB6, 0xB9, 
                  0x1B, 0x82, 0x47, 0xB4, 0x6B, 0x88, 0x3D, 0x21, 
                  0xF7, 0x68, 0xE5, 0x2E, 0x67, 0xF7, 0x8D, 0x99, 
                  0x9F, 0x90, 0xCE, 0xF6, 0x34, 0x60, 0xFE, 0x75, 
                  0xE5, 0x6E, 0x6C, 0x6C, 0xB0, 0xEA, 0x71, 0x71, 
                  0xCF, 0xFF, 0xDB, 0xD5, 0x35, 0x51, 0x05, 0x05 
              }; 
  const unsigned char testPubkey[] = {
                        0x03, 0xFF, 0xFF, 0x40, 0xB2, 0xCC, 0x0D, 0x3A,
                        0x29, 0xFC, 0xDA, 0x41, 0xEA, 0xB5, 0x3E, 0x13, 
                        0x00, 0x70, 0xEF, 0x18, 0xD8, 0x7B, 0x69, 0x80, 
                        0x79, 0x51, 0xFB, 0xE0, 0x13, 0x2A, 0xFF, 0xE1, 0x5B
                      };
  const unsigned char keyct[] = { 
                   0x7C, 0x28, 0x54, 0x61, 0xA4, 0x01, 0x61, 0x22, 
                   0x41, 0xE4, 0x6F, 0xC1, 0xCF, 0x52, 0x69, 0xD3, 
                   0x16, 0xB9, 0x63, 0x6E, 0xFF, 0x0C, 0xA1, 0x46, 
                   0xD3, 0xE9, 0x60, 0x7F, 0xC5, 0x09, 0x00, 0x89, 
                   0x7E, 0x8C, 0x70, 0x6A, 0x44, 0x1E, 0xCF, 0x5E, 
                   0x73, 0x77, 0x29, 0x05, 0x59, 0x26, 0xFB, 0xE4 
                 };
  unsigned char decryptedPrivateKey[32];
  unsigned char dk[65];
  const unsigned char *passphrase = (const unsigned char*)"test";
  int dkret = testDecryption(passphrase, sl, 162511, ct, testPubkey, keyct, decryptedPrivateKey); 
  deserializeArray(decryptedPrivateKey, 0, dk, 32, 1, 1);
  if (!dkret || strcmp((const char*)dk, "4a9f87c9384a60d1fe95f5cc6c94e54d76457798b687639fb83f2d0cf365ad7b")) {
    res = 1;
  } 
  if (!res) {
    printf("all tests passed!");
  } else {
    printf("some tests failed");
  }
  return (res);
}

int bytesToKey(const unsigned char *pass, const unsigned char *salt, int rounds, unsigned char *key, unsigned char *iv) {
  int ok;
  ok = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha512(), salt, pass, 4, rounds, key, iv); 
  if (ok != 32) {
    return (0);
  }
  return (ok);
}

int decrypt(const unsigned char *in, const unsigned char* key, const unsigned char *iv, int plen, unsigned char *out) {
  int ok = 0, flen = 0;
  EVP_CIPHER_CTX ctx;
  EVP_CIPHER_CTX_init(&ctx);
  EVP_CIPHER_CTX_set_padding(&ctx, 0);
  if (EVP_DecryptInit_ex(&ctx, EVP_aes_256_cbc(), NULL, key, iv) && 
      EVP_DecryptUpdate(&ctx, out, &plen, in, plen) &&
      EVP_DecryptFinal_ex(&ctx, out + plen, &flen)) {
    ok = 1;  
  } 
  EVP_CIPHER_CTX_cleanup(&ctx);
  return ok;
}

int hash(const unsigned char *in, const int len, unsigned char *out) { 
  unsigned int md_len;
  const EVP_MD *md = EVP_sha256();
  EVP_MD_CTX *ctx = EVP_MD_CTX_create(); 
  EVP_DigestInit_ex(ctx, md, NULL);
  EVP_DigestUpdate(ctx, in, len);
  EVP_DigestFinal_ex(ctx, out, &md_len);
  EVP_MD_CTX_destroy(ctx);
  EVP_cleanup();
  return 1;
} 

int doubleHash(const unsigned char *in, const int len,  unsigned char *out) {
  unsigned char intermediateHash[32];
  if(hash(in, len, intermediateHash)) {
    return hash(intermediateHash, 32, out);
  }
  return 0;
}

int testDecryption(const unsigned char *passphrase, const unsigned char *salt, const int rounds, const unsigned char *cipherText, const unsigned char *testPubkey, const unsigned char *keyct, unsigned char *decryptedPrivateKey) {
  unsigned char key[32], iv[16], masterKeyPlainText[32], hash[32];
  if(!bytesToKey(passphrase, salt, rounds, key, iv)) {
    return 1; 
  }
  if (!decrypt(cipherText, (const unsigned char*)key, (const unsigned char*)iv, 48, masterKeyPlainText)) {
    return 1;
  }
  if (!doubleHash(testPubkey, 33, hash)) {
    return 1; 
  }
  if (!decrypt(keyct, masterKeyPlainText, hash, 32, decryptedPrivateKey)) {
    return 1;
  } 
  return 0;
}

int deserializeArray(const unsigned char *input, const unsigned int offset, unsigned char *output, unsigned int maxLen, const unsigned int outputHex, const unsigned int nullTerminate) {
  unsigned int i = 0;
  if (!maxLen) {
    maxLen = (uint8_t)input[offset - 1];
  }
  while(i < maxLen) {
    if (outputHex) {
      char c[] = { ((input[i + offset] & 0xf0) >> 4), (input[i + offset] & 0x0f) };
      for (int j = 0; j < 2; j++) {
        if ((int)c[j] < 10) {
          output[i*2+j] = 0x30 | c[j];
        } else {
          output[i*2+j] = 0x60 | (c[j] - 0x09);
        }
      }
    } else {
      output[i] = input[i + offset];
    }
    i++;
  }
  if (nullTerminate) {
    output[i * (outputHex ? 2 : 1)] = '\0'; 
  }
  return (maxLen);
}
