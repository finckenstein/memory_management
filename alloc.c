#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct memoryManagement{
    size_t size;
} metaData;

struct memoryManagement* startOfHeap = NULL;
struct memoryManagement* endOfHeap = NULL;

int alignSize(size_t size){
    if(size%8 != 0){
        int difference = 8 - (size % 8);
        return (size + difference);
    }
    else{
        return size;
    }
}
// void printMetaDataAtBreakpoint(metaData* tempPtr){
//     printf("\n");
//     printf("MY ADDRESS: %ld\n" , (long int) tempPtr);
//     printf("pointer to start of segement: %ld\n" , (long int) &tempPtr[1]);
//     printf("size of segement: %ld\n" , tempPtr->size);
//     printf("pointer to end of segement: %ld\n" , (long int) &tempPtr[abs(tempPtr->size /8) + 1]);
//     printf("\n");
// }
// void printAllMetaData(){
//     metaData* tempPtr = startOfHeap;
//     printf("HEAP STARTS AT: %ld\n" , (long int) startOfHeap);
//     if (startOfHeap == NULL){
//         printf("Heap is empty\n");
//         return;
//     }
//     printf("\nTHIS IS ALL THE METADATA:\n");
//     while(tempPtr <= endOfHeap){
//         printMetaDataAtBreakpoint(tempPtr);
//         tempPtr = &tempPtr[abs(tempPtr->size / 8) + 1];
//     }
//     printf("HEAP ENDS AT: %ld\n" , (long int) endOfHeap);
// }

bool isDataSegementLargeEnough(metaData* ptr, size_t size){
    size_t distance = (&ptr[abs(ptr->size) + 1] - &ptr[1]);
    distance = alignSize(distance);

    return distance >= size;
}

metaData* addressOfSmallerAndFreeSegement(size_t sizeOfBreak){
    metaData* tempPtr = startOfHeap;
    const long int zero = 0;

    if(startOfHeap == NULL){
        return NULL;
    }

    long int segementSize = startOfHeap->size;

    while (tempPtr <= endOfHeap){
        segementSize = (long int) tempPtr->size;
        if(isDataSegementLargeEnough(tempPtr, sizeOfBreak) && zero >= segementSize){
            return tempPtr;
        }
        tempPtr = &tempPtr[abs(tempPtr->size / 8) + 1];
    }
    return NULL;
}


metaData* setProgramBreak(size_t sizeOfBreak){
    metaData* brk = sbrk(sizeof(metaData));
    sbrk(sizeOfBreak);

    brk->size = sizeOfBreak;

    if(startOfHeap == NULL){
        startOfHeap = brk;
    }
    endOfHeap = brk;
    return brk;
}

void useFreeSpace(metaData* ptr, size_t newSize){
    metaData* movedMetaData;
    const size_t sizeOfOldValue = ptr->size;

    if(abs(sizeOfOldValue) == abs(newSize)){
        ptr->size = newSize;
        return;
    }
    else{
        ptr->size = newSize;
        movedMetaData = &ptr[abs(ptr->size / 8) + 1];
        movedMetaData->size = (abs(sizeOfOldValue) - newSize) * -1;
    }

    if(ptr == endOfHeap){
        endOfHeap = movedMetaData;
    }
}

void *mymalloc(size_t size){
    if (size == 0){
        return NULL;
    }

    size = alignSize(size);
    metaData* addrToFreeAndSmallSegement = addressOfSmallerAndFreeSegement(size);

    if(addrToFreeAndSmallSegement != NULL){
        useFreeSpace(addrToFreeAndSmallSegement, size);
        return &addrToFreeAndSmallSegement[1];
    }
    else{
        metaData* programBreak = setProgramBreak(size);
        return &programBreak[1];
    }
}

void *mycalloc(size_t nmemb, size_t size){
    if (nmemb == 0 || size == 0){
        return NULL;
    }

    void* ptrToBlock = mymalloc(size * nmemb);
    memset(ptrToBlock, 0, size * nmemb);
    return ptrToBlock;
}

metaData* getPreviousMetaData(metaData* ptr){
    metaData* tempPtr = startOfHeap;
    while(&tempPtr[abs(tempPtr->size / 8) + 1] <= ptr){
        tempPtr = &tempPtr[abs(tempPtr->size / 8) + 1];
    }
    return tempPtr;
}

void myfree(void *ptr){
    metaData* currMetaData = ptr;
    currMetaData = &currMetaData[-1];

    if(ptr == NULL){
        return;
    }
    currMetaData->size = -1 * currMetaData->size;
}

void *myrealloc(void *ptr, size_t size){

    const int newSize = (int) alignSize(size);
    if (ptr == NULL){
        return mymalloc(size);
    }
    else if (size == 0){
        myfree(ptr);
        return NULL;
    }
    metaData* tempPtr = ptr;
    tempPtr = &tempPtr[-1];
    const int oldSize = (int) tempPtr->size;

    if(newSize > oldSize){
        tempPtr->size = tempPtr->size * -1;
        metaData* newProgramBreak = mymalloc(newSize);
        memcpy(newProgramBreak, ptr, oldSize);
        return newProgramBreak;
    }
    else if(newSize == oldSize){
        return ptr;
    }
    else{
        tempPtr->size = newSize;
        metaData* metaDataIndicatingFreeSegement = &tempPtr[abs(tempPtr->size / 8) + 1];
        metaDataIndicatingFreeSegement->size = (oldSize - newSize) * -1;
        return ptr;
    }

    return NULL;
}


/*
 * Enable the code below to enable system allocator support for your allocator.
 * Doing so will make debugging much harder (e.g., using printf may result in
 * infinite loops).
 */

void *malloc(size_t size) { return mymalloc(size); }
void *calloc(size_t nmemb, size_t size) { return mycalloc(nmemb, size); }
void *realloc(void *ptr, size_t size) { return myrealloc(ptr, size); }
void free(void *ptr) { myfree(ptr); }