#include "WorkerInputData.h"

WorkerInputData::WorkerInputData(string inputDirectory) {
    // TODO: this is a bit brain dead. Maybe just fail/throw an error if there needs to be a change?
    // transfer workersPerShift to non-const so it can be changed when validated
    workersPerShift = vector<vector<int>>(NUM_DAYS, vector<int>(MAX_SHIFTS));
    for (int i = 0; i < NUM_DAYS; i++) {
        for (int j = 0; j < MAX_SHIFTS; j++) {
            workersPerShift[i][j] = WORKERS_PER_SHIFT[i][j];
        }
    }

    // read in data from files
    readFiles(inputDirectory);
    readLikes(inputDirectory);

    buildWorkersAvailable();

    validate(cerr);
}

// adds timeslots to a vector containing all of the timeslots available for
// each shift.
void WorkerInputData::buildWorkersAvailable() {
    workersAvailable = vector<vector<vector<TimeSlotNode *>>>(NUM_DAYS, vector<vector<TimeSlotNode *>>(MAX_SHIFTS));
    for (size_t i = 0; i < workerList.size(); i++) {  // loop all workers
        // loop all Shifts
        for (size_t j = 0; j < workerList[i]->getAvailability()->size(); j++) {
            TimeSlotNode *newShift = (*workerList[i]->getAvailability())[j];
            int day = newShift->get_day();
            int shift = newShift->get_shift();

            workersAvailable[day][shift].push_back(newShift);
        }
    }
}

// copy constructor
WorkerInputData::WorkerInputData(const WorkerInputData &other) {
    workersPerShift = other.workersPerShift;
    workerList = other.workerList;
    workersAvailable = other.workersAvailable;

    // reset all the run dependent values of workers and TimeSlotNodes to 
    // starting values
    for (size_t i = 0; i < workerList.size(); i++) {
        workerList[i]->resetValues();
    }
}

// TODO: is there a way to do this without using the directory iterator?
void WorkerInputData::readFiles(string &inputDirectory) {
    if (inputDirectory[inputDirectory.size() - 1] != '/') {
        inputDirectory += '/';  // make sure always ends in a slash
    }

    for (const auto &entry : filesystem::directory_iterator(inputDirectory)) {
        string filename = entry.path();
        if (filename == inputDirectory + "workerLikes.txt") {
            continue;
        }

        ifstream infile;
        open_or_die(infile, filename);

        string name;
        int max_shifts;
        getline(infile, name);
        infile >> max_shifts;
        WorkerNode *newWorker = new WorkerNode(name, max_shifts);

        string dayName;
        string shiftName;
        int day;
        int shift;
        double priority;
        while (infile >> dayName) {
            infile >> shiftName >> priority;

            day = dayString_to_int(dayName);
            if (day == -1) {
                cerr << "Invalid Day Name: " << dayName << " in file "
                     << filename << endl;
                continue;
            }

            shift = shiftString_to_int(shiftName);
            if (shift == -1) {
                cerr << "Invalid Shift Name: " << shiftName << " in file "
                     << filename << endl;
                continue;
            }

            newWorker->addShift(newWorker, day, shift, priority);
        }
        workerList.push_back(newWorker);
    }
}

int dayString_to_int(string dayName) {
    for (int i = 0; i < NUM_DAYS; i++) {
        if (dayName == dayNames[i]) {
            return i;
        }
    }
    return -1;
}

int shiftString_to_int(string shiftName) {
    for (int i = 0; i < MAX_SHIFTS; i++) {
        if (shiftName == shiftNames[i]) {
            return i;
        }
    }
    return -1;
}

// TODO: the way these files are formatted is bad. Why not just have it the same way as the input files where there is only a space that separates it? The information should probably just be put into the main info file, seperated by whitespace
void WorkerInputData::readLikes(string inputDirectory) {
    // input directory guaranteed to have slash because called after readFile
    // which fixes the slash problem // TODO: this is a dubious claim that might change with further development. We should just add the check anyway? What is the least bad way to do this? Have a wrapper function that adds the slash then calls both? Is the slash even necessary?
    for (const auto &entry : filesystem::directory_iterator(inputDirectory)) {
        string filename = entry.path();
        if (filename == inputDirectory + "workerLikes.txt") {
            ifstream infile;
            open_or_die(infile, filename);
            string line;
            while (getline(infile, line)) {
                if (line == "") {  // blank lines allowed
                    continue;
                }
                string worker1;
                string worker2;
                size_t i = 0;
                while (i < line.size() and line[i] != ':') {
                    worker1 += line[i];
                    i++;
                }
                if (i == line.size()) {
                    throw runtime_error(
                        "In workerLikes.txt, there is a line with no ':'");
                }

                if (worker1.size() == 0) {
                    throw runtime_error(
                        "In workerLikes.txt, there is a ':' as the first "
                        "character");
                }

                worker1.resize(worker1.size() - 1);  // remove that trailing space

                // +3 because want to start after the " : "
                worker2 = line.substr(worker1.size() + 3, line.size() - worker1.size() - 3);

                if (worker2.size() == 0) {
                    throw runtime_error(
                        "In workerLikes.txt, there is a ':' as the last character");
                }

                WorkerNode *WorkerNode1 = nullptr;
                WorkerNode *WorkerNode2 = nullptr;
                for (auto it = workerList.begin(); it != workerList.end(); it++) {
                    if ((*it)->getName() == worker1) {
                        WorkerNode1 = *it;
                    } else if ((*it)->getName() == worker2) {
                        WorkerNode2 = *it;
                    }
                }

                if (WorkerNode1 == nullptr) {
                    throw runtime_error("In workerLikes.txt, name " + worker1 +
                                        " not found");
                }
                if (WorkerNode2 == nullptr) {
                    throw runtime_error("In workerLikes.txt, name " + worker2 +
                                        " not found");
                }
                WorkerNode1->addLikedCoworker(WorkerNode2);
            }
        }
    }
}




