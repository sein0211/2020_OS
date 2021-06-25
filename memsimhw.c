//
// Virual Memory Simulator Homework
// One-level page table system with FIFO and LRU
// Two-level page table system with LRU
// Inverted page table with a hashing system 
// Submission Year:2020
// Student Name:이세인
// Student Number:B811129
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define PAGESIZEBITS 12			// page size = 4Kbytes
#define VIRTUALADDRBITS 32		// virtual address space size = 4Gbytes

struct procEntry {
	char *traceName;			// the memory trace name
	int pid;					// process (trace) id
	int ntraces;				// the number of memory traces
	int num2ndLevelPageTable;	// The 2nd level page created(allocated);
	int numIHTConflictAccess; 	// The number of Inverted Hash Table Conflict Accesses
	int numIHTNULLAccess;		// The number of Empty Inverted Hash Table Accesses
	int numIHTNonNULLAcess;		// The number of Non Empty Inverted Hash Table Accesses
	int numPageFault;			// The number of page faults
	int numPageHit;				// The number of page hits
	struct firstpageTableEntry *firstPageTable;

	FILE *tracefp;
    unsigned *addr_array;
    struct pageTableEntry *pageTable;

};
struct pageTableEntry{
    int procnum;
    int framenum;
    unsigned address;
    struct pageTableEntry *next;
    struct pageTableEntry *back;
    struct pageTableEntry *nexth;
    struct pageTableEntry *backh;
};
struct firstpageTableEntry{
    int have2ndtable;
    struct pageTableEntry *secondPageTable;
};

