//
// 2020
// OPERATING SYSTEMS
// CPU Schedule Simulator Homework
// Student Number : B811129
// Name : 이세인
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <limits.h>

#define SEED 10

// process states
#define S_IDLE 0			
#define S_READY 1
#define S_BLOCKED 2
#define S_RUNNING 3
#define S_TERMINATE 4

int NPROC, NIOREQ, QUANTUM;

struct ioDoneEvent {
	int procid;
	int doneTime; //iorequest끝난시간
	int len;
	struct ioDoneEvent *prev;
	struct ioDoneEvent *next;
} ioDoneEventQueue, *ioDoneEvent;

struct process {
	int id;
	int len;		// for queues
	int targetServiceTime;
	int serviceTime; //서비스 받은 시간
	int startTime;
	int endTime;
	char state;
	int priority;
	int saveReg0, saveReg1;
	struct process *prev;
	struct process *next;
} *procTable;

struct process idleProc;
struct process readyQueue;
struct process *readyQueueTail = &readyQueue;//me
struct ioDoneEvent *ioDoneEventQueueTail = &ioDoneEventQueue;//me
struct process *blockedNow = NULL;//me
struct process *runningProc = &idleProc;

int cpuReg0, cpuReg1;
int currentTime = 0; //전체 시뮬레이션 시간
int *procIntArrTime, *procServTime, *ioReqIntArrTime, *ioServTime;

int schedule;
void compute() {
	// DO NOT CHANGE THIS FUNCTION
	cpuReg0 = cpuReg0 + runningProc->id;
	cpuReg1 = cpuReg1 + runningProc->id;
	//printf("In computer proc %d cpuReg0 %d\n",runningProc->id,cpuReg0);
}

void initProcTable() {
	int i;
	for (i = 0; i < NPROC; i++) {
		procTable[i].id = i;
		procTable[i].len = 0;
		procTable[i].targetServiceTime = procServTime[i];
		procTable[i].serviceTime = 0;
		procTable[i].startTime = 0;
		procTable[i].endTime = 0;
		procTable[i].state = S_IDLE;
		procTable[i].priority = 0;
		procTable[i].saveReg0 = 0;
		procTable[i].saveReg1 = 0;
		procTable[i].prev = NULL;
		procTable[i].next = NULL;
	}
	for (i = 0; i < NIOREQ; i++) {
		ioDoneEvent[i].prev = NULL;
		ioDoneEvent[i].next = NULL;
	}
	idleProc.id = -1;
	idleProc.state = S_IDLE;
	idleProc.serviceTime = 0;
	idleProc.prev = NULL;
	idleProc.next = NULL;
	cpuReg0=0;
	cpuReg1=0;
}