/*
 * name:      open_or_die
 * purpose:   Open a file, or throw a runtime error if it cannot be opened
 * arguments: The stream to open the file in, and a string with the file name
 * returns:   None.
 * effects:   Opens the file in the provided stream.
 */
template <typename streamtype>
void WorkerInputData::open_or_die(streamtype &stream, std::string fileName) {
    stream.open(fileName);
    if (not stream.is_open()) {
        throw std::runtime_error("Unable to open file " + fileName);
    }
}



// checks to see if there is any chance of a valid solution, as well is for
// some other basic pitfalls
// if ask user for input is true, then asks user if they want to decrease the
//    required workers on that shift or quit the program. Otherwise, automatically
//    makes and reports choice.
void WorkerInputData::validate(ostream &output) {
    validateNoRepeatWorkers();
    validateNoRepeatBlocks();
    validateWorkersRequired(output);
}

void WorkerInputData::validateWorkersRequired(ostream &output) {
    bool firstAsk = true;
    bool foundProblem = false;
    for (int i = 0; i < NUM_DAYS; i++) {
        for (int j = 0; j < MAX_SHIFTS; j++) {
            int newSize = workersAvailable[i][j].size();
            if (newSize < workersPerShift[i][j]) {
                if (firstAsk) {
                    output << "Invalid Schedule. Not enough Workers to fill all "
                              "required spots"
                           << endl;
                    firstAsk = false;
                }
                output << "  Update " << dayNames[i] << " " << shiftNames[j]
                       << " from " << workersPerShift[i][j] << " to " << newSize
                       << " workers required" << endl;
                workersPerShift[i][j] = newSize;
                foundProblem = true;
            }
        }
    }
    if (foundProblem) {
        output << "Would you like to make the above changes to the schedule? "
                  "[yn] ";
        string response;
        cin >> response;
        if (response != "y") {
            throw runtime_error("Invalid Schedule. Too few workers available for shift(s)");
        }
    }
}

void WorkerInputData::validateNoRepeatWorkers() {
    unordered_set<string> namesSoFar;
    for (auto it = workerList.begin(); it != workerList.end(); it++) {
        string currName = (*it)->getName();
        if (namesSoFar.find(currName) != namesSoFar.end()) {  // found repeat
            throw runtime_error("More than 1 worker with name " + currName);
        }
        namesSoFar.insert(currName);
    }
}

void WorkerInputData::validateNoRepeatBlocks() {
    for (int i = 0; i < NUM_DAYS; i++) {
        for (int j = 0; j < MAX_SHIFTS; j++) {
            unordered_set<string> namesSoFar;
            for (auto k = workersAvailable[i][j].begin();
                 k != workersAvailable[i][j].end(); k++) {
                auto found = namesSoFar.find((*k)->get_parent()->getName());
                if (found != namesSoFar.end()) {  // found repeat
                    string errorMessage = "Worker " +
                                          (*k)->get_parent()->getName() +
                                          " has duplicate blocks in " +
                                          dayNames[i] + " " + shiftNames[j];
                    throw runtime_error(errorMessage);
                }
                namesSoFar.insert((*k)->get_parent()->getName());
            }
        }
    }
}



vector<vector<vector<TimeSlotNode *>>> &WorkerInputData::getWorkersAvailable() {
    return workersAvailable;
}

vector<TimeSlotNode *> &WorkerInputData::getWorkersAvailable(int day, int shift) {
    return workersAvailable[day][shift];
}

int WorkerInputData::getWorkersPerShift(int day, int shift) {
    return workersPerShift[day][shift];
}

vector<WorkerNode *> &WorkerInputData::getWorkerList() {
    return workerList;
}

WorkerNode *WorkerInputData::getWorker(int listIndex) {
    return workerList[listIndex];
}

int WorkerInputData::getNumWorkers() {
    return workerList.size();
}
