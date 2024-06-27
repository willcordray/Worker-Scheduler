// TODO: file headers and function contracts

#ifndef WORKER_DATA_H
#define WORKER_DATA_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <unordered_set>


#include "ScheduleData.h"
#include "TimeSlotNode.h"
#include "WorkerNode.h"


class WorkerInputData {
public:
    WorkerInputData(string inputDirectory);
    WorkerInputData(const WorkerInputData &other);

    vector<vector<vector<TimeSlotNode *>>> &getWorkersAvailable();
    vector<TimeSlotNode *> &getWorkersAvailable(int day, int shift);
    int getWorkersPerShift(int day, int shift);
    vector<WorkerNode *> &getWorkerList();
    WorkerNode *getWorker(int listIndex);
    int getNumWorkers();


private:
    vector<vector<int>> workersPerShift; // [NUM_DAYS][MAX_SHIFTS]
    vector<WorkerNode *> workerList;
    vector<vector<vector<TimeSlotNode *>>> workersAvailable; // [NUM_DAYS][MAX_SHIFTS]



    void readFiles(string &inputDirectory);
    void readHeader(ifstream &infile, string &name, int &maxShifts);
    void readShifts(ifstream &infile, string filename, WorkerNode *currWorker);
    vector<string> readLikes(ifstream &infile);
    void processLikes(vector<vector<string>> &likes);

    WorkerNode *findWorker(string name);
    int dayStringToInt(string dayName);
    int shiftStringToInt(string shiftName);
    template <typename streamtype>
    void open_or_die(streamtype &stream, std::string fileName);

    void buildWorkersAvailable();

    void validate(ostream &output);
    void validateNoRepeatWorkers();
    void validateNoRepeatBlocks();
    void validateWorkersRequired(ostream &output);
};

#endif
