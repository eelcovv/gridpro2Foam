/* This file belongs to the GridPro2FOAM distribution
   Developed by Vincent Rivola and Martin Spel
   R.Tech SARL
   Parc Technologique Cap Delta
   09340 Verniolle
   France
   For contact information: http://www.rtech.fr/contact.html
   or email: support@rtech-engineering.com
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include <vector>
#include <list>
#include <string>
#include <map>
#include <utility>
#include <algorithm>
#include "ctype.h"



#include "Block.hpp"
#include "Patch.hpp"
#include "RTechTimer.hpp"
#include "gridpro2FOAM.hpp"


#define MAXLABEL 10000

using namespace std;


int GetLine(char *line, FILE *fp)
{
	while( fgets(line,255,fp),line[0]=='#');
	return feof(fp);
}

string ToString(int value,int digitsCount)
{
	ostringstream os;
	os<<setfill('0')<<setw(digitsCount)<<value;
	return os.str();
}

static bool compareFaceSumId(Face *f1, Face *f2)
{ return (f1->SumId()<f2->SumId());
}

static bool compareFaceBC(Face *f1, Face *f2)
{ bool sameBC = (f1->GetBC()==f2->GetBC());
bool sameOwner = (f1->GetOwner()==f2->GetOwner());
if (f1->IsPeriodic() && f2->IsPeriodic())
	return (f1->GetBC()<f2->GetBC()) || (sameBC && f1->GetPeriodicFaceId()<f2->GetPeriodicFaceId()) ;

return (f1->GetBC()<f2->GetBC()) || (sameBC && (f1->GetOwner()<f2->GetOwner())) ||
		(sameBC && sameOwner && (f1->GetNeighbor()<f2->GetNeighbor()));
}

void WriteHeader(FILE *f,string root, string cas, string classe, string object, int facesSize, int nodesSize, int cellCount)
{
	fprintf(f,"FoamFile\n{\n version 2.0;\n format ascii;\n\n root \"%s\";\n case \"%s\";\n instance \"\"constant\"\";\n local \"PolyMesh\";\n\n class %s;\n", root.c_str(), cas.c_str(), classe.c_str());
	if (object=="owner" || object=="neighbour")
		fprintf(f,"\n note \"nCells:%d nActiveFaces:%d nActivePoints:%d\";\n\n", cellCount, facesSize, nodesSize);
	else {}
	fprintf(f," object %s;\n}\n\n", object.c_str());
}

main(int argc, char *argv[])
{
	FILE *f;
	FILE *f2;
	int meshType=1;
	bool mediumMesh = false;
	bool coarseMesh = false;
	bool axisym=false;
	bool forceCreateDirectory = false;
	int compte ;
	int count ;
	int id1, id2 ;
	RTechTimer timer;
	double scaling=1;
	list <int> cyclicPatches;
	list <int> solidPatches;
	vector <int> boundaryZonePatches;
	vector<string>::iterator itS ;
	vector<string>::iterator itS2 ;
	string openFoamDir ;
	string fullPathToDir ;
	string openFoamSubDir = "constant/polyMesh" ;
	//string openFoamSubDir = "constant/polyMesh" ;
	string filename  ;
	string filenamepath ;
	const char * filenamepathChar  ;
	const char * cmd  ;

	// version nbr
	const string VERSION = "5.15" ;

	vector<string> ofBCNative ;
	vector<string> ofBCNativeInGridPro ;
	// be sure to follow the same order ing than in az_mngr/ptymap.OpenFOAM
	compte = 5 ;
	ofBCNative.push_back( "patch") ;
	ofBCNative.push_back( "symmetryPlane") ;
	ofBCNative.push_back( "empty") ;
	ofBCNative.push_back( "wedge" ) ;
	ofBCNative.push_back( "cyclic") ;
	ofBCNative.push_back( "wall" ) ;
	ofBCNative.push_back( "processor") ;
	for (itS = ofBCNative.begin() ; itS != ofBCNative.end()  ; itS++)
	{
		string s = (*itS);
		string prefix = "_"+ToString(compte,3)+"_" ;
		ofBCNativeInGridPro.push_back( prefix + s);
		compte++ ;
	}


	if (argc<3)
	{ printf("Usage: %s <gridfile> <openFOAM directory> [OPTIONS] \n", argv[0]);
	exit(1);
	}
	else //(argc>=2)
	{
		for (int readArg=3; readArg<argc; readArg++)
		{
			if (!strcmp("--help",argv[readArg]))
			{ printf("options (* default):\n");
			printf("--medium       creates a medium mesh\n");
			printf("--coarse       creates a coarse mesh\n");
			printf("--axisym       creates an axisymetric mesh from a 2D file\n");
			printf("--scale        creates a scaled mesh with scale equal to the folowing argument\n");
			printf("\n");
			return 1;
			}
			else if (!strcmp("--f",argv[readArg]))
			{
				forceCreateDirectory = true ;
			}

			else if (!strcmp("--medium",argv[readArg]))
			{ printf("you're trying to create a medium mesh\n");
			meshType = 2;
			mediumMesh = true;
			}
			else if (!strcmp("--coarse",argv[readArg]))
			{ printf("you're trying to create a coarse mesh\n");
			meshType = 4;
			coarseMesh = true;
			}
			else if (!strcmp("--axisym",argv[readArg]))
			{ printf("you're trying to create an axi-symmetric mesh\n");
			axisym = true;
			}
			else if (!strcmp("--scale",argv[readArg]))
			{
				printf("you're scaling your model with scale equal to %lg\n", atof(argv[readArg+1]));
				scaling=atof(argv[readArg+1]);
				readArg=readArg+1;//the folowing argument being the scale value, it mustn't be read
			}
			/*
			else if (!strcmp("--dir",argv[readArg]))
			{
				printf("openFOAM directory is : %s\n", argv[readArg+1]);
				dir = (argv[readArg+1]);
				readArg=readArg+1;//the folowing argument being the scale value, it mustn't be read
			}
			 */

			else if (!strcmp("--cyclic", argv[readArg]))
			{ printf("internal patch %d will be treated as a cyclic patch\n", atoi(argv[readArg+1]));
			int cyclic=atoi(argv[readArg+1]);
			readArg=readArg+1;//the folowing argument being the scale value, it mustn't be read
			cyclicPatches.push_back(cyclic);
			}
			else if (!strcmp("--solid", argv[readArg]))
			{ printf("internal patch %d will be treated as two solid patches\n", atoi(argv[readArg+1]));
			int solid=atoi(argv[readArg+1]);
			readArg=readArg+1;//the folowing argument being the scale value, it mustn't be read
			solidPatches.push_back(solid);
			}
			else if (!strcmp("--boundaryZone", argv[readArg]))
			{ printf("internal patch %d will be treated as boundary patches\n", atoi(argv[readArg+1]));
			int boundaryZone=atoi(argv[readArg+1]);
			readArg=readArg+1;//the folowing argument being the scale value, it mustn't be read
			boundaryZonePatches.push_back(boundaryZone);
			}
			else
			{
				printf("unknown command line option try --help\n");
				return 1;
			}
		}
	}

	printf(" --> Reading ....\n");
	timer.Restart();
	vector<Block*> blocks;
	Block *block;

	// file input error handling
	f=fopen(argv[1], "r");
	if (!f)
	{ printf("Could not open: %s \n", argv[1]);
	exit(1);
	}
	char newName[255];
	sprintf(newName, "%s.cgns", argv[1]);

	// inser input directory
	openFoamDir=string(argv[2]);
	fullPathToDir = openFoamDir +"/"+openFoamSubDir+"/";
	//cout << "Grid For OpenFoam written in "<< fullPathToDir << endl ;
	//filename = "testIfCanWriteInDir";
	filenamepath = fullPathToDir ;
	filenamepathChar = filenamepath.c_str();

	if (forceCreateDirectory)
	{
		cout << PREFIX_ACTION << "creating the directory " << filenamepathChar << "\n";
		string s =  "mkdir -p "+filenamepath ;
		cmd = s.c_str() ;
		system(cmd);
	}
	filename = "Generated_From_gridPro2Foam_"+VERSION;
	filenamepath = fullPathToDir + filename ;
	filenamepathChar = filenamepath.c_str();
	f2=fopen(filenamepathChar, "w");
	if (!f2)
	{
		cout << "Could not create file '" << filename << "'\n" ;
		cout << "Make sure the directory '" << fullPathToDir << "' exists\n" ;
		cout << "Make sure you have the correct permission for the file '" << filename << "' exists\n" ;
		exit(1);
	}
	fclose(f2);




	// skip header starting with #
	int ch;
	while (ch=getc(f), ch=='#')
	{ while (ch=getc(f), ch!='\n'&&ch!=EOF);
	}
	ungetc(ch, f);

	// read IJK and coordinates
	int I, J, K;
	while(fscanf(f, "%d%d%d", &I, &J, &K)==3)
	{ block = new Block(I, J, K, blocks.size());
	block->ReadGridPro(f, meshType, axisym, scaling);
	block->CreateBoundaryZoneVector(boundaryZonePatches.size());
	blocks.push_back(block);
	}
	bool is2D = (K==1);
	fclose(f);

	// now handle the connectivity
	sprintf(newName, "%s.conn", argv[1]);
	f=fopen(newName, "r");
	if (!f)
	{ printf("Could not open: %s \n", newName);
	exit(1);
	}

	////////////////// face BC /////////////////////////:
	// get the surfaces and the mappings between gridpro surfaces and openFOAM
	char line[255], label[255];
	int id;
	int nrSurfLabels=0;
	while (!GetLine(line,f))
	{ if (isdigit(line[0]))
	{
		if (strstr(line, "surf labe"))
		{ sscanf(line, "%d", &nrSurfLabels);
		break;
		}
	}
	}
	map<int, string> bcMapping;
	map<int, string> bcMapping2;
	//Get the type of boundary condition
	for (int i=0; i<nrSurfLabels; i++)
	{
		GetLine(line,f);
		sscanf(line, "%s%d", label, &id);
		pair<int, string> a(id, string(label));

		if (bcMapping.count(id))
		{
			bcMapping2.insert(a);
		}
		else
		{
			bcMapping.insert(a);
		}

		if (find(solidPatches.begin(), solidPatches.end(), id)!=solidPatches.end())
		{
			printf("Found id in connectivity file that has solid block interface\n");
			pair<int, string> a(5000+id, string(label)+"-bis");
			bcMapping.insert(a);
		}
	}


	////////////////// Block BC /////////////////////////:
	rewind(f);
	int nrBlockLabels=0;
	while (!GetLine(line,f))
	{ if (isdigit(line[0]))
	{
		if (strstr(line, "block label"))
		{ sscanf(line, "%d", &nrBlockLabels);
		break;
		}
	}
	}
	vector <int> blockLabel;
	map<int, string> blockMapping;
	for (int i=0; i<nrBlockLabels; i++)
	{
		GetLine(line,f);
		sscanf(line, "%s%d", label, &id);
		pair<int, string> a(id, string(label));
		blockMapping.insert(a);
		blockLabel.push_back(id);
	}
	int nrBlockBC=blockMapping.size();


	rewind(f);
	GetLine(line,f);
	// error in connectivity file
	int nBlockConn=atoi(line);
	if (nBlockConn!=blocks.size())
	{ printf("Connectivity file is not compatible, it has %d blocks while the grid contains %d blocks\n",
			nBlockConn, blocks.size());
	exit(1);
	}

	char faceName[255], interfaceName[255];
	int index[6], nChar, b, B, neighbor[6], transform[3];
	string axisMap[6];

	char axisMap0[4];
	char axisMap1[4];
	char axisMap2[4];
	char axisMap3[4];
	char axisMap4[4];
	char axisMap5[4];

	char faceType[6];


	printf(" --> %d blocks ....\n", blocks.size());
	printf(" --> Building patches ....\n");



	int nrPeriodicFaces=0;
	// list with BCs that are periodic
	list <int> periodicBCs;



	// vector containing the patches
	vector <Patch*> patches;
	Patch *patch;
	int blockBC, BC;
	for(int n=0; n<blocks.size();n++)
	{
		GetLine(line,f);
		nChar=sscanf(line, "%c %d %c %d %d %s %c %d %d %s %c %d %d %s %c %d %d %s %c %d %d %s %c %d %d %s %d",
				&B,&b,
				&faceType[0],&index[0],&neighbor[0],axisMap0,
				&faceType[1],&index[1],&neighbor[1],axisMap1,
				&faceType[2],&index[2],&neighbor[2],axisMap2,
				&faceType[3],&index[3],&neighbor[3],axisMap3,
				&faceType[4],&index[4],&neighbor[4],axisMap4,
				&faceType[5],&index[5],&neighbor[5],axisMap5,
				&blockBC);
		//printf("%c %d %c %d %d %s\n", B, b, faceType[0], index[0], neighbor[0], axisMap0);
		axisMap[0]=string(axisMap0);
		axisMap[1]=string(axisMap1);
		axisMap[2]=string(axisMap2);
		axisMap[3]=string(axisMap3);
		axisMap[4]=string(axisMap4);
		axisMap[5]=string(axisMap5);
		if(nChar!=27)
		{ printf("Something is wrong in conn-file.\n");
		printf("%s\n",line);
		printf("%d\n", nChar);
		exit(1);
		}
		blocks[n]->SetBC(blockBC);
		int myRange[6], neighborRange[6];
		for (int iface=0; iface<6; iface++)
		{ bool internal = (faceType[iface]=='b' || neighbor[iface]!=0);  // an internal patch
		bool internalSurface = internal && faceType[iface]=='s'; // an internal surface
		bool periodic = (faceType[iface]=='p');
		bool solidInternal = false;
		bool boundaryInternal = false;
		short periodicSide = (index[iface]<0) ? -1: 1;

		int indexBZ=-1;
		if (internal or periodic)
		{ // Internal boundary condition
			BC = abs(index[iface]);
			if (internalSurface)
			{ // check if this internal surface is defined as cyclic
				if (find(cyclicPatches.begin(), cyclicPatches.end(), BC)!=cyclicPatches.end())
				{ periodic=1;
				}
				if (find(solidPatches.begin(), solidPatches.end(), BC)!=solidPatches.end())
				{ solidInternal=true;
				}
				for (int nBZ=0; nBZ<boundaryZonePatches.size(); nBZ++)
				{ if (boundaryZonePatches[nBZ]==index[iface])
				{ boundaryInternal=true;
				indexBZ=nBZ;
				break;
				}
				}
			}
			if (solidInternal)
			{ if (index[iface]<0)
			{ // the 'other' side, we change the sign since the first faces need to be internal (having id -1)
				BC = 5000 + abs(BC);
			}
			patch=new Patch(BC, blocks[n], iface, NULL, "", false);
			blocks[n]->AddExternalPatch(patch);
			patches.push_back(patch);
			printf("Added patch with BC %d\n", index[iface]);
			continue;
			}

			if (boundaryInternal)
			{ blocks[n]->AddBoundaryZone(iface, indexBZ);
			}


			int neighborBlock=neighbor[iface]-1;
			// create a new patch
			if (periodic)
			{ // see if this is available in BCmappings
				if (bcMapping.count(BC))
				{
					map<int, string>::iterator it=bcMapping.find(0);
					it= bcMapping.find(BC);
					pair<int, string> a(BC+9000, it->second);
					bcMapping.insert(a);
				}
				BC+=9000;
				patch=new Patch(BC, blocks[n], iface, blocks[neighborBlock], axisMap[iface], periodic, periodicSide);
				blocks[n]->AddExternalPatch(patch);
				nrPeriodicFaces+=patch->NrFaces();
				periodicBCs.push_back(BC);
			} else
			{ patch=new Patch(0, blocks[n], iface, blocks[neighborBlock], axisMap[iface], periodic, periodicSide);
			}
		} else
		{ // External boundaries to be done
			patch=new Patch(index[iface], blocks[n], iface, NULL, "", false);
			blocks[n]->AddExternalPatch(patch);
		}

		patches.push_back(patch);
		}
	}
	fclose(f);

	// Try to open a pty file for the block BCs
	sprintf(newName, "%s.pty", argv[1]);
	f=fopen(newName, "r");

	if (f)
	{
		printf(" --> A property file is found, block conditions are read from %s.pty\n", argv[1]);
		printf(" --> These overwrite the block conditions in the %s.conn file\n", argv[1]);
		while (!GetLine(line,f))
		{ if (strstr(line, "3D properties"))
		{ sscanf(line, "%d", &nrBlockLabels);
		break;
		}
		}
		blockMapping.clear();
		blockLabel.clear();

		printf(" --> %d block labels found\n", nrBlockLabels);
		//Get the type of boundary condition
		int ibl;
		for (int i=0; i<nrBlockLabels; i++)
		{
			GetLine(line,f);
			sscanf(line, "%d%s", &id, label);
			pair<int, string> a(id, string(label));
			blockMapping.insert(a);
			blockLabel.push_back(id);
		}
		rewind(f);
		while (!GetLine(line,f))
		{ if (line[0]=='B')
		{ sscanf(line+1, "%d%d", &ibl, &BC);
		blocks[ibl-1]->SetBC(BC);
		}
		}
		fclose(f);
	}

	int pointCount=0, cellCount=0;
	for (int n=0; n<blocks.size(); n++)
	{ blocks[n]->CountPoints(pointCount);
	}

	printf("Total number of points before merging the nodes is %d\n", pointCount);
	// Make points unique
	printf(" --> Making matching points unique ....\n");
	for (int n=0; n<patches.size(); n++)
	{ patches[n]->MakeEdgePointsUnique();
	}

	// Writing of Points.txt
	printf(" --> Writing points ....\n");
	pointCount=0, cellCount=0;
	for (int n=0; n<blocks.size(); n++)
	{
		// for debugging purposes to print equivalent: blocks[n]->PrintEquivalent();
		blocks[n]->CountPoints(pointCount);
		blocks[n]->CountCells(cellCount);
	}

	printf("Total number of points is %d\n", pointCount);
	printf("Total number of cells is %d\n",  cellCount);





	printf("Create faces.....\n");
	// Creation of the faces:
	list <Face*> faces;
	for (int n=0; n<blocks.size(); n++)
	{ blocks[n]->CreateFaces(faces);
	}

	// give the faces an id
	int faceCount=0;
	for (list <Face*>::iterator i=faces.begin(); i!=faces.end(); i++)
	{ (*i)->SetIdFace(faceCount++);
	}


	printf("Handle periodic boundary conditions phase-I.....\n");
	list <Face*>::iterator i, j ;
