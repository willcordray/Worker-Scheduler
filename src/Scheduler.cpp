#include "Scheduler.h"

/********************************* Constructor ********************************/

Scheduler::Scheduler(WorkerInputData &data, unsigned int newSeed) : inputData(data) {
    seed = newSeed;
    calculated = false;
    addTinyPriorityChange(newSeed);
}

// normalizes priorities based on (X - min) / (max - min) = newPriority
void Scheduler::addTinyPriorityChange(unsigned int randSeed) {
    srand(randSeed);
    int n = inputData.getNumWorkers();

    for (int i = 0; i < n; i++) {
        const vector<TimeSlotNode *> &timeslots = inputData.getWorker(i)->getAvailability();
        for (auto slot = timeslots.begin(); slot != timeslots.end(); slot++) {
            // tiny change adds some minor variance so that this can be rerun
            // with different random values, and get different (maybe better)
            // results
            double tinyChange = ((double)rand() / RAND_MAX) / tinyChangeDivisor;
            double wiggledPriority = (*slot)->getTruePriority() + tinyChange;

            (*slot)->setPriority(wiggledPriority);
        }
    }
}

/***************************** Schedule Population ****************************/

void Scheduler::calculate() {
    calculated = true;
    initialAllocation();
    graphBalance();

    validateSolution();  // check to make sure nothing went wrong
}

// takes a timeslot node and adds it to the final schedule
void Scheduler::addAllocation(TimeSlotNode *toAssign) {
    int day = toAssign->getDay();
    int shift = toAssign->getShift();
    finalSchedule[day][shift].push_back(toAssign);

    toAssign->getParent()->allocateBlock(toAssign);
}

// takes a timeslot node and removes it from the final schedule
void Scheduler::removeAllocation(TimeSlotNode *toRemove) {
    int day = toRemove->getDay();
    int shift = toRemove->getShift();
    for (auto it = finalSchedule[day][shift].begin();
         it != finalSchedule[day][shift].end(); it++) {
        if (toRemove == *it) {
            finalSchedule[day][shift].erase(it);
            break;
        }
    }

    toRemove->getParent()->deallocateBlock(toRemove);  // remove from allocated
}

void Scheduler::initialAllocation() {
    vector<pair<int, int>> shifts;
    for (int i = 0; i < NUM_DAYS; i++) {  // loop all shifts
        for (int j = 0; j < MAX_SHIFTS; j++) {
            shifts.push_back({i, j});
        }
    }

    // randomize the order of when shifts are allocated
    shuffle(shifts.begin(), shifts.end(), default_random_engine(rand()));

    int numShifts = NUM_DAYS * MAX_SHIFTS;
    for (int i = 0; i < numShifts; i++) {
        // convert combined to individual days and shifts
        int day = shifts[i].first;
        int shift = shifts[i].second;
        const vector<TimeSlotNode *> &currShift = inputData.getWorkersAvailable(day, shift);
        if (inputData.getWorkersPerShift(day, shift) > 0) {
            initialOneSlot(currShift);
        }
    }
}

// initially allocate all of the TAs for one timeslot
void Scheduler::initialOneSlot(const vector<TimeSlotNode *> &currQueue) {
    if (currQueue.size() == 0) {  // shift with no available TAs
        return;
    }

    int day = currQueue.front()->getDay();  // all same shift time
    int shift = currQueue.front()->getShift();
    // assigning all of the workers for this shift
    for (int i = 0; i < inputData.getWorkersPerShift(day, shift); i++) {
        TimeSlotNode *topTimeNode;
        topTimeNode = findMaxTimeSlotPriority(currQueue);
        vector<TimeSlotNode *> topPriority;
        double highestPriority = topTimeNode->getPriority(finalSchedule, false);
        for (auto it = currQueue.begin(); it != currQueue.end(); it++) {
            if (!(*it)->getUsed() and
                (*it)->getPriority(finalSchedule, false) == highestPriority) {
                topPriority.push_back(*it);
            }
        }
        // among the shifts with highest priority, select the person who has 
        // the lowest number of shifts
        int mostAvailability = INT_MIN;
        int indexMostAvailability = -1;
        for (size_t j = 0; j < topPriority.size(); j++) {
            int currAvailability 
                    = topPriority[j]->getParent()->getShiftsRemaining();
            if (currAvailability > mostAvailability) {
                mostAvailability = currAvailability;
                indexMostAvailability = j;
            }
        }
        addAllocation(topPriority[indexMostAvailability]);
    }
}