void procExecSim(struct process *(*scheduler)()) {
	
	int pid, qTime = 0, cpuUseTime = 0, nproc = 0, termProc = 0, nioreq = 0;
	char nextState = S_IDLE;
	int nextForkTime, nextIOReqTime;
	int isrun=0;//me
	int isnewpro=0;
	int isq=0;
	nextForkTime = procIntArrTime[nproc];
	nextIOReqTime = ioReqIntArrTime[nioreq];

	int havetoSche = 0;

	while (1) {
		isnewpro=0;isrun=0;isq=0;
		if (termProc == NPROC) {
			break;//모든 프로세스의 상태가 터미널이면 함수종료
		}
		currentTime++;//시뮬레이션 타임
		qTime++;//퀸텀증가
		runningProc->serviceTime++;
		if (runningProc != &idleProc) cpuUseTime++;//cpu사용하는 시간

		// MUST CALL compute() Inside While loop
		compute();

		if (currentTime == nextForkTime) { /* CASE 2 : a new process created */
			isnewpro=1;
			if(runningProc->state==S_RUNNING)
				isrun=1;
			if (nproc == 0)
			{
				readyQueue.next = &procTable[nproc];
				procTable[nproc].prev = &readyQueue;
				readyQueueTail = &procTable[nproc];
			}
			else if (runningProc->state == S_IDLE) {
				
				readyQueueTail->next = &procTable[nproc];
				procTable[nproc].prev = readyQueueTail;
				readyQueueTail = &procTable[nproc];
				readyQueueTail->state = S_READY;
				
			}
			else {
				int allterminate = 1;
				struct process *b = &readyQueue;

				while (b->next != NULL) {
					b = b->next;
					if (b->state != S_TERMINATE) {
						allterminate = 0;
						break;
					}
				}
				readyQueueTail->next = &procTable[nproc];
				procTable[nproc].prev = readyQueueTail;
				readyQueueTail = &procTable[nproc];

				if (allterminate == 0) {
					runningProc->prev->next = runningProc->next;
					runningProc->next->prev = runningProc->prev;
					readyQueueTail->next = runningProc;
					runningProc->prev = readyQueueTail;
					runningProc->next = NULL;
					readyQueueTail = runningProc;

					runningProc->state = S_READY;
				}
			}
			procTable[nproc].startTime = currentTime;
			procTable[nproc].state = S_READY;//정보 설정

			havetoSche = 1;
			nproc++;
			nextForkTime = currentTime + procIntArrTime[nproc];
		}

		if (qTime == QUANTUM) { /* CASE 1 : The quantum expires */
			isq=1;
			if (runningProc == &idleProc) {}
			else {
				(runningProc->priority)--;
				runningProc->state = S_READY;
				if (runningProc->next != NULL) {//레디큐에 레디상태인 다른 프로세스가 있을때
					runningProc->prev->next = runningProc->next;
					runningProc->next->prev = runningProc->prev;
					readyQueueTail->next = runningProc;
					runningProc->prev = readyQueueTail;
					runningProc->next = NULL;
					readyQueueTail = runningProc;
				}
				havetoSche = 1;
				if(schedule==1){
					struct process *c = &readyQueue;
					int num = 0;//레디큐에 있는 레디상태의 프로세스 개수
					while (c->next != NULL) {
						c = c->next;
						if (c->state == S_READY) {
							num++;
						}
					}
					if (num == 1){
						havetoSche = 1;}
				}
			}
			qTime = 0;
		}

		struct ioDoneEvent* a = &ioDoneEventQueue; /* CASE 3 : IO Done Event */
		char ori = runningProc->state;
		while (a->next != NULL){
			a = a->next;
			if (a->doneTime == currentTime) {

				if(procTable[a->procid].state == S_TERMINATE){//이미 끝남
					if(runningProc->state == S_RUNNING&&runningProc->next!=NULL){
						runningProc->prev->next=runningProc->next;
						runningProc->next->prev=runningProc->prev;
						readyQueueTail->next=runningProc;
						runningProc->prev=readyQueueTail;
						readyQueueTail=runningProc;
						runningProc->next=NULL;
						}	
						runningProc->state=S_READY;
						havetoSche = 1;
						continue;
					}
				if (procTable[a->procid].serviceTime == procTable[a->procid].targetServiceTime){
					procTable[a->procid].state = S_TERMINATE;//동시에 끝남
					havetoSche = 1;
					continue;
				}
				if(ori==S_IDLE&&isnewpro==1){
					procTable[a->procid].prev->next = procTable[a->procid].next;
					procTable[a->procid].next->prev = procTable[a->procid].prev;
					readyQueueTail->next = &procTable[a->procid];
					procTable[a->procid].prev = readyQueueTail;
					procTable[a->procid].next=NULL;
					readyQueueTail = &procTable[a->procid];
					procTable[a->procid].state = S_READY;
					havetoSche = 1;
					continue;
				}
				if(ori==S_IDLE){
					procTable[a->procid].state = S_READY;

					if(procTable[a->procid].next!=NULL){
						procTable[a->procid].prev->next = procTable[a->procid].next;
						procTable[a->procid].next->prev = procTable[a->procid].prev;
						readyQueueTail->next = &procTable[a->procid];
						procTable[a->procid].prev = readyQueueTail;
						procTable[a->procid].next=NULL;
						readyQueueTail = &procTable[a->procid];//블락풀린거뒤로
					}

					havetoSche = 1;
					continue;
				}
				
				if (ori== S_RUNNING||isrun==1||isq==1) {
					runningProc->state = S_READY;
					procTable[a->procid].state = S_READY;

					if(procTable[a->procid].next!=NULL){
					procTable[a->procid].prev->next = procTable[a->procid].next;
					procTable[a->procid].next->prev = procTable[a->procid].prev;
					readyQueueTail->next = &procTable[a->procid];
					procTable[a->procid].prev = readyQueueTail;
					procTable[a->procid].next=NULL;
					readyQueueTail = &procTable[a->procid];//블락풀린거뒤로
					}
					
					runningProc->prev->next=runningProc->next;
					runningProc->next->prev=runningProc->prev;
					readyQueueTail->next=runningProc;
					runningProc->prev=readyQueueTail;
					readyQueueTail=runningProc;
					runningProc->next=NULL;//그리고 나서 실행중이던거 뒤로
					}
				havetoSche = 1;
			}
		}

		if ((nioreq < NIOREQ) && cpuUseTime == nextIOReqTime) { /* CASE 5: reqest IO operations (only when the process does not terminate) */
			
			runningProc->state = S_BLOCKED;
			blockedNow = runningProc;
			if(isq != 1)(runningProc->priority)++;
			ioDoneEvent[nioreq].procid = runningProc->id;
			ioDoneEventQueueTail->next = &ioDoneEvent[nioreq];
			ioDoneEvent[nioreq].prev = ioDoneEventQueueTail;
			ioDoneEventQueueTail = &ioDoneEvent[nioreq];//ioDoneEvent큐에 ioDoneEvent연결

			ioDoneEvent[nioreq].doneTime = currentTime + ioServTime[nioreq];//정보 설정

			havetoSche = 1;
			nioreq++;
			nextIOReqTime = cpuUseTime + ioReqIntArrTime[nioreq];
		}

		if (runningProc->serviceTime == runningProc->targetServiceTime) { /* CASE 4 : the process job done and terminates */
			runningProc->state = S_TERMINATE;
			runningProc->endTime = currentTime;

			runningProc->saveReg0=(runningProc->id)*(runningProc->targetServiceTime);
			runningProc->saveReg1=(runningProc->id)*(runningProc->targetServiceTime);
			termProc++;
			havetoSche = 1;
		}

		if (havetoSche == 1)// call scheduler() if needed
		{
			struct process *e = &readyQueue;
			int isidle = 1;//idle하다
			while (1) {//레디가 하나도 없으면 idle로 바꿈
				e = e->next;
				if (e->state == S_READY) {
					isidle = 0;
					break;
				}
				if(e==readyQueueTail)
				break;
			}
			if (isidle == 1) {
				runningProc = &idleProc;
				runningProc->state=S_IDLE;//?
				qTime=0;
			}
			if (isidle == 0) {
				scheduler();
				qTime = 0;
				runningProc->state = S_RUNNING;	
			}
			havetoSche = 0;
		}
	} // while loop
}

