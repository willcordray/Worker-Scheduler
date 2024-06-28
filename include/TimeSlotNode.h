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

    void restoreDefaults();

    bool operator==(const TimeSlotNode& other) const;
    bool operator!=(const TimeSlotNode& other) const;

    WorkerNode *getParent() const;

    double getPriority(const vector<vector<vector<TimeSlotNode *>>> &workers, bool useTruePriority) const;

    double getTruePriority() const; // todo: turn these to camel case
    int getDay() const;
    int getShift() const;
    bool getUsed() const;
    bool getSeen() const;
    TimeSlotNode *getPrev() const;

    void setTruePriority(double newPriority);
    void setPriority(double newPriority);
    void setSeen(bool newValue);
    void setUsed(bool newValue);
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