void oneLevelVMSim(int simType,int nFrame,int numProcess,struct procEntry procTable[]) {

    int m,i,j;
    if(simType ==0){

        struct pageTableEntry *last_enrtry=NULL;
        struct pageTableEntry *first_enrtry=NULL;
        int fillpart_frame=0;

        for(m=0; m < numProcess; m++){
            procTable[m].pageTable=(struct pageTableEntry*)malloc(sizeof(struct pageTableEntry) * 1048576);

            for(j=0; j<1048576; j++){
    		    procTable[m].pageTable[j].framenum=-1;//page table -1로 초기화
                procTable[m].pageTable[j].procnum =-1;
                procTable[m].pageTable[j].next=NULL;  
		    }
        }
        for(i=0;i<procTable[0].ntraces;i++){//trace개수 만큼 (i번째 주소)
        
            for(m=0;m<numProcess;m++){//프로세스 개수만큼 (m번째 프로세스)

                if(procTable[m].pageTable[procTable[m].addr_array[i]].framenum != -1&&
                procTable[m].pageTable[procTable[m].addr_array[i]].procnum == m){//hit
                    procTable[m].numPageHit++;
                    continue;
                }

                if(procTable[m].pageTable[procTable[m].addr_array[i]].framenum == -1 ||
                (procTable[m].pageTable[procTable[m].addr_array[i]].framenum != -1&&
                procTable[m].pageTable[procTable[m].addr_array[i]].procnum != m)){//fault

                    procTable[m].numPageFault++;
                    if(fillpart_frame != nFrame){//frame이 남아있으면
                        procTable[m].pageTable[procTable[m].addr_array[i]].framenum=0;
                        procTable[m].pageTable[procTable[m].addr_array[i]].procnum=m;
                        procTable[m].pageTable[procTable[m].addr_array[i]].address=procTable[m].addr_array[i];
                        fillpart_frame++;
                        if(last_enrtry==NULL){
                            last_enrtry=&(procTable[m].pageTable[procTable[m].addr_array[i]]);
                            first_enrtry=&(procTable[m].pageTable[procTable[m].addr_array[i]]);
                        }
                        else{
                            last_enrtry->next=&(procTable[m].pageTable[procTable[m].addr_array[i]]);
                            last_enrtry=&(procTable[m].pageTable[procTable[m].addr_array[i]]);
                        }
                        continue;
                    }
                    if(fillpart_frame == nFrame){//frame이 없으면
                        first_enrtry->framenum=-1;
                        first_enrtry->procnum=-1;
                        procTable[m].pageTable[procTable[m].addr_array[i]].framenum=0;
                        procTable[m].pageTable[procTable[m].addr_array[i]].procnum=m;
                        procTable[m].pageTable[procTable[m].addr_array[i]].address=procTable[m].addr_array[i];
                        
                        last_enrtry->next=&(procTable[m].pageTable[procTable[m].addr_array[i]]);
                        last_enrtry=&(procTable[m].pageTable[procTable[m].addr_array[i]]);

                        struct pageTableEntry *a=first_enrtry->next;
                        first_enrtry->next=NULL;
                        first_enrtry= a;
                        continue;
                    }
                }     
            }
        }
    }
    if(simType ==1){
        struct pageTableEntry *last_enrtry=NULL;
        struct pageTableEntry *first_enrtry=NULL;
        int fillpart_frame=0;

        for(m=0; m < numProcess; m++){
            procTable[m].pageTable=(struct pageTableEntry*)malloc(sizeof(struct pageTableEntry) * 1048576);

            for(j=0; j<1048576; j++){
    		    procTable[m].pageTable[j].framenum=-1;//page table -1로 초기화
                procTable[m].pageTable[j].procnum =-1;
                procTable[m].pageTable[j].next=NULL;
                procTable[m].pageTable[j].back=NULL;  
		    }
        }
        int m;
        for(i=0;i<procTable[0].ntraces;i++){//trace개수 만큼 (i번째 주소)
        
            for(m=0;m<numProcess;m++){//프로세스 개수만큼 (m번째 프로세스)

                if(procTable[m].pageTable[procTable[m].addr_array[i]].framenum != -1&&
                procTable[m].pageTable[procTable[m].addr_array[i]].procnum == m){//hit
                    procTable[m].numPageHit++;

                    if(first_enrtry==last_enrtry){}
                    else if(first_enrtry == &(procTable[m].pageTable[procTable[m].addr_array[i]])){//1번

                        struct pageTableEntry *a = first_enrtry->next;
                        first_enrtry->next->back=NULL;
                        first_enrtry->next = NULL;
                        first_enrtry=a;

                        last_enrtry->next = &(procTable[m].pageTable[procTable[m].addr_array[i]]);
                        procTable[m].pageTable[procTable[m].addr_array[i]].back = last_enrtry;
                        last_enrtry=&(procTable[m].pageTable[procTable[m].addr_array[i]]);

                    }
                    else if(last_enrtry != &(procTable[m].pageTable[procTable[m].addr_array[i]])){//2번
                        procTable[m].pageTable[procTable[m].addr_array[i]].back->next = procTable[m].pageTable[procTable[m].addr_array[i]].next;
                        procTable[m].pageTable[procTable[m].addr_array[i]].next->back = procTable[m].pageTable[procTable[m].addr_array[i]].back;
                        procTable[m].pageTable[procTable[m].addr_array[i]].next =NULL;
                        last_enrtry->next=&(procTable[m].pageTable[procTable[m].addr_array[i]]);
                        procTable[m].pageTable[procTable[m].addr_array[i]].back =last_enrtry;
                        last_enrtry=&(procTable[m].pageTable[procTable[m].addr_array[i]]);
                    }
                    continue;
                }

                if(procTable[m].pageTable[procTable[m].addr_array[i]].framenum == -1 ||
                (procTable[m].pageTable[procTable[m].addr_array[i]].framenum != -1&&
                procTable[m].pageTable[procTable[m].addr_array[i]].procnum != m)){//fault

                    procTable[m].numPageFault++;
                    if(fillpart_frame != nFrame){//frame이 남아있으면
                        procTable[m].pageTable[procTable[m].addr_array[i]].framenum=0;
                        procTable[m].pageTable[procTable[m].addr_array[i]].procnum=m;
                        procTable[m].pageTable[procTable[m].addr_array[i]].address=procTable[m].addr_array[i];
                        fillpart_frame++;
                        if(last_enrtry==NULL){
                            last_enrtry=&(procTable[m].pageTable[procTable[m].addr_array[i]]);
                            first_enrtry=&(procTable[m].pageTable[procTable[m].addr_array[i]]);
                        }
                        else{
                            last_enrtry->next=&(procTable[m].pageTable[procTable[m].addr_array[i]]);
                            procTable[m].pageTable[procTable[m].addr_array[i]].back=last_enrtry;
                            last_enrtry=&(procTable[m].pageTable[procTable[m].addr_array[i]]);
                        }
                        continue;
                    }
                    if(fillpart_frame == nFrame){//frame이 없으면
                        first_enrtry->framenum=-1;
                        first_enrtry->procnum=-1;
                        procTable[m].pageTable[procTable[m].addr_array[i]].framenum=0;
                        procTable[m].pageTable[procTable[m].addr_array[i]].procnum=m;
                        procTable[m].pageTable[procTable[m].addr_array[i]].address=procTable[m].addr_array[i];
                        
                        last_enrtry->next=&(procTable[m].pageTable[procTable[m].addr_array[i]]);
                        procTable[m].pageTable[procTable[m].addr_array[i]].back=last_enrtry;
                        last_enrtry=&(procTable[m].pageTable[procTable[m].addr_array[i]]);

                        struct pageTableEntry *a = first_enrtry->next;
                        first_enrtry->next->back=NULL;
                        first_enrtry->next = NULL;
                        first_enrtry=a;
                        continue;
                    }
                }     
            }
        }
    }

	for(i=0; i < numProcess; i++) {
		printf("**** %s *****\n",procTable[i].traceName);
		printf("Proc %d Num of traces %d\n",i,procTable[i].ntraces);
		printf("Proc %d Num of Page Faults %d\n",i,procTable[i].numPageFault);
		printf("Proc %d Num of Page Hit %d\n",i,procTable[i].numPageHit);
		assert(procTable[i].numPageHit + procTable[i].numPageFault == procTable[i].ntraces);
	}
    for(m=0; m < numProcess; m++){
            free(procTable[m].pageTable);}
	
}  
void twoLevelVMSim(int simType,int firstLevelBits,int nFrame,int numProcess,struct procEntry procTable[]) {
	
	    int m,i,j;
        struct pageTableEntry *forprint = NULL;
        struct pageTableEntry *last_enrtry=NULL;
        struct pageTableEntry *first_enrtry=NULL;
        int fillpart_frame=0;

        int a=1;
        int b=1;

        for(i=0;i<firstLevelBits;i++)
            a=a*2;

        for(i=0;i<20-firstLevelBits;i++)
            b=b*2;

        for(m=0; m < numProcess; m++){
            procTable[m].firstPageTable=(struct firstpageTableEntry*)malloc(sizeof(struct firstpageTableEntry) * a);

            for(j=0; j<a; j++){
    		    procTable[m].firstPageTable[j].have2ndtable=-1;
                procTable[m].firstPageTable[j].secondPageTable =NULL; 
		    }
        }
        unsigned PT1;
        unsigned PT2;

        for(i=0;i<procTable[0].ntraces;i++){//trace개수 만큼 (i번째 주소)
        
            for(m=0;m<numProcess;m++){//프로세스 개수만큼 (m번째 프로세스)
                
                PT1 = (procTable[m].addr_array[i]/b);
                PT2 = procTable[m].addr_array[i]-(PT1*b);

                if(procTable[m].firstPageTable[PT1].have2ndtable != -1&&
                    procTable[m].firstPageTable[PT1].secondPageTable[PT2].framenum != -1&&
                    procTable[m].firstPageTable[PT1].secondPageTable[PT2].procnum == m){//hit
                    procTable[m].numPageHit++;

                    if(first_enrtry==last_enrtry){}
                    else if(first_enrtry == &(procTable[m].firstPageTable[PT1].secondPageTable[PT2])){//1번

                        struct pageTableEntry *a = first_enrtry->next;
                        first_enrtry->next->back=NULL;
                        first_enrtry->next = NULL;
                        first_enrtry=a;

                        last_enrtry->next = &(procTable[m].firstPageTable[PT1].secondPageTable[PT2]);
                        procTable[m].firstPageTable[PT1].secondPageTable[PT2].back = last_enrtry;
                        last_enrtry=&(procTable[m].firstPageTable[PT1].secondPageTable[PT2]);

                    }
                    else if(last_enrtry != &(procTable[m].firstPageTable[PT1].secondPageTable[PT2])){//2번
                        procTable[m].firstPageTable[PT1].secondPageTable[PT2].back->next = procTable[m].firstPageTable[PT1].secondPageTable[PT2].next;
                        procTable[m].firstPageTable[PT1].secondPageTable[PT2].next->back = procTable[m].firstPageTable[PT1].secondPageTable[PT2].back;
                        procTable[m].firstPageTable[PT1].secondPageTable[PT2].next =NULL;
                        last_enrtry->next=&(procTable[m].firstPageTable[PT1].secondPageTable[PT2]);
                        procTable[m].firstPageTable[PT1].secondPageTable[PT2].back =last_enrtry;
                        last_enrtry=&(procTable[m].firstPageTable[PT1].secondPageTable[PT2]);
                    }
                    continue;
                }

                else if(procTable[m].firstPageTable[PT1].have2ndtable == -1 ||
                procTable[m].firstPageTable[PT1].secondPageTable[PT2].framenum == -1||
                procTable[m].firstPageTable[PT1].secondPageTable[PT2].procnum != m){//fault

                    procTable[m].numPageFault++;
                    if(fillpart_frame != nFrame){//frame이 남아있으면
                        
                        if(procTable[m].firstPageTable[PT1].secondPageTable == NULL){
                            procTable[m].firstPageTable[PT1].secondPageTable=(struct pageTableEntry*)malloc(sizeof(struct pageTableEntry) * b);
                            for(j=0; j<b; j++){
    		                    procTable[m].firstPageTable[PT1].secondPageTable[j].procnum = -1;
                                procTable[m].firstPageTable[PT1].secondPageTable[j].framenum = -1;
                                procTable[m].firstPageTable[PT1].secondPageTable[j].next = NULL;
                                procTable[m].firstPageTable[PT1].secondPageTable[j].back = NULL;
		                    }
                            procTable[m].num2ndLevelPageTable++;
                            procTable[m].firstPageTable[PT1].have2ndtable=0;
                        }
                        
                        procTable[m].firstPageTable[PT1].secondPageTable[PT2].framenum=0;
                        procTable[m].firstPageTable[PT1].secondPageTable[PT2].procnum=m;
                        procTable[m].firstPageTable[PT1].secondPageTable[PT2].address=procTable[m].addr_array[i];
                        fillpart_frame++;
                        if(last_enrtry==NULL){
                            last_enrtry=&(procTable[m].firstPageTable[PT1].secondPageTable[PT2]);
                            first_enrtry=&(procTable[m].firstPageTable[PT1].secondPageTable[PT2]);
                        }
                        else{
                            last_enrtry->next=&(procTable[m].firstPageTable[PT1].secondPageTable[PT2]);
                            procTable[m].firstPageTable[PT1].secondPageTable[PT2].back=last_enrtry;
                            last_enrtry=&(procTable[m].firstPageTable[PT1].secondPageTable[PT2]);
                        }
                        continue;
                    }
                    if(fillpart_frame == nFrame){//frame이 없으면
                        first_enrtry->framenum=-1;
                        first_enrtry->procnum=-1;
                        if(procTable[m].firstPageTable[PT1].secondPageTable == NULL){
                            procTable[m].firstPageTable[PT1].secondPageTable=(struct pageTableEntry*)malloc(sizeof(struct pageTableEntry) * b);
                            for(j=0; j<b; j++){
    		                    procTable[m].firstPageTable[PT1].secondPageTable[j].procnum = -1;
                                procTable[m].firstPageTable[PT1].secondPageTable[j].framenum = -1;
                                procTable[m].firstPageTable[PT1].secondPageTable[j].next = NULL;
                                procTable[m].firstPageTable[PT1].secondPageTable[j].back = NULL;
		                    }
                            procTable[m].num2ndLevelPageTable++;
                            procTable[m].firstPageTable[PT1].have2ndtable=0;
                        }
                        procTable[m].firstPageTable[PT1].secondPageTable[PT2].framenum=0;
                        procTable[m].firstPageTable[PT1].secondPageTable[PT2].procnum=m;
                        procTable[m].firstPageTable[PT1].secondPageTable[PT2].address=procTable[m].addr_array[i];
                        
                        last_enrtry->next=&(procTable[m].firstPageTable[PT1].secondPageTable[PT2]);
                        procTable[m].firstPageTable[PT1].secondPageTable[PT2].back=last_enrtry;
                        last_enrtry=&(procTable[m].firstPageTable[PT1].secondPageTable[PT2]);

                        struct pageTableEntry *a = first_enrtry->next;
                        first_enrtry->next->back=NULL;
                        first_enrtry->next = NULL;
                        first_enrtry=a;
                        continue;
                    }
                }     
            }
        }

    for(i=0; i < numProcess; i++) {
		printf("**** %s *****\n",procTable[i].traceName);
		printf("Proc %d Num of traces %d\n",i,procTable[i].ntraces);
		printf("Proc %d Num of second level page tables allocated %d\n",i,procTable[i].num2ndLevelPageTable);
		printf("Proc %d Num of Page Faults %d\n",i,procTable[i].numPageFault);
		printf("Proc %d Num of Page Hit %d\n",i,procTable[i].numPageHit);
		assert(procTable[i].numPageHit + procTable[i].numPageFault == procTable[i].ntraces);
	}
    for(m=0; m < numProcess; m++){
        free(procTable[m].firstPageTable);}
    
}

