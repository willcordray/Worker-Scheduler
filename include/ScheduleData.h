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

static const int NUM_SPACES_PRINTING = 2;
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


/* Second version set up for a different test below: */

// // includes all the basic stuff that every class needs, 
// // including user defined constants

// #ifndef SCHEDULE_DATA_H
// #define SCHEDULE_DATA_H

// #include <string>

// using namespace std;

// static const int NUM_DAYS = 6; // num days to schedule
// static const string dayNames[NUM_DAYS] = {"Monday",   "Tuesday", "Wednesday",
//                                    "Thursday", "Friday",  "Sunday"};

// static const int MAX_SHIFTS = 10; // maximum number of shifts in one day
// static const string shiftNames[MAX_SHIFTS] = {
//     "9:00am-10:15am", "10:30am-11:45am", "noon-1:15pm", "1:30pm-2:45pm", "3:00pm-4:15pm",
//     "4:30pm-5:45pm",   "6pm-7:15pm",  "7:30pm-8:45pm", "9:00pm-10:15pm", "10:30pm-11:45pm"};


// static const int TAS_PER_SHIFT[NUM_DAYS][MAX_SHIFTS] = {
//     {2, 2, 2, 2, 2, 2, 3, 3, 3, 3}, // Monday
//     {2, 2, 2, 2, 2, 2, 2, 3, 3, 3}, // Tuesday
//     {2, 2, 2, 2, 2, 2, 2, 2, 2, 0}, // Wednesday
//     {2, 2, 2, 2, 2, 2, 2, 2, 2, 0}, // Thursday
//     {2, 2, 2, 2, 2, 2, 0, 0, 0, 0}, // Friday
//     {2, 2, 2, 2, 2, 2, 2, 2, 2, 0}, // Sunday
// };

// // when printing final schedule, how many spaces to put to either side of the
// // name within the boxes
// static const int NUM_SPACES_PRINTING = 2;
// // print with the extra space on left (true) or right (false)
// static const bool PRINT_CENTERED_LEFT = true;




// // PENALTIES (positive = discourage)
// static const double doubleShiftPenalty = 1; // Two shifts back to back. Does NOT also apply double day penalty
// static const double doubleDayPenalty = 0.5; // Two shifts in one day.

// // BONUSES: (positive = encourage)
// static const double coworkerPreferenceBonus = 1; // with a person they want to work with

// // PROPORTION:
// static const double averageProportion = 0.7; // average priority (happiness) of TAs
// static const double lowestProportion = 0.2; // priority of lowest TA
// static const double overbookedRange = 0.1; // difference between most overbooked and least overbooked TA





// int toCombined(const int days, const int shift, int shifts_per_day[]);
// void toDaysAndShift(int combined, int &days, int &shift, int shifts_per_day[]);

// int dayString_to_int(string dayName);
// int shiftString_to_int(string shiftName);


// #endif

