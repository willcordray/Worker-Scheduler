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
    ~WorkerNode();

    void resetRunValues();

    void setNoPath(bool newValue);
    bool getNoPath();

    const vector<TimeSlotNode *> &getAvailability() const;
    const unordered_set<WorkerNode *> &getLikedCoworkers() const;
    const vector<TimeSlotNode *> &getAllocations() const;
    const string getName() const;
    int getShiftsRemaining() const;
    int getMaxShifts() const;
    int getRelativeBooking() const;

    void updateShiftsRemaining(int updateFactor); // TODO: this should not be public

    void addShift(int day, int shift, double priority);
    void addLikedCoworker(WorkerNode *newWorker);

    void allocateBlock(TimeSlotNode *toChoose); // TODO: remove all "problem"
    void deallocateBlock(TimeSlotNode *toRemove);

    void printBasic(ostream &output);
    void printFull(ostream &output);

private:
    void restoreInitialValues();


    string name;

    vector<TimeSlotNode *> timesAvailable;
    vector<TimeSlotNode *> timesAllocated;

    unordered_set<WorkerNode *> likedCoworkers;

    int maxShifts;        // max number of shifts this worker can take
    int shiftsRemaining;  // number of shifts left to assign
    int relativeBooking; // TODO: remove this and compute it on the fly
    bool noPath;     // a path was not found
};

#endif
