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

LeafData_t* newLeaf (int subject_id_length,int subject_name_length){
	LeafData_t *LeafData;
	LeafData=(LeafData_t *) calloc (1,sizeof(LeafData_t));
		if(LeafData==NULL){printf("no free memory error\n");exit(0);}
	LeafData->next=(LeafData_t*) calloc(1,sizeof(LeafData_t *));
		if(LeafData->next==NULL){printf("no free memory error\n");exit(0);}
	LeafData->subject_id = (char*) calloc(subject_id_length+1,sizeof(char));
		if(LeafData->subject_id==NULL){printf("no free memory error\n");exit(0);}
	LeafData->subject_name = (char*) calloc(subject_name_length+1,sizeof(char));
		if(LeafData->subject_name==NULL){printf("no free memory error\n");exit(0);}
return LeafData;
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