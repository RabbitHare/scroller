#include <Tileset.h>
#include <Texture.h>
#include <sstream>
#include <vector>
#include <string.h>

static std::vector<Tileset> tilesets;

int AddTileset ()
{
	tilesets.push_back (Tileset());
	return tilesets.size()-1;
}
Tileset* GetTileset (int index)
{
	return &tilesets[index];
}
int GetNumTilesets ()
{
	return tilesets.size();
}

void PurgeTilesets ()
{
	tilesets.clear();
}

/*
================
 Clear
================
*/
Tileset::Tileset ()
{
	texture = 0;
	Clear ();
	AddClearTile ();
}
Tileset::~Tileset()
{
	Clear();
}

void
Tileset::Clear ()
{
	// free texture
	if (texture)
	{
		glDeleteTextures (1, &texture);
		texture = 0;
	}
	name[0] = '\0';
	texName[0] = '\0';
	texInterp = GL_LINEAR;
	tileW = tileH = 32.;
	width = height = 0;
	verts.clear();
	tris.clear();
	maskTris.clear();
	tiles.clear();

}
void Tileset::AddClearTile ()
{
	Tile t;
	t.numTris = 0;
	for (int i=0; i < R_NUM_PASSES+1; i++)
		t.tris[i] = 0;
	t.numVerts = 0;
	t.verts = 0;
	t.numMaskTris = 0;
	t.minDepth = t.maxDepth = 0.f;
	t.flip = 0;
	tiles.push_back (t);
}
/*
====================
 AddFlippedTile
====================
*/
void Tileset::AddFlippedTile (int t)
{
	float tw = tileW * 0.5;
	// create flipped verts and tris
	int nVerts = verts.size();
	int nTris = tris.size();

	verts.resize (verts.size() + tiles[t].numVerts);
	int v = tiles[t].verts,
	    tr = tiles[t].tris[0];
	for (int i=0; i < tiles[t].numVerts; i++)
	{
		Vec3f co = verts[v+i].co;
		co.x = tw - (co.x - tw);
		verts[i+nVerts].co = co;
		verts[i+nVerts].tco = verts[v+i].tco;
		verts[i+nVerts].no = verts[v+i].no;
	}

	tris.resize (tris.size() + tiles[t].numTris);
	for (int i=0; i < tiles[t].numTris; i++)
		for (int j=0; j < 3; j++)
			tris[i+nTris].p[2-j] = nVerts + tris[tr+i].p[j] - v;
	//flip mask
	int nMTris = maskTris.size();
	int mtris = tiles[t].maskTris;
	maskTris.resize (maskTris.size() + tiles[t].numMaskTris);
	for (int i=0; i < tiles[t].numMaskTris; i++)
		for (int j=0; j < 3; j++)
		{
			Vec2f co = maskTris[i+mtris].p[j];
			co.x = tw - (co.x - tw);
			maskTris[i+nMTris].p[2-j] = co;

			co = maskTris[i+mtris].norms[(4-j)%3];
			co.x = -co.x;
			maskTris[i+nMTris].norms[j] = co;
		}

	Tile ft;
	ft.minDepth = tiles[t].minDepth;
	ft.maxDepth = tiles[t].maxDepth;
	for (int i=0; i < R_NUM_PASSES+1; i++)
		ft.tris[i] = tiles[t].tris[i] - tiles[t].tris[0] + nTris;

	ft.numTris = tiles[t].numTris;
	ft.verts = nVerts;
	ft.numVerts = tiles[t].numVerts;
	ft.maskTris = nMTris;
	ft.numMaskTris = tiles[t].numMaskTris;
	ft.flip = -t;
	tiles[t].flip = tiles.size();

	tiles.push_back (ft);
}

