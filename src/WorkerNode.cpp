#include "WorkerNode.h"

WorkerNode::WorkerNode(string newName, int newMaxShifts) {
    restoreDefaults();

    name = newName;
    maxShifts = newMaxShifts;
}

void WorkerNode::resetValues() {
    restoreDefaults();

    timesAllocated.clear();
    // reset the values of the time slot nodes;
    for (auto it = timesAvailable.begin(); it != timesAvailable.end(); it++) {
        (*it)->restoreDefaults();
    }
}

void WorkerNode::restoreDefaults() {
    shiftsRemaining = maxShifts;
    relativeBooking = -1 * maxShifts;
    noPath = false;
}

void WorkerNode::setNoPath(bool newValue) {
    noPath = newValue;
}

bool WorkerNode::getNoPath() {
    return noPath;
}

const vector<TimeSlotNode *> &WorkerNode::getAvailability() const {
    return timesAvailable;
}

const unordered_set<WorkerNode *> &WorkerNode::getLikedCoworkers() const {
    return likedCoworkers;
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


void WorkerNode::addShift(int day, int shift, double priority) {
    TimeSlotNode *newShift = new TimeSlotNode(this, day, shift, priority);

    timesAvailable.push_back(newShift);
}

void WorkerNode::addLikedCoworker(WorkerNode *newWorker) {
    likedCoworkers.insert(newWorker);
}

void WorkerNode::allocateBlock(TimeSlotNode *toChoose) {
    updateShiftsRemaining(-1); // removing a block
    toChoose->setUsed(true);

    for (auto it = timesAvailable.begin(); it != timesAvailable.end(); it++) {
        if (*it == toChoose) {
            timesAllocated.push_back(*it);
            return;
        }
    }
    cerr << "We should never reach here allocate" << endl;  // TODO: Remove
}

void WorkerNode::deallocateBlock(TimeSlotNode *toRemove) {
    updateShiftsRemaining(1); // removing a block
    toRemove->setUsed(false);

    for (auto it = timesAllocated.begin(); it != timesAllocated.end(); it++) {
        if (*it == toRemove) {
            timesAllocated.erase(it);
            return;
        }
    }
    cerr << "We should never reach here deallocate" << endl;  // TODO: Remove
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
