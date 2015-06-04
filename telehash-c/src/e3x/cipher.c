#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "e3x.h"
#include "util.h"
#include "tgc.h"

e3x_cipher_t e3x_cipher_sets[CS_MAX];
e3x_cipher_t e3x_cipher_default = NULL;

uint8_t e3x_cipher_init(lob_t options)
{
  e3x_cipher_default = NULL;
  memset(e3x_cipher_sets, 0, CS_MAX * sizeof(e3x_cipher_t));
  
  e3x_cipher_sets[CS_1a] = cs1a_init(options);
  if(e3x_cipher_sets[CS_1a]) e3x_cipher_default = e3x_cipher_sets[CS_1a];
  if(lob_get(options, "err")) return 1;
  tgc_addRoot(e3x_cipher_sets[CS_1a]);

  e3x_cipher_sets[CS_2a] = cs2a_init(options);
  if(e3x_cipher_sets[CS_2a]) e3x_cipher_default = e3x_cipher_sets[CS_2a];
  if(lob_get(options, "err")) return 1;
  tgc_addRoot(e3x_cipher_sets[CS_2a]);

  e3x_cipher_sets[CS_3a] = cs3a_init(options);
  if(e3x_cipher_sets[CS_3a]) e3x_cipher_default = e3x_cipher_sets[CS_3a];
  if(lob_get(options, "err")) return 1;
  tgc_addRoot(e3x_cipher_sets[CS_3a]);

  return 0;
}

void e3x_cipher_free()
{
  if(e3x_cipher_sets[CS_1a]){
    free(e3x_cipher_sets[CS_1a]);
    tgc_rmRoot(e3x_cipher_sets[CS_1a]);
  }
  if(e3x_cipher_sets[CS_2a]){
    free(e3x_cipher_sets[CS_2a]);
    tgc_rmRoot(e3x_cipher_sets[CS_2a]);
  }
  if(e3x_cipher_sets[CS_3a]){
    free(e3x_cipher_sets[CS_3a]);
    tgc_rmRoot(e3x_cipher_sets[CS_3a]);
  }
}

e3x_cipher_t e3x_cipher_set(uint8_t csid, char *str)
{
  uint8_t i;

  for(i=0; i<CS_MAX; i++)
  {
    if(!e3x_cipher_sets[i]) continue;
    if(e3x_cipher_sets[i]->csid == csid) return e3x_cipher_sets[i];
    if(str)
    {
      // hex match
      if(strcasecmp(e3x_cipher_sets[i]->hex,str) == 0) return e3x_cipher_sets[i];
      // if they list alg's they support, match on that too
      if(e3x_cipher_sets[i]->alg && strstr(e3x_cipher_sets[i]->alg,str)) return e3x_cipher_sets[i];
    }
  }

  return NULL;
}

