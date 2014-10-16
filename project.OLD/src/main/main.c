#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "TREE.h"
#include "STACK.h"

#define LAST 4
Head_t *HEAD;
Stack_t* STACK;

Node_t* newNode (int size){
	Node_t *Node;
	Node=(Node_t*) calloc(1,sizeof(Node_t));
			if(Node==NULL){printf("no free memory error\n");exit(0);}
	Node->value = (int*) calloc(size,sizeof(int));
			if(Node->value==NULL){printf("no free memory error\n");exit(0);}
	Node->pointer=(Node_t**) calloc(size+1,sizeof(Node_t *));
		if(Node->pointer==NULL){printf("no free memory error\n");exit(0);}
return Node;
}
void freeNode(Node_t *Node,int size){
	  free(Node->pointer);
	  free(Node->value);
	  free(Node);
}

int removeLeafData(LeafData_t* LeafData,char *subject){
	LeafData_t *tmpLeafPtr;
	tmpLeafPtr=LeafData;
	while(LeafData!=NULL){
	if(strcmp(LeafData->subject_id,subject)==0){
		tmpLeafPtr->next=LeafData->next;
		free(LeafData);
		return 1;
	}
	tmpLeafPtr=LeafData;
	LeafData=LeafData->next;
	}
	return 0;
}

int main (){
	int argc=0; char **argv;
	int bufsize=1024; /*A defined integer for our buffer size*/
	int escape=FALSE;
	char *buf, *tok;


	initialize();
	printCommands();

	buf=(char *) calloc(bufsize,sizeof(char));
		if(buf==NULL){printf("no free memory error\n");exit(0);}
	argv=(char**) calloc(10,sizeof(char*));
		if(argv==NULL){printf("no free memory error\n");exit(0);}

	 while(fgets(buf, bufsize, stdin) != NULL && escape==FALSE){
		 for(tok = strtok(buf," \n");tok;tok=strtok(0," -\r\n")){
				  argv[argc]=(char *)malloc((strlen(tok)+1) * sizeof(char));
				  strcpy(argv[argc],tok);
				  printf("%s ",argv[argc]);
				  argc++;
		  }
		  argc--;
		  if(strcmp(argv[0],"exit")==0){
		  		escape=TRUE;
		  	}else{
		  		if(Parser(argc,argv)!=TRUE)
					{printf("Unknown Command\n");printCommands();}
		  	}
		  printf("\n");
		  for(;argc>=0;argc--){
			  free(argv[argc]);
			  argv[argc]=NULL;
			  }
		  argc=0;
	 }

	 free(argv);
	 free(buf);

return 0;
}