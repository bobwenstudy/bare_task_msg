#include <stdint.h>
#include <stddef.h>
#include "bare_heap.h"

#define BLOCK_ALLOCATED         0x80000000
#define portBYTE_ALIGNMENT      4
#define portBYTE_ALIGNMENT_MASK (portBYTE_ALIGNMENT - 1)
#define HEAP_STRUCT_SIZE        sizeof(BlockLink_t)
#define heapMINIMUM_BLOCK_SIZE  ((uint32_t)(HEAP_STRUCT_SIZE << 1))

/* Define the linked list structure.  This is used to link free blocks in order
 * of their memory address. */
typedef struct A_BLOCK_LINK
{
    struct A_BLOCK_LINK *pxNextFreeBlock; /*<< The next free block in the list. */
    uint32_t xBlockSize;                  /*<< The size of the free block. */
} BlockLink_t;

/* Keeps track of the number of calls to allocate and free memory as well as the
 * number of free bytes remaining, but says nothing about fragmentation. */
static BlockLink_t xStart = {0}, *pxEnd = 0;

static struct xHeapStats
{
    uint32_t xFreeBytesRemaining;
    uint32_t xMinimumEverFreeBytesRemaining;
    uint32_t xNumberOfSuccessfulAllocations;
    uint32_t xNumberOfSuccessfulFrees;
} heapstat;

/*
 * Inserts a block of memory that is being freed into the correct position in
 * the list of free memory blocks.  The block being freed will be merged with
 * the block in front it and/or the block behind it if the memory blocks are
 * adjacent to each other.
 */
static void prvInsertBlockIntoFreeList(BlockLink_t *pxBlockToInsert)
{ /* */
    BlockLink_t *pxIterator;
    uint8_t *puc;

    /* Iterate through the list until a block is found that has a higher address
     * than the block being inserted. */
    for (pxIterator = &xStart; pxIterator->pxNextFreeBlock < pxBlockToInsert;
         pxIterator = pxIterator->pxNextFreeBlock)
    {
        /* Nothing to do here, just iterate to the right position. */
    }

    /* Do the block being inserted, and the block it is being inserted after
     * make a contiguous block of memory? */
    puc = (uint8_t *)pxIterator;

    if ((puc + pxIterator->xBlockSize) == (uint8_t *)pxBlockToInsert)
    {
        pxIterator->xBlockSize += pxBlockToInsert->xBlockSize;
        pxBlockToInsert = pxIterator;
    }

    /* Do the block being inserted, and the block it is being inserted before
     * make a contiguous block of memory? */
    puc = (uint8_t *)pxBlockToInsert;

    if ((puc + pxBlockToInsert->xBlockSize) == (uint8_t *)pxIterator->pxNextFreeBlock)
    {
        if (pxIterator->pxNextFreeBlock != pxEnd)
        {
            /* Form one big block from the two blocks. */
            pxBlockToInsert->xBlockSize += pxIterator->pxNextFreeBlock->xBlockSize;
            pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock->pxNextFreeBlock;
        }
        else
        {
            pxBlockToInsert->pxNextFreeBlock = pxEnd;
        }
    }
    else
    {
        pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;
    }

    /* If the block being inserted plugged a gab, so was merged with the block
     * before and the block after, then it's pxNextFreeBlock pointer will have
     * already been set, and should not be set here as that would make it point
     * to itself. */
    if (pxIterator != pxBlockToInsert)
    {
        pxIterator->pxNextFreeBlock = pxBlockToInsert;
    }
}

/* Create a couple of list links to mark the start and end of the list. */

/*-----------------------------------------------------------*/

