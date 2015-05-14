
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*/
// 20040924 - Updated by dmp to include more str functions, and use ASM
// where possible. ASM shamelessly stolen from linux-2.6.8.1
// include/asm-i386/string.h
#include "boot.h"


void BootPciInterruptEnable()  {    __asm__ __volatile__  (  "sti" ); }


void * memcpy(void * to, const void * from, size_t n)
{
    int d0, d1, d2;
    __asm__ __volatile__(
               "rep ; movsl\n\t"
               "testb $2,%b4\n\t"
              "je 1f\n\t"
               "movsw\n"
              "1:\ttestb $1,%b4\n\t"
             "je 2f\n\t"
             "movsb\n"
               "2:"
             : "=&c" (d0), "=&D" (d1), "=&S" (d2)
        :"0" (n/4), "q" (n),"1" ((long) to),"2" ((long) from)
               : "memory");
    return (to);
}


int strlen(const char * s)
{
    int d0;
    register int __res;
    __asm__ __volatile__(
            "repne\n\t"
               "scasb\n\t"
            "notl %0\n\t"
               "decl %0"
        :"=c" (__res), "=&D" (d0) :"1" (s),"a" (0), "0" (0xffffffffu));
    return __res;
}


int tolower(int ch) 
{
      if ( (unsigned int)(ch - 'A') < 26u )
            ch += 'a' - 'A';
      return ch;
}

int isspace (int c)
{
      if (c == ' ' || c == '\t' || c == '\r' || c == '\n') return 1;
      return 0;
}

void * memset(void *s, int c,  size_t count)
{
      int d0, d1;
    __asm__ __volatile__(
            "rep\n\t"
            "stosb"
            : "=&c" (d0), "=&D" (d1)
            :"a" (c),"1" (s),"0" (count)
            :"memory");
    return s;
}
                