//RR,SJF(Modified),SRTN,Guaranteed Scheduling(modified),Simple Feed Back Scheduling
struct process* RRschedule() {
	struct process *a = &readyQueue;
	while (1) {
		a = a->next;
		if (a->state == S_READY)
			break;
	}
	runningProc = a;
	runningProc->state = S_RUNNING;
}
struct process* SJFschedule() {
	struct process *a = (readyQueue.next);
		if(a->state!=S_READY){
			while(1){
				if(a->state==S_READY)
					break;
				a=a->next;
			}
		}
	
	struct process *min = a;
	while(1){
		if(((min->targetServiceTime) > (a->targetServiceTime))&&(a->state==S_READY))
			min=a;
		if(a->id==readyQueueTail->id)
				break;
		a=a->next;
	}
	runningProc = min;
	runningProc->state = S_RUNNING;
}
struct process* SRTNschedule() {
	struct process *a = (readyQueue.next);
		if(a->state!=S_READY){
			while(1){
				if(a->state==S_READY)
					break;
				a=a->next;
			}
		}
	struct process *min = a;
	while(1){
		if(((min->targetServiceTime-min->serviceTime) > (a->targetServiceTime-a->serviceTime))&&(a->state==S_READY))
			min=a;
		if(a->id==readyQueueTail->id)
				break;
		a=a->next;
	}
	runningProc = min;
	runningProc->state = S_RUNNING;
}
struct process* GSschedule() {
	struct process *a = (readyQueue.next);
		if(a->state!=S_READY){
			while(1){
				if(a->state==S_READY){
					break;
				}
				a=a->next;
			}
		}
	
