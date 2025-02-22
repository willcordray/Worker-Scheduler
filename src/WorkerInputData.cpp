#include "WorkerInputData.h"

/******************************** Constructors ********************************/

WorkerInputData::WorkerInputData(string inputDirectory) {
    // TODO: this is a bit brain dead. Maybe just fail/throw an error if 
    //       there needs to be a change?
    // transfer workersPerShift to non-const so it can be changed when validated
    workersPerShift = vector<vector<int>>(NUM_DAYS, vector<int>(MAX_SHIFTS));
    for (int i = 0; i < NUM_DAYS; i++) {
        for (int j = 0; j < MAX_SHIFTS; j++) {
            workersPerShift[i][j] = WORKERS_PER_SHIFT[i][j];
        }
    }

    // read in data from files
    readFiles(inputDirectory);

    buildWorkersAvailable();
    normalizePriority();

    validate(cerr);
}

WorkerInputData::~WorkerInputData() {
    for (size_t i = 0; i < workerList.size(); i++) {
        delete workerList[i]; // TODO: set these pointer values to NULL after free (same for other pointers as well)
    }
}

void WorkerInputData::resetValues() {
    for (size_t i = 0; i < workerList.size(); i++) {
        workerList[i]->resetRunValues();
    }
}


// adds timeslots to a vector containing all of the timeslots available for
// each shift.
void WorkerInputData::buildWorkersAvailable() {
    workersAvailable = vector<vector<vector<TimeSlotNode *>>>(NUM_DAYS, vector<vector<TimeSlotNode *>>(MAX_SHIFTS));
    for (size_t i = 0; i < workerList.size(); i++) {  // loop all workers
        // loop all Shifts
        for (size_t j = 0; j < workerList[i]->getAvailability().size(); j++) {
            TimeSlotNode *newShift = workerList[i]->getAvailability()[j];
            int day = newShift->getDay();
            int shift = newShift->getShift();

            workersAvailable[day][shift].push_back(newShift);
        }
    }
}


// normalizes priorities based on (X - min) / (max - min) = newPriority
void WorkerInputData::normalizePriority() {
    int n = workerList.size();

    pair<double, double> minAndMax = findMinMaxPriority();
    double minPriority = minAndMax.first;
    double maxPriority = minAndMax.second;

    // normalize all priorities
    for (int i = 0; i < n; i++) {
        const vector<TimeSlotNode *> timeslots = workerList[i]->getAvailability();
        for (auto slot = timeslots.begin(); slot != timeslots.end(); slot++) {
            double newPriority = (*slot)->getTruePriority() - minPriority;
            if (maxPriority - minPriority != 0) {  // prevent div 0 errors
                newPriority /= (maxPriority - minPriority);
            } else {  // all values same, normalize to 1
                newPriority = 1;
            }


            (*slot)->setTruePriority(newPriority);
        }
    }
}

// first is minimum priority of any worker, second is maximum
pair<double, double> WorkerInputData::findMinMaxPriority() {
    int n = workerList.size();
    if (n == 0) {
        return {0, 0};
    }
    
    // get min and max priorities
    double minPriority;
    double maxPriority;
    bool firstTime = true;
    for (int i = 0; i < n; i++) {
        vector<TimeSlotNode *> timeslots = workerList[i]->getAvailability();
        for (auto slot = timeslots.begin(); slot != timeslots.end(); slot++) {
            if (firstTime or (*slot)->getTruePriority() < minPriority) {
                minPriority = (*slot)->getTruePriority();
            }
            if (firstTime or (*slot)->getTruePriority() > maxPriority) {
                maxPriority = (*slot)->getTruePriority();
            }

            firstTime = false;
        }
    }
    return {minPriority, maxPriority};
}



/******************************** File Reading ********************************/

// TODO: is there a way to do this without using the directory iterator?
void WorkerInputData::readFiles(string &inputDirectory) {
    if (inputDirectory[inputDirectory.size() - 1] != '/') {
        inputDirectory += '/';  // make sure always ends in a slash
    }

    vector<vector<string>> likes;
    for (const auto &entry : filesystem::directory_iterator(inputDirectory)) {
        string filename = entry.path();

        ifstream infile;
        openOrRuntimeError(infile, filename);

        string name;
        int maxShifts;
        readHeader(infile, name, maxShifts);
        WorkerNode *newWorker = new WorkerNode(name, maxShifts);


        readShifts(infile, filename, newWorker); // populates worker node
        likes.push_back(readLikes(infile));
        
        workerList.push_back(newWorker);
        infile.close();
    }

    processLikes(likes); // add all the likes to worker nodes
}

void WorkerInputData::readHeader(ifstream &infile, string &name, int &maxShifts) {
        getline(infile, name);
        infile >> maxShifts;

        // TODO: this is a bit of a hack
        char c;
        infile.get(c); // grab the trailing newlines
        infile.get(c);
}