// finds the unused timeslotnode within the queue that has the highest priority
TimeSlotNode *Scheduler::findMaxTimeSlotPriority(const vector<TimeSlotNode *> &currQueue) {
    TimeSlotNode *topTimeNode = nullptr;
    double highestPriority;
    for (auto it = currQueue.begin(); it != currQueue.end(); it++) {
        if (not (*it)->getUsed() and
            (topTimeNode == nullptr or
             (*it)->getPriority(finalSchedule, false) > highestPriority)) {
            topTimeNode = *it;
            highestPriority = (*it)->getPriority(finalSchedule, false);
        }
    }

    if (topTimeNode == nullptr) {
        throw runtime_error(
            "Error: trying to allocated more time slots, but all time slots "
            "are already used");
    }
    return topTimeNode;
}


void Scheduler::graphBalance() {
    WorkerNode *min;
    WorkerNode *max;
    findMinMaxWorkerBooking(&min, &max);

    // loops until all workers are evenly allocated
    while (abs(max->getRelativeBooking() - min->getRelativeBooking()) > 1) {
        WorkerNode *currWorker = max;

        // try to find path
        bool madeChange = searchWorker(currWorker);

        // if a path was found, the graph is changed and other nodes for 
        // which a path didn't exist might exist now
        if (madeChange) {
            resetNoPath();
        } else {
            cerr << "didn't find a path" << endl; // todo: debugging
            currWorker->setNoPath(true);
        }

        if (!findMinMaxWorkerBooking(&min, &max)) {  // no more workers that are unmarked
            cerr << "No more paths" << endl;
            return;
        }
    }
}

// finds the workers with the highest and lowest booking
bool Scheduler::findMinMaxWorkerBooking(WorkerNode **min, WorkerNode **max) {
    int n = inputData.getNumWorkers();

    bool foundWorker = false;
    for (int i = 0; i < n; i++) {
        WorkerNode *currWorker = inputData.getWorker(i);
        // only include workers that can be searched
        if (!currWorker->getNoPath()) {
            int currBooking = currWorker->getRelativeBooking();
            if (!foundWorker || currBooking < (*min)->getRelativeBooking()) {
                *min = currWorker;
            }
            if (!foundWorker || currBooking > (*max)->getRelativeBooking()) {
                *max = currWorker;
            }

            foundWorker = true;
        }
    }
    return foundWorker;
}


// takes a worker, and calls findPath to graph search to remove an allocations
bool Scheduler::searchWorker(WorkerNode *currWorker) {
    double bestPathVal;
    bool foundPath = false;
    const vector<TimeSlotNode *> &blocks = currWorker->getAllocations();
    vector<TimeSlotNode *> bestPath;
    for (auto it = blocks.begin(); it != blocks.end(); it++) {
        pair<double, TimeSlotNode *> result = findPath(*it);
        if (result.second != nullptr and (!foundPath or result.first > bestPathVal)) {
            bestPathVal = result.first;
            buildPath(bestPath, result.second);
            foundPath = true;
        }
    }

    if (foundPath) {
        makeChanges(bestPath);
    }
    return foundPath;
}

void Scheduler::resetSearchValues() {
    int n = inputData.getNumWorkers();
    for (int i = 0; i < n; i++) {
        const vector<TimeSlotNode *> &slots = inputData.getWorker(i)->getAvailability();
        for (size_t j = 0; j < slots.size(); j++) {
            slots[j]->setSeen(false);
            slots[j]->setPrev(nullptr);
        }
    }
}

void Scheduler::resetNoPath() {
    int n = inputData.getNumWorkers();
    for (int i = 0; i < n; i++) {
        inputData.getWorker(i)->setNoPath(false);
    }
}

pair<double, TimeSlotNode *> Scheduler::findPath(TimeSlotNode *overbooked) {
    resetSearchValues();

    priority_queue<pair<double, TimeSlotNode *>> paths;
    paths.push({-overbooked->getPriority(finalSchedule, false), overbooked});
    overbooked->setSeen(true);

    // double is the value of the current path, and the timeslotnode is the 
    // next node to drop from allocations
    pair<double, TimeSlotNode *> bestPath = {0, nullptr};
    while(!paths.empty()) {
        pair<double, TimeSlotNode *> currPath = paths.top();
        paths.pop();

        // trying to find a timeslotnode that can replace the current node
        findNodeToAdd(paths, bestPath, currPath, overbooked);
    }

    return bestPath;
}

