#include "Scheduler.h"

Scheduler::Scheduler(WorkerInputData &data, unsigned int newSeed) : inputData(data) {
    finalSchedule = vector<vector<vector<TimeSlotNode *>>>(NUM_DAYS, vector<vector<TimeSlotNode *>>(MAX_SHIFTS));
    seed = newSeed;
    calculated = false;
    addTinyPriorityChange(newSeed);
}

/* TODO: could this be done in pre-processing? Only do the tiny change here. 
   Also, we should scale the tiny change based off of the range of the initial number to remove the arbitrary restriction */

// normalizes priorities based on (X - min) / (max - min) = newPriority
void Scheduler::addTinyPriorityChange(unsigned int randSeed) {
    srand(randSeed);
    int n = inputData.getNumWorkers();

    for (int i = 0; i < n; i++) {
        const vector<TimeSlotNode *> timeslots = inputData.getWorker(i)->getAvailability();
        for (auto slot = timeslots.begin(); slot != timeslots.end(); slot++) {
            // tiny change adds some minor variance so that this can be rerun
            // with different random values, and get different (maybe better)
            // results
            double tinyChange = ((double)rand() / RAND_MAX) / tinyChangeDivisor;
            double wigglePriority = (*slot)->getTruePriority() + tinyChange;

            (*slot)->setPriority(wigglePriority);
        }
    }
}


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

    toAssign->getParent()->allocateBlock(toAssign); /* this extra thing feels unnecessary. Maybe just compute the TA allocation on the fly when needed? */
}

// takes a timeslot node and removes it from the final schedule
void Scheduler::removeAllocation(TimeSlotNode *toRemove) {
    // remove from final schedule
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
    int numShifts = NUM_DAYS * MAX_SHIFTS;
    for (int i = 0; i < NUM_DAYS; i++) {  // loop all shifts
        for (int j = 0; j < MAX_SHIFTS; j++) {
            shifts.push_back({i, j});
        }
    }
    shuffle(shifts.begin(), shifts.end(), default_random_engine(rand()));
    // get random shift, then make allocations. Done in random order to prevent
    // tendency to overload some workers
    // Note: random needs to be done as one number, not rand day and rand shift
    //       because otherwise shifts on days with fewer shifts will be
    //       overrepresented
    for (int i = 0; i < numShifts; i++) {
        // convert combined to individual days and shifts
        int day = shifts[i].first;
        int shift = shifts[i].second;
        vector<TimeSlotNode *> currShift = inputData.getWorkersAvailable(day, shift);
        if (inputData.getWorkersPerShift(day, shift) > 0) {
            initialOneSlot(&currShift);
        }
    }
}

// initially allocate all of the TAs for one timeslot
void Scheduler::initialOneSlot(vector<TimeSlotNode *> *currQueue) {
    if (currQueue->size() == 0) {  // shift with no available TAs
        return;
    }

    int day = currQueue->front()->getDay();  // all same shift time
    int shift = currQueue->front()->getShift();
    for (int i = 0; i < inputData.getWorkersPerShift(day, shift); i++) {  // assign [num] TAs
        TimeSlotNode *topTimeNode;
        topTimeNode = findMaxTimeSlot(currQueue);
        vector<TimeSlotNode *> topPriority;
        double highestPriority = topTimeNode->getPriority(finalSchedule, false);
        for (auto it = currQueue->begin(); it != currQueue->end(); it++) {
            if (not (*it)->getUsed() and
                (*it)->getPriority(finalSchedule, false) == highestPriority) {
                topPriority.push_back(*it);
            }
        }
        //  TODO: this isn't necessarily a problem, so why is it an error?
        if (topPriority.size() != 1) {
            cerr << "Size not 1: " << topPriority.size() << endl;
        }
        int mostAvailability = INT_MIN; // TODO: change to a first time system?
        int indexMostAvailability = -1;
        for (size_t j = 0; j < topPriority.size(); j++) {
            if (topPriority[j]->getParent()->getShiftsRemaining() >
                mostAvailability) {
                mostAvailability =
                    topPriority[j]->getParent()->getShiftsRemaining();
                indexMostAvailability = j;
            }
        }
        if (indexMostAvailability == -1) {
            cerr << "indexMostAvailablility is -1" << endl;
        }
        addAllocation(topPriority[indexMostAvailability]);
    }
}

