/*
 * Main.c
 *
 *  Created on: Feb 12, 2013
 *      Author: yakub
 */
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "ChunkCodes.h"

char fileName[] = "sphere.3DS";
//structure fo a vertice
struct verticeSt{ 
	float x, y, z; 
};
//structure for face (a triangle)
struct faceSt{ 
	unsigned short p1, p2, p3; 
};
//chunk structure
struct ChunkSt {
	unsigned short id;
	unsigned int size;
	unsigned int bytesRead;
};

void skipCnk(FILE *fp, struct ChunkSt* chn);
void readCnk(FILE *fp, struct ChunkSt* chn);
void read3DSCnk(FILE *fp, struct ChunkSt* chn);
void printCnkInfo(struct ChunkSt* chn);
void readFaces(FILE *fp, struct ChunkSt* chn);
void readVertices(FILE *fp, struct ChunkSt* chn);
void readObject(FILE *fp, struct ChunkSt* chn);
void readObjectMesh(FILE *fp, struct ChunkSt* chn);
void readMaterial(FILE *fp, struct ChunkSt* chn);
void readMatDiffuse(FILE *fp, struct ChunkSt* chn);
void readTxtVert(FILE *fp, struct ChunkSt* chn);
void readMeshMat(FILE *fp, struct ChunkSt* chn);
int getString(FILE *fp);




int main(){

	//open file
	FILE *fp =fopen(fileName, "rb");
	struct ChunkSt cnk;

	//read in first chunk's data into structure
	readCnk(fp,&cnk);
	printCnkInfo(&cnk);

	//keep reading all the internal chunks of the first chunk (4d4d)
	while(cnk.bytesRead < cnk.size){

		struct ChunkSt tmp;	
		//read chunk
		readCnk(fp, &tmp);
		printCnkInfo(&tmp);

		//if chunk is EDIT3DS chunk, read its chunks
		if(tmp.id == EDIT3DS){
			read3DSCnk(fp,&tmp);
		}

		//skip past just read chunk in the file to start of the next chunk
		skipCnk(fp, &tmp);
		//update bytes read
		cnk.bytesRead +=tmp.size;
	}

	//close the file
	fclose(fp);

	return 0;
}

//function to read a chunk of file
void readCnk(FILE *fp, struct  ChunkSt* chn){

	unsigned int read=0;
	//read in first 6 bytes -first two are chunk id, other 4 are chunk size
	read = fread(&chn->id, 1, 2, fp);
	read += fread(&chn->size, 1, 4, fp);
	chn->bytesRead=read;
}
//function to skip past a chunk in the file to the start of the next chunk
void skipCnk(FILE *fp, struct ChunkSt* chn){

	int buf[50000] ={0};
	fread(buf, 1, chn->size - chn->bytesRead, fp);
}

//function to print chunk info
 void printCnkInfo(struct ChunkSt* chn){
    printf("Chunk ID: 0x %04x   Size of Chunk: %u\n", chn->id, chn->size);
}
//function to read EDTI3DS chunk
void read3DSCnk(FILE *fp, struct ChunkSt* chn){

	while(chn->bytesRead < chn->size){

		struct ChunkSt tmp;
		readCnk(fp, &tmp);
		printCnkInfo(&tmp);
		switch( tmp.id){
			//read named object if chunk id is ox4000
            case NAMED_OBJECT:
				readObject(fp,&tmp);
				break;
			case MATERIAL:
				readMaterial(fp,&tmp);
            default:
                skipCnk(fp, &tmp);
        }
		//increment read bytes of the passed chunk
		chn->bytesRead += tmp.size;
	}
}

//function to read the number of faces in the object
void readFaces(FILE *fp, struct ChunkSt* chn){

	int noFaces = 0,i=0;
	struct faceSt *faces;
    chn->bytesRead += fread(&noFaces, 1, 2, fp);  
    printf( "Number of Faces %u\n", noFaces);
	//allocate memory for face structures
	faces =(struct faceSt*)malloc(sizeof(struct faceSt)*noFaces);
	//read in faces from the file and increment chunk bytes read
	chn->bytesRead += fread(faces, 1, noFaces*sizeof( struct faceSt), fp);

	for(i=0;i<noFaces;i++){
		printf("\tFace: Index1:%d Index2:%d Index3:%d \n",faces[i].p1,faces[i].p2,faces[i].p3);
	}
	free(faces);
	//read face informatin from sub-chunk
	readObjectMesh(fp,chn);
}

//function to read the number of vertices in the object
void readVertices(FILE *fp, struct ChunkSt* chn){

	int noVert = 0,i=0;
	struct verticeSt *verts;
    chn->bytesRead += fread(&noVert, 1, 2, fp);  
    printf( "Number of Vertices %u\n", noVert);

	//allocate memory for vertice structures
	verts =(struct verticeSt*)malloc(sizeof(struct verticeSt)*noVert);
	//read in faces from the file and increment chunk bytes read
	chn->bytesRead += fread((void *)verts, 1, noVert*sizeof( struct verticeSt), fp);
	for(i=0;i<noVert;i++){
		printf("\tVertice: X:%.2f Y:%.2f Z:%.2f \n",verts[i].x,verts[i].y,verts[i].z);
	}
	skipCnk(fp, chn);
	free(verts);
}

