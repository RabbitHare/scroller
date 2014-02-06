#include <Map.h>
#include <Tileset.h>
#include <Camera.h>
#include <GL/gl.h>
#include <Entity.h>
#include <Config.h>
#include <Renderer.h>
#include <Entity2.h>
#include <GameGlobals.h>

#include <libxml/xmlwriter.h>


static Map map;
Map* GetMap ()
{
	return &map;
}

struct debug_t
{
	CTri ctri;
};
static debug_t debug;

Map::Map ()
{
//	Clear ();
}
Map::~Map ()
{
	//Clear ();
}
void Map::Clear ()
{
	if (layers.size())
		layers.clear();
	if (GetNumTilesets())
		PurgeTilesets();
}	
void Map::CollisionCheck (Entity2 &entity, CollisionResult &result)
{
	Layer *lay = &layers[0]; //FIXME
	Tileset &tileset = *GetTileset (lay->tileset);
	result.hasCollided = false;	
	result.fraction = 1000.0;
	result.normal = Vec3f(0,0,0);
	if (!lay)
		return;
	Vec3f start = entity.pos,
	      end = entity.nextPos;
	Vec2f aabb = entity.bounds.half;
	int minX,maxX,
	    minY,maxY;
	Vec3f min = start;
	Vec3f max = end;
	for (int i=0;i < 3;i++)
		if (max.V[i] < min.V[i])
		{
			float t = min.V[i];
			min.V[i] = max.V[i];
			max.V[i] = t;
		}
	min.x -= aabb.x;
	min.y -= aabb.y;
	max.x += aabb.x;
	max.y += aabb.y;

	minX = int (min.x)/32;
	minY = int (-max.y)/32;
	maxX = int (max.x+32.0)/32;
	maxY = int (-min.y+32.0)/32;
	minX = std::max (minX,0);
	minY = std::max (minY,0);
	maxX = std::min (maxX,lay->width);
	maxY = std::min (maxY,lay->height);
	CTri tri;
	for (int y = minY; y < maxY; y++)
		for (int x = minX; x < maxX; x++)
		{
			Tile &tile = tileset.tiles[lay->tiles[y*lay->width+x]];
			Vec3f tilePos(float(x*32),-float(y*32),0.0);
			for (int f = 0; f < tile.numMaskTris; f++)
			{
				
			 	for (int v=0; v < 3; v++)
				{
					int i = tile.maskTris+f;
					tri.p[v] = tileset.maskTris[i].p[v]+tilePos;
					tri.norms[v] = tileset.maskTris[i].norms[v];
				}
				CollisionResult res;
				HalfAabbCollisionTriangle (entity.bounds, start, end, tri, res);
				if (res.hasCollided)
				{

					if (res.fraction < result.fraction)
					{
						debug.ctri = tri;
						result = res;
					}
				}
						
			}
		}
	if (!result.hasCollided)
		result.fraction = 1.0;
	
}
void Map::Update ()
{
	for (size_t i=0; i < layers.size(); i++)
		layers[i].Update();
	Entity2 *e;
	e = g_entities;
	//remove entities flagged for deletion first
	while (e)
	{
		Entity2 *n = e->next;
		if (e->destroy)
			DeleteEntity (e);
		e = n;
	}
	e = g_entities;
	while (e)
	{
		e->age += PHYSICS_FRAME_LEN;
		if (e->Update)
		{
			e->Update (e);
		}
		e = e->next;
	}
}

void Map::Render (Camera &camera)
{
	if (layers.size()==0) return;
	Layer &lay = layers[0];
	Tileset &tileset = *GetTileset (lay.tileset);

	Renderer *renderer = GetRenderer();
	bool mask = renderer->ShowMask();

	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	Vec3f ctran = camera.mat.GetTranslation();
	glTranslatef(-ctran.V[0],-ctran.V[1],-ctran.V[2]);
	
	
	if (!mask)
	{
		glEnable (GL_TEXTURE_2D);
		glBindTexture (GL_TEXTURE_2D, tileset.texture);
		glColor3f (1.0,1.0,1.0);
	}


	
	renderer->SetRenderPass (R_OPAQUE);
	lay.Render (camera);

	renderer->SetRenderPass (R_ALPHA);
	glEnable (GL_BLEND); glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask (0);
	lay.Render (camera);
	glDepthMask (1);
	glDisable (GL_BLEND);
	
	if (!mask)
	{
		glDisable (GL_TEXTURE_2D);
	}
	if (mask)
	{
		// draw the triangle that was last collided with
		glDisable (GL_DEPTH_TEST);
		glColor3f (0.,0.,1.);
		glBegin (GL_TRIANGLES);
		for (int i=0; i < 3; i++)
			glVertex2f (debug.ctri.p[i].x,debug.ctri.p[i].y);
		glEnd ();
		// draw normals
		glColor3f (0.,0.,0.);
		glBegin (GL_LINES);
		for (int i=0; i < 3; i++)
		{
			Vec3f p1 = (debug.ctri.p[(i+1)%3] + debug.ctri.p[i])*0.5;
			Vec3f p2 = p1 + debug.ctri.norms[i]*32.0;
			glVertex2f (p1.x,p1.y);
			glVertex2f (p2.x,p2.y);
		}
		glEnd ();

		glEnable (GL_DEPTH_TEST);
	}
}


void Map::MakeNew ()
{
	Clear ();
	layers.push_back (Layer ());
	Layer &lay = layers.back();
	lay.SetSize(256, 64);

	AddTileset ();
	GetTileset(0)->AddClearTile ();
	GetTileset(0)->width = 1;
	GetTileset(0)->height = 1;
	filename = "Untitled";
}

