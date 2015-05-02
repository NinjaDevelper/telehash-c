#include "sha256.h"
#include "boehm_gc.h"

void hmac_256(const unsigned char *key, size_t keylen, const unsigned char *input, size_t ilen, unsigned char output[32])
{
  sha256_hmac(key, keylen, input, ilen, output, 0);
}
