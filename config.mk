CC = gcc
AR = ar

INSTALL_DIR = /usr/local

CPPFLAGS = -MMD -Isrc
CFLAGS = -Wall -Wextra -pedantic --std=c23
LDFLAGS = -L./build -laeolus
