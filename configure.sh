#!/bin/sh

CC=cc
AR=ar
INSTALL_DIR=/usr/local/bin
LIB_DIR=/usr/local/lib

usage() {
    cat << EOF
Usage: $0 [OPTION]...
Create config.mk file

OPTIONS:
    -c      Set compiler (default: $CC)
    -a      Set archiver (default: $AR)
    -i      Set install directory (default: $INSTALL_DIR)
    -l      Set linker search directory (default: $LIB_DIR)
    -h      Display this help message
EOF
}

while getopts 'c:a:i:l:h' OPT; do
    case $OPT in
        c)
            CC=$OPTARG
            ;;
        a)
            AR=$OPTARG
            ;;
        i)
            INSTALL_DIR=$OPTARG
            ;;
        l)
            LIB_DIR=$OPTARG
            ;;
        h)
            usage
            exit 0
            ;;
        ?)
            usage
            exit 1
            ;;
    esac
done

cat << EOF > config.mk
CC = $CC
AR = $AR
INSTALL_DIR = $INSTALL_DIR
LIB_DIR = $LIB_DIR
EOF
