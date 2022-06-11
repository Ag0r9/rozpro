#define _GNU_SOURCE
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define RDY 0
#define CNF 1
#define FGT 2
#define SEK 3
#define OKSEK 4
#define NOSEK 5
#define HSP 6
#define OKHSP 7
#define NOHSP 8

