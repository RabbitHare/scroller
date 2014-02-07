#include <Layer.h>
#include <Renderer.h>
#include <Camera.h>
#include <Tileset.h>
#include <GL/gl.h>
#include <Entity.h>
#include <GameGlobals.h>

void Layer::Update ()
{
	/*
	Entity *ent =  entities;
	for (; ent; ent = ent->next)
		ent->Update ();
	*/
}

void Layer::Render (Camera &camera)
{

	Renderer *ren = GetRenderer();
	int rpass = ren->GetRenderPass();
	Tileset *tset = GetTileset (tileset);
	ren->BindTileset (tileset);

	
	//------ Draw Tiles ---------
	
	// clip rect
	float cw,
	      ch;
	Vec2f s = camera.GetDrawScale (depth + tset->minDepth);
	cw = 1.0/(s.x*tileW);
	ch = 1.0/(s.y*tileH);
	ClipRecti clip;
	float x,y;
	x = (camera.mat.GetOrigin().x*s.x - 1.0) * cw;
	y = (-camera.mat.GetOrigin().y*s.y - 1.0) * ch;
	clip.x1 = (int)x;
	clip.y1 = (int)y;
	clip.x2 = int(x + cw*2.0) + 1;
	clip.y2 = int(y + ch*2.0) + 1;

	clip.x1 = std::max (clip.x1,0);
	clip.y1 = std::max (clip.y1,0);

	clip.x2 = std::min (clip.x2, width);
	clip.y2 = std::min (clip.y2, height);
	if (ren->ShowMask())
		ren->DrawMask (tiles, width, height, tileW, tileH, clip);
	else
		ren->DrawTiles (tiles, width, height, tileW, tileH, clip);

	//---------- Render Entities ---------
	if (rpass == R_ALPHA)
	{
		/*
		Entity *ent = entities;
		for (; ent; ent = ent->next)
			ent->Render (camera);
			*/

		Entity *e;
		e = g_entities;
		while (e)
		{
			if (e->Render)
				e->Render (e, camera);
			e = e->next;
		}
	}
	
	glBindTexture (GL_TEXTURE_2D, tset->texture);
	glColor3f (1.0,1.0,1.0);

	//--------- Draw foreground translucent tile tris
	if (!ren->ShowMask() && rpass == R_ALPHA)
	{
		ren->SetRenderPass (R_ALPHA_FRONT);
		ren->DrawTiles (tiles, width, height, tileW, tileH, clip);
		ren->SetRenderPass (R_ALPHA);
	}

}

