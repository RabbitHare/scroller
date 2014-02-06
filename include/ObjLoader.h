#ifndef __OBJ_LOADER__
#define __OBJ_LOADER_

#include <TMesh.h>

struct Mesh;

bool LoadObj (const char *fd, Mesh *mesh);
bool LoadObj (const char *fd, TMesh &mesh);
#endif

