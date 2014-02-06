#ifndef __TILESET_H__
#define __TILESET_H__

#include <Mesh.h>
#include <Collision.h>
#include <Enums.h>

#include <vector>
#include <libxml/xmlwriter.h>

struct TsetVert
{
	Vec3f co,
	      no;
	Vec2f tco; // texture coord
};
struct TsetTri
{
	int p[3];
};
struct Tile
{
	int numTris,
	    numMaskTris,
	    numVerts;
	int tris[R_NUM_PASSES+1],
	    maskTris,
	    verts;
	float minDepth,maxDepth;

	int flip; // The index of its flipped equivalent if it exists,
	          // else zero. If this is a flipped tile it is
		  // the original tile index negated.
};
class Tileset
{
public:
	unsigned int texture;
	char name[64];
	char texName[64];
	int texInterp;
	float tileW,tileH;
	int width,height; // in tiles

	std::vector<TsetTri> tris;
	std::vector<TsetVert> verts;
	
	std::vector<CTri> maskTris;
	std::vector<Tile> tiles;

	float minDepth,
	      maxDepth;

	Tileset ();
	~Tileset ();
	void Clear ();
	void AddClearTile ();
	// adds a flipped tile
	// parameter: t = index to a tile
	void AddFlippedTile (int t);

	bool LoadFile (const char *fn);
private:
	void ParseTile (xmlDocPtr doc, xmlNodePtr node);
	void ParseMesh (xmlDocPtr doc, xmlNodePtr node);
};

int AddTileset();
Tileset* GetTileset(int index);
int GetNumTilesets ();
void PurgeTilesets ();

#endif