int memcmp(const void * cs,const void * ct,size_t count)
{
        const unsigned char *su1, *su2;
        int res = 0;

    for( su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
        if ((res = *su1 - *su2) != 0) break;
    return res;
}

char * strcpy(char * dest,const char *src)
{
    int d0, d1, d2;
    __asm__ __volatile__(
               "1:\tlodsb\n\t"
            "stosb\n\t"
               "testb %%al,%%al\n\t"
               "jne 1b"
            : "=&S" (d0), "=&D" (d1), "=&a" (d2)
             :"0" (src),"1" (dest) : "memory");
    return dest;
}

char * strncpy(char * dest,const char *src,int count)
{
    int d0, d1, d2, d3;
    __asm__ __volatile__(
            "1:\tdecl %2\n\t"
            "js 2f\n\t"
            "lodsb\n\t"
               "stosb\n\t"
              "testb %%al,%%al\n\t"
            "jne 1b\n\t"
              "rep\n\t"
           "stosb\n"
         "2:"
              : "=&S" (d0), "=&D" (d1), "=&c" (d2), "=&a" (d3)
        :"0" (src),"1" (dest),"2" (count) : "memory");
    return dest;
}

char * strstr(const char * s1,const char * s2)
{
        int l1, l2;
    
    l2 = strlen(s2);
    if (!l2) return (char *) s1;
        l1 = strlen(s1);
    while (l1 >= l2) {
        l1--;
        if (!memcmp(s1,s2,l2)) return (char *) s1;
        s1++;
    }
    return NULL;
}

char * strpbrk(const char * cs,const char * ct)
{
        const char *sc1,*sc2;

        for( sc1 = cs; *sc1 != '\0'; ++sc1) {
        for( sc2 = ct; *sc2 != '\0'; ++sc2) {
            if (*sc1 == *sc2) return (char *) sc1;
        }
    }
    return NULL;
}

char * strsep(char **s, const char *ct)
{
        char *sbegin = *s, *end;

           if (sbegin == NULL) return NULL;

    end = strpbrk(sbegin, ct);
    if (end) *end++ = '\0';
    *s = end;
    return sbegin;
}

int strncmp(const char * cs,const char * ct,size_t count)
{
    register int __res;
    int d0, d1, d2;
    __asm__ __volatile__(
        "1:\tdecl %3\n\t"
        "js 2f\n\t"
        "lodsb\n\t"
        "scasb\n\t"
        "jne 3f\n\t"
        "testb %%al,%%al\n\t"
        "jne 1b\n"
        "2:\txorl %%eax,%%eax\n\t"
        "jmp 4f\n"
        "3:\tsbbl %%eax,%%eax\n\t"
        "orb $1,%%al\n"
        "4:"
            :"=a" (__res), "=&S" (d0), "=&D" (d1), "=&c" (d2)
            :"1" (cs),"2" (ct),"3" (count));
    return __res;
}

int atoi(const char *str)
{
    int i, res = 0; // Initialize result

    // Iterate through all characters of input string and update result
    for (i = 0; str[i] >= '0' && str[i] <= '9'; ++i)
        res = res*10 + str[i] - '0';

    // return result.
    return res;
}

int strcmp(const char *s1, const char *s2)
{
   while (*s1 == *s2++)
       if (*s1++ == 0)
           return (0);
   return (*(const unsigned char *)s1 - *(const unsigned char *)(s2 - 1));
}



char *strrchr0(char *string, char ch) 
{
        char *ptr = string;
    while(*ptr != 0) {
        if(*ptr == ch) {
            return ptr;
        } else {
            ptr++;
        }
    }
    return NULL;
}

void chrreplace(char *string, char search, char ch) {
    char *ptr = string;
    while(*ptr != 0) {
        if(*ptr == search) {
            *ptr = ch;
        } else {
            ptr++;
        }
    }
}

/* Shamelessly copied from linux-2.6.15.1/lib/string.c */
void *memmove(void *dest, const void *src, size_t count)
{
    char *tmp;
    const char *s;

    if (dest <= src) {
        tmp = dest;
        s = src;
        while (count--)
            *tmp++ = *s++;
    } else {
        tmp = dest;
        tmp += count;
        s = src;
        s += count;
        while (count--)
            *--tmp = *--s;
    }
    return dest;
}

/* -------------------------------------------------------------------- */


unsigned int MemoryManagerStartAddress=0;

int MemoryManagerGetFree(void) {
    
    unsigned char *memsmall = (void*)MemoryManagerStartAddress;
    int freeblocks = 0;
    int counter;
    for (counter=0;counter<0x400;counter++) {
        if (memsmall[counter]==0x0) freeblocks++;    
    }
    return freeblocks;
        
}

void MemoryManagementInitialization(void * pvStartAddress, u32 dwTotalMemoryAllocLength)
{
    unsigned char *mem = pvStartAddress;
    //if (dwTotalMemoryAllocLength!=0x1000000) return 1;

    MemoryManagerStartAddress = (unsigned int)pvStartAddress;

    // Prepare the memory cluster Table to be free
    memset(mem,0x0,0x4000);
    // Set the first cluster to be "reserved"
    mem[0] = 0xff;

    return ;
}


void * t_malloc(size_t size)
{
    unsigned char *memsmall = (void*)MemoryManagerStartAddress;
    unsigned int counter;
    unsigned int dummy = 0;
    unsigned int blockcount = 0;
    unsigned int temp;

    // this is for 16 kbyes allocations (quick & fast)
    if (size<(0x4000+1)) {
        for (counter=1;counter<0x400;counter++) {
            if (memsmall[counter]==0)
            {
                memsmall[counter] = 0xAA;
                return (void*)(MemoryManagerStartAddress+counter*0x4000);
            }
        }
        // No free Memory found
        return 0;
    }

    // this is for 64 kbyte allocations (also quick)
    if (size<(0x10000+1)) {
        for (counter=1;counter<0x400;counter++) {
            if (memcmp(&memsmall[counter],&dummy,4)==0)
            {
                dummy = 0xB8BADCFE;
                memcpy(&memsmall[counter],&dummy,4);
                return (void*)(MemoryManagerStartAddress+counter*0x4000);
            }
        }
        // No free Memory found
        return 0;
    }

    if (size<(5*1024*1024+1)) {

        for (counter=1;counter<0x400;counter++) {
            unsigned int needsectory;
            unsigned int foundstart;

            temp = (size & 0xffffc000) + 0x4000;
            needsectory = temp / 0x4000;

            //printf("Need Sectors %x\n",needsectory);

            foundstart = 1;
            for (blockcount=0;blockcount<needsectory;blockcount++) {
                if (memsmall[counter+blockcount]!=0    ) {
                    foundstart = 0;
                    break;
                }
            }

            if (foundstart == 1)
            {
                // We found a free sector
                //printf("Found Sectors Start %x\n",counter);
                memset(&memsmall[counter],0xFF,needsectory);
                memsmall[counter] = 0xBB;
                memsmall[counter+1] = 0xCC;
                memsmall[counter+needsectory-2] = 0xCC;
                memsmall[counter+needsectory-1] = 0xBB;

                return (void*)(MemoryManagerStartAddress+counter*0x4000);
            }

        }
        return 0;
    }

    return 0;

}



void t_free (void *ptr)
{
    unsigned char *memsmall = (void*)MemoryManagerStartAddress;
    unsigned int temp;
    unsigned int dummy = 0;
    unsigned int point = (unsigned int)ptr;

    // this is the offset of the Free thing
    temp = point - MemoryManagerStartAddress;

    if ((temp & 0xffffc000) == temp)
    {
        // Allignement OK
        temp = temp / 0x4000;
        //printf("Free %x\n",temp);

        if (memsmall[temp] == 0xAA)
        {
            // Found Small Block, free it
            ptr = NULL;
            memsmall[temp] = 0x0;
            return;
        }

        dummy = 0xB8BADCFE;
        if (memcmp(&memsmall[temp],&dummy,4)==0)
        {
            // Found 64 K block, free it
            ptr = NULL;
            dummy = 0;
            memset(&memsmall[temp],dummy,4);
            return;
        }


        dummy = 0xFFFFCCBB;
        if (memcmp(&memsmall[temp],&dummy,4)==0)
        {
            unsigned int counter;
            // Found 64 K block, free it
            //printf("Found Big block %x\n",temp);
            ptr = NULL;
            for (counter=temp;counter<0x400;counter++)
            {
                if ((memsmall[counter]==0xCC)&&(memsmall[counter+1]==0xBB))
                {
                    // End detected
                    memsmall[counter]=0;
                    memsmall[counter+1]=0;
                    return;
                }
                memsmall[counter]=0;
            }
            return;
        }

    }

}



void * malloc(size_t size) {

    size_t temp;
    unsigned char *tempmalloc;
    unsigned int *tempmalloc1;
    unsigned int *tempmalloc2;
         __asm__ __volatile__  (  "cli" );
         
    temp = (size+0x200) & 0xffFFff00;

    tempmalloc = t_malloc(temp);
    tempmalloc2 = (unsigned int*)tempmalloc;

    tempmalloc = (unsigned char*)((unsigned int)(tempmalloc+0x100) & 0xffFFff00);
    tempmalloc1 = (unsigned int*)tempmalloc;
    tempmalloc1--;
    tempmalloc1--;
    tempmalloc1[0] = (unsigned int)tempmalloc2;
    tempmalloc1[1] = 0x1234567;
    __asm__ __volatile__  (  "sti" );
        
    return tempmalloc;
}

void free(void *ptr) {

    unsigned int *tempmalloc1;
        __asm__ __volatile__  (  "cli" );
          
          if (ptr == NULL) return;
            
    tempmalloc1 = ptr;
    tempmalloc1-=2;
    ptr = (unsigned int*)tempmalloc1[0];
        if (tempmalloc1[1]!= 0x1234567) {
            __asm__ __volatile__  (  "sti" );
            return ;
    }        
    t_free(ptr);
    __asm__ __volatile__  (  "sti" );

}
 
 



void ListEntryInsertAfterCurrent(LIST_ENTRY *plistentryCurrent, LIST_ENTRY *plistentryNew)
{
    plistentryNew->m_plistentryPrevious=plistentryCurrent;
    plistentryNew->m_plistentryNext=plistentryCurrent->m_plistentryNext;
    plistentryCurrent->m_plistentryNext=plistentryNew;
    if(plistentryNew->m_plistentryNext!=NULL) {
        plistentryNew->m_plistentryNext->m_plistentryPrevious=plistentryNew;
    }
}

void ListEntryRemove(LIST_ENTRY *plistentryCurrent)
{
    if(plistentryCurrent->m_plistentryPrevious) {
        plistentryCurrent->m_plistentryPrevious->m_plistentryNext=plistentryCurrent->m_plistentryNext;
    }
    if(plistentryCurrent->m_plistentryNext) {
        plistentryCurrent->m_plistentryNext->m_plistentryPrevious=plistentryCurrent->m_plistentryPrevious;
    }
}

int _strncmp(const char *sz1, const char *sz2, int nMax)
{
    while((*sz1) && (*sz2) && nMax--) {
    if(*sz1 != *sz2) return (*sz1 - *sz2);
        sz1++; sz2++;
    }
    if(nMax==0) return 0;
    if((*sz1) || (*sz2)) return 0;
    return 0; // used up nMax
}

int _strncasecmp(const char *sz1, const char *sz2, int nMax)
{
    while((*sz1) && (*sz2) && nMax--) {
    if(tolower(*sz1) != tolower(*sz2)) return (*sz1 - *sz2);
        sz1++; sz2++;
    }

    if(nMax==0) return 0;

    if((tolower(*sz1)) || (tolower(*sz2))) return 0;
    return 0; // used up nMax
}

void * MmAllocateContiguousMemoryEx(unsigned int NumberOfBytes,
                                    unsigned long LowestAcceptableAddress,
                                    unsigned long HighestAcceptableAddress,
                                    unsigned long Alignment,
                                    unsigned long Protect
                                   )
/*++

Routine Description:

    This function allocates a range of physically contiguous non-cached,
    non-paged memory.  This is accomplished by using MmAllocateContiguousMemory
    which uses nonpaged pool virtual addresses to map the found memory chunk.

    Then this function establishes another map to the same physical addresses,
    but this alternate map is initialized as non-cached.  All references by
    our caller will be done through this alternate map.

    This routine is designed to be used by a driver's initialization
    routine to allocate a contiguous block of noncached physical memory for
    things like the AGP GART.

Arguments:

    NumberOfBytes - Supplies the number of bytes to allocate.

    LowestAcceptableAddress - Supplies the lowest physical address
                              which is valid for the allocation.  For
                              example, if the device can only reference
                              physical memory in the 8M to 16MB range, this
                              value would be set to 0x800000 (8Mb).

    HighestAcceptableAddress - Supplies the highest physical address
                               which is valid for the allocation.  For
                               example, if the device can only reference
                               physical memory below 16MB, this
                               value would be set to 0xFFFFFF (16Mb - 1).

    Alignment - Supplies the desired page alignment for the allocation.  The
                alignment is treated as a power of two.  The minimum alignment
                is PAGE_SIZE.

    Protect - Supplies the type of protection and cache mapping to use for the
              allocation.

Return Value:

    NULL - a contiguous range could not be found to satisfy the request.

    NON-NULL - Returns a pointer (virtual address in the nonpaged portion
               of the system) to the allocated physically contiguous
               memory.

--*/
{
    unsigned long PhysicalAperture;
    MMPTE TempPte;
    unsigned long LowestAcceptablePageFrameNumber;
    unsigned long HighestAcceptablePageFrameNumber;
    unsigned long PfnAlignment;
    unsigned long PfnAlignmentMask;
    unsigned long NumberOfPages;
    unsigned long PfnAlignmentSubtraction;
    unsigned char OldIrql;
    unsigned long PageFrameNumber;
    PMMPFN PageFrame;
    unsigned long ContiguousCandidatePagesFound;
    unsigned long EndingPageFrameNumber;
    unsigned long PageFrameNumberToGrab;

    if(NumberOfBytes == 0)
        return NULL;

    //
    // Determine which system memory aperature to use.  If this is a video
    // memory request, use the write-combined memory aperture, otherwise use the
    // standard memory aperture.
    //

    if (Protect & PAGE_OLD_VIDEO) {
        PhysicalAperture = MM_WRITE_COMBINE_APERTURE;
        Protect = (Protect & ~PAGE_OLD_VIDEO);
    } else {
        PhysicalAperture = 0;
    }

    //
    // Convert the protect code to a PTE mask.
    //

    if (!MiMakeSystemPteProtectionMask(Protect, &TempPte)) {
        return NULL;
    }

    //
    // Convert the supplied physical addresses into page frame numbers.
    //

    LowestAcceptablePageFrameNumber = (unsigned long)(LowestAcceptableAddress >> PAGE_SHIFT);
    HighestAcceptablePageFrameNumber = (unsigned long)(HighestAcceptableAddress >> PAGE_SHIFT);

    if (HighestAcceptablePageFrameNumber > MM_CONTIGUOUS_MEMORY_LIMIT) {
        HighestAcceptablePageFrameNumber = MM_CONTIGUOUS_MEMORY_LIMIT;
    }

    if (LowestAcceptablePageFrameNumber > HighestAcceptablePageFrameNumber) {
        LowestAcceptablePageFrameNumber = HighestAcceptablePageFrameNumber;
    }

    //
    // Compute the alignment of the allocation in terms of pages.  The alignment
    // should be a power of two.
    //

    ASSERT((Alignment & (Alignment - 1)) == 0);

    PfnAlignment = (unsigned long)(Alignment >> PAGE_SHIFT);

    if (PfnAlignment == 0) {
        PfnAlignment = 1;
    }

    //
    // Compute the alignment mask to round a page frame number down to the
    // nearest alignment boundary.
    //

    PfnAlignmentMask = ~(PfnAlignment - 1);

    //
    // Compute the number of pages to allocate.
    //

    NumberOfPages = BYTES_TO_PAGES(NumberOfBytes);

    //
    // Compute the number of pages to subtract from an aligned page frame number
    // to get to the prior candidate ending page frame number.
    //

    PfnAlignmentSubtraction = ((NumberOfPages + PfnAlignment - 1) &
        PfnAlignmentMask) - NumberOfPages + 1;


/* Deactivated for now
    //
    // Now ensure that we can allocate the required number of pages.
    //

    MI_LOCK_MM(&OldIrql);

    if (MmAvailablePages < NumberOfPages) {
        MI_UNLOCK_MM(OldIrql);
        return NULL;
    }
*/
    //
    // Search the page frame database for a range that satisfies the size and
    // alignment requirements.
    //

    PageFrameNumber = HighestAcceptablePageFrameNumber + 1;

InvalidCandidatePageFound:
    PageFrameNumber = (PageFrameNumber & PfnAlignmentMask) -
        PfnAlignmentSubtraction;
    ContiguousCandidatePagesFound = 0;

    while ((long)PageFrameNumber >= (long)LowestAcceptablePageFrameNumber) {

        PageFrame = MI_PFN_ELEMENT(PageFrameNumber);

        //
        // If we have a page frame that's already being used for a physical
        // mapping, then this is an invalid candidate page.
        //

        if (PageFrame->Pte.Hard.Valid != 0) {
            goto InvalidCandidatePageFound;
        }

        //
        // If we have a page frame that's busy and is locked for I/O, then we
        // can't relocate the page, so this is an invalid candidate page.
        //

        if ((PageFrame->Busy.Busy != 0) && (PageFrame->Busy.LockCount != 0)) {
            goto InvalidCandidatePageFound;
        }

        //
        // This page can be used to help satisfy the request.  If we haven't
        // found the required number of physical pages yet, then continue the
        // search.
        //

        ContiguousCandidatePagesFound++;

        if (ContiguousCandidatePagesFound < NumberOfPages) {
            PageFrameNumber--;
            continue;
        }

        //
        // Verify that the starting page frame number is correctly aligned.
        //

        ASSERT((PageFrameNumber & (PfnAlignment - 1)) == 0);

        //
        // We found a range of physical pages of the requested size.
        //

        EndingPageFrameNumber = PageFrameNumber + NumberOfPages - 1;

        //
        // First, allocate all of the free pages in the range so that any
        // relocations we do won't go into our target range.
        //

        for (PageFrameNumberToGrab = PageFrameNumber;
            PageFrameNumberToGrab <= EndingPageFrameNumber;
            PageFrameNumberToGrab++) {

            PageFrame = MI_PFN_ELEMENT(PageFrameNumberToGrab);

            if (PageFrame->Busy.Busy == 0) {

                //
                // Detach the page from the free list.
                //
/*Deactivate for now
                MiRemovePageFromFreeList(PageFrameNumberToGrab);
*/
                //
                // Convert the page frame to a physically mapped page.
                //

                TempPte.Hard.PageFrameNumber = PageFrameNumberToGrab +
                    PhysicalAperture;
                MI_WRITE_PTE(&PageFrame->Pte, TempPte);

                //
                // Increment the number of physically mapped pages.
                //
/*Deactivated for now
                MmAllocatedPagesByUsage[MmContiguousUsage]++;
*/
            }
        }

        //
        // Second, relocate any non-pinned pages in the range.  The above loop
        // will allocate physically mapped pages and there won't be any pinned
        // pages already existing in the range due to the above candidate page
        // checks.
        //

        for (PageFrameNumberToGrab = PageFrameNumber;
            PageFrameNumberToGrab <= EndingPageFrameNumber;
            PageFrameNumberToGrab++) {

            PageFrame = MI_PFN_ELEMENT(PageFrameNumberToGrab);

            if (PageFrame->Pte.Hard.Valid == 0) {

                //
                // Relocate the page.
                //
/*Deactivate for now
                MiRelocateBusyPage(PageFrameNumberToGrab);
*/
                //
                // Convert the page frame to a physically mapped page.
                //

                TempPte.Hard.PageFrameNumber = PageFrameNumberToGrab +
                    PhysicalAperture;
                MI_WRITE_PTE(&PageFrame->Pte, TempPte);

                //
                // Increment the number of physically mapped pages.
                //
/*Deactivated for now
                MmAllocatedPagesByUsage[MmContiguousUsage]++;
*/
            }
        }

        //
        // Mark the last page of the allocation with a flag so that we
        // can later determine the size of this allocation.
        //

        MI_PFN_ELEMENT(EndingPageFrameNumber)->Pte.Hard.GuardOrEndOfAllocation = 1;

        //
        // Write combined accesses may not check the processor's cache, so force
        // a flush of the TLB and cache now to ensure coherency.
        //
        // Flush the cache for uncached allocations so that all cache lines from
        // the page are out of the processor's caches.  The pages are likely to
        // be shared with an external device and the external device may not
        // snoop cache lines.
        //
/*Deacitvate for now
        if (Protect & (PAGE_WRITECOMBINE | PAGE_NOCACHE)) {
            KeFlushCurrentTbAndInvalidateAllCaches();
        }
*/
//        MI_UNLOCK_MM(OldIrql);

        return MI_CONVERT_PFN_TO_PHYSICAL(PageFrameNumber);
    }

//    MI_UNLOCK_MM(OldIrql);

    return NULL;
}

unsigned long
MmGetPhysicalAddress(
    void * BaseAddress
    )
/*++

Routine Description:

    This function returns the corresponding physical address for a
    valid virtual address.

Arguments:

    BaseAddress - Supplies the virtual address for which to return the
                  physical address.

Return Value:

    Returns the corresponding physical address.

--*/
{
    unsigned long PhysicalAddress;
    PMMPTE PointerPte;
#if DEBUG
    unsigned long PageFrameNumber;
    PMMPFN PageFrame;
#endif

    PointerPte = MiGetPdeAddress(BaseAddress);
    if (PointerPte->Hard.Valid == 0) {
        goto InvalidAddress;
    }

    if (PointerPte->Hard.LargePage == 0) {

        PointerPte = MiGetPteAddress(BaseAddress);
        if (PointerPte->Hard.Valid == 0) {
            goto InvalidAddress;
        }

        PhysicalAddress = BYTE_OFFSET(BaseAddress);

    } else {

        PhysicalAddress = BYTE_OFFSET_LARGE(BaseAddress);
    }

    PhysicalAddress += (PointerPte->Hard.PageFrameNumber << PAGE_SHIFT);

#if DEBUG
    //
    // Verify that the base address is either a physically mapped page (either a
    // contiguous memory allocation or part of XBOXKRNL.EXE) or that it's I/O
    // lock count is non-zero (a page that's been liked with a service like
    // MmLockUnlockBufferPages).
    //

    PageFrameNumber = PhysicalAddress >> PAGE_SHIFT;

    if (PageFrameNumber <= MM_HIGHEST_PHYSICAL_PAGE) {

        PageFrame = MI_PFN_ELEMENT(PageFrameNumber);

        if (PageFrame->Pte.Hard.Valid == 0) {
            ASSERT(PageFrame->Busy.LockCount != 0);
        }
    }
#endif

    return PhysicalAddress;

InvalidAddress:
    printk(("\n         MmGetPhysicalAddress failed, base address was %p", BaseAddress));
    return 0;
}

bool MiMakeSystemPteProtectionMask(
    unsigned long Protect,
    PMMPTE ProtoPte
    )
/*++

Routine Description:

    This routine translates the access protection code used by external APIs
    to the PTE bit mask that implements that policy.

Arguments:

    Protect - Supplies the protection code (e.g., PAGE_READWRITE).

    ProtoPte - Supplies a pointer to the variable that will receive
               the PTE protection mask (e.g., MM_PTE_READWRITE).

Return Value:

    TRUE if the protection code was successfully decoded, else FALSE.

Environment:

    Kernel mode.

--*/
{
    unsigned long Mask;

    Mask = 0;

    //
    // Check for unknown protection bits.
    //

    if (Protect & ~(PAGE_NOCACHE | PAGE_WRITECOMBINE | PAGE_READWRITE |
        PAGE_READONLY)) {
        return false;
    }

    //
    // Only one of the page protection attributes may be specified.
    //

    switch (Protect & (PAGE_READONLY | PAGE_READWRITE)) {

        case PAGE_READONLY:
            Mask = (MM_PTE_VALID_MASK | MM_PTE_DIRTY_MASK | MM_PTE_ACCESS_MASK);
            break;

        case PAGE_READWRITE:
            Mask = (MM_PTE_VALID_MASK | MM_PTE_WRITE_MASK | MM_PTE_DIRTY_MASK |
                MM_PTE_ACCESS_MASK);
            break;

        default:
            return false;
    }

    //
    // Only one of the cache protection attributes may be specified.
    //

    switch (Protect & (PAGE_NOCACHE | PAGE_WRITECOMBINE)) {

        case 0:
            break;

        case PAGE_NOCACHE:
            Mask |= MM_PTE_CACHE_DISABLE_MASK;
            break;

        case PAGE_WRITECOMBINE:
            Mask |= MM_PTE_WRITE_THROUGH_MASK;
            break;

        default:
            return false;
    }

    ProtoPte->Long = Mask;

    return true;
}
