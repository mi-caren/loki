#include <stdlib.h>
#include <string.h>

#include "vvec.h"


#define VVECHEAD(VEC)    ((VoidVecHeader*)((char*)VEC - sizeof(VoidVecHeader)))

typedef struct {
    size_t len;
    size_t cap;
    size_t sizeof_type;
    size_t cur;
} VoidVecHeader;


void* _vvec_new(size_t sizeof_type, size_t initial_size) {
    if (initial_size == 0) {
        initial_size = 1;
    }

    size_t cap = 1;
    while (cap < initial_size) {
        cap *= 2;
    }

    char* buf = (char*)malloc(sizeof(VoidVecHeader) + sizeof_type * cap);
    if (buf == NULL) return NULL;

    VoidVecHeader* header = (VoidVecHeader*)buf;
    header->len = 0;
    header->cap = cap;
    header->sizeof_type = sizeof_type;
    header->cur = 0;

    return buf + sizeof(VoidVecHeader);
}

inline void _vvec_free(VoidVecType vec) {
    free(VVECHEAD(vec));
}

/* Double the capacity of the vector */
static VoidVecType _vvec_grow(VoidVecType* vec) {
    size_t sizeof_type = VVECHEAD(*vec)->sizeof_type;
    size_t cap = VVECHEAD(*vec)->cap;
    char* new = (char*)realloc(
        (void*)VVECHEAD(*vec),
        sizeof(VoidVecHeader) + sizeof_type*cap*2
    );
    if (new == NULL)
        return NULL;

    // If we just took a Vec as parameter: void* _vecGrow(Vec vec)
    // the operation below could not be possible
    // and the user would be forced to reassign the variable every time he calls _vecGrow:
    // myvec = _vecGrow(myvec);
    *vec = new + sizeof(VoidVecHeader);
    VVECHEAD(*vec)->cap *= 2;

    return *vec;
}

static VoidVecType _vvec_make_space(VoidVecType* vec, size_t space) {
    size_t cap = VVECHEAD(*vec)->cap;
    if (cap >= space)
        return *vec;

    while (cap < space) {
        cap *= 2;
    }

    size_t sizeof_type = VVECHEAD(*vec)->sizeof_type;
    char* new = (char*)realloc(
        (void*)VVECHEAD(*vec),
        sizeof(VoidVecHeader) + sizeof_type*cap
    );
    if (new == NULL)
        return NULL;

    *vec = new + sizeof(VoidVecHeader);
    VVECHEAD(*vec)->cap = cap;

    return *vec;
}

VoidVecType _vvec_repeat_append(VoidVecType* vec, void* el, size_t n) {
    size_t len = VVECHEAD(*vec)->len;
    size_t total_space = len + n;
    if (total_space > VVECHEAD(*vec)->cap) {
        if (_vvec_make_space(vec, total_space) == NULL)
            return NULL;
    }

    size_t sizeof_type = VVECHEAD(*vec)->sizeof_type;
    for (size_t i = 0; i < n; i++) {
        char* dest = (char*)*vec + sizeof_type*(len + i);
        memcpy(
            dest,
            el,
            sizeof_type
        );
    }
    VVECHEAD(*vec)->len += n;

    return *vec;
}

/*
 * Push a new element into the vector.
 * The element must be a void* to let this function.
 * work with any type
 */
void* _vvec_push(VoidVecType* vec, void* el) {
    if (VVECHEAD(*vec)->len == VVECHEAD(*vec)->cap) {
        // vecPush calls _vecGrow which reallocs the vector to increase its size.
        // For this reason we need a Vec* and not just a Vec.
        // In this way we can modify reference to the vector without needing
        // the user to reassign it.
        if (_vvec_grow(vec) == NULL)
            return NULL;
    }

    char* dest = (char*)*vec;
    memcpy(
        dest + VVECHEAD(*vec)->len * VVECHEAD(*vec)->sizeof_type,
        el,
        VVECHEAD(*vec)->sizeof_type
    );
    VVECHEAD(*vec)->len++;

    return *vec;
}