	struct process *min = a;
	while(1){
		if((((double)(min->serviceTime)/(double)(min->targetServiceTime)) > ((double)(a->serviceTime)/(double)(a->targetServiceTime)))&&(a->state==S_READY))
			min=a;
		if(a->id==readyQueueTail->id)
				break;
		a=a->next;
	}
	runningProc = min;
	runningProc->state = S_RUNNING;
}
struct process* SFSschedule()  {
	struct process *a = (readyQueue.next);
		if(a->state!=S_READY){
			while(1){
				if(a->state==S_READY){
					break;
				}
				a=a->next;
			}
		}
	
	struct process *max = a;
	while(1){
		if(max->priority < a->priority&&(a->state==S_READY))
			max=a;
		if(a->id==readyQueueTail->id)
				break;
		a=a->next;
	}
	runningProc = max;
	runningProc->state = S_RUNNING;
}
void printResult() {
	// DO NOT CHANGE THIS FUNCTION
	int i;
	long totalProcIntArrTime = 0, totalProcServTime = 0, totalIOReqIntArrTime = 0, totalIOServTime = 0;
	long totalWallTime = 0, totalRegValue = 0;
	for (i = 0; i < NPROC; i++) {
		totalWallTime += procTable[i].endTime - procTable[i].startTime;
		/*
		printf("proc %d serviceTime %d targetServiceTime %d saveReg0 %d\n",
			i,procTable[i].serviceTime,procTable[i].targetServiceTime, procTable[i].saveReg0);
		*/
		totalRegValue += procTable[i].saveReg0 + procTable[i].saveReg1;
		/* printf("reg0 %d reg1 %d totalRegValue %d\n",procTable[i].saveReg0,procTable[i].saveReg1,totalRegValue);*/
	}
	for (i = 0; i < NPROC; i++) {
		totalProcIntArrTime += procIntArrTime[i];
		totalProcServTime += procServTime[i];
	}
	for (i = 0; i < NIOREQ; i++) {
		totalIOReqIntArrTime += ioReqIntArrTime[i];
		totalIOServTime += ioServTime[i];
	}

	printf("Avg Proc Inter Arrival Time : %g \tAverage Proc Service Time : %g\n", (float)totalProcIntArrTime / NPROC, (float)totalProcServTime / NPROC);
	printf("Avg IOReq Inter Arrival Time : %g \tAverage IOReq Service Time : %g\n", (float)totalIOReqIntArrTime / NIOREQ, (float)totalIOServTime / NIOREQ);
	printf("%d Process processed with %d IO requests\n", NPROC, NIOREQ);
	printf("Average Wall Clock Service Time : %g \tAverage Two Register Sum Value %g\n", (float)totalWallTime / NPROC, (float)totalRegValue / NPROC);

}