void invertedPageVMSim(int simType,int nFrame,int numProcess,struct procEntry procTable[]) {

    int i,j,m;
    struct pageTableEntry *last_enrtry=NULL;
    struct pageTableEntry *first_enrtry=NULL;
    struct pageTableEntry **hash_table=malloc(sizeof(struct pageTableEntry*) * nFrame);
    int fillpart_frame=0;
    int ishit=0;
    for(m=0; m < numProcess; m++){
        procTable[m].pageTable=(struct pageTableEntry*)malloc(sizeof(struct pageTableEntry) * 1048576);

        for(j=0; j<1048576; j++){
    		procTable[m].pageTable[j].framenum=-1;//page table -1로 초기화
            procTable[m].pageTable[j].procnum =-1;
            procTable[m].pageTable[j].next=NULL;
            procTable[m].pageTable[j].back=NULL; 
            procTable[m].pageTable[j].nexth=NULL;
            procTable[m].pageTable[j].backh=NULL; 
		}
    }

    for(i=0; i<nFrame; i++){
        hash_table[i]=NULL; 
	}

    for(i=0;i<procTable[0].ntraces;i++){//trace개수 만큼 (i번째 주소)
        
        for(m=0;m<numProcess;m++){//프로세스 개수만큼 (m번째 프로세스)

            struct pageTableEntry *forprint = NULL;
            int hash_index = (procTable[m].addr_array[i]+m) % nFrame;
            ishit=0;
            if(hash_table[hash_index]==NULL){procTable[m].numIHTNULLAccess++;}

            if(hash_table[hash_index]!=NULL){//노드 달려있으면
                procTable[m].numIHTNonNULLAcess++;
                struct pageTableEntry *p=malloc(sizeof(struct pageTableEntry));
                p=hash_table[hash_index];
                while(p!=NULL){
                    procTable[m].numIHTConflictAccess++;
                    if(p->framenum != -1&&p->procnum == m&&p->address==procTable[m].addr_array[i]){
                        ishit=1;
                        break;
                    }
                    p=p->nexth;
                }

                if(ishit==1){//hit
                    procTable[m].numPageHit++;

                    if(first_enrtry==last_enrtry){}
                    else if(first_enrtry == &(procTable[m].pageTable[procTable[m].addr_array[i]])){//1번

                        struct pageTableEntry *a = first_enrtry->next;
                        first_enrtry->next->back=NULL;
                        first_enrtry->next = NULL;
                        first_enrtry=a;

                        last_enrtry->next = &(procTable[m].pageTable[procTable[m].addr_array[i]]);
                        procTable[m].pageTable[procTable[m].addr_array[i]].back = last_enrtry;
                        last_enrtry=&(procTable[m].pageTable[procTable[m].addr_array[i]]);

                    }
                    else if(last_enrtry != &(procTable[m].pageTable[procTable[m].addr_array[i]])){//2번
                        procTable[m].pageTable[procTable[m].addr_array[i]].back->next = procTable[m].pageTable[procTable[m].addr_array[i]].next;
                        procTable[m].pageTable[procTable[m].addr_array[i]].next->back = procTable[m].pageTable[procTable[m].addr_array[i]].back;
                        procTable[m].pageTable[procTable[m].addr_array[i]].next =NULL;
                        last_enrtry->next=&(procTable[m].pageTable[procTable[m].addr_array[i]]);
                        procTable[m].pageTable[procTable[m].addr_array[i]].back =last_enrtry;
                        last_enrtry=&(procTable[m].pageTable[procTable[m].addr_array[i]]);
                    }
                    continue;
                }
            }   
            

            if(hash_table[hash_index]==NULL||ishit==0){//fault

                procTable[m].numPageFault++;
                if(fillpart_frame != nFrame){//frame이 남아있으면

                    procTable[m].pageTable[procTable[m].addr_array[i]].framenum=0;
                    procTable[m].pageTable[procTable[m].addr_array[i]].procnum=m;
                    procTable[m].pageTable[procTable[m].addr_array[i]].address=procTable[m].addr_array[i];

                    if(hash_table[hash_index]==NULL){
                        
                        hash_table[hash_index]=&(procTable[m].pageTable[procTable[m].addr_array[i]]);
                        procTable[m].pageTable[procTable[m].addr_array[i]].backh=hash_table[hash_index];
                    }
                    else{
                        procTable[m].pageTable[procTable[m].addr_array[i]].nexth=hash_table[hash_index];
                        procTable[m].pageTable[procTable[m].addr_array[i]].nexth->backh=&(procTable[m].pageTable[procTable[m].addr_array[i]]);
                        procTable[m].pageTable[procTable[m].addr_array[i]].backh=hash_table[hash_index];
                        hash_table[hash_index]=&(procTable[m].pageTable[procTable[m].addr_array[i]]);
                    }

                    fillpart_frame++;
                    procTable[m].pageTable[procTable[m].addr_array[i]].framenum=0;
                        procTable[m].pageTable[procTable[m].addr_array[i]].procnum=m;
                        procTable[m].pageTable[procTable[m].addr_array[i]].address=procTable[m].addr_array[i];
                    
                        if(last_enrtry==NULL){
                            last_enrtry=&(procTable[m].pageTable[procTable[m].addr_array[i]]);
                            first_enrtry=&(procTable[m].pageTable[procTable[m].addr_array[i]]);
                        }
                        else{
                            last_enrtry->next=&(procTable[m].pageTable[procTable[m].addr_array[i]]);
                            procTable[m].pageTable[procTable[m].addr_array[i]].back=last_enrtry;
                            last_enrtry=&(procTable[m].pageTable[procTable[m].addr_array[i]]);
                        }
                    continue;
                }
                if(fillpart_frame == nFrame){//frame이 없으면
                
                    int index = (first_enrtry->address+first_enrtry->procnum) % nFrame;
                    if(hash_table[index]==first_enrtry){//맨앞
                        if(first_enrtry->nexth==NULL){//에 하나
                            hash_table[index]=NULL;
                        }
                        else{//뒤에 달려있음
                            hash_table[index]=first_enrtry->nexth;
                            first_enrtry->nexth->backh=hash_table[index];
                            first_enrtry->backh=NULL;
                            first_enrtry->nexth=NULL;
                        }
                    }
                    else{//맨앞이 아닐때
                        struct pageTableEntry *a=hash_table[index];
                        while(a!=NULL){
                            if(a==first_enrtry){
                                break;
                            }
                            a=a->nexth;
                        }
                        if(a->nexth==NULL){//맨 끝
                            a->backh->nexth=NULL;
                            a->back=NULL;
                        }
                        else{//중간
                            a->backh->nexth=a->nexth;
                            a->nexth->backh=a->backh;
                            a->nexth=NULL;
                            a->backh=NULL;
                        }
                    }

                    if(hash_table[hash_index]==NULL){
                        hash_table[hash_index]=&(procTable[m].pageTable[procTable[m].addr_array[i]]);
                        procTable[m].pageTable[procTable[m].addr_array[i]].backh=hash_table[hash_index];
                    }
                    else{
                        
                        procTable[m].pageTable[procTable[m].addr_array[i]].nexth=hash_table[hash_index];
                        procTable[m].pageTable[procTable[m].addr_array[i]].nexth->backh=&(procTable[m].pageTable[procTable[m].addr_array[i]]);
                        procTable[m].pageTable[procTable[m].addr_array[i]].backh=hash_table[hash_index];
                        hash_table[hash_index]=&(procTable[m].pageTable[procTable[m].addr_array[i]]);
                    }

                    first_enrtry->framenum=-1;
                    first_enrtry->procnum=-1;
                    procTable[m].pageTable[procTable[m].addr_array[i]].framenum=0;
                    procTable[m].pageTable[procTable[m].addr_array[i]].procnum=m;
                    procTable[m].pageTable[procTable[m].addr_array[i]].address=procTable[m].addr_array[i];
                        
                    last_enrtry->next=&(procTable[m].pageTable[procTable[m].addr_array[i]]);
                    procTable[m].pageTable[procTable[m].addr_array[i]].back=last_enrtry;
                    last_enrtry=&(procTable[m].pageTable[procTable[m].addr_array[i]]);

                    struct pageTableEntry *a = first_enrtry->next;
                    first_enrtry->next->back=NULL;
                    first_enrtry->next = NULL;
                    first_enrtry=a;
                    continue;

                    
                }
            }     
        }
    }
    

    for(i=0; i < numProcess; i++) {
		printf("**** %s *****\n",procTable[i].traceName);
		printf("Proc %d Num of traces %d\n",i,procTable[i].ntraces);
		printf("Proc %d Num of Inverted Hash Table Access Conflicts %d\n",i,procTable[i].numIHTConflictAccess);
		printf("Proc %d Num of Empty Inverted Hash Table Access %d\n",i,procTable[i].numIHTNULLAccess);
		printf("Proc %d Num of Non-Empty Inverted Hash Table Access %d\n",i,procTable[i].numIHTNonNULLAcess);
		printf("Proc %d Num of Page Faults %d\n",i,procTable[i].numPageFault);
		printf("Proc %d Num of Page Hit %d\n",i,procTable[i].numPageHit);
		assert(procTable[i].numPageHit + procTable[i].numPageFault == procTable[i].ntraces);
		assert(procTable[i].numIHTNULLAccess + procTable[i].numIHTNonNULLAcess == procTable[i].ntraces);
	}
    for(m=0; m < numProcess; m++){
        free(procTable[m].pageTable);}
}

