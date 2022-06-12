#include "zmienne.h"

void deleteById(std::vector<Info>* q, int idx) {
    (*q).erase((*q).begin()+idx);
}

bool sortByTime(Info i1, Info i2) {
    return i1.time > i2.time;
}

int send_msg(int msg, int id_, int type) {
    msg++;
    MPI_Send(&msg, 1, MPI_INT, id_, type, MPI_COMM_WORLD);
    return msg;
}

int encode(int id, int time) {
    return id + time * 100;
}

int findPosition(std::vector<Info>* q, int value) {
    for (int i=0; i<q->size(); i++) {
        if (q->at(i).id == value)
            return i;
    }
    return -1;
}

Info decode(int code) {
    Info info;
    info.id = code % 100;
    info.time = code / 100;
    return info;
}