void Scheduler::findNodeToAdd(priority_queue<pair<double, TimeSlotNode *>> &paths, pair<double, TimeSlotNode *> &bestPath, pair<double, TimeSlotNode *> currPath, TimeSlotNode *start) {
    TimeSlotNode *initial = currPath.second;

    // populates neighbors of current node
    // neighbors are unused shifts of people in the same timeslot that can 
    // replace the current shift.
    int day = initial->getDay(), shift = initial->getShift();
    const vector<TimeSlotNode *> &neighbors = inputData.getWorkersAvailable(day, shift);

    // don't want to change the booking of the neighbor, so find another 
    //shift they are on and remove it.
    for (size_t i = 0; i < neighbors.size(); i++) {
        if (!neighbors[i]->getSeen() && !neighbors[i]->getUsed()) { // not already on a path
            neighbors[i]->setPrev(currPath.second);
            neighbors[i]->setSeen(true);

            // if underbooked person, valid path
            if (validPath(start, neighbors[i])) {
                double pathValue = currPath.first + neighbors[i]->getPriority(finalSchedule, false);
                if (bestPath.second == nullptr or pathValue > bestPath.first) {
                    bestPath = {pathValue, neighbors[i]};
                }
            }

            // shift to remove from the same person
            findNodeToDrop(paths, neighbors[i], currPath.first);
        }
    }
}

void Scheduler::findNodeToDrop(priority_queue<pair<double, TimeSlotNode *>> &paths, TimeSlotNode *neighbor, double currPathValue) {
    // the pool for allocations is different from the pool for neighbors, so 
    // all nodes in allocations is a potential replacement
    const vector<TimeSlotNode *> &allocations = neighbor->getParent()->getAllocations();
    for (size_t j = 0; j < allocations.size(); j++) {
        // shift has to already be used, and cannot be in another path
        if (!allocations[j]->getSeen()) {
            allocations[j]->setPrev(neighbor); // maintain the path
            allocations[j]->setSeen(true);
            double newValue = currPathValue + neighbor->getPriority(finalSchedule, false) - allocations[j]->getPriority(finalSchedule, false);
            paths.push({newValue, allocations[j]});
        }
    }
}

bool Scheduler::validPath(TimeSlotNode *start, TimeSlotNode *end) {
    return abs(start->getParent()->getRelativeBooking() - end->getParent()->getRelativeBooking()) > 1;
}


void Scheduler::buildPath(vector<TimeSlotNode *> &path, TimeSlotNode *end) {
    path.clear();

    TimeSlotNode *curr = end;
    while (curr != nullptr) {
        path.push_back(curr);
        curr = curr->getPrev();
    }
    reverse(path.begin(), path.end());
}


// path size must be even
// path goes allocated -> not allocated -> allocated -> etc. (ends on not
//      allocated)
void Scheduler::makeChanges(vector<TimeSlotNode *> &path) {
    bool allocated = true;
    for (auto it = path.begin(); it != path.end(); it++) {
        if (allocated) {
            removeAllocation(*it);
        } else {
            addAllocation(*it);
        }

        allocated = !allocated;
    }
}

/********************************* Validation *********************************/

// validate that a solution works, i.e. all shifts have correct number of workers,
// as well as under the hood problems like values of used and seen
void Scheduler::validateSolution() {
    validateWorkersOnShift();
    validateNoDuplicateWorkers();
    validateUsed();
}

void Scheduler::validateWorkersOnShift() {
    for (int i = 0; i < NUM_DAYS; i++) {
        for (int j = 0; j < MAX_SHIFTS; j++) {
            // correct number of workers on shift
            int size = finalSchedule[i][j].size();
            if (inputData.getWorkersPerShift(i, j) != size) {
                string message = "Error: Wrong number of workers on shift " +
                                 dayNames[i] + " " + shiftNames[j];
                throw runtime_error(message);
            }
        }
    }
}

void Scheduler::validateNoDuplicateWorkers() {
    for (int i = 0; i < NUM_DAYS; i++) {
        for (int j = 0; j < MAX_SHIFTS; j++) {
            // no duplicate workers on same shift
            unordered_set<string> namesSoFar;
            for (auto it = finalSchedule[i][j].begin();
                 it != finalSchedule[i][j].end(); it++) {
                string name = (*it)->getParent()->getName();
                if (namesSoFar.find(name) != namesSoFar.end()) {
                    string message =
                        "Error: " + name + " is on " +
                        dayNames[i] + " " + shiftNames[j] + " more than once";
                    throw runtime_error(message);
                }
                namesSoFar.insert(name);
            }
        }
    }
}

void Scheduler::validateUsed() {
    for (int i = 0; i < NUM_DAYS; i++) {
        for (int j = 0; j < MAX_SHIFTS; j++) {
            // no duplicate workers on same shift
            unordered_set<string> namesSoFar;
            for (auto it = finalSchedule[i][j].begin();
                 it != finalSchedule[i][j].end(); it++) {
                if (!(*it)->getUsed()) {
                    string name = (*it)->getParent()->getName();
                    string message = "Error: " + name + ", with block" 
                                     + to_string((*it)->getDay()) + " : " 
                                     + to_string((*it)->getShift()) 
                                     + ", is on shift but is not marked as used";
                    throw runtime_error(message);
                }
            }
        }
    }
}

/********************************* Statistics *********************************/

