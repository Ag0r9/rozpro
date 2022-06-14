#ifndef ZMIENNE_H
#define ZMIENNE_H

#include <mpi.h>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <bits/stdc++.h>

#define MAX(a,b) (((a)>(b))?(a):(b))

extern bool isActive;

extern int CLOCK;
extern int LAST_REQ;
extern int STATE;

extern int size;
extern int rank;

extern int hospital;
extern int wait_for_resource;

typedef struct {
    int id;
    int time;
} Info;

extern std::vector<Info> hsp_queue;

extern std::mutex stateMutex;
extern std::condition_variable canGoFurther;

void send_msg(int msg, int id_, int type); 

#define HOSPITAL 1
#define OKHOSPITAL 2

#endif