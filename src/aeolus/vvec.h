#ifndef VVEC_H
#define VVEC_H

/*   Void Vec   */

#include <stdio.h>


#define VoidVec(TYPE)                    TYPE*
#define vvec_new(TYPE)                   _vvec_new(sizeof(TYPE), 0)
#define vvec_new_with_cap(TYPE, CAP)     _vvec_new(sizeof(TYPE), CAP)
#define vvec_repeat_append(VEC, EL, N)   _vvec_repeat_append((void**)&VEC, &EL, N)
#define vvec_push(VEC, EL)               _vvec_push((void**)&VEC, &EL)
#define vvec_insert(VEC, INDEX, EL)      _vvec_insert((Vec*)&VEC, INDEX, &EL)
#define vvec_push_const(TYPE, VEC, EL)\
    {\
        TYPE el = EL;\
        _vvec_push(VEC, &el);\
    }
/* Removes the last element of the vector and returns a pointer to it, casting the return value
   to (TYPE*). The TYPE passed MUST be the type of the elements that the vector contains.
   This macro can help avoid errors by avoiding you to assign the return value of vecPop
   to a wrong pointer type. */
#define vvec_pop(TYPE, VEC)      (TYPE*)_vvec_pop(VEC)
#define vvec_free(TYPE, VEC)     _vvec_free((TYPE)VEC)
/* TYPE is the type CONTAINED in the vector */
#define VVEC_FOREACH(TYPE, EL, VEC) \
    for (TYPE* EL = vvec_begin(VEC); EL != NULL; EL = vvec_next(VEC))

#define VVEC_FOREACH_REV(TYPE, EL, VEC) \
    for (TYPE* EL = vvec_end(VEC); EL != NULL; EL = vvec_prev(VEC))

#define VVEC_FOR(IDX, VEC) \
    for (size_t IDX = 0; IDX < vvec_len(VEC); IDX++)

typedef void* VoidVecType;

void* _vvec_new(size_t sizeof_type, size_t initial_size);
void _vvec_free(VoidVecType vec);

VoidVecType _vvec_repeat_append(VoidVecType* vec, void* el, size_t n);
void* _vvec_push(VoidVecType* vec, void* el);
/* Inserts a new element at the specified position,
   shifting all elements after it to the right */
void* _vvec_insert(VoidVecType* vec, unsigned int index, void* el);
/* Removes and returns the element at the specified position,
   shifting all elements after it to the left */
void* vvec_remove(VoidVecType vec, unsigned int index);
/*
 * Removes the last element of the vector and returns a pointer to it.
 * The element is removed by just decreasing the length of the vector by one,
 * so the returned element remains a valid pointer until a new push
 * operation is performed.
 * If you need to use the returned value of a pop even after making some pushs,
 * remember to copy the content of the memory location referenced by the pop return value:
 *      int* tmp = vecPop(vector);
 *      int x;
 *      memcpy(&x, tmp, sizeof(int));
 *      int newVal = 5;
 *      VECPUSH(vector, newVal);
 *
 * Because vectors must be generic we can only return pointers,
 * and not the actual element because we don't know which type
 * the vector contains.
 * There's the risk of creating error not detected because, by returning a
 * void pointer, that value can be assigned to any pointer.
 * This for example is legal but will probably produce runtime memory errors:
 *      VEC(int) vector = VEC_NEW(int);
 *      char* c = vecPop(vector);
 * It's even easier to make mistakes if the vector contains pointers.
 * The VEC_POP macro can help spotting errors because will cast the return value
 * of vecPop.
 */
void* _vvec_pop(VoidVecType vec);

size_t vvec_len(VoidVecType vec);

void vvec_empty(VoidVecType vec);

/* Resets the vector iterator to its first element and returns that element */
void* vvec_begin(VoidVecType vec);
/* Sets the vector iterator to its last element and returns that element */
void* vvec_end(VoidVecType vec);

void* vvec_next(VoidVecType vec);
void* vvec_prev(VoidVecType vec);
void* vvec_set_at(VoidVecType vec, size_t pos);

void* vvec_curr(VoidVecType vec);
void* vvec_first(VoidVecType vec);
void* vvec_last(VoidVecType vec);

void* vvec_at(VoidVecType vec, size_t pos);
size_t vvec_curr_idx(VoidVecType vec);


#endif