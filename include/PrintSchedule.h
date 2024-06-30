#ifndef PRINT_SCHEDULE_H
#define PRINT_SCHEDULE_H

#include <iostream>
#include <fstream>
#include <vector>


#include "ScheduleData.h"
#include "TimeSlotNode.h"
#include "WorkerNode.h"


class PrintSchedule {
public:
    void printSchedule(ostream &output,
                    const vector<vector<vector<TimeSlotNode *>>> &workers);

private:
    void determineMaxSize(vector<int> &maxSize,
                          const vector<vector<vector<TimeSlotNode *>>> &workers);
    void printBlankline(ostream &output, vector<int> &maxSize);
    void printDashes(ostream &output, int numDashes);
    void printEven(ostream &output, string toPrint, int size);
    void printDayNames(ostream &output, vector<int> &maxSize);
};

#endif
