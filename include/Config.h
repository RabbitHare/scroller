#ifndef __CONFIG_H__
#define __CONFIG_H__

enum RenderFlag
{
	R_SHOW_MASK = 0
};
void InitVideo (int w, int h, bool full=false);
void SetRenderFlagi (RenderFlag flag, int value);
void GetRenderFlagi (RenderFlag flag, int &value);

void LoadConfigFile (const char *fn);

#endif

