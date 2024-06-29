// Node class for an individual Worker

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <limits.h>
#include <stdlib.h>  // rand()
#include <random>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_set>
#include <vector>

#include "WorkerNode.h"
#include "TimeSlotNode.h"
#include "ScheduleData.h"
#include "WorkerInputData.h"
#include "PrintSchedule.h"

using namespace std;

class Scheduler {
public:
    Scheduler(WorkerInputData &data, unsigned int newSeed);

    void validate(ostream &output);

    void calculate();

    double getAverage();
    int getRange();
    double getLeastHappy();

    void printWorkers(ostream &output);
    void printWorkerShifts(ostream &output);
    void printScheduleShifts(ostream &output);
    void printWorkerShiftNum(ostream &output);
    void printStats(ostream &output);
    void printFinalSchedule(ostream &output);

private:

    WorkerInputData &inputData;
    PrintSchedule schedulePrinter;

    vector<vector<vector<TimeSlotNode *>>> finalSchedule; // [NUM_DAYS][MAX_SHIFTS]


    void addTinyPriorityChange(unsigned int randSeed);

    void validateSolution();



    void addAllocation(TimeSlotNode *toAssign);
    void removeAllocation(TimeSlotNode *toRemove);

    void initialAllocation();
    void initialOneSlot(vector<TimeSlotNode *> *currQueue);
    TimeSlotNode * findMaxTimeSlot(vector<TimeSlotNode *> *currQueue);

    void graphBalance();
    bool findMinMaxWorker(WorkerNode **min, WorkerNode **max);

    bool searchWorker(WorkerNode *currWorker);
    pair<double, TimeSlotNode *> findPath(TimeSlotNode *overbooked);
    void findNeighbors(vector<TimeSlotNode *> &neighbors, TimeSlotNode *initial);
    vector<TimeSlotNode *> buildPath(TimeSlotNode *end);

    void resetSeen();
    void resetPrev();
    void resetNoPath();
    void makeChanges(vector<TimeSlotNode *> path);

    // stats
    double findAverage(int &leastIndex,
                       int &mostIndex,
                       double &leastPriority, double &mostPriority);


    // helper functions for final scheduler printing
    static bool orderWorkers(TimeSlotNode *t1, TimeSlotNode *t2);


    const double tinyChangeDivisor = 1'000'000;
    bool calculated;    // whether schedule has been calculated
    unsigned int seed;  // seed of this run
    int numPaths = 0;
};

#endif