TimeSlotNode *Scheduler::findMaxTimeSlot(vector<TimeSlotNode *> *currQueue) {
    TimeSlotNode *topTimeNode = nullptr;
    bool foundTimeSlot = false;
    double highestPriority;
    for (auto it = currQueue->begin(); it != currQueue->end(); it++) {
        if (not (*it)->getUsed() and
            (not foundTimeSlot or
             (*it)->getPriority(finalSchedule, false) > highestPriority)) {
            topTimeNode = *it;
            highestPriority = (*it)->getPriority(finalSchedule, false);
            foundTimeSlot = true;
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
    findMinMaxWorker(&min, &max);

    // loops until all workers are evenly allocated
    while (abs(max->getRelativeBooking() - min->getRelativeBooking()) > 1) {
        WorkerNode *currWorker = max;

        bool madeChange = searchWorker(currWorker);
        // try to find path

        if (madeChange) {
            resetNoPath();
        } else {
            cerr << "didn't find a path" << endl;
            currWorker->setNoPath(true);
        }

        if (not findMinMaxWorker(&min, &max)) {  // no more workers that are unmarked
            cerr << "No more paths" << endl;
            return;
        }
    }
}

bool Scheduler::findMinMaxWorker(WorkerNode **min, WorkerNode **max) {
    int n = inputData.getNumWorkers();
    if (n > 0) {
        *min = inputData.getWorker(0);
        *max = inputData.getWorker(0);
    } else {
        return false;
    }

    bool foundWorker = false;
    for (int i = 0; i < n; i++) {
        WorkerNode *currWorker = inputData.getWorker(i);
        if (not currWorker->getNoPath()) {
            if (currWorker->getRelativeBooking() < (*min)->getRelativeBooking()) {
                *min = currWorker;
            }
            if (currWorker->getRelativeBooking() > (*max)->getRelativeBooking()) {
                *max = currWorker;
            }


            foundWorker = true;
        }
    }
    return foundWorker;
}

void Scheduler::resetNoPath() {
    int n = inputData.getNumWorkers();
    for (int i = 0; i < n; i++) {
        inputData.getWorker(i)->setNoPath(false);
    }
}

// takes a worker, and calls the recursive findPath to try to change their shifts
bool Scheduler::searchWorker(WorkerNode *currWorker) {
    double bestPathVal;  // always less
    bool foundPath = false;
    const vector<TimeSlotNode *> blocks = currWorker->getAllocations();
    vector<TimeSlotNode *> bestPath;  // variable that will hold best path
    for (auto it = blocks.begin(); it != blocks.end(); it++) {
        pair<double, TimeSlotNode *> result = findPath(*it);
        if (result.second != nullptr and (!foundPath or result.first > bestPathVal)) {
            bestPathVal = result.first;
            bestPath = buildPath(result.second);
            foundPath = true;
        }
    }

    if (foundPath) {
        makeChanges(bestPath);
    }
    return foundPath;
}

void Scheduler::resetSeen() {
    int n = inputData.getNumWorkers();
    for (int i = 0; i < n; i++) {
        const vector<TimeSlotNode *> nodes = inputData.getWorker(i)->getAvailability();
        for (size_t j = 0; j < nodes.size(); j++) {
            nodes[j]->setSeen(false);
        }
    }
}

void Scheduler::resetPrev() {
    int n = inputData.getNumWorkers();
    for (int i = 0; i < n; i++) {
        vector<TimeSlotNode *> slots = inputData.getWorker(i)->getAvailability();
        for (size_t j = 0; j < slots.size(); j++) {
            slots[j]->setPrev(nullptr);
        }
    }
}

pair<double, TimeSlotNode *> Scheduler::findPath(TimeSlotNode *overbooked) {
    resetSeen();
    resetPrev();
    int prevBooking = overbooked->getParent()->getRelativeBooking(); // we are trying to find someone who is less overbooked

    priority_queue<pair<double, TimeSlotNode *>> paths;
    paths.push({-overbooked->getPriority(finalSchedule, false), overbooked});
    overbooked->setSeen(true);

    pair<double, TimeSlotNode *> bestPath = {0, nullptr};
    while(!paths.empty()) {
        pair<double, TimeSlotNode *> currPath = paths.top();
        paths.pop();

        vector<TimeSlotNode *> neighbors = findNeighbors(currPath.second);

        // shift to add in place of current
        for (size_t i = 0; i < neighbors.size(); i++) {
            if (neighbors[i]->getSeen()) { // already on a path
                continue;
            }

            neighbors[i]->setPrev(currPath.second);
            neighbors[i]->setSeen(true);

            // underbooked person, so valid path
            if (abs(neighbors[i]->getParent()->getRelativeBooking() - prevBooking) > 1) {
                double pathValue = currPath.first + neighbors[i]->getPriority(finalSchedule, false);
                if (bestPath.second == nullptr or pathValue > bestPath.first) {
                    bestPath = {pathValue, neighbors[i]};
                    numPaths++;
                }
            }

            // shift to remove from the same person
            const vector<TimeSlotNode *> allocations = neighbors[i]->getParent()->getAllocations();
            for (size_t j = 0; j < allocations.size(); j++) {
                if (!allocations[j]->getUsed() || allocations[j]->getSeen()) {
                    continue;
                }


                if (allocations[j] != neighbors[i]) { // different shift than the one we're adding in
                    allocations[j]->setPrev(neighbors[i]); // maintain the path
                    allocations[j]->setSeen(true); // what happens if you don't do this? Shouldn't technically cause an infinite loop, and maybe even should be disabled. If it was disable, would necessitate a different way to track the path
                    double newValue = currPath.first + neighbors[i]->getPriority(finalSchedule, false) - allocations[j]->getPriority(finalSchedule, false);
                    paths.push({newValue, allocations[j]});
                }
            }
        }
    }

    return bestPath;
}

// rename to find replacements?
vector<TimeSlotNode *> Scheduler::findNeighbors(TimeSlotNode *initial) {
    int day = initial->getDay(), shift = initial->getShift();
    vector<TimeSlotNode *> potentialNeighbors = inputData.getWorkersAvailable(day, shift);


    vector<TimeSlotNode *> neighbors;
    for (size_t i = 0; i < potentialNeighbors.size(); i++) {
        TimeSlotNode *currNode = potentialNeighbors[i];
        if (!currNode->getUsed() && currNode != initial) {
            neighbors.push_back(currNode);
        }
    }

    return neighbors;
}
// make sure the one we're adding is not allocated (neighbors), and the one we're removing is allocated (allocations)

vector<TimeSlotNode *> Scheduler::buildPath(TimeSlotNode *end) {
    vector<TimeSlotNode *> result;
    result.push_back(end);
    while (end != nullptr && end->getPrev() != nullptr) {
        result.push_back(end->getPrev());
        end = end->getPrev();
    }
    reverse(result.begin(), result.end());
    return result;
}

/* [1] allison, brooke john
   [2] allison, brooke steve

allison -> steve
*/

// double newPathValue = currPathValue +
//                                   (*toAdd)->getPriority(finalSchedule, false) -
//                                   (*toRemove)->getPriority(finalSchedule, false)



// path size must be even
// path goes allocated -> not allocated -> allocated -> etc. (ends on not
//      allocated)
void Scheduler::makeChanges(vector<TimeSlotNode *> path) {
    bool allocated = true;
    for (auto it = path.begin(); it != path.end(); it++) {
        if (allocated) {
            removeAllocation(*it);
        } else {
            addAllocation(*it);
        }

        allocated = not allocated;
    }
}

// validate that a solution works, i.e. all shifts have correct number of workers,
// as well as under the hood problems like values of used and seen
void Scheduler::validateSolution() {
    unordered_set<TimeSlotNode *> inFinal;
    for (int i = 0; i < NUM_DAYS; i++) {
        for (int j = 0; j < MAX_SHIFTS; j++) {
            // correct number of workers on shift
            int size = finalSchedule[i][j].size();
            if (inputData.getWorkersPerShift(i, j) != size) {
                string message = "Error: Wrong number of workers on shift " +
                                 dayNames[i] + " " + shiftNames[j];
                throw runtime_error(message);
            }

            // no duplicate workers on same shift
            unordered_set<string> namesSoFar;
            for (auto it = finalSchedule[i][j].begin();
                 it != finalSchedule[i][j].end(); it++) {
                auto found = namesSoFar.find((*it)->getParent()->getName());
                if (found != namesSoFar.end()) {  // found repeat
                    string message =
                        "Error: " + (*it)->getParent()->getName() + " is on " +
                        dayNames[i] + " " + shiftNames[j] + " more than once";
                    throw runtime_error(message);
                }
                namesSoFar.insert((*it)->getParent()->getName());

                inFinal.insert(*it);
            }

        }
    }

    int n = inputData.getNumWorkers();
    for (int i = 0; i < n; i++) {
        WorkerNode *currWorker = inputData.getWorker(i);
        const vector<TimeSlotNode *> allocations = currWorker->getAllocations();
        for (auto slot = allocations.begin(); slot != allocations.end();
             slot++) {
            auto loc = inFinal.find((*slot));
            if (loc == inFinal.end()) {  // not found
                string message =
                    "Error: " + (*slot)->getParent()->getName() + " " +
                    dayNames[(*slot)->getDay()] + " " +
                    shiftNames[(*slot)->getShift()] +
                    " is marked as allocated, but is not in final scheduler";
                throw runtime_error(message);
            }
            inFinal.erase((*slot));
        }
    }
    // in final schedule, but not in worker allocated array
    if (inFinal.size() != 0) {
        string message = "Error: there are " + to_string(inFinal.size()) +
                         " shift(s) in the final schedule that are not in workers "
                         "allocated arrays";
        throw runtime_error(message);
    }
}



double Scheduler::getAverage() {
    if (not calculated) {
        throw runtime_error("Tried to get average before calculating");
    }

    int mostIndex;
    int leastIndex;
    double leastPriority;
    double mostPriority;
    return findAverage(leastIndex, mostIndex, leastPriority, mostPriority);
}

int Scheduler::getRange() {
    WorkerNode *min;
    WorkerNode *max;
    findMinMaxWorker(&min, &max);

    return (max->getRelativeBooking() - min->getRelativeBooking());
}

double Scheduler::getLeastHappy() {
    int mostIndex;
    int leastIndex;
    double leastPriority;
    double mostPriority;
    findAverage(leastIndex, mostIndex, leastPriority, mostPriority);

    return leastPriority;
}

// TODO: remove these print functions?
// printing all basic worker info (name, max num shifts, total available shifts)
void Scheduler::printWorkers(ostream &output) {
    int n = inputData.getNumWorkers();
    for (int i = 0; i < n; i++) {
        inputData.getWorker(i)->printBasic(output);
    }
}

// printing all worker info, including all shifts worker focused printing
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
               << " out of " << currWorker->getMaxShifts() << endl;
    }
}

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
    output << "Most Happy Worker: " << (inputData.getWorker(mostIndex))->getName() << " with "
           << mostPriority << endl;
    output << "Least Happy Worker: " << (inputData.getWorker(leastIndex))->getName() << " with "
           << leastPriority << endl;
}

