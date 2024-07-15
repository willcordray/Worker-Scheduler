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
    /******************************* Constructor ******************************/
    Scheduler(WorkerInputData &data, unsigned int newSeed);

    /*************************** Schedule Population **************************/
    void calculate();

    /******************************* Statistics *******************************/
    double getAverage();
    int getRange();
    double getLeastHappy();

    /******************************** Printing ********************************/
    void printStats(ostream &output);

    void printFinalSchedule(ostream &output);
    void printScheduleShifts(ostream &output);

    void printWorkers(ostream &output);
    void printWorkerShifts(ostream &output);
    void printWorkerShiftNum(ostream &output);

private:
    WorkerInputData &inputData;
    PrintSchedule schedulePrinter;

    vector<vector<vector<TimeSlotNode *>>> finalSchedule = vector<vector<vector<TimeSlotNode *>>>(NUM_DAYS, vector<vector<TimeSlotNode *>>(MAX_SHIFTS));

    const double tinyChangeDivisor = 1'000'000;
    bool calculated;    // whether schedule has been calculated
    unsigned int seed;  // seed of this run


    /******************************* Constructor ******************************/
    void addTinyPriorityChange(unsigned int randSeed);

    /*************************** Schedule Population **************************/
    void addAllocation(TimeSlotNode *toAssign);
    void removeAllocation(TimeSlotNode *toRemove);

    void initialAllocation();
    void initialOneSlot(const vector<TimeSlotNode *> &currQueue);
    TimeSlotNode *findMaxTimeSlotPriority(const vector<TimeSlotNode *> &currQueue);

    void graphBalance();
    bool findMinMaxWorkerBooking(WorkerNode **min, WorkerNode **max);

    bool searchWorker(WorkerNode *currWorker);
    void resetSearchValues();
    void resetNoPath();
    pair<double, TimeSlotNode *> findPath(TimeSlotNode *overbooked);
    void findNodeToAdd(priority_queue<pair<double, TimeSlotNode *>> &paths, pair<double, TimeSlotNode *> &bestPath, pair<double, TimeSlotNode *> currPath, TimeSlotNode *start);
    void findNodeToDrop(priority_queue<pair<double, TimeSlotNode *>> &paths, TimeSlotNode *neighbor, double currPathValue);
    bool validPath(TimeSlotNode *start, TimeSlotNode *end);

    void buildPath(vector<TimeSlotNode *> &path, TimeSlotNode *end);
    void makeChanges(vector<TimeSlotNode *> &path);
    void resetAllMemoizedPriorities();


    /******************************* Validation *******************************/
    void validateSolution();
    void validateWorkersOnShift();
    void validateNoDuplicateWorkers();
    void validateUsed();

    /******************************* Statistics *******************************/
    double findAverage(int &leastIndex,
                       int &mostIndex,
                       double &leastPriority, double &mostPriority);


    /******************************** Printing ********************************/
    static bool sortAlphabetical(TimeSlotNode *t1, TimeSlotNode *t2);
};

#endif
