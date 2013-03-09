
#include "string.h"
#include "stdlib.h"
#include "math.h"
#include "ChunkCodes.h"
#include "DataStructures.h"
#include "ModelLoader.h"
#define DEBUG

ModelLoader::ModelLoader(void){
}


ModelLoader::~ModelLoader(void){
}



int ModelLoader::loadModel(char* fileName){

	struct ChunkSt chn ;
	FILE* file = fopen(fileName, "rb");
	//read first chunk
    readCnk(file, &chn);
	//parse all other chunks
    parseCnk(&model,file,&chn);
    fclose(file);
	
	return 0;
}






 //function to read a ztring
int ModelLoader::getString(char* s,FILE *fp){

	int index = 0;
    char tmp[100] = {0};
    fread(tmp,1,1,fp);
    while(*(tmp + index++)!= 0){
		fread(tmp + index, 1, 1, fp);
    }
    strcpy(s, tmp);
    return (int)(strlen(tmp) + 1);
}



//function to read texture coordinates data
void ModelLoader::readTxtCoord(Model* ob,FILE *fp, struct ChunkSt* chn){
	int numVert = 0;
	Mesh* m = &(ob->meshs[ob->numMeshs-1]);

	//get number of vertices wtih this texture
    chn->bytesRead += fread(&numVert,1,2,fp); 
	//allocate and read in textured vertices
	m->tex =new Texture[numVert];
    chn->bytesRead += fread((void*)m->tex, 1, numVert*sizeof(struct Texture), fp);
	

	std::vector<float> v;
	//convert to array of floats
	for(int x=0; x<numVert; x++){    
		 v.push_back(m->tex[x].tu);
		 v.push_back(m->tex[x].tv);	
    }
	int s=v.size();
	m->textureArray = new float[s];
	for(int i=0;i<s;i++){
		 m->textureArray[i]=v[i];
	}

	skipCnk(fp,chn);
}


//function to read the vertices in the object
void ModelLoader::readVertices(Model* ob,FILE *fp, struct ChunkSt* chn){

	int noVert = 0;
    chn->bytesRead += fread(&noVert, 1, 2, fp);  
 
	Mesh* m = &(ob->meshs[ob->numMeshs-1]);
	m->verts = new Vertice[noVert];
	m->numVert=noVert;
	//read in faces from the file and increment chunk bytes read
	chn->bytesRead += fread((void *)m->verts, 1, noVert*sizeof( struct Vertice), fp);
	//get the total number of vertices
	m->totalNumVerts=m->numVert*3;

	std::vector<float> v;
	for(int x=0; x<noVert; x++){
        
		 v.push_back(m->verts[x].x);
		 v.push_back(m->verts[x].y);
		 v.push_back(m->verts[x].z);		
    }
	int s=v.size();
	m->verticeArray= new float[s];
	for(int i=0;i<s;i++){
		m->verticeArray[i]=v[i];
	}

	skipCnk(fp, chn);
}

//function to read the faces in the object
void ModelLoader::readFaces(Model* ob,FILE *fp, struct ChunkSt* chn){

	int noFaces = 0;
	Mesh* m = &(ob->meshs[ob->numMeshs-1]);
    
    chn->bytesRead += (unsigned int)fread(&noFaces, 1, 2, fp);  
	m->numFace=noFaces;
	m->faces = new Face[noFaces];
	
	//read in faces from the file and increment chunk bytes read
	chn->bytesRead += (unsigned int)fread(m->faces, 1, noFaces*sizeof( struct Face), fp);

	//get the total number of vertices
	m->totalNumFace=m->numFace*3;

	//convert to array of shorts
	std::vector<unsigned short> v;
	for(int x=0; x<noFaces; x++){
        
		 v.push_back(m->faces[x].p1);
		 v.push_back(m->faces[x].p2);
		 v.push_back(m->faces[x].p3);	
    }
	int s=v.size();
	m->faceArray= new unsigned short[s];
	for(int i=0;i<s;i++){
		m->faceArray[i]=v[i];
	}

	//read face material informatin from sub-chunk
	parseCnk(ob,fp,chn);
}





