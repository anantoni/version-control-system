#ifndef TREE_H_
#define TREE_H_

#define TRUE 1
#define FALSE 0

typedef struct Node_t{
	struct Node_t **pointer;
	int* value;
}Node_t;

typedef struct LeafData_t{
	struct LeafData_t *next;
	char *subject_id;
	char *subject_name;
	int grade;
}LeafData_t;

typedef struct {
	Node_t *root;
	int height;
}Head_t;

Node_t* newNode (int size);
void freeNode(Node_t *Node,int size);
LeafData_t* newLeaf (int subject_id_length,int subject_name_length);
void addLeafData(LeafData_t* Leaf,LeafData_t *newData);
void printLeafData(LeafData_t* LeafData);
int removeLeafData(LeafData_t* LeafData,char *subject);
int removeAllLeafData(LeafData_t* LeafData);
void printCommands();
int Parser (int argc, char** argv);
void fileParser(char *filename);

void initialize();
void insert (char* studID,char *subjID,char *subjName,int grade);
int lookup (int key);
void rlookup (int key1, int key2);
int delete (int key, char *subject);
int sdelete (int key);

#endif /*TREE_H_*/