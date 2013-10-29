#ifndef CAS_H
#define CAS_H

/* 
 * Atomic compare and swap instruction.
 * target : the memory address to perform the atomic op on.
 * cmp    : the value that the address must hold to change it's value.
 * updated: the new value that will be written into target if it is == cmp.
 * return : 0 on successful modification.  !0 otherwise.
 * 
 * Usage
 * -----
 * 
 * Global:
 * volatile int var = 0;
 *
 * In function:
 * int new_var, old_var;
 * do {
 *     old_var = var;
 *     new_var = var;
 *     new_var++;
 * } while (__cas(&var, old_var, new_var));
 * 
 * Now, we know that var is set to new_var.
 */
static inline int 
__cas(unsigned long *target, unsigned long cmp, unsigned long updated)
{
	char z;
	__asm__ __volatile__("lock cmpxchgl %2, %0; setz %1"
			     : "+m" (*target),
			       "=a" (z)
			     : "q"  (updated),
			       "a"  (cmp)
			     : "memory", "cc");
	return (int)!z;
}

#endif
