#ifndef __LAYER_H__
#define __LAYER_H__

#include <stddef.h>
#include <Entity.h>
#include <string>

struct Camera;

struct Layer
{
	Layer *next,*prev;
	int tileset;
	Entity* entities;
	std::string name;

	int width,height; // in tiles
	float tileW,tileH;
	float depth; // z position
	int *tiles;
	Layer ()
	{
		tileset = 0;
		entities = NULL;
		tiles = NULL;
		next = prev = NULL;
		tileW = 32.f;
		tileH = 32.f;
		depth = 0.;
		height = width = 0;
		name = "layer";
	}
	~Layer ()
	{
		if (tiles)
			delete[] tiles;
		/*
		Entity *ent = entities,
		       *n;
		while (ent)
		{
			n = ent->next;
			ent->SetLayer (NULL);
			delete ent;
			ent = n;
		}
		*/
	}
	void SetSize (int w, int h)
	{
		if (tiles)
			delete[] tiles;
		tiles = new int[w*h];
		for (int i=0; i < w*h; i++)
			tiles[i] = 0;
		width = w;
		height = h;
	}

	void Update ();
	void Render (Camera &camera);

	void AddEntity (Entity *ent);
	void RemoveEntity (Entity *ent);
};
#endif

