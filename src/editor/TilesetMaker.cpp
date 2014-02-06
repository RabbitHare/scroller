#include <iostream>
#include <libxml/xmlwriter.h>

#include <Tileset.h>
#include <Mesh.h>
#include <ObjLoader.h>
#include <PoolAllocator.h>
#include <List.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <GL/gl.h>
#include <cfloat>

static Tileset tileset;
static SDL_Surface *texImage=NULL;

const char doc[] = {
	"tset-maker [-mesh *.obj] [-mask *.obj] [-texture TEXTURE] [-tsize X Y] [output file]\n\
	\n\
	-tsize   the tile size. Default size is 32x32.\n\
	\
	-- Texture Interpolation Options --\
		-linear  -nearest\n"
};

static std::string tsetName;




static void
WriteTileset (const char *fn)
{
	using namespace std;
	xmlDocPtr doc;
	xmlTextWriterPtr writer;
	int rc;
	stringstream ss;
	string str;

	writer = xmlNewTextWriterDoc (&doc, 0);
	if (writer == NULL)
	{
		cerr << "***WriteTileset: Error creating xml writer\n";
		return;
	}
	rc = xmlTextWriterStartDocument (writer,NULL,NULL,NULL);
	if (rc < 0)
	{
		cerr << "***WriteTileset: Error at xmlTextWritterStartDocument\n";
		return;
	}

	Tileset &tset = tileset;
	xmlTextWriterStartElement (writer, BAD_CAST "tileset");
	xmlTextWriterWriteFormatElement (writer, BAD_CAST "name", "%s", tsetName.c_str());
	// Texture
	xmlTextWriterStartElement (writer, BAD_CAST "texture");
	xmlTextWriterWriteFormatElement (writer, BAD_CAST "name", "%s", tset.texName);
	
	if (tset.texInterp == GL_NEAREST) str = "nearest";
	else str = "linear";
	xmlTextWriterWriteFormatElement (writer, BAD_CAST "interp", "%s", str.c_str());

	xmlTextWriterEndElement (writer); // texture

	xmlTextWriterWriteFormatElement (writer, BAD_CAST "width", "%d", tset.width);
	xmlTextWriterWriteFormatElement (writer, BAD_CAST "height", "%d", tset.height);
	xmlTextWriterWriteFormatElement (writer, BAD_CAST "tileWidth", "%d", (int)tset.tileW);
	xmlTextWriterWriteFormatElement (writer, BAD_CAST "tileHeight", "%d", (int)tset.tileH);

	//tiles
	xmlTextWriterStartElement (writer, BAD_CAST "tiles");
	for (size_t i=0; i < tset.tiles.size(); i++)
	{
		xmlTextWriterStartElement (writer, BAD_CAST "tile");
		Tile &tile = tset.tiles[i];
		xmlTextWriterWriteFormatElement (writer, BAD_CAST "minDepth", "%f", tile.minDepth);
		xmlTextWriterWriteFormatElement (writer, BAD_CAST "maxDepth", "%f", tile.maxDepth);
		xmlTextWriterWriteFormatElement (writer, BAD_CAST "numTris", "%d", tile.numTris);
		xmlTextWriterWriteFormatElement (writer, BAD_CAST "tris", "%d %d %d", tile.tris[0], tile.tris[1], tile.tris[2]);
		xmlTextWriterWriteFormatElement (writer, BAD_CAST "numVerts", "%d", tile.numVerts);
		xmlTextWriterWriteFormatElement (writer, BAD_CAST "verts", "%d", tile.verts);
		xmlTextWriterWriteFormatElement (writer, BAD_CAST "numMaskTris", "%d", tile.numMaskTris);

		ss.seekp(0);
		for (int i=0; i < tile.numMaskTris; i++)
		{
			CTri &tri = tileset.maskTris[tile.maskTris+i];
			ss << '\n';
			for (int v=0; v<3; v++)
			{
				ss << " ( ";
				ss << tri.p[v].x;
				ss << ' ';
				ss << tri.p[v].y;
				ss << " )";
			}

		}
		if (tile.numMaskTris)
			ss << '\n';
		ss << '\0';
		xmlTextWriterWriteFormatElement (writer, BAD_CAST "maskTris", "%s", ss.str().c_str());

		xmlTextWriterEndElement (writer); // tile
	}
	xmlTextWriterEndElement (writer); // tiles
	//mesh
	xmlTextWriterStartElement (writer, BAD_CAST "mesh");
	xmlTextWriterWriteFormatElement (writer, BAD_CAST "numVerts", "%d", (int)tset.verts.size());
	xmlTextWriterWriteFormatElement (writer, BAD_CAST "numTris", "%d", (int)tset.tris.size());

	ss.seekp(0);
	for (size_t i=0; i < tset.verts.size(); i++)
	{
		ss << "( ";
		for (int v=0; v<3; v++)
		{
			ss << tset.verts[i].co.V[v];
			ss << ' ';
		}
		ss << ") ( ";
		for (int n=0; n<3; n++)
		{
			ss << tset.verts[i].no.V[n];
			ss << ' ';
		}
		ss << ") ( ";
		for (int v=0; v<2; v++)
		{
			ss << tset.verts[i].tco.V[v];
			ss << ' ';
		}
		ss << ")\n";

	}
	ss << '\0';
	xmlTextWriterWriteFormatElement (writer, BAD_CAST "verts", "%s", ss.str().c_str());

	ss.seekp(0);
	for (size_t i=0; i < tset.tris.size(); i++)
	{
		ss << tset.tris[i].p[0];
		ss << ' ';
		ss << tset.tris[i].p[1];
		ss << ' ';
		ss << tset.tris[i].p[2];
		ss << "\n";
	}
	ss << '\0';
	xmlTextWriterWriteFormatElement (writer, BAD_CAST "tris", "%s", ss.str().c_str());
	xmlTextWriterEndElement (writer); // mesh
	xmlTextWriterEndElement (writer); // tileset

	xmlFreeTextWriter (writer);
	xmlSaveFormatFile (fn, doc, 1);
	xmlFreeDoc (doc);

	xmlCleanupParser ();
	xmlMemoryDump ();

}
static Vec2f sepNorms[] = {
			Vec2f(1.,0.),
			Vec2f(0.,1.),
			Vec2f(0.),
			Vec2f(0.),
			Vec2f(0.)};
