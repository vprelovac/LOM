#include <stdio.h>
/*
#define	m  (unsigned long)2147483647
#define	q  (unsigned long)127773

#define	a (unsigned int)16807
#define	r (unsigned int)2836
*/

static unsigned long seed;

static unsigned long mcgn, srgn;
extern int mozeranddebug;
#define MULT 69069L

void rstart (long i1, long i2)
{
    mcgn = (unsigned long)((i1 == 0L) ? 0L : i1 | 1L);
    srgn = (unsigned long)((i2 == 0L) ? 0L : (i2 & 0x7FFL) | 1L);
}

long uni(void)
{
    unsigned long r0, r1;

    r0 = (srgn >> 15);
    r1 = srgn ^ r0;
    r0 = (r1 << 17);
    srgn = r0 ^ r1;
    mcgn = MULT * mcgn;
    r1 = mcgn ^ srgn;
    return (r1 >> 1);
}

long vni(void)
{
    unsigned long r0, r1;

    r0 = (srgn >> 15);
    r1 = srgn ^ r0;
    r0 = (r1 << 17);
    srgn = r0 ^ r1;
    mcgn = MULT * mcgn;
    r1 = mcgn ^ srgn;
    return r1;
}




void            my_srand(unsigned long initial_seed)
{
    //seed = initial_seed;
    //fprintf(stderr,  "%ld\r\n", initial_seed);
    rstart(initial_seed, initial_seed/7);
    // srand(initial_seed);
}

void            my_srandp(unsigned long initial_seed)
{
    //seed = initial_seed;
    //fprintf(stderr,  "%ld\r\n", initial_seed);
    rstart(initial_seed, initial_seed/17);
//    srand(initial_seed);
}

unsigned long   my_rand(void)
{
    //unsigned long i=abs(uni()*rand());
    //if (mozeranddebug)
    //fprintf(stderr,  "%ld\r\n", i);
    /*register int    lo,
                    hi,
                    test;

    hi = seed / q;
    lo = seed % q;

    test = a * lo - r * hi;

    if (test > 0)
        seed = test;
    else
        seed = test + m;

    return seed;*/
    //    return (rand());
    //return (((vni()+rand())>>1));
    return (uni());
}
