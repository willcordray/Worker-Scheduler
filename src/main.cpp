/*
 *  main.cpp
 *  Will Cordray
 *  
 *  Driver file for scheduling Workers into TimeSlots
 * 
 *  usage: "./oh_scheduler [inputFileDirectory]"
 */

// TODO: transition to 8 space indentation

#include <signal.h>  // catch SIGINT

#include <iostream>
#include <string>
#include <limits.h>

#include <chrono>

#include "Scheduler.h"
#include "ScheduleData.h"

using namespace std;

void siginthandler(int param);
void printResult(WorkerInputData &general, unsigned int seed);
void singleSeed(char inputDirectory[], string seedParameter);


// 'pass' something into the siginthandler function. From what I can tell, no
// other way besides a global variable
bool keepGoing;

int main(int argc, char *argv[]) {
    if (argc != 2 and argc != 3) {  // Check for proper amount of arguments
        cerr << "usage: ./oh_scheduler [inputFileDirectory] "
                "(optional)[--seed=]"
             << endl;
        exit(EXIT_FAILURE);
    }

    string directory = argv[1];

    if (argc == 3) {
        string seedParam = argv[2];
        singleSeed(argv[1], seedParam);
        return 0;
    }

    cerr << "Use <Ctrl-C> to terminate the program and print best result" 
         << endl;
    signal(SIGINT, siginthandler);

    double greatest = -1.0;
    unsigned int indexGreatest = 1;

    WorkerInputData general(directory);

    auto t1 = chrono::high_resolution_clock::now();

    unsigned int i = 1;
    bool firstTime = true;
    keepGoing = true;
    while(keepGoing and i != UINT_MAX) {
        Scheduler scheduler(general, i);
        scheduler.calculate(); // create the schedule
        double average = scheduler.getAverage();
        double lowest = scheduler.getLeastHappy();
        int range = scheduler.getRange();
        double result = (averageProportion * average) 
                        + (lowestProportion * lowest) 
                        + (overbookedRange * range);

        if (firstTime or result > greatest) {
            greatest = result;
            indexGreatest = i;
            cerr << "Best Result: Average = " << average << ", lowest = " << lowest 
                 << ", range = " << range << ", seed = " << i << endl;
            firstTime = false;
        }

        if (i % 1000 == 0) { // useful for determining speed
            cerr << "At Seed: " << i << endl;
        }

        i++;
        // break;
    }
    auto t2 = chrono::high_resolution_clock::now();


    cerr << "Final Checked Seed: " << i - 1 << endl;
    printResult(general, indexGreatest);
    cout << endl;

    auto ms_int = chrono::duration_cast<chrono::milliseconds>(t2 - t1); // TODO: add chrono as command line, not just something that always happens
    double timeTaken = (double) ms_int.count() / 1000;
    cout << "Time taken (s): " << timeTaken << endl;
    cout << "Iterations per second: " << (double) (i - 1) / timeTaken << endl;


    return 0;
}

void siginthandler(int param) {
    (void) param;
    keepGoing = false;
    cout << '\n' << endl;
}

void printResult(WorkerInputData &general, unsigned int seed) {
    Scheduler scheduler(general, seed);
    scheduler.calculate();
    scheduler.printWorkerShiftNum(cout);
    scheduler.printFinalSchedule(cout);
    scheduler.printStats(cout);
}

// TODO: this is a tad dangerous. It doesn't check that it is actually --seed=, 
//       only that it is the first parameter
void singleSeed(char inputDirectory[], string seedParameter) { // --seed=
    unsigned int seed = stoi(seedParameter.substr(7, 
                             seedParameter.length() - 7));
    WorkerInputData general(inputDirectory);
    printResult(general, seed);
}
