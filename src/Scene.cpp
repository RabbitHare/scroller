#include <Scene.h>
#include <Entity.h>
#include <Camera.h>
#include <GL/gl.h>
#include <libxml/xmlwriter.h>

static Scene scene;

Scene* GetScene ()
{
	return &scene;

}
void Scene::Render (const Camera &cam)
{
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	Vec3f ctran = cam.mat.GetTranslation();
	glTranslatef(-ctran.V[0],-ctran.V[1],-ctran.V[2]);
	
	map.Render (cam);
}
void Scene::CollisionCheck (Entity &entity, CollisionResult &result)
{
	map.CollisionCheck (entity,result);
}

void Scene::Clear ()
{
	map.Clear();
};
void Scene::MakeNew ()
{
	Clear ();
	map.layers.Append (new Layer (256,64));
	filename = "Untitled";
}

static Layer*
ParseLayer (xmlDocPtr doc, xmlNodePtr node)
{
	using namespace std;
	Layer *lay = new Layer();
	xmlChar *value;
	char* tdata=NULL;
	xmlNodePtr tnode;
	node = node->xmlChildrenNode;

	while (node)
	{

		if (!xmlStrcmp (node->name, BAD_CAST "width"))
		{
			value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
			lay->width = atoi ((char*)value);
			if (value)
				xmlFree (value);
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "height"))
		{
			value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
			lay->height = atoi ((char*)value);
			if (value)
				xmlFree (value);
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "depth"))
		{
			value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
			lay->depth = atof ((char*)value);
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
					lay->tileW = atof ((char*)value);
					if (value)
						xmlFree (value);
				}
				else if (!xmlStrcmp (tnode->name, BAD_CAST "height"))
				{
					value = xmlNodeListGetString (doc, tnode->xmlChildrenNode, 1);
					lay->tileH = atof ((char*)value);
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
		if (lay->tiles)
			delete[] lay->tiles;
		lay->tiles = new int[lay->width*lay->height];
		istringstream ss(tdata);
		int end = lay->width*lay->height;
		for (int i=0; i < end; i++)
		{
			ss >> lay->tiles[i];

			ss.get(); // eat comma
		}
		xmlFree (tdata);
	}

	return lay;
}
/*
=============
 Load
=============
*/
void 
Scene::Load (const char* fn)
{
	using namespace std;


	xmlDocPtr doc;
	xmlNodePtr root,
		   node;
	xmlChar *value;

	doc = xmlParseFile (fn);
	if (doc == NULL)
	{
		cerr << "Could not load level: " << fn << "\n";
		return;
	}
	root = xmlDocGetRootElement (doc);
	if (root == NULL)
	{
		cerr << "Empty level file: " << fn << "\n";
		xmlFreeDoc (doc);
		return;
	}

	node = root->xmlChildrenNode;
	while (node)
	{
		if (!xmlStrcmp (node->name, BAD_CAST "tileset"))
		{
			value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
			map.tileset.LoadFile ((char*)value);
			xmlFree (value);
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "layer"))
		{
			Layer *lay;
			if ((lay = ParseLayer (doc, node)))
			{
				map.layers.Append (lay);
			}
		}
		node = node->next;
	}
	// handle flipped tiles
	Layer *lay = map.layers.First();
	for (; lay != map.layers.End(); lay = lay->next)
		for (int i = 0; i < lay->width*lay->height; i++)
		{
			int t = lay->tiles[i];
			if (t < 0) // a flipped tile
			{
				t = -t;
				if (!map.tileset.tiles[t].flip)
					map.tileset.AddFlippedTile (t);
				t = map.tileset.tiles[t].flip;
			}
			lay->tiles[i] = t;
		}
	
	filename = fn;

	xmlFreeDoc (doc);
	xmlCleanupParser ();
	xmlMemoryDump ();
}
/*
==============
 Save
==============
*/
void
Scene::Save (const char *fn)
{
	using namespace std;
	xmlDocPtr doc;
	xmlTextWriterPtr writer;
	int rc;
	stringstream ss;

	writer = xmlNewTextWriterDoc (&doc, 0);
	if (writer == NULL)
	{
		cerr << "***SaveLevel: Error creating xml writer\n";
		return;
	}
	rc = xmlTextWriterStartDocument (writer,NULL,NULL,NULL);
	if (rc < 0)
	{
		cerr << "***SaveLevel: Error at xmlTextWritterStartDocument\n";
		return;
	}

	xmlTextWriterStartElement (writer, BAD_CAST"level");


	
	// tileset
	xmlTextWriterWriteFormatElement (writer, BAD_CAST "tileset", "%s", map.tileset.name);
	// layers
	Layer *lay = map.layers.First();
	for (;lay != map.layers.End(); lay=lay->next)
	{
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
		for (int y=0; y < lay->height; y++)
		{
			for (int x=0; x < lay->width; x++)
			{
				int t = lay->tiles[y*lay->width+x];
				if (map.tileset.tiles[t].flip < 0) // flipped tile
					t = map.tileset.tiles[t].flip;
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
	xmlTextWriterEndElement (writer); // level

	filename = fn;

	xmlFreeTextWriter (writer);
	xmlSaveFormatFile (fn, doc, 1);
	xmlFreeDoc (doc);

	xmlCleanupParser ();
	xmlMemoryDump ();


}