static const int numSepNorms = 5;
static bool TriangleInBoxTest (Vec2f tri[3], Vec2f bxOrigin, Vec2f bxHalfExtents)
{
	float bmin,bmax,
	      tmin,tmax;
	for (int i=0; i < 3; i++)
	{
		// edge norms
		Vec2f tmp = tri[(i+1)%3] - tri[i];
		sepNorms[2+i] = Vec2f (tmp.y, -tmp.x);
	}

	for (int i=0; i < numSepNorms; i++)
	{
		float sepDotsep = sepNorms[i].Dot(sepNorms[i]);
		if (sepDotsep < 0.001)
			continue;
		float epsilon = 0.001 * sepDotsep;
		// box min & max
		float r = Vec2f::Abs(sepNorms[i]).Dot(bxHalfExtents);
		bmax = sepNorms[i].Dot(bxOrigin) + r;
		bmin = sepNorms[i].Dot(bxOrigin) - r;
		// triangle min & max
		tmin = sepNorms[i].Dot(tri[0]);
		tmax = tmin;
		for (int j=1; j < 3; j++)
		{
			float t = sepNorms[i].Dot(tri[j]);
			if (t < tmin) tmin = t;
			else if (t > tmax) tmax = t;
		}
		if (tmax < bmin+epsilon)
		{
			if (tmin+epsilon < bmin)
				return false;
		}
		else if (tmin > bmax-epsilon)
		{
			if (tmax-epsilon > bmax)
				return false;
		}
	}
	// it's inside the box
	return true;
}