void
Tileset::ParseTile (xmlDocPtr doc, xmlNodePtr node)
{
	using namespace std;
	node = node->xmlChildrenNode;
	xmlChar *value;
	char *mtris=NULL;
	Tile tile;
	tile.maskTris = maskTris.size();
	tile.numTris = 0;
	tile.numMaskTris = 0;
	tile.flip = 0;

	while (node)
	{
		value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
		if (!xmlStrcmp (node->name, BAD_CAST "minDepth"))
			tile.minDepth = atof ((char*)value);
		else if (!xmlStrcmp (node->name, BAD_CAST "maxDepth"))
			tile.maxDepth = atof ((char*)value);
		else if (!xmlStrcmp (node->name, BAD_CAST "numTris"))
			tile.numTris = atoi ((char*)value);
		else if (!xmlStrcmp (node->name, BAD_CAST "tris"))
		{
			char *pch = strtok ((char*)value, " ");
			for (int i=0; i<R_NUM_PASSES; i++)
			{
				tile.tris[i] = atoi (pch);
				pch = strtok (NULL, " ");
			}
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "numVerts"))
			tile.numVerts = atoi ((char*)value);
		else if (!xmlStrcmp (node->name, BAD_CAST "verts"))
			tile.verts = atoi ((char*)value);
		else if (!xmlStrcmp (node->name, BAD_CAST "numMaskTris"))
			tile.numMaskTris = atoi ((char*)value);
		else if (!xmlStrcmp (node->name, BAD_CAST "maskTris"))
			mtris = (char*)xmlNodeListGetString (doc, node->xmlChildrenNode, 1);

		if (value) xmlFree (value);
		node = node->next;
	}
	
	if (mtris)
	{
		istringstream ss (mtris);
		for (int i=0; i < tile.numMaskTris; i++)
		{
			CTri tri;
			for (int j=0; j < 3; j++)
			{
				while (ss.get() != '(') { };
				ss >> tri.p[j].x;
				ss >> tri.p[j].y;
			}
			// find the normals
			for (int j=0; j < 3; j++)
			{
				Vec2f n = tri.p[(j+1)%3] - tri.p[j];
				tri.norms[j] = Vec2f (n.y,-n.x);
				tri.norms[j].Normalize();
			}
			maskTris.push_back (tri);
		}
		xmlFree (mtris);
	}

	tile.tris[R_NUM_PASSES] = tile.tris[0] + tile.numTris;

	tiles.push_back (tile);


}
void
Tileset::ParseMesh (xmlDocPtr doc, xmlNodePtr node)
{
	using namespace std;
	char *vstr=NULL,*tstr=NULL;
	node = node->xmlChildrenNode;
	xmlChar *value;
	while (node)
	{
		if (!xmlStrcmp (node->name, BAD_CAST "numVerts"))
		{
			value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
			int n = atoi ((char*)value);
			verts.resize (n);
			if (value) xmlFree (value);
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "numTris"))
		{
			value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);

			int n = atoi ((char*)value);
			tris.resize (n);

			if (value) xmlFree (value);
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "verts"))
		{
			vstr = (char*)xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "tris"))
		{
			tstr = (char*)xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
		}
		node = node->next;
	}
	// verts
	if (vstr)
	{
		istringstream ss(vstr);
		for (size_t i=0; i < verts.size(); i++)
		{
			while (ss.get() != '(') { };
			for (int v=0; v < 3; v++)
			{
				ss >> verts[i].co.V[v];
			}
			while (ss.get() != '(') { };
			for (int v=0; v < 3; v++)
			{
				ss >> verts[i].no.V[v];
			}
			while (ss.get() != '(') { };
			for (int v=0; v < 2; v++)
			{
				ss >> verts[i].tco.V[v];
			}
		}
		xmlFree (vstr);
	}
	// Triangles
	if (tstr)
	{
		istringstream ss(tstr);
		for (size_t i=0; i < tris.size(); i++)
		{
			for (int v=0; v < 3; v++)
			{
				ss >> tris[i].p[v];
			}
		}
		xmlFree (tstr);
	}

}
bool Tileset::LoadFile (const char *fn)
{
	using namespace std;
	xmlDocPtr doc;
	xmlNodePtr root,
		   node;

	// start clean
	Clear ();

	doc = xmlParseFile (fn);
	if (doc == NULL)
	{
		cerr << "Could not load tileset: " << fn << "\n";
		return false;
	}
	root = xmlDocGetRootElement (doc);
	if (root == NULL)
	{
		cerr << "Empty tileset file: " << fn << "\n";
		xmlFreeDoc (doc);
		return false;
	}

	node = root->xmlChildrenNode;
	
	xmlChar *value;
	while (node)
	{
		if (!xmlStrcmp (node->name, BAD_CAST "texture"))
		{
			xmlNodePtr tnode = node->xmlChildrenNode;
			while (tnode)
			{
				value = NULL;
				if (!xmlStrcmp (tnode->name, BAD_CAST "name"))
				{
					value = xmlNodeListGetString (doc, tnode->xmlChildrenNode, 1);
					int n = strlen ((char*)value);
					if (n > 63) n = 63;
					strncpy (texName, (char*)value, n);
					texName[n] = '\0';
				}
				else if (!xmlStrcmp (tnode->name, BAD_CAST "interp"))
				{
					value = xmlNodeListGetString (doc, tnode->xmlChildrenNode, 1);
					if (!strcmp ((char*)value, "nearest"))
						texInterp = GL_NEAREST;
					else texInterp = GL_LINEAR;
				}
				if (value)
					xmlFree (value);

				tnode = tnode->next;
			}
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "width"))
		{
			value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
			width = atoi ((char*)value);
			if (value) xmlFree (value);
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "height"))
		{
			value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
			height = atoi ((char*)value);
			if (value) xmlFree (value);
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "tileWidth"))
		{
			value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
			tileW = atoi ((char*)value);
			if (value) xmlFree (value);
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "tileHeight"))
		{
			value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
			tileH = atoi ((char*)value);
			if (value) xmlFree (value);
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "tiles"))
		{
			xmlNodePtr tnode = node->xmlChildrenNode;
			while (tnode)
			{
				if (!xmlStrcmp (tnode->name, BAD_CAST "tile"))
				{
					ParseTile (doc, tnode);
				}
				tnode = tnode->next;
			}
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "mesh"))
		{
			ParseMesh (doc, node);
		}

		node = node->next;
	}

	// find min and max depths
	minDepth = maxDepth = 0.;
	for (size_t i=0; i < tiles.size(); i++)
	{
		if (tiles[i].minDepth < minDepth)
			minDepth = tiles[i].minDepth;
		if (tiles[i].maxDepth > maxDepth)
			maxDepth = tiles[i].maxDepth;
	}
	// set tileset name
	int len = strlen (fn);
	if (len > 63) len = 63;
	strncpy (this->name, fn, len);
	this->name[len] = '\0';
	// load texture
	if (texName[0])
		texture = LoadTexture (texName,texInterp);

	xmlFreeDoc (doc);
	xmlCleanupParser ();
	xmlMemoryDump ();
	return true;
}


