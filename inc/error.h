#ifndef LOKI_ERROR_H
#define LOKI_ERROR_H

#include "utils/result.h"

typedef enum {
    SUCCESS = OK_CODE,

    ERR_TERM_READ_ATTR,
    ERR_TERM_WRITE_ATTR,
    ERR_TERM_GET_CURSOR_POS,
    ERR_TERM_ESC_SEQ,

    ERR_CMD_NOT_KNOWN,
    ERR_CMD_NEW,
} ErrorCode;


#endif