double Scheduler::getAverage() {
    if (!calculated) {
        throw runtime_error("Tried to get average before calculating");
    }

    // findAverage populates variables with the appropiate values, but 
    // they are not used
    int mostIndex;
    int leastIndex;
    double leastPriority;
    double mostPriority;
    return findAverage(leastIndex, mostIndex, leastPriority, mostPriority);
}

int Scheduler::getRange() {
    if (!calculated) {
        throw runtime_error("Tried to get range before calculating");
    }

    WorkerNode *min;
    WorkerNode *max;
    findMinMaxWorkerBooking(&min, &max);

    return (max->getRelativeBooking() - min->getRelativeBooking());
}

double Scheduler::getLeastHappy() {
    if (!calculated) {
        throw runtime_error("Tried to get least happy before calculating");
    }

    int mostIndex;
    int leastIndex;
    double leastPriority;
    double mostPriority;
    findAverage(leastIndex, mostIndex, leastPriority, mostPriority);

    return leastPriority;
}

// TODO: split the finding of the least and most happy worker into seperate 
// functions from finding the average?
double Scheduler::findAverage(int &leastIndex, int &mostIndex,
                              double &leastPriority, double &mostPriority) {
    double totalPriority = 0;
    int totalShifts = 0;
    bool firstWorker = true;
    int n = inputData.getNumWorkers();
    for (int i = 0; i < n; i++) {
        double currPriority = 0;
        WorkerNode *currWorker = inputData.getWorker(i);
        for (auto shift = currWorker->getAllocations().begin();
             shift != currWorker->getAllocations().end(); shift++) {
            currPriority += (*shift)->getPriority(finalSchedule, true);
        }

        int currShifts =  currWorker->getAllocations().size();
        // for calculating overall averages
        totalPriority += currPriority;
        totalShifts += currShifts;

        double currAverage = currPriority / (double) currShifts;

        // finding the most and least happy worker
        if (firstWorker or currAverage < leastPriority) {
            leastPriority = currAverage;
            leastIndex = i;
        }
        if (firstWorker or currAverage > mostPriority) {
            mostPriority = currAverage;
            mostIndex = i;
        }

        firstWorker = false;
    }
    // return average happiness across all workers
    return totalPriority / (double)totalShifts;
}

/********************************** Printing **********************************/

// average priority
// most satisfied worker
// least satisfied worker
void Scheduler::printStats(ostream &output) {
    output << "Stats (seed = " << seed << "):" << endl;
    int mostIndex;
    int leastIndex;
    double leastPriority;
    double mostPriority;
    double averageHappiness =
        findAverage(leastIndex, mostIndex, leastPriority, mostPriority);

    output << "Average Happiness: " << averageHappiness << endl;
    output << "Most Happy Worker: " 
           << (inputData.getWorker(mostIndex))->getName() << " with "
           << mostPriority << endl;
    output << "Least Happy Worker: " 
           << (inputData.getWorker(leastIndex))->getName() << " with "
           << leastPriority << endl;
}

// prints the final schedule according to what has been calculated
void Scheduler::printFinalSchedule(ostream &output) {
    for (int i = 0; i < NUM_DAYS; i++) {
        for (int j = 0; j < MAX_SHIFTS; j++) {
            sort(finalSchedule[i][j].begin(), finalSchedule[i][j].end(),
                 sortAlphabetical);
        }
    }
    schedulePrinter.printSchedule(output, finalSchedule);
}

bool Scheduler::sortAlphabetical(TimeSlotNode *t1, TimeSlotNode *t2) {
    return (t1->getParent()->getName() < t2->getParent()->getName());
}

// prints all the workers available at each time slot
void Scheduler::printScheduleShifts(ostream &output) {
    // printSchedule(output, workersAvailable);
    schedulePrinter.printSchedule(output, inputData.getWorkersAvailable());
}

// printing all basic worker info (name, max num shifts, total available shifts)
void Scheduler::printWorkers(ostream &output) {
    int n = inputData.getNumWorkers();
    for (int i = 0; i < n; i++) {
        inputData.getWorker(i)->printBasic(output);
    }
}

// printing all worker info, including all shifts
void Scheduler::printWorkerShifts(ostream &output) {
    int n = inputData.getNumWorkers();
    for (int i = 0; i < n; i++) {
        inputData.getWorker(i)->printFull(output);
    }
}

// prints all workers and how many shifts they're scheduled for, as well
// as how that number compares to their desired max shifts
void Scheduler::printWorkerShiftNum(ostream &output) {
    int n = inputData.getNumWorkers();
    for (int i = 0; i < n; i++) {
        WorkerNode *currWorker = inputData.getWorker(i);
        output << currWorker->getName() << " on "
               << currWorker->getMaxShifts() - currWorker->getShiftsRemaining()
               << " out of " << currWorker->getMaxShifts() 
               << " shifts" <<  endl;
    }
}