//function to read material mesh
void ModelLoader::readMeshMat(Model* ob,FILE *fp, struct ChunkSt* chn){
	
	char nm[256];
	unsigned short *facesAssigned;
	int numFacesAssigned = 0;
	int len,matID;

	//read  material name
	len = getString(nm,fp);
    chn->bytesRead += len;

	//read how many faces assigned to this material
    chn->bytesRead += (unsigned int)fread(&numFacesAssigned,1,2,fp);
	//allocate memory for faces assigned to this material and read them in
	facesAssigned = new unsigned short[numFacesAssigned];
    chn->bytesRead += (unsigned int)fread(facesAssigned, 1, numFacesAssigned*sizeof(unsigned short), fp);

	//identify material by comparing its name to the list of materials in the object
	for(int i=0; i < ob->numMat; i++){	
		if(strcmp(nm, ob->materials[i].name)==0){
			matID=i;
		}
    }
	//set material ID for each face in the object
	Mesh* m = &(ob->meshs[ob->numMeshs-1]);
    for(int i=0; i<numFacesAssigned; i++){
        int ind = facesAssigned[i];
		m->faces[ind].matID= matID;
    }
	delete[] facesAssigned;
}



//function to skip past a chunk in the file to the start of the next chunk
void ModelLoader::skipCnk(FILE *fp, struct ChunkSt* chn){

	int buf[50000] ={0};
	fread(buf, 1, chn->size - chn->bytesRead, fp);
}


//function to read a chunk of file
void ModelLoader::readCnk(FILE *fp, struct  ChunkSt* chn){

	unsigned int read=0;
	//read in first 6 bytes -first two are chunk id, other 4 are chunk size
	read = fread(&chn->id, 1, 2, fp);
	read += fread(&chn->size, 1, 4, fp);
	chn->bytesRead=read;
}

//function to read object name from chunk 0x4000
void ModelLoader::readObjectName(Model* ob,FILE *fp, struct ChunkSt* chn){
	Mesh* m = &(ob->meshs[ob->numMeshs-1]);
	//read object name
	int characterlen = getString(m->meshName,fp);
    chn->bytesRead += characterlen;
	parseCnk(ob,fp,chn);
}


//get the name of  texture file
void ModelLoader::getTextureFileName(Model* ob,FILE* fp,struct ChunkSt* chn){
	Material* mat = &(ob->materials[ob->numMat-1]);
	chn->bytesRead += getString(mat->textureFile,fp);
}

//get the name of material
void ModelLoader::getMaterialName(Model* ob,FILE* fp,struct ChunkSt* chn){
	Material* mat = &(ob->materials[ob->numMat-1]);
	chn->bytesRead += getString(mat->name,fp);
}

//function to read meterial diffusion
void ModelLoader::readMatDiffuse(Model* ob,FILE *fp, struct ChunkSt* chn){

	//structure to hold color
	struct ColorSt{ unsigned char r, g, b; };
    struct ColorSt diffClr;
	Material* mat = &(ob->materials[ob->numMat-1]);
	//read in header
	char header[6];
    chn->bytesRead += fread(header, 1, 6, fp);
	//read in color
	chn->bytesRead += fread(&diffClr, 1, 3, fp);
	mat->Color.r=diffClr.r;
	mat->Color.g=diffClr.g;
	mat->Color.b=diffClr.b;

	skipCnk(fp,chn);
}


//function to parse a chunk nased on its ID
 void ModelLoader::parseCnk(Model* ob,FILE *fp,struct ChunkSt* chn){

      while(chn->bytesRead < chn->size){
            struct ChunkSt tmp;
            readCnk(fp, &tmp);
#ifdef DEBUG
            printCnkInfo(&tmp);
#endif
			Mesh m;
			switch( tmp.id){
				//chunk wil all object data ,eg materials, vertices ect
			    case EDIT3DS:
					 //recursive call to parse internal chunks of EDIT3DS chunk
					 parseCnk(ob,fp,&tmp);
					 break;
				//material info chunk
				case MATERIAL:
					 Material mat;
					 //add material to object
					 ob->materials.push_back(mat);
                     ob->numMat++;
					 parseCnk(ob,fp,&tmp);
					 break;
				case MAT_NAME:
					  //read material name
					  getMaterialName(ob,fp,&tmp);
					  break;
				//diffuse color chunk
				case MAT_DIFFUSE: 
					  readMatDiffuse(ob,fp,&tmp);
					  break;
				//texture chunk
				case MAT_TEXMAP:
					  //recursive call to parse inner chunks
					  parseCnk(ob,fp,&tmp);
					  break;
				//texture file name
				case MAT_TEXFLNM: 
					 getTextureFileName(ob,fp,&tmp);
					 break;

				// Object section
			    case NAMED_OBJECT: 
					// Mesh m;
					 //add mesh to object
					 ob->meshs.push_back(m);
					 ob->numMeshs++;
				     readObjectName(ob,fp,&tmp);
					 break;
				//object mesh chunk
				case OBJ_MESH:
					 //recursive call to parse vertices and faces
					 parseCnk(ob,fp,&tmp);
					 break;
				case MESH_VERTICES:
					 readVertices(ob,fp,&tmp);
					 break;
			    case MESH_FACES:
					 readFaces(ob,fp,&tmp);
                     break;
				//texture coordinates mapping
				case MESH_TEX_VERT:
					 readTxtCoord(ob,fp,&tmp);
					 break;
				case MESH_MATER: 
				     //read mesh material
				     readMeshMat(ob,fp, &tmp);
					 break;
				default:
					 skipCnk(fp, &tmp);
            }
            chn->bytesRead += tmp.size;
      }
}

