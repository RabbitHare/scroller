#include <Renderer.h>
#include <GL/gl.h>

#include <Tileset.h>

Renderer *renderer=NULL;

Renderer::Renderer ()
{
	mRenderPass = R_OPAQUE;
	mTileset = 0;
	mShowMask = false;
}
void Renderer::DrawTiles (int tiles[], int w, int h, float tileW, float tileH, ClipRecti &clip)
{
	Tileset *tset = GetTileset (mTileset);

	int t;
	Vec2f offset ((float)clip.x1*tileW, -(float)clip.y1*tileH);
	int tris,numTris;

	int endx = clip.x2,
	    endy = clip.y2;
	Vec2f tran(0.,0.);
	glTranslatef (offset.x, offset.y, 0.);
	for (int y=clip.y1; y < endy; y++)
	{
		float trX=0.;
		for (int x=clip.x1; x < endx; x++)
		{
			t = y*w+x;
			Tile &tile = tset->tiles[tiles[t]];
			tris = tile.tris[mRenderPass];
			numTris = tile.tris[mRenderPass+1] - tris;
			if (numTris)
			{
				glTranslatef (trX, 0., 0.);
				tran.x += trX;
				trX = 0.;

				glBegin (GL_TRIANGLES);
				for (int i = 0; i < numTris; i++)
				{
					TsetTri &tri = tset->tris[tris+i];
					for (int j = 0; j < 3; j++)
					{

						glTexCoord2fv (tset->verts[tri.p[j]].tco.V);
						glVertex3fv (tset->verts[tri.p[j]].co.V);
					}
				}
				glEnd ();
			}
			trX += tileW;

		}
		glTranslatef(-tran.x, -tileH, 0.0);
		tran.y += -tileH;
		tran.x = 0.;
	}
	glTranslatef (-tran.x - offset.x,-tran.y - offset.y, 0.);

}
void Renderer::DrawMask (int tiles[], int w, int h, float tileW, float tileH, ClipRecti &clip)
{
	Tileset *tset = GetTileset (mTileset);

	int t;
	Vec2f offset ((float)clip.x1*tileW, -(float)clip.y1*tileH);

	int endx = clip.x2,
	    endy = clip.y2;
	Vec2f tran(0,0);
	glTranslatef (offset.x, offset.y, 0.);
	for (int y=clip.y1; y < endy; y++)
	{
		float trX=0.;
		for (int x=clip.x1; x < endx; x++)
		{
			t = y*w+x;
			Tile &tile = tset->tiles[tiles[t]];
			if (tile.numMaskTris)
			{
				glTranslatef (trX, 0., 0.);
				tran.x += trX;
				trX = 0.;

				glBegin (GL_TRIANGLES);
				for (int i = 0; i < tile.numMaskTris; i++)
				{
					CTri &tri = tset->maskTris[tile.maskTris+i];
					for (int j = 0; j < 3; j++)
					{
						glVertex2fv (tri.p[j].V);
					}
				}
				glEnd ();

			}
			trX += tileW;

		}
		glTranslatef(-tran.x, -tileH, 0.0);
		tran.y += -tileH;
		tran.x = 0.;
	}
	glTranslatef (-tran.x - offset.x,-tran.y - offset.y, 0.);

}

void SetRenderer (Renderer *ren)
{
	renderer = ren;
}
Renderer* GetRenderer ()
{
	return renderer;
}
