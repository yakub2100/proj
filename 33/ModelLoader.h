#ifndef MODEL_LOADER
#define MODEL_LOADER

#include "DataStructures.h"

class ModelLoader
{

public:
	ModelLoader(void);
	~ModelLoader(void);

	Model model;
	int loadModel(char* fileName);
	void test(Model* ob);
	void getNormals(Mesh* m);

private:
	void parseCnk(Model* ob,FILE *fp,struct ChunkSt* chn);
	void readCnk(FILE *fp, struct  ChunkSt* chn);
	void skipCnk(FILE *fp, struct ChunkSt* chn);
	int getString(char* s,FILE *fp);
	void readMeshMat(Model* ob,FILE *fp, struct ChunkSt* chn);
	void readFaces(Model* ob,FILE *fp, struct ChunkSt* chn);
	void readVertices(Model* ob,FILE *fp, struct ChunkSt* chn);
	void readTxtCoord(Model* ob,FILE *fp, struct ChunkSt* chn);
	void readObjectName(Model* ob,FILE *fp, struct ChunkSt* chn);
	void getTextureFileName(Model* ob,FILE* fp,struct ChunkSt* chn);
	void getMaterialName(Model* ob,FILE* fp,struct ChunkSt* chn);
	void readMatDiffuse(Model* ob,FILE *fp, struct ChunkSt* chn);
	
	void printCnkInfo(struct ChunkSt* chn);
	
	

};

#endif