//DEBUG things
#ifdef DEBUG

void ModelLoader::test(Model* ob){

      for( int i=0; i<ob->numMeshs; i++ ){
            Mesh* m = &(ob->meshs[i]);
			printf("Shape: %s\n", m->meshName);
            printf("Num faces: %d\n", m->numFace);

            for(int cc=0; cc<m->numFace; cc++){
				printf("\t %i, \t %i \t %i\n",m->faces[cc].p1, m->faces[cc].p2, m->faces[cc].p3);
            }

			printf("Num Vertices: %d\n", m->numVert);
            for(int  x=0; x<m->numVert; x++){
                printf("\t %.2f, \t %.2f \t %.2f\n",m->verts[x].x,m->verts[x].y,m->verts[x].z);
            }

		/*	if( m->tex != nullptr ){
				printf("Num textures: %d\n", m->numVert);
                for(int x=0; x<m->numVert;x++){
					printf("\t %.2f, \t %.2f\n",m->tex[x].tu, m->tex[x].tv );
                }
            }

            if( ob->numMat > 0 ){
                printf("Material vs Faces: %d\n", m->numFace);
                for(int cc=0; cc<m->numFace; cc++){
                    printf("\t MaterialID: %d",m->faces[cc].matID );
                    int ID = m->faces[cc].matID;
					printf("\t, Name: %s\n", ob->materials[ID].name);
                }
            }*/
      }
}

int main(){
	ModelLoader m;
	m.loadModel("box.3DS");
	m.test(&m.model);
	m.getNormals(&m.model.meshs[0]);
	
	for(int i=0;i<m.model.meshs[0].extr.size();i++){

		printf("% .2f ",m.model.meshs[0].extr[i].normal.x);
		printf("% .2f ",m.model.meshs[0].extr[i].normal.y);
		printf("% .2f ",m.model.meshs[0].extr[i].normal.z);
		printf("\n");
	}
	printf("%d",m.model.meshs[0].extr.size());
	


	
	printf("Meshs:%d",m.model.numMeshs);

	return 0;
}

//function to print chunk info
 void ModelLoader::printCnkInfo(struct ChunkSt* chn){
    printf("Chunk ID: 0x %04x   Size of Chunk: %u\n", chn->id, chn->size);
}


#endif




//function to calcute normals array
void ModelLoader::getNormals(Mesh* m){

	for(int x=0;x<m->numFace;x++){
		Extra* e1=new  Extra;
		e1->vertice=m->verts[m->faces[x].p1];

		Extra* e2=new  Extra;
		e2->vertice=m->verts[m->faces[x].p2];

		Extra* e3=new  Extra;
		e3->vertice=m->verts[m->faces[x].p3];

		//calculate edge vectors (p2-p1, p3-p1)
		float ev1x=m->verts[m->faces[x].p2].x - m->verts[m->faces[x].p1].x;
		float ev1y=m->verts[m->faces[x].p2].y - m->verts[m->faces[x].p1].y;
		float ev1z=m->verts[m->faces[x].p2].z - m->verts[m->faces[x].p1].z;

		float ev2x=m->verts[m->faces[x].p3].x - m->verts[m->faces[x].p1].x;
		float ev2y=m->verts[m->faces[x].p3].y - m->verts[m->faces[x].p1].y;
		float ev2z=m->verts[m->faces[x].p3].z - m->verts[m->faces[x].p1].z;

		//calculate normal vector (N=v1*v2)
		float nvx=(ev1y*ev2z)-(ev1z*ev2y);
		float nvy=(ev1z*ev2x)-(ev1x*ev2z);
		float nvz=(ev1x*ev2y)-(ev1y*ev2x);

		//normalise magnitude to 1
		if(nvx!=0)nvx=nvx/abs(nvx);
		if(nvy!=0)nvy=nvy/abs(nvy);
		if(nvz!=0)nvz=nvz/abs(nvz);

		
		//store 

		Vertice* n=new  Vertice;
		n->x=nvx;
		n->y=nvy;
		n->z=nvz;
		e1->normal=*n;
		e2->normal=*n;
		e3->normal=*n;
		
		m->extr.push_back(*e1);
		m->extr.push_back(*e2);
		m->extr.push_back(*e3);
	
	}


}