#define HANDLE_PERIODIC
#ifdef HANDLE_PERIODIC

	list <Face*> periodicFaces;
	// Handle periodic boundary conditions
	//    Loop over all periodic faces, check if the four periodic nodes are found in
	//    another face, set matching id
	for (i=faces.begin(); i!=faces.end(); i++)
	{ // get the sum of all periodic nodes having BC of this face
		BC = (*i)->GetBC();
		if (find(periodicBCs.begin(), periodicBCs.end(), BC)==periodicBCs.end())
			continue; // BC not in list of periodic BCs
		periodicFaces.push_back(*i);
		(*i)->SetPeriodic();
		(*i)->ComputeSumPeriodicId();
	}
	printf("Handle periodic boundary conditions phase II.....\n");

	bool foundMatch;
	int nrMatchingPeriodicFaces=0;
	int nrMatchingPeriodicFacesCandidates=0;


	//#define DEBUG_SIEMENS
#ifdef DEBUG_SIEMENS
	FILE *fsiem=fopen("siemens.txt", "w");
#endif

	PeriodicStats pstat;
	for (i=periodicFaces.begin(); i!=periodicFaces.end(); i++)
	{ foundMatch=false;
	BC = (*i)->GetBC();
	if (find(periodicBCs.begin(), periodicBCs.end(), BC)==periodicBCs.end())
		continue; // BC not in list of periodic BCs
	nrMatchingPeriodicFacesCandidates++;
	// continue if this face is already matched
	if ((*i)->GetPeriodicFaceId()>0) continue;
	j=i; j++;
	for (; j!=periodicFaces.end(); j++)
	{ if ((*j)->GetBC()!=BC) continue;
	if ((*j)->SumPeriodicId() != (*i)->SumId()) continue;
	if (!(*j)->PeriodicMatch(*i)) continue;

	//printf("Bingo!!! %d matches with %d\n", (*i)->GetIdFace(), (*j)->GetIdFace());
	(*i)->SetMatchingFace((*j));
	(*j)->SetMatchingFace((*i));
	nrMatchingPeriodicFaces++;
	foundMatch=true;
	if ((*i)->GetPeriodicSide()>0)
	{ (*i)->SetPeriodicFaceId(nrMatchingPeriodicFaces);
	(*j)->SetPeriodicFaceId(nrMatchingPeriodicFaces+nrPeriodicFaces/2);
	} else
	{ (*j)->SetPeriodicFaceId(nrMatchingPeriodicFaces);
	(*i)->SetPeriodicFaceId(nrMatchingPeriodicFaces+nrPeriodicFaces/2);
	}
#ifdef DEBUG_SIEMENS
	fprintf(fsiem, "found match between face %d and %d\n",  (*i)->GetIdFace(), (*j)->GetIdFace());
	(*i)->PrintDebug(fsiem);
	(*j)->PrintDebug(fsiem);
#endif


	// check the coordinate positions of the two faces
	BC = (*i)->GetBC();
	if (find(periodicBCs.begin(), periodicBCs.end(), BC)!=periodicBCs.end())
	{ (*i)->Print();
	(*i)->AnalysePeriodic(&pstat, BC);
	(*j)->Print();
	(*j)->AnalysePeriodic(&pstat, BC);
	// Find the points that are supposed to match..
	// For the matching point, find the coordinate that remains fixed, this is the rotational axis
	printf("\n");
	} else
	{ printf("Periodic not found\n");
	exit(1);
	}
	break;
	}
	if (!foundMatch)
	{
		printf("No match found for periodic face\n");
		//exit(1);
	}
	}
