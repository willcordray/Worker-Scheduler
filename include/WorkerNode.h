// Node class for an individual Worker

#ifndef WorkerNode_H
#define WorkerNode_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>

#include "ScheduleData.h"
#include "TimeSlotNode.h"

using namespace std;

class TimeSlotNode;  // circular reference shenanigans

class WorkerNode {
public:
    WorkerNode(string newName, int newMaxShifts);

    void resetValues();

    void set_seen(bool newValue);
    bool get_seen();

    void set_noPath(bool newValue);
    bool get_noPath();

    const vector<TimeSlotNode *> *getAvailability() const;
    const unordered_set<WorkerNode *> *getLikedCoworkers() const;
    const vector<TimeSlotNode *> &getAllocations() const;
    const string getName() const;
    int getShiftsRemaining() const;
    int getMaxShifts() const;
    int getRelativeBooking() const;

    void updateShiftsRemaining(int updateFactor);

    void addShift(WorkerNode *thisWorker, int day, int shift, double priority);
    void addLikedCoworker(WorkerNode *newWorker);

    void allocateBlock(TimeSlotNode *toChoose, bool &problem); // TODO: remove all "problem"
    void deallocateBlock(TimeSlotNode *toRemove, bool &problem);

    void printBasic(ostream &output);
    void printFull(ostream &output);

private:
    string name;

    vector<TimeSlotNode *> timesAvailable;
    vector<TimeSlotNode *> timesAllocated;

    unordered_set<WorkerNode *> likedCoworkers;

    int maxShifts;        // max number of shifts this worker can take
    int shiftsRemaining;  // number of shifts left to assign
    int relativeBooking;
    bool seen;
    bool noPath;     // a path was not found
};

#endif
