#include <GObjType.h>
#include <GameGlobals.h>
#include <iostream>
#include <libxml/xmlwriter.h>


GObjType g_objTypes[MAX_GOBJ_TYPES];
int g_numObjTypes = 0;

const char* g_objTypeCats[] = {"enemy","modifier",NULL};


void InitGObjType ( GObjType *t)
{
	t->name[0] = '\0';
	t->desc[0] = '\0';
}

void LoadGObjTypes ()
{
	using namespace std;


	xmlDocPtr doc;
	xmlNodePtr root,
		   node;
	xmlChar *value;



	// PARSE GAME OBJECT FILE
	const char* fn = "gameObjects";
	doc = xmlParseFile (fn);
	if (doc == NULL)
	{
		cerr << "Could not load gameObjects: " << fn  << "\n";
		return;
	}
	root = xmlDocGetRootElement (doc);
	if (root == NULL)
	{
		cerr << "Empty gameObjects file: " << fn << "\n";
		xmlFreeDoc (doc);
		return;
	}

	int t=0;
	GObjType *type;
	for (int i=0; i<MAX_GOBJ_TYPES; i++)
		InitGObjType( g_objTypes+i);

	const char **cat = g_objTypeCats;

	node = root->xmlChildrenNode;
	while (node)
	{
		int c=0;
		while (cat[c])
		{
			if (!xmlStrcmp (node->name, BAD_CAST cat[c]))
			{
				t++;
				type = g_objTypes+t;
				strcpy(type->cat, cat[c]);
				xmlNodePtr n = node->xmlChildrenNode;
				while (n)
				{
					if (!xmlStrcmp (n->name, BAD_CAST "name"))
					{
						value = xmlNodeListGetString (doc, n->xmlChildrenNode, 1);
						strcpy(type->name, (const char*)value);

						if (value)
							xmlFree (value);
					}
					if (!xmlStrcmp (n->name, BAD_CAST "desc"))
					{
						value = xmlNodeListGetString (doc, n->xmlChildrenNode, 1);
						strcpy(type->desc, (const char*)value);


						if (value)
							xmlFree (value);
					}

					n = n->next;
				}

				break;
			}
			c++;
		}

		node = node->next;
	}

	g_numObjTypes = t;

	xmlFreeDoc (doc);
	xmlCleanupParser ();
	xmlMemoryDump ();

}