static void
ParseLayer (Layer &lay, xmlDocPtr doc, xmlNodePtr node)
{
	using namespace std;
	xmlChar *value;
	char* tdata=NULL;
	xmlNodePtr tnode;
	node = node->xmlChildrenNode;

	while (node)
	{

		if (!xmlStrcmp (node->name, BAD_CAST "width"))
		{
			value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
			lay.width = atoi ((char*)value);
			if (value)
				xmlFree (value);
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "height"))
		{
			value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
			lay.height = atoi ((char*)value);
			if (value)
				xmlFree (value);
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "depth"))
		{
			value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
			lay.depth = atof ((char*)value);
			if (value)
				xmlFree (value);
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "tile"))
		{
			tnode = node->xmlChildrenNode;
			while (tnode)
			{
				if (!xmlStrcmp (tnode->name, BAD_CAST "width"))
				{
					value = xmlNodeListGetString (doc, tnode->xmlChildrenNode, 1);
					lay.tileW = atof ((char*)value);
					if (value)
						xmlFree (value);
				}
				else if (!xmlStrcmp (tnode->name, BAD_CAST "height"))
				{
					value = xmlNodeListGetString (doc, tnode->xmlChildrenNode, 1);
					lay.tileH = atof ((char*)value);
					if (value)
						xmlFree (value);
				}
				else if (!xmlStrcmp (tnode->name, BAD_CAST "data"))
				{
					tdata = (char*)xmlNodeListGetString (doc, tnode->xmlChildrenNode, 1);
				}
				tnode = tnode->next;
			}
		}
		node = node->next;
	}
	if (tdata)
	{
		if (lay.tiles)
			delete[] lay.tiles;
		lay.tiles = new int[lay.width*lay.height];
		istringstream ss(tdata);
		int end = lay.width*lay.height;
		for (int i=0; i < end; i++)
		{
			ss >> lay.tiles[i];

			ss.get(); // eat comma
		}
		xmlFree (tdata);
	}

}
/*
=============
 Load
=============
*/
void 
Map::Load (xmlDocPtr doc, const char* fn)
{
	xmlNodePtr root, node;
	xmlChar *value;

	root = xmlDocGetRootElement (doc);
	node = root->xmlChildrenNode;
	while (node)
	{
		if (!xmlStrcmp (node->name, BAD_CAST "tilesets"))
		{
			xmlNodePtr tnode = node->xmlChildrenNode;
			while (tnode)
			{
				if (!xmlStrcmp (tnode->name, BAD_CAST "tileset"))
				{
					value = xmlNodeListGetString (doc, tnode->xmlChildrenNode, 1);
					Tileset* tset = GetTileset (AddTileset());
					tset->LoadFile ((char*)value);
					if (value)
						xmlFree (value);
				}

				tnode = tnode->next;
			}
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "layer"))
		{
			layers.push_back (Layer());
			ParseLayer (layers.back(),doc, node);
		}
		node = node->next;
	}
	// handle flipped tiles
	for (size_t l = 0; l < layers.size(); l++)
	{
		Layer &lay = layers[0];
		Tileset &tileset = *GetTileset (lay.tileset);
		for (int i = 0; i < lay.width*lay.height; i++)
		{
			int t = lay.tiles[i];
			if (t < 0) // a flipped tile
			{
				t = -t;
				if (!tileset.tiles[t].flip)
					tileset.AddFlippedTile (t);
				t = tileset.tiles[t].flip;
			}
			lay.tiles[i] = t;
		}
	}
	filename = fn;

}
/*
==============
 Save
==============
*/
void
Map::Save (xmlTextWriterPtr writer, const char *fn)
{
	using namespace std;

	stringstream ss;


	
	// tilesets
	xmlTextWriterStartElement (writer, BAD_CAST "tilesets");
	int numTsets = GetNumTilesets ();
	for (int i = 0; i < numTsets; i++)
		xmlTextWriterWriteFormatElement (writer, BAD_CAST "tileset", "%s", GetTileset(i)->name);
	xmlTextWriterEndElement (writer); // tilesets
	// layers
	for (size_t l=0; l < layers.size(); l++)
	{
		Layer *lay = &layers[l];
		xmlTextWriterStartElement (writer, BAD_CAST "layer");
		// width height
		xmlTextWriterWriteFormatElement (writer, BAD_CAST "width", "%d", lay->width);
		xmlTextWriterWriteFormatElement (writer, BAD_CAST "height", "%d", lay->height);
		xmlTextWriterWriteFormatElement (writer, BAD_CAST "depth", "%f", lay->depth);
		// tiles
		xmlTextWriterStartElement (writer, BAD_CAST "tile");
		xmlTextWriterWriteFormatElement (writer, BAD_CAST "width", "%f", lay->tileW);
		xmlTextWriterWriteFormatElement (writer, BAD_CAST "height", "%f", lay->tileH);
		// data
		ss.seekp(0);
		ss << '\n';
		Tileset &tileset = *GetTileset (lay->tileset);
		for (int y=0; y < lay->height; y++)
		{
			for (int x=0; x < lay->width; x++)
			{
				int t = lay->tiles[y*lay->width+x];
				if (tileset.tiles[t].flip < 0) // flipped tile
					t = tileset.tiles[t].flip;
				ss << t;
				ss << ',';
			}
			ss << '\n';
		}
		ss << '\0';
		xmlTextWriterWriteElement (writer, BAD_CAST "data", BAD_CAST ss.str().c_str());
		xmlTextWriterEndElement (writer); // tile
		

		xmlTextWriterEndElement (writer); // layer
	}

	filename = fn;



}