static bool
TransparentTriTest (Vec2f p[3])
{
	const int FIXED_FRAC=16;
	const int FIXED_ONE = 1 << FIXED_FRAC;
	Vec2f p1,p2,p3;
	float p1p2Frac,p1p3Frac,p2p3Frac;
	int startFrac1,startFrac2,
	    endFrac1,endFrac2,
	    startFrac,endFrac;
	int startx,starty,
	    endx,endy;
	int startxFix,endxFix;
	// sort by y
	if (p[0].y > p[1].y)
	{
		p1 = p[0];
		p2 = p[1];
	}
	else
	{
		p1 = p[1];
		p2 = p[0];
	}
	if (p[2].y > p1.y)
	{
		p3 = p2;
		p2 = p1;
		p1 = p[2];
	}
	else if (p[2].y > p2.y)
	{
		p3 = p2;
		p2 = p[2];
	}
	else p3 = p[2];

	p1.x = int(p1.x * texImage->w);
	p1.y = int(p1.y * texImage->h);
	p2.x = int(p2.x * texImage->w);
	p2.y = int(p2.y * texImage->h);
	p3.x = int(p3.x * texImage->w);
	p3.y = int(p3.y * texImage->h);

	float div;
	div = p1.y - p2.y;
	if (div == 0.)
		p1p2Frac = p2.x-p1.x;
	else
		p1p2Frac = (p2.x-p1.x)/div;

	div = p1.y - p3.y;
	if (div == 0.)
		return false;
	p1p3Frac = (p3.x-p1.x)/div;
	
	div = p2.y - p3.y;
	if (div == 0.)
		p2p3Frac = 0;
	else 
		p2p3Frac = (p3.x-p2.x)/div;
	if (p1p2Frac < p1p3Frac)
	{
		startFrac1 = int(p1p2Frac * FIXED_ONE);
		endFrac1 = int(p1p3Frac * FIXED_ONE);

		startFrac2 = int(p2p3Frac * FIXED_ONE);
		endFrac2 = endFrac1;
	}
	else
	{
		startFrac1 = int(p1p3Frac * FIXED_ONE);
		endFrac1 = int(p1p2Frac * FIXED_ONE);

		startFrac2 = startFrac1;
		endFrac2 = int(p2p3Frac * FIXED_ONE);
	}
	startFrac = startFrac1;
	endFrac = endFrac1;

	starty = p1.y;
	endy = p2.y;
	startxFix = p1.x*FIXED_ONE;
	endxFix = startxFix;

	if (starty == endy)
	{
		startxFix += startFrac;
		endxFix += endFrac;
	}

	int tx,ty;
	int t=2;
	while (t--)
	{
		for (int y=starty; y > endy; y--)
		{
			startxFix += startFrac;
			endxFix += endFrac;
			startx = startxFix >> FIXED_FRAC;
			endx = endxFix >> FIXED_FRAC;
			for (int x=startx; x < endx; x++)
			{
				tx = x;
				ty = y;
				if (tx < 0)
				{
					tx = -tx & (texImage->w-1);
					tx = texImage->w - tx;
				}
				else tx &= texImage->w-1;

				if (ty < 0)
				{
					ty = -ty & (texImage->h-1);
					ty = texImage->h - ty;
				}
				else ty &= texImage->h-1;

				// examine texel
				int p = ty*texImage->w + tx;
				int alpha;
				alpha = ((uint8_t*)texImage->pixels)[p*4+3];

				if (alpha < 255) return true;

			}
		}
		starty = p2.y;
		endy = p3.y;
		startFrac = startFrac2;
		endFrac = endFrac2;
	}

	return false;
}
static void
BuildTiles (Tileset &tset, TMesh &mesh, TMesh &mask)
{
	
	float	tw = tset.tileW,
		th = tset.tileH;
	float epsilon = 0.01;
	int numTiles,
	    numTris=0,
	    numVerts=0;
	Vec3f min(0.0,0.0,0.0),max(0.0,0.0,0.0);


	//find tileset bounds
	TVert *tve = mesh.verts.First();
	for (; tve != mesh.verts.End(); tve = tve->next)
	{
		for (int i=0; i < 3; i++)
		{
			if (tve->co.V[i] < min.V[i])
				min.V[i] = tve->co.V[i];
			else if (tve->co.V[i] > max.V[i])
				max.V[i] = tve->co.V[i];
		}
	}

	// start at top left corner
	Vec2f start(int(min.V[0]/tw) * tw, int(max.V[1]/th) * th);
	if (min.V[0] < 0 && min.V[0] < start.x-epsilon) start.x -= tw;
	if (max.V[1] > 0 && max.V[1] > start.y+epsilon) start.y += th;
	Vec2f clip;
	float w = (max.x - start.x)/tw;
	float h = -(min.y - start.y)/th;
	if (float((int)w) < w) w+=1.0;
	if (float((int)h) < h) h+=1.0;
	tset.width = (int)w;
	tset.height = (int)h;
	tset.AddClearTile ();
	numTiles = w*h;
	tset.tiles.resize (tset.tiles.size() + numTiles*2);
	
	TMesh tileM;
	int t = 1;
	int x,y;
	for (clip.y=start.y, y=0; y < tset.height; y++,clip.y-=th)
		for (clip.x=start.x, x=0; x < tset.width; x++,clip.x+=tw, t++)
		{
			tileM.Clear();
			for (TVert *v=mesh.verts.First(); v < mesh.verts.End(); v = v->next)
				v->tmp.v = NULL;

			Tile &tile = tset.tiles[t];
			tile.minDepth = 0.;
			tile.maxDepth = 0.;
			tile.verts = tset.verts.size();
			tile.numVerts = 0;
			tile.flip = 0;
			TFace *f = mesh.faces.First();
			for (; f != mesh.faces.End(); f = f->next)
			{

				Vec2f p[3];
				for (int i=0; i < 3; i++)
					p[i] = f->p[i].v->co;

				if (TriangleInBoxTest (p, Vec2f (clip.x+tw*0.5,clip.y-th*0.5), Vec2f (tw*0.5, th*0.5)))
				{
					Vec2f texTri[3];
					// NEW FACE
					TFace *tf = new TFace ();
					tileM.faces.Append (tf);
					for (int i=0; i < 3; i++)
					{
						if (f->p[i].v->tmp.v)
							tf->p[i].v = f->p[i].v->tmp.v;
						else 
						{
							// NEW VERT
							TVert *v = new TVert();
							tileM.verts.Append (v);
							tile.numVerts++;
							tf->p[i].v = v;
							
							// make original mesh face temporarily reference new vert
							f->p[i].v->tmp.v = v;
							// copy data
							v->co = f->p[i].v->co;
							v->tco = f->p[i].v->tco;
							v->no = f->p[i].v->no;

							// make relative to tile pos
							v->co.x -= clip.x;
							v->co.y -= clip.y;
						}
						// link face and vert together
						tf->p[i].f = tf;
						tf->p[i].next = tf->p[i].v->fverts;
						tf->p[i].v->fverts = tf->p+i;

						texTri[i] = tf->p[i].v->tco;
					}

					tile.numTris++;

					// test for transparency
					if (texImage && texImage->format->BytesPerPixel == 4 &&
							TransparentTriTest (texTri))
					{
						tf->flags |= F_ALPHA;
						std::cout << " tile " << t << " has transparent tris\n";
					}
					
				}
			}
			// add verts and tris to tileset
			for (TVert *v=tileM.verts.First(); v < tileM.verts.End(); v = v->next)
			{
				v->tmp.num = tset.verts.size();

				TsetVert vert;
				vert.co = v->co;
				vert.tco = v->tco;
				tset.verts.push_back (vert);
				numVerts++;

			}
			tile.numTris = 0;
			// OPAQUE TRIS
			tile.tris[0] = tileset.tris.size();
			for (TFace *f=tileM.faces.First(); f < tileM.faces.End(); f = f->next)
			{
				if (f->flags & F_ALPHA)
					continue;
				TsetTri tri;
				for (int i=0; i < 3; i++)
					tri.p[i] = f->p[i].v->tmp.num;
				tileset.tris.push_back (tri);
				tile.numTris++;
				numTris++;

			}
			// TRANSLUCENT TRIS
			bool hasAlpha = false;
			// background
			tile.tris[1] = tileset.tris.size();
			for (TFace *f=tileM.faces.First(); f < tileM.faces.End(); f = f->next)
			{
				if ((f->flags & F_ALPHA) == 0)
					continue;
				hasAlpha = true;
				if (f->p[0].v->co.z > 0.) // foreground
					continue;
				TsetTri tri;
				for (int i=0; i < 3; i++)
					tri.p[i] = f->p[i].v->tmp.num;
				tileset.tris.push_back (tri);
				tile.numTris++;
				numTris++;

			}
			// foreground
			tile.tris[2] = tileset.tris.size();
			for (TFace *f=tileM.faces.First(); f < tileM.faces.End(); f = f->next)
			{
				if ((f->flags & F_ALPHA) == 0)
					continue;
				hasAlpha = true;
				if (f->p[0].v->co.z <= 0.) // background
					continue;
				TsetTri tri;
				for (int i=0; i < 3; i++)
					tri.p[i] = f->p[i].v->tmp.num;
				tileset.tris.push_back (tri);
				tile.numTris++;
				numTris++;

			}
			if (hasAlpha) // set all tris in tile to be drawn as translucent
				tile.tris[1] = tile.tris[0];

			tile.tris[R_NUM_PASSES] = tile.tris[0] + tile.numTris;

		}
	// MASK
	t = 1;
	for (clip.y=start.y, y=0; y < tset.height; y++,clip.y-=th)
		for (clip.x=start.x, x=0; x < tset.width; x++,clip.x+=tw, t++)
		{
			Tile &tile = tset.tiles[t];
			tile.numMaskTris = 0;
			tile.maskTris = tset.maskTris.size();
			TFace *f = mask.faces.First();
			for (; f != mask.faces.End(); f = f->next)
			{

				Vec2f p[3];
				for (int i=0; i < 3; i++)
					p[i] = f->p[i].v->co;
				if (TriangleInBoxTest (p, Vec2f (clip.x+tw*0.5,clip.y-th*0.5), Vec2f (tw*0.5,th*0.5)))
				{
					CTri tri;
					for (int i=0; i < 3; i++)
						tri.p[i] = p[i] - clip;
					for (int i=0; i < 3; i++)
					{
						Vec2f n = tri.p[(i+1)%3] - tri.p[i];
						tri.norms[i] = Vec2f (n.y,-n.x);
						tri.norms[i].Normalize();
					}
					tileset.maskTris.push_back (tri);
					tile.numMaskTris++;
				}
			}
		}
	
}
static void
MakeTileset (Tileset &tset, TMesh &mesh, TMesh &mask)
{
	BuildTiles (tset, mesh, mask);


	// find min & max depths
	for (size_t t = 0; t < tset.tiles.size(); t++)
	{
		for (int i = 0; i < tset.tiles[t].numVerts; i++)
		{
			TsetVert &v = tset.verts[tset.tiles[t].verts+i];
			if (v.co.z < tset.tiles[t].minDepth)
				tset.tiles[t].minDepth = v.co.z;
			else if (v.co.z > tset.tiles[t].maxDepth)
				tset.tiles[t].maxDepth = v.co.z;
		}

	}
}
int main (int argc, char *argv[])
{
	TMesh mesh,mask;
	char *texName=NULL;
	bool tsizeSet=false;

	tileset.Clear ();

	if (argc < 6)
	{
		puts (doc);
		return 0;
	}
	
	for (int i=1; i < argc-1; i++)
	{
		if (!strcmp (argv[i],"-mesh"))
		{
			i++;
			if (LoadObj (argv[i],mesh) == false)
				return 1;
		}
		else if(!strcmp (argv[i],"-mask"))
		{
			i++;
			if (LoadObj (argv[i],mask) == false)
				return 1;
		}
		else if(!strcmp (argv[i], "-texture"))
		{
			i++;
			texName = argv[i];
		}
		else if (!strcmp (argv[i], "-tsize"))
		{
			i++;
			tileset.tileW = atof (argv[i]);
			i++;
			tileset.tileH = atof (argv[i]);
			tsizeSet = true;
		}
		else if (!strcmp (argv[i], "-name"))
		{
			i++;
			tsetName = argv[i];
		}
		else if (!strcmp (argv[i], "-linear"))
			tileset.texInterp = GL_LINEAR;
		else if (!strcmp (argv[i], "-nearest"))
			tileset.texInterp = GL_NEAREST;
		else
		{
			std::cout << "unknown argument: " << argv[i] << "\n";
			exit(1);
		}
	}
	if (!tsizeSet) // tile size not specified; use default
	{
		tileset.tileW = 32.0;
		tileset.tileH = 32.0;
	}
	int n = strlen (texName);
	if (n > 63) n = 63;
	strncpy(tileset.texName, texName, n);
	tileset.texName[n] = '\0';
	// load image
	if (texName)
	{
		texImage = IMG_Load (texName);
		if (!texImage)
		{
			std::cerr << "*** IMG_Load: " << IMG_GetError() << std::endl;
		}
	}

	MakeTileset (tileset, mesh, mask);

	WriteTileset (argv[argc-1]);
	if (texImage)
		SDL_FreeSurface (texImage);
	return 0;
}