#ifdef DEBUG_SIEMENS
	fclose(fsiem);
#endif
	// output points
	filename = "points";
	filenamepath = fullPathToDir + filename ;
	filenamepathChar = filenamepath.c_str();
	f=fopen(filenamepathChar, "w");
	if (!f)
	{
		cout << "Could not create file '" << filename << "'\n" ;
		cout << "Make sure the directory '" << fullPathToDir << "' exists\n" ;
		cout << "Make sure you have the correct permission for the file '" << filename << "' exists\n" ;
		exit(1);
	}
	WriteHeader(f,openFoamDir+"/run/tutorials/icoFoam",
			"drivenCavityReal2d","vectorField","points",0,0,0);
	fprintf(f,"%d \n(\n",pointCount);
	for (unsigned int n=0; n<blocks.size(); n++)
	{
		blocks[n]->WritePoints(f);
	}
	fprintf(f,")");
	fclose(f);

	pstat.Print();

	if (nrPeriodicFaces!=nrMatchingPeriodicFaces*2)
	{ printf("Internal error in finding periodic connectivity\n");
	printf("Number of periodic faces: %d\n", nrPeriodicFaces);
	printf("Number of matching periodic faces: %d\n", nrMatchingPeriodicFaces);
	printf("Number of matching periodic face candidates: %d\n", nrMatchingPeriodicFacesCandidates);
	nrMatchingPeriodicFaces=0;
	for (i=faces.begin(); i!=faces.end(); i++)
	{ foundMatch=false;
	BC=(*i)->GetBC();
	if (find(periodicBCs.begin(), periodicBCs.end(), BC)==periodicBCs.end()) continue;
	for (j=faces.begin(); j!=faces.end(); j++)
	{ if ((*j)->PeriodicMatch(*i))
	{ nrMatchingPeriodicFaces++;
	printf("Bingo!!! %d matches with %d\n", (*i)->GetIdFace(), (*j)->GetIdFace());
	foundMatch=true;
	}
	}
	if (!foundMatch)
	{
		printf("No match found for periodic face\n");
		(*i)->PrintDebug();
		exit(1);
	}


	}
	printf("Number of matching periodic faces: %d\n", nrMatchingPeriodicFaces);

	//exit(1);
	}

	printf("%d periodic faces found\n", nrPeriodicFaces);
	printf("%d matching pairs of periodic faces found\n", nrMatchingPeriodicFaces);
