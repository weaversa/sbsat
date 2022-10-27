#include "sbsat.h"
#include "sbsat_solvers.h"
#include "solver.h"

//This allocates a new block of smurfs states and attaches them to the previous block
//The blocks are connected by a linked list accessed through ->pNext
void allocate_new_SmurfStatesTable(SmurfStatesTableStruct *arrCurrSmurfStatesTable, uint32_t size) {
  d6_printf2("Adding a new Smurf state table (%p)\n", arrCurrSmurfStatesTable);
  size = SMURF_TABLE_SIZE;
  arrCurrSmurfStatesTable->pNext = (SmurfStatesTableStruct *)sbsat_malloc(1, sizeof(SmurfStatesTableStruct), 9, "arrCurrSmurfStatesTable->pNext");
  arrCurrSmurfStatesTable->pNext->arrStatesTable = (uint8_t *)sbsat_calloc(size, sizeof(uint8_t), 9, "arrCurrSmurfStatesTable->pNext->arrStatesTable");
  arrCurrSmurfStatesTable->pNext->curr_size = 0;
  arrCurrSmurfStatesTable->pNext->max_size = size;
  arrCurrSmurfStatesTable->pNext->pNext = NULL;
}

void FreeSmurfStatesTable(SmurfManager *SM) {
  SmurfStatesTableStruct *pTemp;
  for(SmurfStatesTableStruct *pIter = SM->arrSmurfStatesTableHead; pIter != NULL;) {
    pTemp = pIter;
    pIter = pIter->pNext;
    sbsat_free((void **)&pTemp->arrStatesTable);
    sbsat_free((void **)&pTemp);
  }
  
  SM->arrSmurfStatesTableHead = NULL;
  SM->arrCurrSmurfStatesTable = NULL;
  SM->pSmurfStatesTableTail = NULL;
}

void UnsetAllStatesVisitedFlag(SmurfManager *SM) {
  for(SmurfStatesTableStruct *pIter = SM->arrSmurfStatesTableHead; pIter != NULL; pIter = pIter->pNext) {
    uint8_t *pState = pIter->arrStatesTable;
    uint8_t *pStateMax = pState + SM->arrCurrSmurfStatesTable->max_size;
    do {
      TypeStateEntry *pTypeState = (TypeStateEntry *)pState;
      uintmax_t size = arrStatesTypeSize[pTypeState->type](pTypeState);
      if(pTypeState->type!=FN_FREE_STATE && pTypeState->visited == 1) {
	//Clear the visited flag of this smurf
	pTypeState->visited = 0;
      }
      pState+=size;
    } while(pState < pStateMax);
  }
}

void UnsetActiveStatesVisitedFlag(SmurfManager *SM) {
  SM->pTrueSmurfState->visited = 0;
  
  //Unset 'visited' flag for all states backwards reachable from the current set of states.
  for(uintmax_t nSmurfIndex = 0; nSmurfIndex < SM->nNumSmurfs; nSmurfIndex++) {
    TypeStateEntry *pTypeState = SM->arrSmurfStates.pList[nSmurfIndex].pCurrentState;
    arrUnsetStateVisitedFlag[pTypeState->type](pTypeState);
  }

  //Unset 'visited' flag for all states backwards reachable from the current set of states.
  for(uintmax_t i = 1; i <= SM->pCurrentState_pundoer.nHead; i++) {
    if(SM->pCurrentState_pundoer.pMemcell[i].pAddress != NULL) { //Skip over the marker
      TypeStateEntry *pTypeState = SM->pCurrentState_pundoer.pMemcell[i].pValue;
      arrUnsetStateVisitedFlag[pTypeState->type](pTypeState);
    }
  }
}

void SetActiveStatesVisitedFlag(SmurfManager *SM) {
  SM->pTrueSmurfState->visited = 1;
  
  //Set 'visited' flag for all current states
  for(uintmax_t nSmurfIndex = 0; nSmurfIndex < SM->nNumSmurfs; nSmurfIndex++) {
    TypeStateEntry *pTypeState = SM->arrSmurfStates.pList[nSmurfIndex].pCurrentState;
    arrSetStateVisitedFlag[pTypeState->type](pTypeState);
  }
  
  //Set 'visited' flag for all states backwards reachable from the current set of states.
  for(uintmax_t i = 1; i <= SM->pCurrentState_pundoer.nHead; i++) {
    if(SM->pCurrentState_pundoer.pMemcell[i].pAddress != NULL) { //Skip over the marker
      TypeStateEntry *pTypeState = SM->pCurrentState_pundoer.pMemcell[i].pValue;
      arrSetStateVisitedFlag[pTypeState->type](pTypeState);
    }
  }
}

