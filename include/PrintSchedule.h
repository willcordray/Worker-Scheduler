#ifndef PRINT_SCHEDULE_H
#define PRINT_SCHEDULE_H

#include <fstream>
#include <iostream>
#include <vector>

#include "ScheduleData.h"
#include "TimeSlotNode.h"
#include "WorkerNode.h"

class PrintSchedule {
public:
    void printSchedule(ostream &output,
                       const vector<vector<vector<TimeSlotNode *>>> &workers);

private:
    void populateMaxSize(vector<int> &maxSize,
                         const vector<vector<vector<TimeSlotNode *>>> &workers);
    int findLineLength(vector<int> &maxSize);

    void printHeader(ostream &output, int lineLength, vector<int> &maxSize);
    void printRow(ostream &output, int lineLength, int shift,
                  vector<int> &maxSize,
                  const vector<vector<vector<TimeSlotNode *>>> &workers);

    void printBlankline(ostream &output, vector<int> &maxSize);
    void printDashes(ostream &output, int numDashes);
    void printEven(ostream &output, string toPrint, int size);
    void printSpaces(ostream &output, int numSpaces);

    void printDayNames(ostream &output, vector<int> &maxSize);
};

#endif