#endif




	// Sort the faces with respect to their sumId to accelerate the merge phase
	faces.sort(compareFaceSumId);

	timer.Stop();
	double timeReading=timer.WALL_microsec();
	timer.Restart();

	faceCount=0;
	for (list <Face*>::iterator i=faces.begin(); i!=faces.end(); i++)
	{ (*i)->SetIdFace(faceCount++);
	}

	printf("The number of faces (before merging) is %d\n",(int)  faces.size());
	// all faces have been created, but some are doubly defined
	// we need to merge them
	printf(" --> Merging faces ....\n\n");
	int facesDone=0;
	int facesToBeDone=faces.size();
	for (i=faces.begin(); i!=faces.end(); i++)
	{ facesDone++;
	j=i; j++;
	for (; j!=faces.end(); j++)
	{ if ((*j)->SumId() > (*i)->SumId()) break;
	if ((*i)->MergeNeeded(*j)==true)
	{
		if (((*j)->GetOwner()==-1) && ((*i)->GetNeighbor()==-1)) //connection Max(i)/Min(j)
		{
			(*i)->SetNeighbor((*j)->GetNeighbor());
		}
		if (((*i)->GetOwner()==-1) && ((*j)->GetNeighbor()==-1)) //connection Max(j)/Min(i)
		{
			(*i)->SetOwner((*i)->GetNeighbor());
			(*i)->SetNeighbor((*j)->GetOwner());
		}
		if (((*i)->GetNeighbor()==-1) && ((*j)->GetNeighbor()==-1)) //connection Max(i)/Max(j)
		{
			(*i)->SetNeighbor((*j)->GetOwner());
		}
		if (((*i)->GetOwner()==-1) && ((*j)->GetOwner()==-1)) //connection Min(i)/Min(j)
		{
			(*i)->SetOwner((*i)->GetNeighbor());
			(*i)->SetNeighbor((*j)->GetNeighbor());
		}

		faces.erase(j);//this is taking a long time if you use gcc to compile
		facesToBeDone--; // if we found a corresponding face, quit the loop over j
		break;
	}
	}
	if ((*i)->GetOwner()==-1)
	{
		(*i)->SetOwner((*i)->GetNeighbor());
		(*i)->SetNeighbor(-1);

	}
	if (facesDone%(facesToBeDone/100)==0)
	{ printf ("\033M Faces treated done: %d %\n", ((int)(facesDone/(facesToBeDone/100.))+1));
	fflush(stdout);
	}
	}

	printf("The number of faces after merging is %d\n", facesDone);



	// check if all faces that are external have a BC:
	int error=0;
	FILE *fdebug=fopen("debugTecplot.dat", "w");
	for (i=faces.begin(); i!=faces.end(); i++)
	{ if (((*i)->GetNeighbor()==-1) && (*i)->IsInternal())
	{ printf("No neighbor was found, though face is internal\n");
	printf("SumId of this face is %d\n", (*i)->SumId());
	printf("BCorg of this face is %d\n", (*i)->GetBCorg());
	printf("BC of this face is %d\n", (*i)->GetBC());
	(*i)->Print();
	(*i)->WriteTecplot(fdebug);
	(*i)->PrintDebug();
	/*
      for (j=faces.begin(); j!=faces.end(); j++)
      {
        if ((*i)->MergeNeeded(*j))
        { printf("Mergable with j:\n");
          (*j)->PrintDebug();
        }
        if ((*i)->SumId()==(*j)->SumId())
        { printf("Possibly mergable with j:\n");
          (*j)->PrintDebug();
        }
      }
	 */

	error++;
	}
	if ((*i)->GetOwner()==-1)
	{ printf("No owner found\n");
	error++;
	}
	}
	if (error)
	{ printf("%d/%d errors occured\n", error, faces.size());
	exit(1);
	}
	fclose(fdebug);

	timer.Stop();
	double timeMerging=timer.WALL_microsec();
	timer.Restart();
	// sort faces on ascending BC and according to their owner and neighbor ID
	printf(" --> Sorting faces...\n");  fflush(stdout);
	faces.sort(compareFaceBC);


	timer.Stop();
	double timeSorting=timer.WALL_microsec();
	timer.Restart();

	// output faces
	printf(" --> Output faces ....\n");
	filename = "faces";
	filenamepath = fullPathToDir + filename ;
	filenamepathChar = filenamepath.c_str();
	f=fopen(filenamepathChar, "w");
	if (!f)
	{
		cout << "Could not create file '" << filename << "'\n" ;
		cout << "Make sure the directory '" << fullPathToDir << "' exists\n" ;
		cout << "Make sure you have the correct permission for the file '" << filename << "' exists\n" ;
		exit(1);
	}

	WriteHeader(f,openFoamDir+"/run/tutorials/icoFoam", "cylinderEulerFromGridPro_fine", "faceList", "faces", faces.size(), pointCount, cellCount);
	fprintf(f,"%d\n(\n",(int)  faces.size());
	for (i=faces.begin(); i!=faces.end(); i++) (*i)->Print(f);
	fprintf(f,")\n");
	fclose(f);


	// output owner
	printf(" --> Output owners ....\n");
	filename = "owner";
	filenamepath = fullPathToDir + filename ;
	filenamepathChar = filenamepath.c_str();
	f=fopen(filenamepathChar, "w");
	if (!f)
	{
		cout << "Could not create file '" << filename << "'\n" ;
		cout << "Make sure the directory '" << fullPathToDir << "' exists\n" ;
		cout << "Make sure you have the correct permission for the file '" << filename << "' exists\n" ;
		exit(1);
	}

	WriteHeader(f,openFoamDir+"/run/tutorials/icoFoam", "cylinderEulerFromGridPro_fine", "labelList", "owner", faces.size(), pointCount, cellCount);
	fprintf(f,"%d\n(\n", (int) faces.size());
	for (i=faces.begin(); i!=faces.end(); i++)
	{fprintf(f,"%d\n",(*i)->GetOwner());}
	fprintf(f,")\n");
	fclose(f);

	// output neighbour
	printf(" --> Output neighbours ....\n");
	filename = "neighbour";
	filenamepath = fullPathToDir + filename ;
	filenamepathChar = filenamepath.c_str();
	f=fopen(filenamepathChar, "w");
	if (!f)
	{
		cout << "Could not create file '" << filename << "'\n" ;
		cout << "Make sure the directory '" << fullPathToDir << "' exists\n" ;
		cout << "Make sure you have the correct permission for the file '" << filename << "' exists\n" ;
		exit(1);
	}
	WriteHeader(f,openFoamDir+"/run/tutorials/icoFoam","cylinderEulerFromGridPro_fine", "labelList", "neighbour", faces.size(), pointCount, cellCount);
	fprintf(f,"%d\n(\n", (int) faces.size());
	for (i=faces.begin(); i!=faces.end(); i++)
	{fprintf(f,"%d\n",(*i)->GetNeighbor());}
	fprintf(f,")\n");
	fclose(f);


	count=0;
	int start=1;
	int n=0;
	// get BC of first face in list
	i=faces.begin();
	BC=(*i)->GetBC();
	for (i=faces.begin(); i!=faces.end(); i++)
	{
		if ((*i)->GetBC()!=BC) // if this one is different than previous
		{
			BC=(*i)->GetBC();
			n++;
		}
		count++;
	}

	// output boundary
	printf(" --> Output boundary ....\n");
	i=faces.begin();
	BC=(*i)->GetBC();
	count=0;
	start=1;

	filename = "boundary";
	filenamepath = fullPathToDir + filename ;
	filenamepathChar = filenamepath.c_str();
	f=fopen(filenamepathChar, "w");
	if (!f)
	{
		cout << "Could not create file '" << filename << "'\n" ;
		cout << "Make sure the directory '" << fullPathToDir << "' exists\n" ;
		cout << "Make sure you have the correct permission for the file '" << filename << "' exists\n" ;
		exit(1);
	}

	WriteHeader(f,openFoamDir+"run/tutorials/icoFoam","cylinderEulerFromGridPro_fine", "polyBoundaryMesh", "boundary", faces.size(), pointCount, cellCount);
	fprintf(f,"%d\n(\n", n);
	map<int, string>::iterator it=bcMapping.find(0);
	map<int, string>::iterator it2=bcMapping2.find(0);
	string BCname;
	string BCtype;
	cout << PREFIX_INFO  << "Recognized OpenFOAM BC types" << endl ;

	itS2 = ofBCNativeInGridPro.begin();
	for (itS = ofBCNative.begin() ; itS != ofBCNative.end()  ; itS++ , itS2 ++)
	{
		string s = (*itS);
		string s2 = (*itS2);
		cout << PREFIX_INFO  <<" - " << setw(25) << s << setw(25) << s2 << endl ;
	}

	// for 2d, look for the 2 side patches
	int side1 = 0 ;
	int side2 = 0 ;
	int maxNbrFace = 0;
	int countSameMaxNbr = 0 ;
	int nbr ;
	if (is2D)
	{
		for (i=faces.begin(); i!=faces.end(); i++)
		{
			//printf("Faces BC: %d\n", (*i)->GetBC());
			if ((*i)->GetBC()!=BC) // if the BC of this face is different than for the previous face
			{
				if (BC!=-1)
				{
					nbr = (count - (start-1)) ;
					if ( nbr > maxNbrFace )
					{
						maxNbrFace = nbr ;
						side1 = BC ;
						countSameMaxNbr = 1 ;
					}
					else if (nbr ==  maxNbrFace )
					{
						side2 = BC ;
						countSameMaxNbr ++ ;
					}
				}
				start=count+1;
				BC=(*i)->GetBC();
			}
			count++;
		}
		nbr = (count - (start-1)) ;
		if ( nbr > maxNbrFace )
		{
			maxNbrFace = nbr ;
			side1 = BC ;
			countSameMaxNbr = 1 ;
		}
		else if (nbr ==  maxNbrFace )
		{
			side2 = BC ;
			countSameMaxNbr ++ ;
		}
	}
	if (countSameMaxNbr == 2 )
	{
		cout << PREFIX_INFO << "2 BCs have the same highest number of faces: they are assigned to 'back' and 'front' labels with an 'empty' type\n" ;
		bcMapping[side1] = "front";
		bcMapping2[side1] = "_007_empty";
		bcMapping[side2] = "back";
		bcMapping2[side2] = "_007_empty";
	}

	// write boundary
	i=faces.begin();
	BC=(*i)->GetBC();
	count=0;
	start=1;
	for (i=faces.begin(); i!=faces.end(); i++)
	{
		//printf("Faces BC: %d\n", (*i)->GetBC());
		if ((*i)->GetBC()!=BC) // if the BC of this face is different than for the previous face
		{
			if (BC!=-1)
			{
				cout << PREFIX_ACTION  << "Processing  BC " << BC << endl ;
				// type
				if (bcMapping2.count(BC))
				{
					it2= bcMapping2.find(BC);
					BCtype=(*it2).second.c_str();
				}
				else
				{
					char tmpString[255];
					sprintf(tmpString, "%03d", BC);
					BCtype="Unknown-"+string(tmpString);
					pair<int, string> a(id, BCtype);
					bcMapping2.insert(a);
				}
				// label
				if (bcMapping.count(BC))
				{
					it= bcMapping.find(BC);
					BCname=(*it).second.c_str();
				}
				else
				{
					char tmpString[255];
					sprintf(tmpString, "%03d", BC);
					BCname="Unknown-"+string(tmpString);
					pair<int, string> a(id, BCname);
					bcMapping.insert(a);
				}
				// swap BCname & BCtype if needed
				string s  ;
				bool BCnameIsOfBCNative = false ;
				bool BCtypeIsOfBCNative = false ;
				id1 = 0 ;
				id2 = 0 ;
				compte = 0 ;
				for (itS = ofBCNativeInGridPro.begin() ; itS != ofBCNativeInGridPro.end()  ; itS++)
				{
					s = (*itS);
					if (BCname == s)
					{
						BCnameIsOfBCNative = true ;
						id1 = compte ;
					}
					if (BCtype == s)
					{
						BCtypeIsOfBCNative = true ;
						id2 = compte ;
					}
					compte ++ ;
				}
				if (BCnameIsOfBCNative && !BCtypeIsOfBCNative)
				{
					s = BCname ;
					BCname = BCtype  ;
					BCtype = ofBCNative[id1];
					cout << PREFIX_INFO << " - label : " << BCname << endl ;
					cout << PREFIX_INFO << " - type  : " << BCtype << endl ;
				}
				else if (!BCnameIsOfBCNative && !BCtypeIsOfBCNative)
				{
					cout << PREFIX_INFO <<"2 labels are defined for surface id :" << BC << endl ;
					cout << PREFIX_INFO <<" - " << BCname << endl ;
					cout << PREFIX_INFO <<" - " << BCtype << endl ;
					cout << PREFIX_INFO <<"And none of them is from the OpenFOAM type" << endl ;

					//exit(1);
				}
				else if (BCnameIsOfBCNative && BCtypeIsOfBCNative)
				{
					cout << PREFIX_INFO <<"2 OpenFOAM BC types are defined for surface id :" << BC << endl ;
					cout << PREFIX_INFO <<" - " << ofBCNative[id1] << endl ;
					cout << PREFIX_INFO <<" - " << ofBCNative[id2] << endl ;
					cout << PREFIX_INFO <<"The order might not be correct: check tghe boundary file for BC : "<< BC  << endl ;
					//exit(1);
				}
				else
				{
					BCtype = ofBCNative[id2];
					cout << PREFIX_INFO <<" - label : " << BCname << endl ;
					cout << PREFIX_INFO <<" - type  : " << BCtype << endl ;
				}


				if (find(periodicBCs.begin(), periodicBCs.end(), BC)==periodicBCs.end())
				{
					fprintf(f,"%s\n{\n  type %s;\n  nFaces %d;\n  startFace %d;\n}\n\n", BCname.c_str(), BCtype.c_str(), (count - (start-1)) , (start-1));
				}
				else
				{
					//fprintf(f,"Cyclic-%s\n{\n  type %s;\n  nFaces %d;\n  startFace %d;\n}\n\n", BCname.c_str(), "cyclic", (count - (start-1)) , (start-1));
					fprintf(f,"Cyclic-%s\n{\n  type %s;\n  nFaces %d;\n  startFace %d;\n}\n\n", BCname.c_str(), BCtype.c_str(), (count - (start-1)) , (start-1));
				}
			} else
			{ printf("Not processing BC -1\n");
			}
			start=count+1;
			BC=(*i)->GetBC();
		}
		count++;
	}
	cout << PREFIX_ACTION  << "Processing  BC " << BC << endl ;
	// type
	if (bcMapping2.count(BC))
	{
		it2= bcMapping2.find(BC);
		BCtype=(*it2).second.c_str();
	}
	else
	{
		char tmpString[255];
		sprintf(tmpString, "%03d", BC);
		BCtype="Unknown-"+string(tmpString);
		pair<int, string> a(id, BCtype);
		bcMapping2.insert(a);
	}
	// label
	if (bcMapping.count(BC))
	{
		it= bcMapping.find(BC);
		BCname=(*it).second.c_str();
	}
	else
	{
		char tmpString[255];
		sprintf(tmpString, "%03d", BC);
		BCname="Unknown-"+string(tmpString);
		pair<int, string> a(id, BCname);
		bcMapping.insert(a);
	}
	// swap BCname & BCtype if needed
	string s  ;
	bool BCnameIsOfBCNative = false ;
	bool BCtypeIsOfBCNative = false ;
	id1 = 0 ;
	id2 = 0 ;
	compte = 0 ;
	for (itS = ofBCNativeInGridPro.begin() ; itS != ofBCNativeInGridPro.end()  ; itS++)
	{
		s = (*itS);
		if (BCname == s)
		{
			BCnameIsOfBCNative = true ;
			id1 = compte ;
		}
		if (BCtype == s)
		{
			BCtypeIsOfBCNative = true ;
			id2 = compte ;
		}
		compte ++ ;
	}
	if (BCnameIsOfBCNative && !BCtypeIsOfBCNative)
	{
		s = BCname ;
		BCname = BCtype  ;
		BCtype = ofBCNative[id1];
		cout << PREFIX_INFO << " - label : " << BCname << endl ;
		cout << PREFIX_INFO << " - type  : " << BCtype << endl ;
	}
	else if (!BCnameIsOfBCNative && !BCtypeIsOfBCNative)
	{
		cout << PREFIX_INFO <<"2 labels are defined for surface id :" << BC << endl ;
		cout << PREFIX_INFO <<" - " << BCname << endl ;
		cout << PREFIX_INFO <<" - " << BCtype << endl ;
		cout << PREFIX_INFO <<"And none of them is from the OpenFOAM type" << endl ;

		//exit(1);
	}
	else if (BCnameIsOfBCNative && BCtypeIsOfBCNative)
	{
		cout << PREFIX_INFO <<"2 OpenFOAM BC types are defined for surface id :" << BC << endl ;
		cout << PREFIX_INFO <<" - " << ofBCNative[id1] << endl ;
		cout << PREFIX_INFO <<" - " << ofBCNative[id2] << endl ;
		cout << PREFIX_INFO <<"The order might not be correct: check tghe boundary file for BC : "<< BC  << endl ;
		//exit(1);
	}
	else
	{
		BCtype = ofBCNative[id2];
		cout << PREFIX_INFO <<" - label : " << BCname << endl ;
		cout << PREFIX_INFO <<" - type  : " << BCtype << endl ;
	}


	if (find(periodicBCs.begin(), periodicBCs.end(), BC)==periodicBCs.end())
	{
		fprintf(f,"%s\n{\n  type %s;\n  nFaces %d;\n  startFace %d;\n}\n\n", BCname.c_str(), BCtype.c_str(), (count - (start-1)) , (start-1));
	}
	else
	{
		//fprintf(f,"Cyclic-%s\n{\n  type %s;\n  nFaces %d;\n  startFace %d;\n}\n\n", BCname.c_str(), "cyclic", (count - (start-1)) , (start-1));
		fprintf(f,"Cyclic-%s\n{\n  type %s;\n  nFaces %d;\n  startFace %d;\n}\n\n", BCname.c_str(), BCtype.c_str(), (count - (start-1)) , (start-1));
	}

	fprintf(f,")\n\n");
	fclose(f);

	/*For debugging you can visualise faces under Tecplot
  f=fopen("OpenFoamFiles/Temp/faces.tec", "w");
  for (i=faces.begin(); i!=faces.end(); i++)
  { (*i)->WriteTecplot(f);
  }
  fclose(f);*/


	// cellZones
	// loop over number of block BCs
	// loop over blocks and compare
	// use cell ids
	if (nrBlockLabels)
	{ int nrCells;
	printf(" --> Writing %d block labels\n", nrBlockLabels);
	filename = "cellZones";
	filenamepath = fullPathToDir + filename ;
	filenamepathChar = filenamepath.c_str();
	f=fopen(filenamepathChar, "w");
	if (!f)
	{
		cout << "Could not create file '" << filename << "'\n" ;
		cout << "Make sure the directory '" << fullPathToDir << "' exists\n" ;
		cout << "Make sure you have the correct permission for the file '" << filename << "' exists\n" ;
		exit(1);
	}
	WriteHeader(f,openFoamDir+"run/tutorials/icoFoam","cylinderEulerFromGridPro_fine", "regIOobject", "cellZones", faces.size(), pointCount, cellCount);

	int nrActiveBlockLabels=0;
	for (int i=0; i<nrBlockLabels; i++)
	{
		BC=blockLabel[i];
		it= blockMapping.find(BC);
		BCname=(*it).second.c_str();
		nrCells=0;
		for(int n=0; n<blocks.size();n++) if (blocks[n]->GetBC()==BC) nrCells+=blocks[n]->NrCells();
		if (nrCells>0) nrActiveBlockLabels++;
	}

	fprintf(f, "%d\n(\n", nrActiveBlockLabels);  // Open cellZones parenthesis
	for (int i=0; i<nrBlockLabels; i++)
	{ BC=blockLabel[i];
	it = blockMapping.find(BC);
	BCname=(*it).second.c_str();
	/*
	it2= blockMapping2.find(BC);
	BCtype=(*it2).second.c_str();
	 */
	nrCells=0;
	for(int n=0; n<blocks.size();n++) if (blocks[n]->GetBC()==BC) nrCells+=blocks[n]->NrCells();
	if (nrCells==0) continue;
	printf("Blocklabel %d %s\n", BC, BCname.c_str());
	fprintf(f, "%s\n{ \n type cellZone;\n  cellLabels  List<label>\n%d\n(\n", BCname.c_str(), nrCells);
	for(int n=0; n<blocks.size();n++)
	{ if (blocks[n]->GetBC()==BC)
	{ blocks[n]->PrintCellList(f);
	}
	}
	fprintf(f, ");\n}\n");
	}
	fprintf(f, ")\n");    // Close cellZones parenthesis
	fclose(f);
	}

	if (boundaryZonePatches.size())
	{
		filename = "cellZones";
		filenamepath = fullPathToDir + filename ;
		filenamepathChar = filenamepath.c_str();
		f=fopen(filenamepathChar, "w");
		if (!f)
		{
			cout << "Could not create file '" << filename << "'\n" ;
			cout << "Make sure the directory '" << fullPathToDir << "' exists\n" ;
			cout << "Make sure you have the correct permission for the file '" << filename << "' exists\n" ;
			exit(1);
		}
		WriteHeader(f,openFoamDir+"run/tutorials/icoFoam","cylinderEulerFromGridPro_fine", "regIOobject", "cellZones", faces.size(), pointCount, cellCount);
		fprintf(f, "%d\n(\n", boundaryZonePatches.size());  // Open cellZones parenthesis
		for (int nBZ=0; nBZ<boundaryZonePatches.size(); nBZ++)
		{ int nrCells=0;
		for(int n=0; n<blocks.size();n++)
		{
			nrCells+=blocks[n]->NrBoundaryZoneCells(nBZ);
		}
		fprintf(f, "boundaryZone%03d\n{ \n type cellZone;\n  cellLabels  List<label>\n%d\n(\n", nBZ+1, nrCells);
		for(int n=0; n<blocks.size();n++)
		{
			blocks[n]->PrintBoundaryCellList(f, nBZ);
		}
		fprintf(f, ");\n}\n");
		}
		fprintf(f, ")\n");    // Close cellZones parenthesis
		fclose(f);
	}




	printf("Times:\n");
	printf("  Reading: %1.2lg\n", timeReading/1e6);
	printf("  Merging: %1.2lg\n", timeMerging/1e6);
	printf("  Sorting: %1.2lg\n", timeSorting/1e6);
}
