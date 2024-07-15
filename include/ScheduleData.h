// TODO: there is probably a way to remove this file and input this information
//       in another way.

// includes all the basic stuff that every class needs, including constants
#ifndef SCHEDULE_DATA_H
#define SCHEDULE_DATA_H

#include <string>

using namespace std;

static const int NUM_DAYS = 6;
static const string dayNames[NUM_DAYS] = {"Monday",   "Tuesday", "Wednesday", // TODO: why is this name not in caps?
                                   "Thursday", "Friday",  "Sunday"};

static const int MAX_SHIFTS = 8; // maximum number of shifts in one day
static const string shiftNames[MAX_SHIFTS] = {
    "10:30-11:45", "12:00-1:15", "1:30-2:45", "3:00-4:15",
    "4:30-5:45",   "6:00-7:15",  "7:30-8:45", "9:00-10:15"};


static const int WORKERS_PER_SHIFT[NUM_DAYS][MAX_SHIFTS] = {
    {2, 2, 2, 2, 2, 2, 2, 2},
    {2, 2, 2, 2, 2, 3, 3, 3},
    {2, 2, 2, 2, 2, 2, 2, 2},
    {2, 2, 2, 2, 2, 2, 2, 2},
    {2, 2, 2, 2, 2, 0, 0, 0},
    {2, 2, 2, 2, 2, 2, 2, 2},
};

static const int NUM_SPACES_PRINTING = 2; // number of spaces between the vertical bars and the content
static const bool PRINT_CENTERED_LEFT = false; // print with the extra space on left (true) or right (false)

// PENALTIES
static const double doubleShiftPenalty = 1; // Two shifts back to back. Does NOT also apply double day penalty
static const double doubleDayPenalty = 0.5; // Two shifts in one day.

// BONUSES:
static const double coworkerPreferenceBonus = 1; // with a person they want to work with

// PROPORTION:
static const double averageProportion = 0.7; // average priority (happiness) of TAs
static const double lowestProportion = 0.2; // priority of lowest TA
static const double overbookedRange = 0.1; // difference between most overbooked and least overbooked TA


#endif