void WorkerInputData::readShifts(ifstream &infile, string filename, WorkerNode *currWorker) {
    string lineContents;

    string dayName;
    string shiftName;
    double priority;

    int day;
    int shift;
    while (getline(infile, lineContents) && lineContents != "") {
        istringstream line(lineContents);
        line >> dayName >> shiftName >> priority;

        day = dayStringToInt(dayName);
        shift = shiftStringToInt(shiftName);
        if (day == -1) {
            cerr << "Invalid Day Name: " << dayName << " in file "
                 << filename << endl;
        } else if (shift == -1) {
            cerr << "Invalid Shift Name: " << shiftName << " in file "
                 << filename << endl;
        } else if (line.fail()) { // most likely couldn't read double
            cerr << "File reading fail in " << filename 
                 << ". Most likely couldn't read priority" << endl;
        } else { // no problems, so add the shift
            currWorker->addShift(day, shift, priority);
        }
    }
}

vector<string> WorkerInputData::readLikes(ifstream &infile) {
    vector<string> currLikes;
    string name;
    while (getline(infile, name)) {
        currLikes.push_back(name);
    }
    return currLikes;
}

void WorkerInputData::processLikes(vector<vector<string>> &likes) {
    for (size_t i = 0; i < likes.size(); i++) {
        for (string name : likes[i]) {
            WorkerNode *liked = findWorker(name);
            if (liked != nullptr) {
                workerList[i]->addLikedCoworker(liked);
            } else {
                cerr << name << ", liked by " << workerList[i]->getName() 
                     << ", is was not found" << endl;
            }
        }
    }
}

WorkerNode *WorkerInputData::findWorker(string name) {
    for (size_t i = 0; i < workerList.size(); i++) {
        if (workerList[i]->getName() == name) {
            return workerList[i];
        }
    }
    return nullptr;
}



int WorkerInputData::dayStringToInt(string dayName) {
    for (int i = 0; i < NUM_DAYS; i++) {
        if (dayName == dayNames[i]) {
            return i;
        }
    }
    return -1;
}

int WorkerInputData::shiftStringToInt(string shiftName) {
    for (int i = 0; i < MAX_SHIFTS; i++) {
        if (shiftName == shiftNames[i]) {
            return i;
        }
    }
    return -1;
}

/*
 * name:      open_or_die
 * purpose:   Open a file, or throw a runtime error if it cannot be opened
 * arguments: The stream to open the file in, and a string with the file name
 * returns:   None.
 * effects:   Opens the file in the provided stream.
 */
template <typename streamtype>
void WorkerInputData::openOrRuntimeError(streamtype &stream, std::string fileName) {
    stream.open(fileName);
    if (not stream.is_open()) {
        throw std::runtime_error("Unable to open file " + fileName);
    }
}


/****************************** Input Validation ******************************/

// checks for basic validity of input files, like no workers with the same name
void WorkerInputData::validate(ostream &output) {
    validateNoRepeatWorkers();
    validateNoRepeatBlocks();
    validateWorkersRequired(output);
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
    // loop through all blocks within a slot in the schedule, and make sure 
    // they all belong to different people
    for (int i = 0; i < NUM_DAYS; i++) {
        for (int j = 0; j < MAX_SHIFTS; j++) {
            unordered_set<string> namesInSlot;
            for (auto k = workersAvailable[i][j].begin();
                    k != workersAvailable[i][j].end(); k++) {
                string name = (*k)->getParent()->getName();
                if (namesInSlot.find(name) != namesInSlot.end()) { // repeat
                    string errorMessage = "Worker " +
                                          name + " has duplicate block(s) in " +
                                          dayNames[i] + " " + shiftNames[j];
                    throw runtime_error(errorMessage);
                }
                namesInSlot.insert(name);
            }
        }
    }
}

void WorkerInputData::validateWorkersRequired(ostream &output) {
    bool firstAsk = true;
    bool foundProblem = false;
    for (int i = 0; i < NUM_DAYS; i++) {
        for (int j = 0; j < MAX_SHIFTS; j++) {
            int numWorkers = workersAvailable[i][j].size();
            if (numWorkers < workersPerShift[i][j]) {
                if (firstAsk) {
                    output << "Invalid Schedule. Not enough Workers to fill "
                              "all required spots"
                           << endl;
                    firstAsk = false;
                }
                output << "  Update " << dayNames[i] << " " << shiftNames[j]
                       << " from " << workersPerShift[i][j] << " to "
                       << numWorkers << " workers required?" << endl;
                workersPerShift[i][j] = numWorkers;
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
            throw runtime_error("Invalid Schedule. "
                                "Too few workers available for shift(s)");
        }
    }
}

/***************************** Getters and Setters ****************************/

const vector<vector<vector<TimeSlotNode *>>> &WorkerInputData::getWorkersAvailable() {
    return workersAvailable;
}

const vector<TimeSlotNode *> &WorkerInputData::getWorkersAvailable(int day, int shift) {
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