int main(int argc, char *argv[]) {
	int i,c, simType;
    int numProcess =0;
    int firstLevelBits,PhysicalMemorySizeBits,TraceFileNames;
    if (argc < 5) {
		printf("%s: memsim [-s] simType firstLevelBits PhysicalMemorySizeBits TraceFileNames...\n", argv[0]);
		exit(1);
	}

	simType= atoi(argv[1]);
	firstLevelBits = atoi(argv[2]);
	PhysicalMemorySizeBits = atoi(argv[3]);

    int nFrame=1;
    for(i=0;i<PhysicalMemorySizeBits-12;i++){
        nFrame*=2;
    }
    int phyMemSizeBits=1;
    for(i=0;i<PhysicalMemorySizeBits;i++){
        phyMemSizeBits=phyMemSizeBits*2;
    }
    i=4;
	while(argv[i]!=NULL){
        numProcess++;
        i++;
    }
    struct procEntry procTable[numProcess];

    for(i=0;i<numProcess;i++){
        procTable[i].addr_array=(unsigned*)malloc(sizeof(unsigned) * 1000000);
        procTable[i].pid=i;
        procTable[i].ntraces=0;
        procTable[i].num2ndLevelPageTable=0;
        procTable[i].numIHTConflictAccess=0;
        procTable[i].numIHTNULLAccess=0;
        procTable[i].numIHTNonNULLAcess=0;
        procTable[i].numPageFault=0;
	    procTable[i].numPageHit=0;
        procTable[i].traceName=argv[i+4];//파일 이름 삽입

        printf("process %d opening %s\n",i,argv[i+4]);
        procTable[i].tracefp=fopen(procTable[i].traceName, "r");//파일 열기

        if (procTable[i].tracefp == NULL) {
			printf("ERROR: can't open %s file; exiting...",argv[i+4]);
			exit(1);
		}
    }

    int j;
    for(i=0;i<1000000;i++){
        unsigned addr;
        char rw;

        for(j=0;j<numProcess;j++){
            if(feof(procTable[j].tracefp)){
                for(j=0;j<numProcess;j++){
                fclose(procTable[j].tracefp);}
                goto Exit;
            }
            fscanf(procTable[j].tracefp, "%x %c\n",&addr, &rw);
            procTable[j].addr_array[i]=(addr/4096);
            procTable[j].ntraces++;
        }
    }

    Exit:

	printf("Num of Frames %d Physical Memory Size %ld bytes\n",nFrame, phyMemSizeBits);
	
	if (simType == 0) {
		printf("=============================================================\n");
		printf("The One-Level Page Table with FIFO Memory Simulation Starts .....\n");
		printf("=============================================================\n");
		oneLevelVMSim(simType,nFrame,numProcess,procTable);
	}
	
	if (simType == 1) {
		printf("=============================================================\n");
		printf("The One-Level Page Table with LRU Memory Simulation Starts .....\n");
		printf("=============================================================\n");
		oneLevelVMSim(simType,nFrame,numProcess,procTable);
	}
	
	if (simType == 2) {
		printf("=============================================================\n");
		printf("The Two-Level Page Table Memory Simulation Starts .....\n");
		printf("=============================================================\n");
		twoLevelVMSim(simType,firstLevelBits,nFrame,numProcess,procTable);
	}
	
	if (simType == 3) {
		printf("=============================================================\n");
		printf("The Inverted Page Table Memory Simulation Starts .....\n");
		printf("=============================================================\n");
		invertedPageVMSim(simType,nFrame,numProcess,procTable);
	}

	return(0);
}

