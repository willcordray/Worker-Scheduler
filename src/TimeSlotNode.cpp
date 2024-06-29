#include "TimeSlotNode.h"

TimeSlotNode::TimeSlotNode(WorkerNode *newParent, int newDay, int newShift,
                           double newPriority) {
    resetRunValues();

    parent = newParent;
    day = newDay;
    shift = newShift;
    truePriority = newPriority;
}

void TimeSlotNode::resetRunValues() {
    used = false;
    seen = false;
}

bool TimeSlotNode::operator==(const TimeSlotNode &other) const {
    return (parent == other.parent) and (day == other.day) and
           (shift == other.shift);
}

bool TimeSlotNode::operator!=(const TimeSlotNode &other) const {
    return not (*this == other);
}

double TimeSlotNode::getPriority(
    const vector<vector<vector<TimeSlotNode *>>> &workers,
    bool useTruePriority) const {
    
    return (useTruePriority ? truePriority : priority) - calcPenalty() + calcBonus(workers);
}

/**************************** Penalty Calculation *****************************/

// Note: penalty applies exponentially compared to how many shifts they are on 
// in a row.
double TimeSlotNode::calcPenalty() const {
    double penalty = 0;
    int numDoubleDay = 0;
    int numDoubleShift = 0;
    for (auto allocated = parent->getAllocations().begin();
         allocated != parent->getAllocations().end(); allocated++) {
        // double day
        if ((*allocated)->getDay() == day and (*this != **allocated)) {
            // double shift
            if ((*allocated)->getShift() == shift - 1 or
                (*allocated)->getShift() == shift + 1) {
                numDoubleShift++;
            } else {
                numDoubleDay++;
            }
        }
    }

    penalty += exponeniatePenalty(numDoubleDay, 2, doubleDayPenalty);
    penalty += exponeniatePenalty(numDoubleShift, 2, doubleShiftPenalty);
    return penalty;
}

// Calculates the correct penalty to apply given the number of times the penalty
// occurred, the multiplication factor, as well as the penalty that should be
// applied for each infraction
// NOTE: does not include times when calculating the final return value, because
//       the assumption is that this penalty will also be called for all other
//       instances. For example, in a double shift, both shifts would be
//       individual penalties/infractions.
// times = 1, factor = 2, penalty = 0.5
double TimeSlotNode::exponeniatePenalty(int times, double factor, double penalty) const {
    if (times == 0) {
        return 0;
    }

    // TODO: is this the best punishment function?
    // times - 1 because this is a scaler for if there are multiple problems
    double finalFactor = pow(factor, times - 1);
    
    return (finalFactor * penalty) / (times + 1);
}

double TimeSlotNode::calcBonus(
    const vector<vector<vector<TimeSlotNode *>>> &workers) const {
    double bonus = 0;

    // check for the coworkerPreference bonus:
    //     Note: Bonus applies linearly to how many people they are on shift
    //     with that they like

    unordered_set<WorkerNode *> likes = parent->getLikedCoworkers();
    for (auto toMatch = workers[day][shift].begin(); 
         toMatch != workers[day][shift].end(); toMatch++) {
        if (likes.find((*toMatch)->getParent()) != likes.end()) {
            bonus += coworkerPreferenceBonus;
        }
    }

    return bonus;
}

/***************************** Getters and Setters ****************************/

double TimeSlotNode::getTruePriority() const {
    return truePriority;
}

int TimeSlotNode::getDay() const {
    return day;
}

int TimeSlotNode::getShift() const {
    return shift;
}

WorkerNode *TimeSlotNode::getParent() const {
    return parent;
}

bool TimeSlotNode::getUsed() const {
    return used;
}

bool TimeSlotNode::getSeen() const {
    return seen;
}
TimeSlotNode *TimeSlotNode::getPrev() const {
    return prev;
}

void TimeSlotNode::setTruePriority(double newPriority) {
    truePriority = newPriority;
}

void TimeSlotNode::setPriority(double newPriority) {
    priority = newPriority;
}

void TimeSlotNode::setSeen(bool newValue) {
    seen = newValue;
}

void TimeSlotNode::setUsed(bool newValue) {
    used = newValue;
}

void TimeSlotNode::setPrev(TimeSlotNode *newPrev) {
    prev = newPrev;
}

/*********************************** Printing *********************************/

void TimeSlotNode::printTime(ostream &output) const {
    output << dayNames[day] << " : " << shiftNames[shift];
}

// delete because never used
void TimeSlotNode::printInfo(ostream &output) const {
    output << "    " << parent->getName() << ", " << priority;
}

void TimeSlotNode::print(ostream &output) const {
    output << parent->getName() << "  ";
    printTime(output);
}
