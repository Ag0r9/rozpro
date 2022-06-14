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
extern int process;
extern int wait_for_resource;

typedef struct {
    int id;
    int time;
} Info;

extern std::vector<Info> queue;
extern std::vector<Info> hsp_queue;

extern std::mutex stateMutex;
extern std::condition_variable canGoFurther;

// bool sortByTime(Info i1, Info i2);
void send_msg(int msg, int id_, int type); 
// int encode(int id, int time);
// int findPosition(std::vector<Info>* q, int value); 
// Info decode(int code);
// void deleteById(std::vector<Info>* q, int idx);

// #define READY 0
// #define OK 1
// #define CONFIRM 2
// #define FIGHT 3
// #define SECOND 4
// #define OKSECOND 5
// #define NOSECOND 6
#define HOSPITAL 7
#define OKHOSPITAL 8
// #define NOHOSPITAL 9
// #define ISHURT 10

#endif