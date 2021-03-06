#include "ext.h"
#include "net_loopback.h"
#include "unit_test.h"

int main(int argc, char **argv)
{
  mesh_t meshA = mesh_new(3);
  fail_unless(meshA);
  lob_t secretsA = mesh_generate(meshA);
  fail_unless(secretsA);
  fail_unless(ext_stream(meshA));

  mesh_t meshB = mesh_new(3);
  fail_unless(meshB);
  lob_t secretsB = mesh_generate(meshB);
  fail_unless(secretsB);
  fail_unless(ext_stream(meshB));

  net_loopback_t pair = net_loopback_new(meshA,meshB);
  fail_unless(pair);
  
  link_t linkAB = link_get(meshA, meshB->id->hashname);
  link_t linkBA = link_get(meshB, meshA->id->hashname);
  fail_unless(linkAB);
  fail_unless(linkBA);

  fail_unless(link_resync(linkAB));

//  fail_unless(ext_link_status(linkAB,NULL));
//  fail_unless(ext_link_status(linkBA,NULL));

  return 0;
}