void GarbageCollectSmurfStatesTable(SmurfManager *SM) {
  d4_printf1("SMURF_GC START...");

  SetActiveStatesVisitedFlag(SM);

  uintmax_t numSmurfStatesGarbageCollected = 0;
  //Free up extra state machine memory
  for(SmurfStatesTableStruct *pIter = SM->arrSmurfStatesTableHead; pIter != NULL; pIter = pIter->pNext) {
    uint8_t *pState = pIter->arrStatesTable;
    uint8_t *pStateMax = pState + SM->arrCurrSmurfStatesTable->max_size;
    do {
      TypeStateEntry *pTypeState = (TypeStateEntry *)pState;
      uintmax_t size = arrStatesTypeSize[pTypeState->type](pTypeState);
      if(pTypeState->type!=FN_FREE_STATE && pTypeState->visited == 0) {
	//Free any extra memory from this smurf, also clear the BDD -> SMURF pointer and dereference the BDD
	arrFreeStateEntry[pTypeState->type](SM, pTypeState);
	memset(pTypeState, FN_FREE_STATE, arrStatesTypeSize[pTypeState->type](pTypeState));
	numSmurfStatesGarbageCollected+=1;
      } else if(pTypeState->type!=FN_FREE_STATE && pTypeState->visited == 1) {
	arrCleanUpStateEntry[pTypeState->type](pTypeState);
      }
      pState+=size;
    } while(pState < pStateMax);
    pIter->curr_size = 0;
  }

  SM->arrCurrSmurfStatesTable = SM->arrSmurfStatesTableHead;
  SM->pSmurfStatesTableTail = (void *)(SM->pTrueSmurfState + 1);
  SM->arrSmurfStatesTableHead->curr_size = sizeof(SmurfStateEntry); //For pTrueSmurfState
  
  UnsetActiveStatesVisitedFlag(SM);

  d6_printf2("SmurfState GC removed %ju Smurf States\n", numSmurfStatesGarbageCollected);

  uintmax_t numBDDNodesGarbageCollected = (uintmax_t) cuddGarbageCollect(SM->dd, 1);
  d6_printf2("BDD GC removed %ju BDD nodes\n", numBDDNodesGarbageCollected);
}

//This checks the size of the current block of the smurf states table.
//If the table is full, we will allocate a new block via allocate_new_SmurfStatesTable
//and increment the tail to the next free spot (assuming the current free spot will be used).
uint8_t check_SmurfStatesTableSize(SmurfManager *SM, uintmax_t size) {
  uint8_t ret = NO_ERROR;

  assert(size >= sizeof(TypeStateEntry));
  while(1) {
    if(SM->arrCurrSmurfStatesTable->curr_size+size >= SM->arrCurrSmurfStatesTable->max_size) {
      //Table out of space - increase size
      if(SM->arrCurrSmurfStatesTable->pNext == NULL) {
	allocate_new_SmurfStatesTable(SM->arrCurrSmurfStatesTable, size);
	SM->arrCurrSmurfStatesTable = SM->arrCurrSmurfStatesTable->pNext;
	SM->pSmurfStatesTableTail = SM->arrCurrSmurfStatesTable->arrStatesTable;
	if(SM->arrCurrSmurfStatesTable->curr_size+size > SM->arrCurrSmurfStatesTable->max_size) {
	  fprintf(stderr, "Increase SMURF_TABLE_SIZE to larger than %ju", size);
	  return MEM_ERR;
	}
      } else {
	SM->arrCurrSmurfStatesTable = SM->arrCurrSmurfStatesTable->pNext;
	SM->pSmurfStatesTableTail = SM->arrCurrSmurfStatesTable->arrStatesTable;
      }
    } else if(((TypeStateEntry *)SM->pSmurfStatesTableTail)->type == FN_FREE_STATE) {
      //Free spots - check for vacancy.
      uint8_t all_clear = 1;
      uint8_t *pTempSSTT = SM->pSmurfStatesTableTail;
      uintmax_t blanks_size = 0;
      for(uintmax_t x = 1; x < size; x++) {
	pTempSSTT += 1;
	TypeStateEntry *pTypeState = (TypeStateEntry *)pTempSSTT;
	blanks_size++;
	if(pTypeState->type != FN_FREE_STATE) {
	  blanks_size += arrStatesTypeSize[pTypeState->type](pTypeState);
	  all_clear = 0;
	  break;
	}
      }
      
      if(all_clear==1) {
	break;
      }

      SM->arrCurrSmurfStatesTable->curr_size += blanks_size;
      SM->pSmurfStatesTableTail = SM->pSmurfStatesTableTail + blanks_size;
    } else {
      //chomp an already allocated state
      assert(arrStatesTypeSize[((TypeStateEntry *)SM->pSmurfStatesTableTail)->type]((TypeStateEntry *)SM->pSmurfStatesTableTail) > 1);
      SM->arrCurrSmurfStatesTable->curr_size+=
	arrStatesTypeSize[((TypeStateEntry *)SM->pSmurfStatesTableTail)->type]((TypeStateEntry *)SM->pSmurfStatesTableTail);
      SM->pSmurfStatesTableTail = SM->pSmurfStatesTableTail + arrStatesTypeSize[((TypeStateEntry *)SM->pSmurfStatesTableTail)->type]((TypeStateEntry *)SM->pSmurfStatesTableTail);
    }
  }
  return ret;
}

void chomp_SmurfStatesTable(SmurfManager *SM, uintmax_t size) {
  SM->arrCurrSmurfStatesTable->curr_size+=size;
  SM->pSmurfStatesTableTail = SM->pSmurfStatesTableTail + size;
}

void FreeSmurfStateEntries(SmurfManager *SM) {
  for(SmurfStatesTableStruct *pIter = SM->arrSmurfStatesTableHead; pIter != NULL;) {
    uint8_t *pState = pIter->arrStatesTable;
    uint8_t *pStateMax = pState + SM->arrCurrSmurfStatesTable->max_size;
    do {
      TypeStateEntry *pTypeState = (TypeStateEntry *)pState;
      assert(pTypeState->type < NUM_SMURF_TYPES);
      uintmax_t size = arrStatesTypeSize[pTypeState->type](pTypeState);
      if(pTypeState->type!=FN_FREE_STATE && pTypeState->visited == 0) {
	arrFreeStateEntry[pTypeState->type](SM, pTypeState);
      }
      pState+=size;
    } while(pState < pStateMax);
    pIter = pIter->pNext;
  }
}
