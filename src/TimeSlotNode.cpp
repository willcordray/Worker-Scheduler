#include "TimeSlotNode.h"

TimeSlotNode::TimeSlotNode(WorkerNode *newParent, int newDay, int newShift,
                           double newPriority) {
    parent = newParent;

    day = newDay;
    shift = newShift;
    truePriority = newPriority;
    used = false;
    seen = false;
}

void TimeSlotNode::resetValues() {
    priority = truePriority;
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

double TimeSlotNode::get_priority(
    const vector<vector<vector<TimeSlotNode *>>> &workers,
    bool useTruePriority) const {
    return (useTruePriority ? truePriority : priority) - calcPenalty() + calcBonus(workers);
}

// Note: penalty applies exponentially compared to how many shifts they are on in a row.
double TimeSlotNode::calcPenalty() const {
    double penalty = 0;
    int numDoubleDay = 0;
    int numDoubleShift = 0;
    for (auto allocated = parent->getAllocations().begin();
         allocated != parent->getAllocations().end(); allocated++) {
        // double day
        if ((*allocated)->get_day() == day and (*this != **allocated)) {
            // double shift
            if ((*allocated)->get_shift() == shift - 1 or
                (*allocated)->get_shift() == shift + 1) {
                numDoubleShift++;
            } else {
                numDoubleDay++;
            }
        }
    }

    penalty += exponeniatePenalty(numDoubleDay, 2, doubleDayPenalty);
    penalty += exponeniatePenalty(numDoubleShift, 2, doubleShiftPenalty);
                // penalty += doubleShiftPenalty;
                // penalty += doubleDayPenalty;
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

    double finalFactor = 1;
    for (int i = 0; i < times - 1; i++) {
        finalFactor *= factor;
    }
    return (finalFactor * penalty) / (times + 1);
}

double TimeSlotNode::calcBonus(
    const vector<vector<vector<TimeSlotNode *>>> &workers) const {
    double bonus = 0;

    // check for the coworkerPreference bonus:
    //     Note: Bonus applies linearly to how many people they are one shift
    //     with who they like

    const vector<TimeSlotNode *> *workersToCheck = &(workers[day][shift]);
    const vector<WorkerNode *> *likedCoworkers = parent->getLikedCoworkers();

    // a hash table implementation might be faster at higher values of n, but 
    // n is so small that I don't think the tradeoff is worth it. I might be 
    // wrong though.
    for (auto toCheck = workersToCheck->begin(); toCheck != workersToCheck->end();
         toCheck++) {
        for (auto toMatch = likedCoworkers->begin(); toMatch != likedCoworkers->end(); toMatch++) {
            if ((*toCheck)->get_parent()->getName() == (*toMatch)->getName()) { // found liked
                bonus += coworkerPreferenceBonus;
            }
        }
    }

    return bonus;
}

double TimeSlotNode::get_true_priority() const {
    return truePriority;
}

int TimeSlotNode::get_day() const {
    return day;
}

int TimeSlotNode::get_shift() const {
    return shift;
}

WorkerNode *TimeSlotNode::get_parent() const {
    return parent;
}

bool TimeSlotNode::get_used() const {
    return used;
}

bool TimeSlotNode::getSeen() const {
    return seen;
}
TimeSlotNode *TimeSlotNode::getPrev() const {
    return prev;
}

void TimeSlotNode::set_true_priority(double newPriority) {
    truePriority = newPriority;
}

void TimeSlotNode::set_priority(double newPriority) {
    priority = newPriority;
}

void TimeSlotNode::setSeen(bool newValue) {
    seen = newValue;
}

void TimeSlotNode::set_used(bool newValue) {
    used = newValue;
}

void TimeSlotNode::setPrev(TimeSlotNode *newPrev) {
    prev = newPrev;
}

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