void *bare_heap_malloc(uint32_t xWantedSize)
{
    BlockLink_t *pxBlock, *pxPreviousBlock, *pxNewBlockLink;
    void *pvReturn = 0;

    xWantedSize =
            xWantedSize + HEAP_STRUCT_SIZE + (portBYTE_ALIGNMENT_MASK & ~portBYTE_ALIGNMENT_MASK);

    if (xWantedSize <= heapstat.xFreeBytesRemaining)
    {
        /* Traverse the list from the start (lowest address) block until
         * one  of adequate size is found. */
        pxPreviousBlock = &xStart;
        pxBlock = xStart.pxNextFreeBlock;

        while ((pxBlock->xBlockSize < xWantedSize) && (pxBlock->pxNextFreeBlock != 0))
        {
            pxPreviousBlock = pxBlock;
            pxBlock = pxBlock->pxNextFreeBlock;
        }

        /* If the end marker was reached then a block of adequate size
         * was  not found. */
        if (pxBlock != pxEnd)
        {
            /* Return the memory space pointed to - jumping over the
             * BlockLink_t structure at its start. */
            pvReturn = (uint8_t *)pxPreviousBlock->pxNextFreeBlock + HEAP_STRUCT_SIZE;

            /* This block is being returned for use so must be taken out
             * of the list of free blocks. */
            pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

            /* If the block is larger than required it can be split into
             * two. */
            if ((pxBlock->xBlockSize - xWantedSize) > heapMINIMUM_BLOCK_SIZE)
            {
                /* This block is to be split into two.  Create a new
                 * block following the number of bytes requested. The void
                 * cast is used to prevent uint8_t alignment warnings from the
                 * compiler. */
                pxNewBlockLink = (void *)(((uint8_t *)pxBlock) + xWantedSize);

                /* Calculate the sizes of two blocks split from the
                 * single block. */
                pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - xWantedSize;
                pxBlock->xBlockSize = xWantedSize;

                /* Insert the new block into the list of free blocks. */
                prvInsertBlockIntoFreeList(pxNewBlockLink);
            }

            heapstat.xFreeBytesRemaining -= pxBlock->xBlockSize;

            if (heapstat.xFreeBytesRemaining < heapstat.xMinimumEverFreeBytesRemaining)
            {
                heapstat.xMinimumEverFreeBytesRemaining = heapstat.xFreeBytesRemaining;
            }

            /* The block is being returned - it is allocated and owned
             * by the application and has no "next" block. */
            pxBlock->xBlockSize |= BLOCK_ALLOCATED;
            pxBlock->pxNextFreeBlock = 0;
            heapstat.xNumberOfSuccessfulAllocations++;
        }
    }

    return pvReturn;
}

/*-----------------------------------------------------------*/

void bare_heap_free(void *pv)
{
    uint8_t *puc = (uint8_t *)pv;
    BlockLink_t *pxLink;

    if (pv != 0)
    {
        /* The memory being freed will have an BlockLink_t structure immediately
         * before it. */
        puc -= HEAP_STRUCT_SIZE;

        /* This casting is to keep the compiler from issuing warnings. */
        pxLink = (void *)puc;

        /* Check the block is actually allocated. */
        //		ASSERT ((pxLink->xBlockSize & BLOCK_ALLOCATED) != 0);
        //		ASSERT (pxLink->pxNextFreeBlock == 0);

        if ((pxLink->xBlockSize & BLOCK_ALLOCATED) != 0)
        {
            pxLink->xBlockSize &= ~BLOCK_ALLOCATED;
            if (pxLink->pxNextFreeBlock == 0)
            {
                /* Add this block to the list of free blocks. */
                heapstat.xFreeBytesRemaining += pxLink->xBlockSize;
                heapstat.xNumberOfSuccessfulFrees++;
                prvInsertBlockIntoFreeList(((BlockLink_t *)pxLink));
            }
        }
    }
}

uint32_t bare_heap_get_remain_size(void)
{
    return heapstat.xFreeBytesRemaining;
}

int bare_heap_check_empty(uint32_t size)
{
    return size == heapstat.xFreeBytesRemaining + HEAP_STRUCT_SIZE;
}

void bare_heap_init(uint32_t *heap, uint32_t size)
{
    BlockLink_t *pxFirstFreeBlock;
    uint32_t uxAddress;

    /* Ensure the heap starts on a correctly aligned boundary. */
    uxAddress = (uint32_t)heap;

    /* xStart is used to hold a pointer to the first item in the list of free
     * blocks.  The void cast is used to prevent compiler warnings. */
    xStart.pxNextFreeBlock = (void *)uxAddress;
    xStart.xBlockSize = (uint32_t)0;

    /* pxEnd is used to mark the end of the list of free blocks and is inserted
     * at the end of the heap space. */
    pxEnd = (void *)(uxAddress + size - HEAP_STRUCT_SIZE);
    pxEnd->xBlockSize = 0;
    pxEnd->pxNextFreeBlock = 0;

    /* To start with there is a single free block that is sized to take up the
     * entire heap space, minus the space taken by pxEnd. */
    pxFirstFreeBlock = (void *)uxAddress;
    pxFirstFreeBlock->xBlockSize = (pxEnd - pxFirstFreeBlock) * HEAP_STRUCT_SIZE;
    pxFirstFreeBlock->pxNextFreeBlock = pxEnd;

    /* Only one block exists - and it covers the entire usable heap space. */
    heapstat.xMinimumEverFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;
    heapstat.xFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;
    heapstat.xNumberOfSuccessfulAllocations = heapstat.xNumberOfSuccessfulFrees = 0;
}
