#include "PrintSchedule.h"

void PrintSchedule::printSchedule(
    ostream &output, const vector<vector<vector<TimeSlotNode *>>> &workers) {
    vector<int> maxSize;  // sizes of the largest elements in each column
    populateMaxSize(maxSize, workers);

    int lineLength = findLineLength(maxSize);

    printHeader(output, lineLength, maxSize);

    // print each row of boxes (same shift)
    for (int i = 0; i < MAX_SHIFTS; i++) {  // loop all shift blocks
        printRow(output, lineLength, i, maxSize, workers);
    }
}

// finds the length all lines should be in order to have a straight right border
int PrintSchedule::findLineLength(vector<int> &maxSize) {
    //    first bar + second bar + bars between days
    int lineLength = 1 + 1 + (NUM_DAYS);
    for (size_t i = 0; i < maxSize.size(); i++) {
        lineLength += 2 * NUM_SPACES_PRINTING;  // num spaces of gap
        lineLength += maxSize[i];
    }

    return lineLength;
}

// prints the top row
void PrintSchedule::printHeader(ostream &output, int lineLength,
                                vector<int> &maxSize) {
    printDashes(output, lineLength);
    printDayNames(output, maxSize);
    printDashes(output, lineLength);
}

void PrintSchedule::printRow(
    ostream &output, int lineLength, int shift, vector<int> &maxSize,
    const vector<vector<vector<TimeSlotNode *>>> &workers) {
    size_t largestDay = 0;  // day with the most number of workers on duty
    for (int j = 0; j < NUM_DAYS; j++) {
        largestDay = max(largestDay, workers[j][shift].size());
    }

    //                     in middle  ,      if odd, center name , 0 index
    size_t shiftNameLine = (largestDay / 2) + (largestDay % 2) - 1;

    printBlankline(output, maxSize);
    for (size_t j = 0; j < largestDay; j++) {  // loop all days of shifts
        output << "|";

        // printing of shift name (leftmost box)
        // line number that the shift name should be printed on to center it
        if (shiftNameLine == j) {
            printEven(output, shiftNames[shift], maxSize[0]);
        } else {
            printEven(output, "", maxSize[0]);  // print blank box
        }

        // print all of the workers on shift (across days, one per line)
        for (int k = 0; k < NUM_DAYS; k++) {  // loop all days
            if (j < workers[k][shift].size()) {
                printEven(output, workers[k][shift][j]->getParent()->getName(),
                          maxSize[k + 1]);
            } else {  // already printed all workers, so print a blank box
                printEven(output, "", maxSize[k + 1]);
            }
        }
        output << endl;  // move on to the next line
    }
    printBlankline(output, maxSize);
    printDashes(output, lineLength);
}

// determines the max size of a string in a column for printing
void PrintSchedule::populateMaxSize(
    vector<int> &maxSize,
    const vector<vector<vector<TimeSlotNode *>>> &workers) {
    maxSize.resize(NUM_DAYS + 1);
    // shift names (leftmost column)
    maxSize[0] = 0;
    for (int i = 0; i < MAX_SHIFTS; i++) {
        int size = shiftNames[i].size();
        if (size > maxSize[0]) {
            maxSize[0] = size;
        }
    }

    // largest name of a worker/day name in all remaining columns
    for (int i = 0; i < NUM_DAYS; i++) {
        maxSize[i + 1] = dayNames[i].size();
        for (int j = 0; j < MAX_SHIFTS; j++) {
            for (size_t k = 0; k < workers[i][j].size(); k++) {
                int currSize = workers[i][j][k]->getParent()->getName().size();
                maxSize[i + 1] = max(maxSize[i + 1], currSize);
            }
        }
    }
}

// prints a blank line, with all of the same box borders
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

// prints a line of all dashes
void PrintSchedule::printDashes(ostream &output, int numDashes) {
    for (int i = 0; i < numDashes; i++) {
        output << "-";
    }
    output << endl;
}

// prints the content of a single box, centering "toPrint" within spaces
void PrintSchedule::printEven(ostream &output, string toPrint, int totalChars) {
    int toFill = totalChars -
                 toPrint.size();  // number of characters to fill with spaces

    // beginning gap spaces
    printSpaces(output, NUM_SPACES_PRINTING);

    // left filling spaces (for if the content is smaller than the total characters to print)
    int leftFill = (toFill / 2) + (! PRINT_CENTERED_LEFT and (toFill % 2));
    printSpaces(output, leftFill);

    output << toPrint;  // print the content

    // filling spaces, with extra space (if odd) on right of word
    int rightFill = toFill / 2 + (PRINT_CENTERED_LEFT and (toFill % 2));
    printSpaces(output, rightFill);

    // ending gap spaces
    printSpaces(output, NUM_SPACES_PRINTING);
    output << "|";  // close the box
}

void PrintSchedule::printSpaces(ostream &output, int numSpaces) {
    for (int i = 0; i < numSpaces; i++) {
        output << " ";
    }
}

// prints the day names across the top line
void PrintSchedule::printDayNames(ostream &output, vector<int> &maxSize) {
    printBlankline(output, maxSize);

    output << "|";
    printEven(output, "", maxSize[0]);    // top left box is blank
    for (int i = 0; i < NUM_DAYS; i++) {  // print the days in successive boxes
        printEven(output, dayNames[i], maxSize[i + 1]);
    }
    output << endl;

    printBlankline(output, maxSize);
}