// TODO: make find average return the normalized value
double Scheduler::findAverage(int &leastIndex,
                              int &mostIndex,
                              double &leastPriority, double &mostPriority) {
    double totalPriority = 0;
    int totalShifts = 0;
    bool firstWorker = true;
    int n = inputData.getNumWorkers();
    for (int i = 0; i < n; i++) {
        double currPriority = 0;
        int currShifts = 0;
        WorkerNode *currWorker = inputData.getWorker(i);
        for (auto shift = currWorker->getAllocations().begin();
             shift != currWorker->getAllocations().end(); shift++) {
            // divide by 2 because also counts penalty for other shift on that
            // day, etc.
            currPriority += (*shift)->getPriority(finalSchedule, true);
            currShifts++;
        }
        // for calculating total averages
        totalPriority += currPriority;
        totalShifts += currShifts;

        double currValue = currPriority / (double)currShifts;

        if (firstWorker or currValue < leastPriority) {
            leastPriority = currValue;
            leastIndex = i;
        }
        if (firstWorker or currValue > mostPriority) {
            mostPriority = currValue;
            mostIndex = i;
        }

        firstWorker = false;
    }
    return totalPriority / (double)totalShifts;
}



void Scheduler::printFinalSchedule(ostream &output) {
    for (int i = 0; i < NUM_DAYS; i++) {
        for (int j = 0; j < MAX_SHIFTS; j++) {
            sort(finalSchedule[i][j].begin(), finalSchedule[i][j].end(),
                 orderWorkers);
        }
    }
    schedulePrinter.printSchedule(output, finalSchedule);
}

// prints all the workers available at each time slot
// Timeslot focused printing
void Scheduler::printScheduleShifts(ostream &output) {
    // printSchedule(output, workersAvailable);
    schedulePrinter.printSchedule(output, inputData.getWorkersAvailable());
}


bool Scheduler::orderWorkers(TimeSlotNode *t1, TimeSlotNode *t2) {
    return (t1->getParent()->getName() < t2->getParent()->getName());
}
