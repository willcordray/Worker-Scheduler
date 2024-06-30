#include "PrintSchedule.h"

void PrintSchedule::printSchedule(
    ostream &output, const vector<vector<vector<TimeSlotNode *>>> &workers) {
    vector<int> maxSize;  // max size of a name/thing in each column
    determineMaxSize(maxSize, workers);

    // length of one line
    //    first brace + second brace + num gaps
    int lineLength = 1 + 1 + (NUM_DAYS);
    for (size_t i = 0; i < maxSize.size(); i++) {
        lineLength += 2 * NUM_SPACES_PRINTING;  // num spaces of gap
        lineLength += maxSize[i];
    }

    printDashes(output, lineLength);

    printDayNames(output, maxSize);

    printDashes(output, lineLength);

    for (int i = 0; i < MAX_SHIFTS; i++) {  // loop all shift blocks
        size_t largestDay = 1;              // shift name
        for (int j = 0; j < NUM_DAYS; j++) {
            if (workers[j][i].size() > largestDay) {
                largestDay = workers[j][i].size();
            }
        }
        //                  in middle    if odd, more lower  0 index
        size_t printShiftLoc = largestDay / 2 + (largestDay % 2) - 1;

        printBlankline(output, maxSize);
        for (size_t j = 0; j < largestDay; j++) {  // loop all days of shifts
            output << "|";
            if (printShiftLoc == j) {
                printEven(output, shiftNames[i], maxSize[0]);
            } else {
                printEven(output, "", maxSize[0]);
            }

            for (int k = 0; k < NUM_DAYS; k++) {  // loop all shifts
                if (j < workers[k][i].size()) {
                    printEven(output, workers[k][i][j]->getParent()->getName(),
                              maxSize[k + 1]);
                } else {
                    printEven(output, "", maxSize[k + 1]);
                }
            }
            output << endl;
        }
        printBlankline(output, maxSize);
        printDashes(output, lineLength);
    }

    // printDashes(output, lineLength);
}

// determines the max size of a string in a column for printing
void PrintSchedule::determineMaxSize(
    vector<int> &maxSize, const vector<vector<vector<TimeSlotNode *>>> &workers) {
    maxSize.resize(NUM_DAYS + 1);
    for (int i = 0; i < NUM_DAYS; i++) {
        maxSize[i + 1] = dayNames[i].size();
        for (int j = 0; j < MAX_SHIFTS; j++) {
            for (size_t k = 0; k < workers[i][j].size(); k++) {
                int newSize = workers[i][j][k]->getParent()->getName().size();
                if (newSize > maxSize[i + 1]) {
                    maxSize[i + 1] = newSize;
                }
            }
        }
    }

    maxSize[0] = 0;
    for (int i = 0; i < MAX_SHIFTS; i++) {
        int size = shiftNames[i].size();
        if (size > maxSize[0]) {
            maxSize[0] = size;
        }
    }
}

void PrintSchedule::printBlankline(ostream &output, vector<int> &maxSize) {
    output << "|";
    for (int i = 0; i < NUM_DAYS + 1; i++) {
        for (int j = 0; j < maxSize[i] + 2 * NUM_SPACES_PRINTING; j++) {
            output << " ";
        }
        output << "|";
    }
    output << endl;
}

void PrintSchedule::printDashes(ostream &output, int numDashes) {
    for (int i = 0; i < numDashes; i++) {
        output << "-";
    }
    output << endl;
}

void PrintSchedule::printEven(ostream &output, string toPrint, int size) {
    int toFill = size - toPrint.size();

    // gap spaces
    for (int i = 0; i < NUM_SPACES_PRINTING; i++) {
        output << " ";
    }

    // filling spaces
    for (int i = 0; i < toFill / 2 + (! PRINT_CENTERED_LEFT and (toFill % 2));
         i++) {
        output << " ";
    }

    output << toPrint;

    // filling spaces, with extra space (if odd) on right of word
    for (int i = 0; i < toFill / 2 + (PRINT_CENTERED_LEFT and (toFill % 2));
         i++) {
        output << " ";
    }

    // gap spaces
    for (int i = 0; i < NUM_SPACES_PRINTING; i++) {
        output << " ";
    }
    output << "|";
}

void PrintSchedule::printDayNames(ostream &output, vector<int> &maxSize) {
    printBlankline(output, maxSize);
    output << "|";
    printEven(output, "", maxSize[0]);  // top left box = blank
    for (int i = 0; i < NUM_DAYS; i++) {
        printEven(output, dayNames[i], maxSize[i + 1]);
    }
    output << endl;
    printBlankline(output, maxSize);
}
