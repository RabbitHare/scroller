#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <Geom.h>
#include <Enums.h>



class Renderer
{
public:
	inline void BindTileset (int tset){ mTileset = tset; }
	inline int GetBoundTileset () { return mTileset; }

	inline void SetRenderPass (int pass) { mRenderPass = pass;}
	inline int GetRenderPass () { return mRenderPass; }

	inline void SetShowMask (bool show){ mShowMask = show; }
	inline bool ShowMask (){ return mShowMask; }

	Renderer ();

	virtual void DrawTiles(int tiles[], int w, int h, float tileW, float tileH, ClipRecti &clip);
	virtual void DrawMask (int tiles[], int w, int h, float tileW, float tileH, ClipRecti &clip);
private:
	int mTileset;
	int mRenderPass;

	bool mShowMask;
};

void SetRenderer (Renderer *ren);
Renderer* GetRenderer ();

#endif

