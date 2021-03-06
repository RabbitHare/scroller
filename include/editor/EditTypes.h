#ifndef __TILE_SELECT_H__
#define __TILE_SELECT_H__

#include <Tileset.h>

struct TileSelection
{
	int *tiles;
	int width,height;
	int x,y;
	int offsetX,offsetY;
	int size;
	float minDepth,maxDepth;
	TileSelection ()
	{
		size = 1;
		tiles = new int[size];
		tiles[0] = 0;
		width = height = 1;
		offsetX = offsetY = 1;
		minDepth = maxDepth = 0;
	}
	void Clear ()
	{
		offsetX = 1;
		offsetY = 1;
		width = 1;
		height = 1;
		tiles[0] = 0;
		x = y = 0;
		minDepth = maxDepth = 0;
	}
	TileSelection& operator= (const TileSelection &sel)
	{
		// doesn't copy size or tiles
		x = sel.x;
		y = sel.y;
		width = sel.width;
		height = sel.height;
		offsetX = sel.offsetX;
		offsetY = sel.offsetY;
		minDepth = sel.minDepth;
		maxDepth = sel.maxDepth;
		
		return *this;
	}

};
#endif

