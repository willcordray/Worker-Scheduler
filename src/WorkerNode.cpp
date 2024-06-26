#include "WorkerNode.h"

WorkerNode::WorkerNode(string newName, int newMaxShifts) {
    name = newName;
    maxShifts = newMaxShifts;
    shiftsRemaining = maxShifts;
    relativeBooking = -1 * maxShifts;
    seen = false;
    noPath = false;
}

void WorkerNode::resetValues() {
    shiftsRemaining = maxShifts; // todo: pull this out into seperate function (reused code from constructor)
    relativeBooking = -1 * maxShifts;
    seen = false;
    noPath = false;

    timesAllocated.clear();
    // reset the values of the time slot nodes;
    for (auto it = timesAvailable.begin(); it != timesAvailable.end(); it++) {
        (*it)->resetValues();
    }
}

void WorkerNode::set_seen(bool newValue) {
    seen = newValue;
}

bool WorkerNode::get_seen() {
    return seen;
}

void WorkerNode::set_noPath(bool newValue) {
    noPath = newValue;
}

bool WorkerNode::get_noPath() {
    return noPath;
}

const vector<TimeSlotNode *> *WorkerNode::getAvailability() const {
    return &timesAvailable;
}

const vector<WorkerNode *> *WorkerNode::getLikedCoworkers() const {
    return &likedCoworkers;
}

const vector<TimeSlotNode *> &WorkerNode::getAllocations() const {
    return timesAllocated;
}

const string WorkerNode::getName() const {
    return name;
}

int WorkerNode::getShiftsRemaining() const {
    return shiftsRemaining;
}

int WorkerNode::getMaxShifts() const {
    return maxShifts;
}

int WorkerNode::getRelativeBooking() const {
    return relativeBooking;
}

void WorkerNode::updateShiftsRemaining(int updateFactor) {
    shiftsRemaining += updateFactor;
    relativeBooking -= updateFactor;
}


// TODO: the usage of thisWorker is kinda ugly. Another way to get permanent
// pointer to self?
void WorkerNode::addShift(WorkerNode *thisWorker, int day, int shift, double priority) {
    TimeSlotNode *newShift = new TimeSlotNode(thisWorker, day, shift, priority);

    timesAvailable.push_back(newShift);
}

void WorkerNode::addLikedCoworker(WorkerNode *newWorker) {
    likedCoworkers.push_back(newWorker);
}

void WorkerNode::allocateBlock(TimeSlotNode *toChoose, bool &problem) {
    for (auto it = timesAvailable.begin(); it != timesAvailable.end(); it++) {
        if (*it == toChoose) {
            timesAllocated.push_back(*it);
            return;
        }
    }
    cerr << "We should never reach here allocate" << endl;  // TODO: Remove
    problem = true;
}

void WorkerNode::deallocateBlock(TimeSlotNode *toRemove, bool &problem) {
    for (auto it = timesAllocated.begin(); it != timesAllocated.end(); it++) {
        if (*it == toRemove) {
            timesAllocated.erase(it);
            return;
        }
    }
    cerr << "We should never reach here deallocate" << endl;  // TODO: Remove
    problem = true;
}

void WorkerNode::printBasic(ostream &output) {
    output << "Name: " << name << ", Max Shifts: " << maxShifts
           << ", Total Shifts Available: " << timesAvailable.size() << endl;
}

void WorkerNode::printFull(ostream &output) {
    printBasic(output);
    for (size_t i = 0; i < timesAvailable.size(); i++) {
        timesAvailable[i]->printTime(output);
        output << endl;
    }
    output << endl << endl;
}
