#include "boot.h"
#include "config.h"
#include "cpu.h"


extern void cpuid(int op, int *eax, int *ebx, int *ecx, int *edx)
{
        __asm__("pushl %%ebx\n\t"
        "cpuid\n\t"
        "movl    %%ebx, %%esi\n\t"
        "popl    %%ebx\n\t"
                : "=a" (*eax),
                  "=S" (*ebx),
                  "=c" (*ecx),
                  "=d" (*edx)
                : "a" (op)
                : "cc");
}              

extern void intel_interrupts_on()
{
    unsigned long low, high;
    
   // printk("Disabling local apic...");
    
    /* this is so interrupts work. This is very limited scope --
     * linux will do better later, we hope ...
     */
    rdmsr(0x1b, low, high);
    low &= ~0x800;
    wrmsr(0x1b, low, high);
}

extern void cache_disable(void)
{
    unsigned int tmp;

    /* Disable cache */
    //printk("Disable Cache\n");

    /* Write back the cache and flush TLB */
    asm volatile ("movl  %%cr0, %0\n\t"
              "orl  $0x40000000, %0\n\t"
              "wbinvd\n\t"
              "movl  %0, %%cr0\n\t"
              "wbinvd\n\t"
              : "=r" (tmp) : : "memory");
}

extern void cache_enable(void)
{
    unsigned int tmp;

    asm volatile ("movl  %%cr0, %0\n\t"
              "andl  $0x9fffffff, %0\n\t"
              "movl  %0, %%cr0\n\t"
              :"=r" (tmp) : : "memory");

    //printk("Enable Cache\n");
}


//Thanks to anyone involved in XBMC for CPU freq code!
double RDTSC(void){
    unsigned long a, d;

    rdtsc(a,d);
    debugSPIPrint("d:0x%X, a:0x%X", d, a);
    return (((unsigned long long)a) | (((unsigned long long)d) << 32));
}

extern double getCPUFreq(void){
    double Tcpu_fsb, Tcpu_result, Fcpu;
    u32 Twin_fsb, Twin_result;

    Tcpu_fsb = RDTSC();
    Twin_fsb = IoInputDword(0x8008);

    wait_ms(300);

    Tcpu_result = RDTSC();
    Twin_result = IoInputDword(0x8008);
    
    debugSPIPrint("CPU1: %u, win1: %u, win2: %u", Tcpu_result-Tcpu_fsb, Twin_fsb, Twin_result);

    Fcpu = (Tcpu_result - Tcpu_fsb);
    Fcpu /= (Twin_result - Twin_fsb);
    
    debugSPIPrint("Fcpu: %u", Fcpu / 100.0);

    return Fcpu/100.0;
}
