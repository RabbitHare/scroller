#include <GameLoadSave.h>
#include <Map.h>
#include <NormTurt.h>
#include <StartPos.h>

void GameLoadLevel (const char *fn)
{
	using namespace std;


	xmlDocPtr doc;
	xmlNodePtr root,
		   node,
		   n;
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

			value = NULL;
			Entity *ent = NULL;
			while (n)
			{
				if (!xmlStrcmp (n->name, BAD_CAST "name"))
				{

					if (n->xmlChildrenNode)
					{
						value = xmlNodeListGetString (doc, n->xmlChildrenNode, 1);
						if (!strcmp("NormTurt",(char*)value))
						{
							ent = ENTITY(NewNormTurt ());
							break;
						}
						else if (!strcmp("StartPos",(char*)value))
						{
							ent = ENTITY(NewStartPos ());
							break;
						}
						else std::cout << "Unknown Entity: " << (char*)value << "\n";
					}


					if (value)
						xmlFree (value);
				}
				n = n->next;
			}
			n = node->xmlChildrenNode;
			
			if (ent)
			while (n)
			{

				if (!xmlStrcmp (n->name, BAD_CAST "position"))
				{
					value = xmlNodeListGetString (doc, n->xmlChildrenNode, 1);
					istringstream ss( (char*)value);
					while (ss.get() != '(') { };
					for (int v=0; v < 3; v++)
					{
						ss >> ent->pos.V[v];
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

