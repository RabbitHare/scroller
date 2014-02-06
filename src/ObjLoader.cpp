#include <ObjLoader.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include <Mesh.h>
#include <TMesh.h>
	
	
bool LoadObj (const char *fd, Mesh *mesh)
{
	using namespace std;
	ifstream s;
	string buffer;
	int length;
	s.open (fd, ios::binary);
	if (s.is_open() == false)
	{
		cerr << "Could not open file: " << fd << "\n";
		return false;
	}
	s.seekg (0, ios::end);
	length = s.tellg();
	s.seekg (0, ios::beg);

	buffer.reserve (length);

	char *cstr = new char[length];
	s.read (cstr, length);
	s.close();
	cstr[length-1] = 0;
	buffer = cstr;
	delete[] cstr;

	istringstream ss(buffer);


	mesh->numVerts = 0;
	mesh->numTexVerts = 0;
	mesh->numFaces = 0;	
	char c[4];
	while (1)
	{
		ss.read (c,2);
		// verts
		if (c[0] == 'v' && c[1] == ' ')
		{
			ss >> mesh->verts[mesh->numVerts].V[0];
			ss >> mesh->verts[mesh->numVerts].V[1];
			ss >> mesh->verts[mesh->numVerts].V[2];
			mesh->numVerts++;
		}
		// texture verts
		else if (c[0] == 'v' && c[1] == 't' && ss.peek() == ' ')
		{
			float f;
			ss >> f;
			mesh->texVerts[mesh->numTexVerts].x = f;
			ss >> f;
			mesh->texVerts[mesh->numTexVerts].y = -f;
			mesh->numTexVerts++;
		}
		// faces
		else if (c[0] == 'f' && c[1] == ' ')
		{
			int n;
			for (int i = 0; i < 3; i++)
			{
				ss >> n;
				mesh->faces[mesh->numFaces].v[i] = n-1;
				if (ss.peek() == '/') // texvert
				{
					ss.ignore(); // '/'
					ss >> n;
					mesh->faces[mesh->numFaces].tv[i] = n-1;
				}
				if (ss.peek() == '/') // normal
				{
					ss.ignore(); // '/'
					ss >> n;
				}
			}
			mesh->numFaces++;
		}

		ss.ignore (length,'\n');
		if (ss.eof())
			break;
	}
	std::cout << mesh->numFaces << " numFaces\n";
	std::cout << mesh->numVerts << " numVerts\n";
	return true;

}

bool LoadObj (const char *fd, TMesh &mesh)
{
	using namespace std;
	ifstream s;
	string buffer;
	int length;
	s.open (fd, ios::binary);
	if (s.is_open() == false)
	{
		cerr << "Could not open file: " << fd;
		return false;
	}
	s.seekg (0, ios::end);
	length = s.tellg();
	s.seekg (0, ios::beg);

	buffer.reserve (length);

	char *cstr = new char[length];
	s.read (cstr, length);
	s.close();
	cstr[length-1] = 0;
	buffer = cstr;
	delete[] cstr;

	istringstream ss(buffer);

	TVert *tve;
	TFace *tfa;
	std::vector<TVert*> verts;
	std::vector<Vec2f> texCoords;

	char c[4];
	while (1)
	{
		ss.read (c,2);
		// verts
		if (c[0] == 'v' && c[1] == ' ')
		{
			tve = new TVert ();
			mesh.verts.Append (tve);
			ss >> tve->co.V[0];
			ss >> tve->co.V[1];
			ss >> tve->co.V[2];
			verts.push_back (tve);
		}
		// texture coords
		else if (c[0] == 'v' && c[1] == 't' && ss.peek() == ' ')
		{
			Vec2f tv;
			ss >> tv.x;
			ss >> tv.y;
			tv.y = -tv.y;
			texCoords.push_back (tv);
		}
		// faces
		else if (c[0] == 'f' && c[1] == ' ')
		{
			tfa = new TFace ();
			mesh.faces.Append (tfa);
			int n;
			for (int i = 0; i < 3; i++)
			{
				ss >> n;
				tfa->p[i].v = verts[n-1];
				tfa->p[i].f = tfa;
				tfa->p[i].next = tfa->p[i].v->fverts;
				tfa->p[i].v->fverts = tfa->p+i;

				if (ss.peek() == '/') // texvert
				{
					ss.ignore(); // '/'
					ss >> n;
					tfa->p[i].tco = texCoords[n-1];
				}
				if (ss.peek() == '/') // normal
				{
					ss.ignore(); // '/'
					ss >> n;
				}
			}
		}

		ss.ignore (length,'\n');
		if (ss.eof())
			break;
	}
	// assign texcoords to vertices
	// Have it so for every unique texture coord there is a vertex, so that
	// there is a texture coord per vertex for easy rendering
	TVert *v = mesh.verts.Last();
	for (; v != mesh.verts.End(); v = v->prev)
	{
		TFaceVert *fv = v->fverts;
		if (fv)
		{
			v->tmp.v = NULL;
			v->tco = fv->tco;
			fv = fv->next;
			for (;fv;fv = fv->next)
			{
				for (tve = v; tve; tve = tve->tmp.v)
					if (fv->tco == tve->tco)
					{
						fv->v = tve;
						break;
					}
				if (tve == NULL)
				{
					fv->v = new TVert();
					mesh.verts.Append (fv->v);
					
					fv->v->co = v->co;
					fv->v->tco = fv->tco;
					fv->v->tmp.v = v->tmp.v;
					v->tmp.v = fv->v;
				}
			}
		}
	}

	return true;

}

