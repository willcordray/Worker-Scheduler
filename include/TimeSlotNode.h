// Node class for an individual Worker availibility timeslot

#ifndef TIMESLOTNODE_H
#define TIMESLOTNODE_H

#include <iostream>
#include <vector>

#include "ScheduleData.h"
#include "WorkerNode.h"

using namespace std;

class WorkerNode; // circular reference shenanigans

class TimeSlotNode {
public:
    TimeSlotNode(WorkerNode *newParent, int newDay, int newShift, double newPriority);

    void resetValues();

    bool operator==(const TimeSlotNode& other) const;
    bool operator!=(const TimeSlotNode& other) const;

    WorkerNode *get_parent() const;

    double get_priority(const vector<vector<vector<TimeSlotNode *>>> &workers, bool useTruePriority) const;

    double get_true_priority() const; // todo: turn these to camel case
    int get_day() const;
    int get_shift() const;
    bool get_used() const;
    bool getSeen() const;
    TimeSlotNode *getPrev() const;

    void set_true_priority(double newPriority);
    void set_priority(double newPriority);
    void setSeen(bool newValue);
    void set_used(bool newValue);
    void setPrev(TimeSlotNode *newPrev);


    void printTime(ostream &output) const;
    void printInfo(ostream &output) const;
    void print(ostream &output) const;


private:
    double calcPenalty() const;
    double calcBonus(const vector<vector<vector<TimeSlotNode *>>> &workers) const;
    double exponeniatePenalty(int times, double factor, double penalty) const;

    WorkerNode *parent; // parent worker for this timeslot node

    double truePriority; // change the name of this to normalizedPriority?
    double priority;

    int day;
    int shift;

    bool used;
    
    bool seen;
    TimeSlotNode *prev;
};


#endif
