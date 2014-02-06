#include <EditorLoadSave.h>
#include <Map.h>
#include <MapObj.h>
#include <GameGlobals.h>

/*
=============
LoadLevel
==============
*/

void EditorLoadLevel (const char *fn)
{
	using namespace std;


	xmlDocPtr doc;
	xmlNodePtr root,
		   node,
		   n;
	xmlChar *value=NULL;

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
	GetMap()->Load (doc, fn);

	// Load objects
	while (node)
	{
		if (!xmlStrcmp (node->name, BAD_CAST "objects"))
		{
			node = node->xmlChildrenNode;
			break;
		}
		node = node->next;
	}
	while (node)
	{
		if (!xmlStrcmp (node->name, BAD_CAST "object"))
		{
			n = node->xmlChildrenNode;
			MapObj *obj = NewMapObj (0);

			while (n)
			{
				value = NULL;
				if (!xmlStrcmp (n->name, BAD_CAST "name"))
				{
					int i=0;
					if (n->xmlChildrenNode)
					{
						value = xmlNodeListGetString (doc, n->xmlChildrenNode, 1);
						for (; i < g_numObjTypes; i++)
						{
							if (!strcmp(g_objTypes[i].name,(char*)value))
							{
								obj->type = i;
								break;
							}
						}
					}
					if (i == g_numObjTypes) // object unknown, set type to 0
						obj->type=0;


					if (value)
						xmlFree (value);
				}
				else if (!xmlStrcmp (n->name, BAD_CAST "position"))
				{
					value = xmlNodeListGetString (doc, n->xmlChildrenNode, 1);
					istringstream ss( (char*)value);
					while (ss.get() != '(') { };
					for (int v=0; v < 3; v++)
					{
						ss >> obj->ent.pos.V[v];
					}
					if (value)
						xmlFree (value);
				}
				n = n->next;

			}

		}
		node = node->next;
	}
	xmlFreeDoc (doc);
	xmlCleanupParser ();
	xmlMemoryDump ();
}

/*
 * ==========
   SAVE
   ==========
   */
void EditorSaveLevel (const char *fn)
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

	// objects
	xmlTextWriterStartElement (writer, BAD_CAST "objects");
	Entity2 *e = g_entities;
	while (e)
	{
		if (!e->dead)
			WriteMapObj (writer, MAP_OBJ (e));
		e = e->next;
	}

	xmlTextWriterEndElement (writer); // objects
	GetMap()->Save (writer, fn);

	xmlTextWriterEndElement (writer); // level

	xmlFreeTextWriter (writer);
	xmlSaveFormatFile (fn, doc, 1);
	xmlFreeDoc (doc);

	xmlCleanupParser ();
	xmlMemoryDump ();
}