void* _vvec_insert(VoidVecType* vec, unsigned int index, void* el) {
    if (index > VVECHEAD(*vec)->len)
        return NULL;

    if (VVECHEAD(*vec)->len == VVECHEAD(*vec)->cap) {
        if (_vvec_grow(vec) == NULL)
            return NULL;
    }

    memmove(
        (char*)*vec + (index + 1) * VVECHEAD(*vec)->sizeof_type,
        (char*)*vec + index * VVECHEAD(*vec)->sizeof_type,
        VVECHEAD(*vec)->sizeof_type * (VVECHEAD(*vec)->len - index)
    );
    memcpy(
        (char*)*vec + index * VVECHEAD(*vec)->sizeof_type,
        el,
        VVECHEAD(*vec)->sizeof_type
    );
    VVECHEAD(*vec)->len++;

    return *vec;
}

void* vvec_remove(VoidVecType vec, unsigned int index) {
    if (vvec_at(vec, index) == NULL)
        return NULL;

    memmove(
        (char*)vec + index * VVECHEAD(vec)->sizeof_type,
        (char*)vec + (index + 1) * VVECHEAD(vec)->sizeof_type,
        VVECHEAD(vec)->sizeof_type * (VVECHEAD(vec)->len - index - 1)
    );
    VVECHEAD(vec)->len--;

    return vec;
}

void* _vvec_pop(VoidVecType vec) {
    if (vvec_len(vec) == 0) return NULL;

    void* el = vvec_last(vec);

    VVECHEAD(vec)->len--;
    if (VVECHEAD(vec)->cur >= vvec_len(vec))
        VVECHEAD(vec)->cur = vvec_len(vec) - 1;

    return el;
}

inline size_t vvec_len(void* vec) {
    return VVECHEAD(vec)->len;
}

inline void* vvec_begin(VoidVecType vec) {
    VVECHEAD(vec)->cur = 0;
    return vvec_curr(vec);
}

inline void* vvec_end(VoidVecType vec) {
    if (vvec_len(vec) == 0) return NULL;
    VVECHEAD(vec)->cur = vvec_len(vec) - 1;
    return vvec_curr(vec);
}

inline void vvec_empty(void* vec) {
    VVECHEAD(vec)->len = 0;
}

inline static void* _vvec_curr_unchecked(VoidVecType vec) {
    return (char*)vec + VVECHEAD(vec)->cur * VVECHEAD(vec)->sizeof_type;
}

inline void* vvec_next(VoidVecType vec) {
    if (VVECHEAD(vec)->cur >= vvec_len(vec) - 1) {
        return NULL;
    }

    VVECHEAD(vec)->cur++;
    return vvec_curr(vec);
}

inline void* vvec_prev(VoidVecType vec) {
    if (VVECHEAD(vec)->cur == 0) {
        return NULL;
    }

    VVECHEAD(vec)->cur--;
    return vvec_curr(vec);
}

inline void* vvec_curr(VoidVecType vec) {
    if (VVECHEAD(vec)->len == 0) return NULL;
    if (VVECHEAD(vec)->cur >= vvec_len(vec)) return NULL;
    return _vvec_curr_unchecked(vec);
}

inline size_t vvec_curr_idx(VoidVecType vec) {
    return VVECHEAD(vec)->cur;
}

inline void* vvec_first(VoidVecType vec) {
    return vec;
}

inline void* vvec_last(VoidVecType vec) {
    if (vvec_len(vec) == 0) return NULL;
    return (char*)vec + (vvec_len(vec) - 1) * VVECHEAD(vec)->sizeof_type;
}

inline void* vecAt(VoidVecType vec, size_t pos) {
    if (pos >= vvec_len(vec)) return NULL;
    return (char*)vec + pos * VVECHEAD(vec)->sizeof_type;
}

inline void* vvec_set_at(VoidVecType vec, size_t pos) {
    if (pos >= VVECHEAD(vec)->len)
        return NULL;

    VVECHEAD(vec)->cur = pos;
    return _vvec_curr_unchecked(vec);
}
