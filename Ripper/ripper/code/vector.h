#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 vector.h - header file for vector.c

  This header file, together with vector.c, implement a new data
structure for C, which is basically an infinitely extendable array.
This "vector" type is implemented as a array, plus some additional
information: the physical size of the array, the logical size of the
array, and the size of the individual items in the array.  When
extending a vector would make the logical size exceed the physical
size, the physical size is doubled by allocating a new array, copying
the old information, and freeing the old array.  

  BASIC OPERATIONS: the basic operations defined for this data structure
are the following.  In all of the below, "type" abbreviates any
C type descriptor --- ie anything valid when used as a cast.  

 - Allocate and return (a ptr to) a new vector structure.  Optionally,
   indicate the size that the array will grow to.
	vec_t *new_vec(type)	
	vec_t *new_vecn(type,n)	
 - Deallocate a vector
	void free_vec(type,vec_t *v)
 - Logically empty a vector
	void clear_vec(type, vec_t *v)
 - Find the logical size of the vector
	int vmax(vec_t *v)
 - Return (a ptr to) the i-th element of the vecttor
	type *vref(type,vec_t *v,int i)
 - Set the i-th element of the vector to a new value
	type *vset(type,vec_t *v,int i,type *value)
 - "Extend" a vector by appending a new value to the end of it
	vec_t *ext_vec(type,vec_t *v,type *a)
 - Shorten a vector by logically the last value (or last n values)
Shortening a vector does NOT change the actual data, so a shared
copy of the data (see below) would not be changed.
	vec_t *shorten_vec(type,vec_t *v)
	vec_t *shorten_vecn(type,vec_t *v,int n)
 - Make one vector structure a copy of the second.  No data is
shared between these copies.
	void copy_vec(type,vec_t *v1,vec_t *v2)

   OTHER OPERATIONS: these operations aid in using vectors to
implement ordered and unordered sets.

 * Test if a vector contains a value.
	int vmem(type,vec_t *v,type *value)
 * Test if two vectors are equal (ie have same contents in same order)
	int vequal(type,vec_t *v1, vec_t *v2)
 * Quicksort a vector using a comparison function 
	void vsort(type,vec_t *v,(int *f)(char *,char *))

   OPERATIONS TO ALLOW OPTIMIZATION: finally, a few operations that
can be used to optimize use of arrays wrt time (by finding the
actual C array that stores the data) and wrt space (by allowing
subsequences to be shared.)

 ** Find the base of the array used to hold the data
	type *vbase(type,vec_t *v)
 ** Make one vector structure be a logical read-only copy of the second, 
   but ensure that they initially share the same storage area 
	void share_vec(type,vec_t *v1,vec_t *v2)
	void share_subvec(type,vec_t *v1,vec_t *v2,int lo,int n)

  The type operation is used for two things. 
  1. To cast the result of vrefs and etc to the proper type so that
lint and compilers don't complain (nb. all of these operations are
actually implemented as macros)
  2. To do some partial typechecking. The vector package can't
check that the things stored in the array are of the correct type,
but it does check that they are the same SIZE as the correct type;
this catches some type errors.

 To disable this typechecking, uncommment the "#define NDEBUG" line
below.

******************************************************************************/

#ifndef VECTOR_H
#define VECTOR_H

#ifdef SAVEMEMORY
#define GROW(x) (((int)((x)*1.2)) + 10)
#else
#define GROW(x) ((x)*2)
#endif

#ifdef PORTABLE
#undef assert
#define assert(c)    1
#else
#ifndef ASSERT
/* #define NDEBUG */
#include <assert.h>
#endif
#endif

/* vector type */
typedef struct vec_s {
    int space_sz;
    /* physical dimension */
    int max;
    /* logical dimension */
    int item_sz;
    /* size of each item */
    void *space;      /* memory used to hold the actual data*/
} vec_t;

/* vector functions */
extern void show_vec(vec_t *);
extern vec_t *_new_vec(int, int);
extern void _free_vec(vec_t *v);
extern void *_vref(vec_t *v, int i);
extern void *_vset(vec_t *v, int i, void *a);
extern int _vmem(vec_t *v, void *a);
extern vec_t *_copy_vec(vec_t *v1, vec_t *v2);
extern vec_t *_ext_vec(vec_t *v, void *item);
extern vec_t *_shorten_vecn(vec_t *v, int n);

/* syntax for the vector functions */
#define new_vec(type)       _new_vec(sizeof(type),4)
#define new_vecn(type, n)    _new_vec(sizeof(type),n)
#define free_vec(type, v)    _free_vec(v)
#define clear_vec(type, v)   ((v)->max=0)
#define vmax(v)             ((v)->max)
#define vbase(type, v)       ((type *) v->space)
#define vref(type, v, i)      \
  ((type *) _vref((v),i))
#define vset(type, v, i, a)    \
  (assert(sizeof(type)==sizeof(*a)),\
   assert(sizeof(type)==(v)->item_sz),\
   (type *) _vset((v),i,(void *)a))
#define vequal(type, v1, v2) \
  (assert(sizeof(type)==(v1)->item_sz), \
   assert(sizeof(type)==(v2)->item_sz), \
   !memcmp((char *)(v1),(char *)(v2),sizeof(vec_t)), \
   !memcmp((v1)->space,(v2)->space,(v1)->max*(v1)->item_sz))
#define vmem(type, v, a)      \
  (assert(sizeof(type)==sizeof(*a)),\
   assert(sizeof(type)==(v)->item_sz),\
   _vmem((v),(void *)a))
#define ext_vec(type, v, a)   \
  ( \
   _ext_vec((v),(void *)a))
#define shorten_vec(type, v)   \
  (assert(sizeof(type)==(v)->item_sz),\
   _shorten_vecn(v,1))
#define shorten_vecn(type, v, n)   \
  (assert(sizeof(type)==(v)->item_sz),\
   _shorten_vecn(v,n))
#define vsort(type, v, cmp)     \
  qsort((char *)(v)->space,(v)->max,(v)->item_sz,cmp)
#define share_vec(type, v1, v2) \
   (assert(sizeof(type)==(v2)->item_sz), \
   (vec_t *)memcpy((char *)(v1),(char *)(v2),sizeof(vec_t)))
#define share_subvec(type, v1, v2, lo, n) \
   (assert(sizeof(type)==(v2)->item_sz), \
    ((v1)->item_sz = (v2)->item_sz), \
    ((v1)->space = (void *) (((char *)(v2)->space) + (v1)->item_sz*lo)), \
    ((v1)->space_sz = n*((v1)->item_sz)), \
    ((v1)->max = n), \
    v1)
#define copy_vec(type, v1, v2)  \
  (assert(sizeof(type)==(v2)->item_sz), \
   _copy_vec((v1),(v2)))

#endif
#ifdef __cplusplus
}
#endif
