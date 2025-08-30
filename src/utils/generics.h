#ifndef GENERICS_H
#define GENERICS_H

#define GenericName(TYPE, PREFIX)\
    IF_UNSIGNED(TYPE)(\
        CAT(GENERIC_, CAT_COMMA(TYPE, PAREN_CLOSE(PREFIX))),\
        IF_STRUCT(TYPE)(\
            CAT(GENERIC_, CAT_COMMA(TYPE, PAREN_CLOSE(PREFIX))),\
            CAT(PREFIX, TYPE)\
        )\
    )


#define GENERIC_unsigned                GENERIC_UNSIGNED(
#define GENERIC_UNSIGNED(TYPE, PREFIX)  CAT(CAT(PREFIX, Unsigned), TYPE)
#define GENERIC_struct                  GENERIC_STRUCT(
#define GENERIC_STRUCT(TYPE, PREFIX)    CAT(CAT(PREFIX, Struct), TYPE)

#endif