int main(int argc, char *argv[]) {
	// DO NOT CHANGE THIS FUNCTION
	int i;
	int totalProcServTime = 0, ioReqAvgIntArrTime;
	int SCHEDULING_METHOD, MIN_INT_ARRTIME, MAX_INT_ARRTIME, MIN_SERVTIME, MAX_SERVTIME, MIN_IO_SERVTIME, MAX_IO_SERVTIME, MIN_IOREQ_INT_ARRTIME;

	if (argc < 12) {
		printf("%s: SCHEDULING_METHOD NPROC NIOREQ QUANTUM MIN_INT_ARRTIME MAX_INT_ARRTIME MIN_SERVTIME MAX_SERVTIME MIN_IO_SERVTIME MAX_IO_SERVTIME MIN_IOREQ_INT_ARRTIME\n", argv[0]);
		exit(1);
	}

	SCHEDULING_METHOD = atoi(argv[1]);
	NPROC = atoi(argv[2]);
	NIOREQ = atoi(argv[3]);
	QUANTUM = atoi(argv[4]);
	MIN_INT_ARRTIME = atoi(argv[5]);
	MAX_INT_ARRTIME = atoi(argv[6]);
	MIN_SERVTIME = atoi(argv[7]);
	MAX_SERVTIME = atoi(argv[8]);
	MIN_IO_SERVTIME = atoi(argv[9]);
	MAX_IO_SERVTIME = atoi(argv[10]);
	MIN_IOREQ_INT_ARRTIME = atoi(argv[11]);

	printf("SIMULATION PARAMETERS : SCHEDULING_METHOD %d NPROC %d NIOREQ %d QUANTUM %d \n", SCHEDULING_METHOD, NPROC, NIOREQ, QUANTUM);
	printf("MIN_INT_ARRTIME %d MAX_INT_ARRTIME %d MIN_SERVTIME %d MAX_SERVTIME %d\n", MIN_INT_ARRTIME, MAX_INT_ARRTIME, MIN_SERVTIME, MAX_SERVTIME);
	printf("MIN_IO_SERVTIME %d MAX_IO_SERVTIME %d MIN_IOREQ_INT_ARRTIME %d\n", MIN_IO_SERVTIME, MAX_IO_SERVTIME, MIN_IOREQ_INT_ARRTIME);

	srandom(SEED);


	// allocate array structures
	procTable = (struct process *) malloc(sizeof(struct process) * NPROC);
	ioDoneEvent = (struct ioDoneEvent *) malloc(sizeof(struct ioDoneEvent) * NIOREQ);
	procIntArrTime = (int *)malloc(sizeof(int) * NPROC);
	procServTime = (int *)malloc(sizeof(int) * NPROC);
	ioReqIntArrTime = (int *)malloc(sizeof(int) * NIOREQ);
	ioServTime = (int *)malloc(sizeof(int) * NIOREQ);

	// initialize queues
	readyQueue.next = readyQueue.prev = &readyQueue;
	readyQueue.state = S_IDLE;
	ioDoneEventQueue.next = ioDoneEventQueue.prev = NULL;
	ioDoneEventQueue.doneTime = INT_MAX;
	ioDoneEventQueue.procid = -1;
	ioDoneEventQueue.len = readyQueue.len = 0;

	// generate process interarrival times
	for (i = 0; i < NPROC; i++) {
		procIntArrTime[i] = random() % (MAX_INT_ARRTIME - MIN_INT_ARRTIME + 1) + MIN_INT_ARRTIME;
	}

	// assign service time for each process
	for (i = 0; i < NPROC; i++) {
		procServTime[i] = random() % (MAX_SERVTIME - MIN_SERVTIME + 1) + MIN_SERVTIME;
		totalProcServTime += procServTime[i];
	}

	ioReqAvgIntArrTime = totalProcServTime / (NIOREQ + 1);

	// generate io request interarrival time
	for (i = 0; i < NIOREQ; i++) {
		ioReqIntArrTime[i] = random() % ((ioReqAvgIntArrTime - MIN_IOREQ_INT_ARRTIME) * 2 + 1) + MIN_IOREQ_INT_ARRTIME;
	}

	// generate io request service time
	for (i = 0; i < NIOREQ; i++) {
		ioServTime[i] = random() % (MAX_IO_SERVTIME - MIN_IO_SERVTIME + 1) + MIN_IO_SERVTIME;
	}	
#ifdef DEBUG
	// printing process interarrival time and service time
	printf("Process Interarrival Time :\n");
	for (i = 0; i < NPROC; i++) {
		printf("%d ", procIntArrTime[i]);
	}
	printf("\n");
	printf("Process Target Service Time :\n");
	for (i = 0; i < NPROC; i++) {
		printf("%d ", procTable[i].targetServiceTime);
	}
	printf("\n");
#endif

	// printing io request interarrival time and io request service time
	printf("IO Req Average InterArrival Time %d\n", ioReqAvgIntArrTime);
	printf("IO Req InterArrival Time range : %d ~ %d\n", MIN_IOREQ_INT_ARRTIME,
		(ioReqAvgIntArrTime - MIN_IOREQ_INT_ARRTIME) * 2 + MIN_IOREQ_INT_ARRTIME);

#ifdef DEBUG		
	printf("IO Req Interarrival Time :\n");
	for (i = 0; i < NIOREQ; i++) {
		printf("%d ", ioReqIntArrTime[i]);
	}
	printf("\n");
	printf("IO Req Service Time :\n");
	for (i = 0; i < NIOREQ; i++) {
		printf("%d ", ioServTime[i]);
	}
	printf("\n");
#endif

	struct process* (*schFunc)();
	switch (SCHEDULING_METHOD) {
	case 1: schFunc = RRschedule; schedule=1;break;
	case 2: schFunc = SJFschedule; schedule=2;break;
	case 3: schFunc = SRTNschedule; schedule=3;break;
	case 4: schFunc = GSschedule; schedule=4;break;
	case 5: schFunc = SFSschedule; schedule=5;break;
	default: printf("ERROR : Unknown Scheduling Method\n"); exit(1);
	}
	initProcTable();
	procExecSim(schFunc);
	printResult();

}