//function to read object params
void readObjectMesh(FILE *fp, struct ChunkSt* chn){

	while(chn->bytesRead < chn->size){
		struct ChunkSt tmp;
		readCnk(fp, &tmp);
		printCnkInfo(&tmp);

		switch(tmp.id){
			case MESH_VERTICES:
				readVertices(fp,&tmp);
				break;
			case MESH_FACES:
				readFaces(fp,&tmp);
				break;
			case MESH_TEX_VERT:
                readTxtVert(fp,&tmp);
                break;
            case MESH_MATER:
                readMeshMat(fp,&tmp);
                break;
			default:
				//update curosr in file pointer
				skipCnk(fp,&tmp);
		}
		//increment read bytes of the passed chunk
		chn->bytesRead += tmp.size;
	}
}
//function to read object name
int getString(FILE *fp){

	int index = 0;
	char str[100] = {0};
	fread(str, 1, 1, fp);
	while( *(str + index++) != 0){
		fread(str + index, 1, 1, fp);
	}
	printf("%s \n", str);
	return (strlen(str) + 1);
}

//function to read named object from chunk 0x4000
void readObject(FILE *fp, struct ChunkSt* chn){
	
	//read object name
	int characterlen = getString(fp);
    chn->bytesRead += characterlen;
	//read object data
    while(chn->bytesRead < chn->size) {
        struct ChunkSt tmp = {0};
        readCnk(fp,&tmp);
        printCnkInfo(&tmp);
        switch(tmp.id){
			case OBJ_MESH:
				readObjectMesh(fp, &tmp);
				break;
			default:
				skipCnk(fp, &tmp);
        }
        chn->bytesRead += tmp.size;
    }
}
//function to read material properties
void readMaterial(FILE *fp, struct ChunkSt* chn){

	while(chn->bytesRead < chn->size){
		struct ChunkSt tmp;
		readCnk(fp, &tmp);
		printCnkInfo(&tmp);

		switch(tmp.id){
			case MAT_NAME:
				//read meterial name
				chn->bytesRead +=getString(fp);
				break;
			case MAT_DIFFUSE:
				readMatDiffuse(fp,&tmp);
				break;
			case MAT_TEXMAP:
				//recursive call to read texture
				readMaterial(fp,&tmp);
				break;
			case MAT_TEXFLNM: 
				//read material filename
                chn->bytesRead +=getString(fp);
                break;
			default:
				//update curosr in file pointer
				skipCnk(fp,&tmp);
		}
		//increment read bytes of the passed chunk
		chn->bytesRead += tmp.size;
	}
}

//function to read meterial diffusion
void readMatDiffuse(FILE *fp, struct ChunkSt* chn){

	//structure to hold color
	struct ColorSt{ unsigned char r, g, b; };
    struct ColorSt diffClr;
	//read in header
	char header[6];
    chn->bytesRead += fread(header, 1, 6, fp);
	//read in color
	chn->bytesRead += fread(&diffClr, 1, 3, fp);
	printf("\tColor R:%x G:%x B:%x",diffClr.r,diffClr.g, diffClr.b);
	skipCnk(fp,chn);
}
//function to read texture-vertice data
void readTxtVert(FILE *fp, struct ChunkSt* chn){
	int numVert = 0,i;
	struct TexSt{ float tu, tv; };
	struct TexSt *tex;
    chn->bytesRead += fread(&numVert,1,2,fp); 
	//allocate and read in textured vertices
	tex = (struct TexSt*)malloc(sizeof(struct TexSt)*numVert);
    chn->bytesRead += fread((void*) tex, 1, numVert*sizeof(struct TexSt), fp);
	
	for(i=0;i<numVert;i++){
		printf( "\t Tex Coord: tu: %.2f,  \t tv: %.2f\n",tex[i].tu, tex[i].tv);
	}
	skipCnk(fp,chn);
}

//function to read material mesh
void readMeshMat(FILE *fp, struct ChunkSt* chn){
	
	int *facesAssigned;
	int numFacesAssigned = 0;
	int len,i;

	//read name
	len = getString(fp);
    chn->bytesRead += len;

	//read how many faces assigned to this material
    chn->bytesRead += fread(&numFacesAssigned,1,2,fp);
	//allocate memory for faces assigned to this material and read them in
	facesAssigned = (int*)malloc(sizeof(int)*numFacesAssigned);
    chn->bytesRead += fread(facesAssigned, 1, numFacesAssigned*sizeof(int), fp);

	for(i=0;i<numFacesAssigned;i++){
		printf("\t Face assigned:%d\n",facesAssigned[i]);
	}
	free(facesAssigned);

}