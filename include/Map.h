#ifndef __MAP_H__
#define __MAP_H__

#include <stddef.h>
#include <Tileset.h>
#include <Collision.h>
#include <Geom.h>
#include <vector>
#include <Layer.h>

struct Camera;
struct Entity;



class Map
{
public:
	std::vector<Layer> layers;
	std::string filename;


	Map ();
	~Map ();
	void Clear ();
	void CollisionCheck (Entity &entity,CollisionResult &result);
	void Update ();
	void Render (Camera &camera);

	// make a new empty map
	void MakeNew ();
	// laod from file
	void Load (xmlDocPtr doc, const char* fn);
	// save to file
	void Save (xmlTextWriterPtr writer, const char *fn);
};

Map* GetMap ();
#endif

