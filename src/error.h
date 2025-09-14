#ifndef LOKI_ERROR_H
#define LOKI_ERROR_H

#include "aeolus/result.h"

typedef enum {
    SUCCESS = RES_OK,

    ERR_CMD_NOT_KNOWN,
    ERR_CMD_NEW,

    ERR_CORE_APPEND_NEXT_ROW,
} ErrorCode;


#endif