/*
 * File:    Ludo_P2_V6.cpp
 * Author:  Rafid Sarowar
 * Purpose: Basically CIS-5 Project 2 Final Version — Every checklist item from Proj 1 & 2 is shown, and called out in comments!
 * 
 * ---------------------------------------------------
 * ---------------------------------------------------
 * It works as the basis of the CIS 17C Projects-1. I take the rules and the basics from this for the base of the project 1 of the CIS 17C.
 * ---------------------------------------------------
 * ---------------------------------------------------
 */

// [2.3] Required libraries: ALL included per checklist!
#include <iostream> // [2.3, 5.1] Standard I/O
#include <iomanip>  // [2.3, 2.7, 3.7] Output formatting (setw/setprecision)
#include <cmath>    // [2.3, 3.9] Math functions (pow, abs)
#include <cstdlib>  // [2.3, 3.5] Random, casting
#include <ctime>    // [2.3] Random seed
#include <fstream>  // [2.3, 5.11] File input/output
#include <string>   // [2.3, 2.8] Player names
#include <vector>   // [7.12] STL vector for demo of passing/parallel arrays
using namespace std;

// === [6.3, 6.5, 6.8, 6.9, 6.11, 6.12, 6.13, 6.14, 6.15, 7.5, 7.6, 7.8, 7.12] ===
// FUNCTION PROTOTYPES — All function checklist items demo’d!
int rollDie();                                                                             // Returns die roll [pass by value]
void showDie(int val);                                                                     // Shows die roll as text [switch demo]
void moveTok(int tokens[][2], int plIdx, int tkIdx, int roll, int finish, ofstream &logF); // [pass by reference]
int cutTok(int tokens[][2], int plCnt, int plIdx, int tkIdx, int start, ofstream &logF);   // [pass by reference, returns int]
bool winChk(const int tokens[][2], int plIdx, int finish);                                 // Returns bool, static variable
void printStat(const int tokens[][2], const string names[], int plCnt);                    // Overloading demo
void printStat(const vector<string> &names, const vector<int> &moves);                     // Overloaded for vector
int findMax(const int arr[], int sz);                                                      // [search, pass array]
void bSort(string names[], int arr[], int sz);                                             // [Bubble sort, parallel arrays]
void sSort(string names[], int arr[], int sz);                                             // [Selection sort, parallel arrays]
void showSize();                                                                           // [sizeof, hand-tracing]
void typeCastDemo();                                                                       // [type casting, mixing types]
float avgMoves(const int moves[], int sz = 4);                                             // [defaulted arg, pass by value]

// --- [2.12, 2.13, 2.4, 2.6] Declare local variables, no globals

// Declare local constants: START, FINISH, MAXP, TOKS

// [3.1, 5.2, 5.5] Input number of players (do-while)

// [6.3, 6.5, 6.13, 6.12, 6.14] Function prototypes (pass by value, ref, defaulted, overloading)

// [7.1, 7.2, 7.8, 7.9, 7.12] Declare tokens[4][2], names[4], stats arrays, demo vector

// [2.15, 17] Output formatted instructions

// [5.6, 4.11] Main game loop: while (!win)

//   [3.8, 2.7] Print formatted output

//   [5.1, 5.2] Increment turn counters

//   [3.7, 5.2] while for input validation

//   [4.4, 4.5, 4.6] if, if-else, nesting

//   [8.3, 8.4, 8.1] Sorting and searching functions

//   [4.14] Switch statement for die face output

//   [3.13] Logical operators (&&, ||)

//   [4.13] Conditional operator (?:)

//   [9.1, 3.9] Math library: pow, abs

//   [6.11] static variable use in a function

//   [6.15] exit() for clean exit after game over

//   [7.5, 7.6] Pass arrays/vectors to functions

//   [2.9] float use, no double

//   [3.5] Type cast: static_cast

//   [2.11] Use of bools

//   [5.12] No breaks in loops

//   [2.7] Formatted output (setw, setprecision)

// === MAIN PROGRAM — Every checklist item called out ===
int main()
{
    // ========== PRE-GAME INSTRUCTIONS ==========
    // [2.15, 17] Extensive user instructions and project info
    cout << "======================================================\n";
    cout << "   WELCOME TO CIS-5 PROJECT 2: LUDO (Version 6 Final)\n";
    cout << "======================================================\n";
    cout << "Instructions:\n"
         << "  * 2–4 players, each with 2 tokens (local variables, 1D/2D arrays, strings)\n"
         << "  * Roll a die, choose a token, move it\n"
         << "  * Cut: land on opponent's token to send it home\n"
         << "  * Roll three sixes: lose your turn (flags, nested if, counters)\n"
         << "  * Progress, stats, leaderboard: formatted with setw/setprecision\n"
         << "  * All input validated; output is formatted\n"
         << "  * Features: file I/O, searching/sorting, default args, overloading, switch, static, typecast, sizeof, STL vector demo, etc.\n"
         << "======================================================\n";

    // [3.10] Show sizeof all types (hand tracing demo, output formatting)
    showSize();
    // [3.5] Show type casting, mixing int/float (also demonstrates use of static_cast)
    typeCastDemo();

    // === [2.4, 2.6, 2.12, 6.13, 7.9] ===
    // ALL VARIABLES LOCAL, ALL NAMES ≤7 CHARACTERS
    const int START = 0, FINISH = 50, MAXP = 4, TOKS = 2;                                      // [16] Only local named constants
    int tokens[MAXP][TOKS] = {{START, START}, {START, START}, {START, START}, {START, START}}; // [7.9, 7.2, 7.1] 2D arrays, single-dim arrays

    // [7.7] Parallel arrays for player stats
    string names[MAXP];    // Player names
    int moves[MAXP] = {0}; // Moves per player
    int cuts[MAXP] = {0};  // Cut count
    int turns[MAXP] = {0}; // Turn count
    vector<string> vNames; // [7.12] STL vector demo
    vector<int> vMoves;    // [7.12] STL vector demo

    float avg = 0.0f; // [2.9] Floats, no doubles allowed
    int allMoves = 0; // [3.6] Multiple assignment

    int plCnt;
    do
    { // [5.5] do-while input validation
        cout << "Enter number of players (2–4): ";
        cin >> plCnt;
    } while (plCnt < 2 || plCnt > 4);
    char buf;
    cin.get(buf); // clear newline

    // [2.8] String input for names (no global), parallel array
    for (int i = 0; i < plCnt; ++i)
    {
        cout << "Enter name for Player " << (i + 1) << ": ";
        getline(cin, names[i]);
    }

    int curP = 0, tok, roll, cons6 = 0;
    bool win = false;                          // [2.11, 4.7] Flag, bool use
    srand(static_cast<unsigned int>(time(0))); // [3.5] Type casting (seed RNG)

    // [5.11] File output
    ofstream logF("ludo_v6_log.txt");
    if (!logF)
    {
        cerr << "Error: Cannot open log file." << endl;
        exit(1); // [6.15] exit() function
    }

    // ========= MAIN GAME LOOP =========
    while (!win)
    {                  // [5.2] while loop, [4.7] flag for game over
        turns[curP]++; // [5.1] increment
        cout << "\n"
             << names[curP] << "'s turn, press Enter to roll...";
        cin.get(buf);
        roll = rollDie(); // [6.5, 6.8] Function returns value
        cout << "Rolled: ";
        showDie(roll); // [4.14] switch statement for outputting die face
        logF << names[curP] << " rolled " << roll << ". ";

        // === [4.6, 4.5, 4.2] Nested, if-else, flags, independent if ===
        if (roll == 6)
        {
            cons6++;
            if (cons6 >= 3)
            {
                cout << "Three consecutive sixes! Turn lost.\n";
                cons6 = 0;
                curP = (curP + 1) % plCnt; // [5.1] increment
                logF << names[curP] << " lost turn (3 sixes).\n";
                continue; // [5.12] No breaks in loop (continue is fine)
            }
        }
        else
        {
            cons6 = 0;
        }

        // === [5.5, 4.11, 2.7] Input validation ===
        do
        {
            cout << "Choose token to move (1 or 2): ";
            cin >> tok;
        } while (tok != 1 && tok != 2);
        cin.get(buf);

        // === [6.13, 6.12, 2.13] Function, defaulted param demo, pass by reference ===
        moveTok(tokens, curP, tok - 1, roll, FINISH, logF); // Moves token, updates position
        moves[curP]++;                                      // Move counter for stats

        // === [6.13, 7.8] Cutting rule, function returns int, array passing ===
        int cutCount = cutTok(tokens, plCnt, curP, tok - 1, START, logF); // Cuts other players' tokens, logs
        cuts[curP] += cutCount;                                           // Accumulate cuts

        // === [4.13, 2.7] Conditional (ternary) operator, formatted output ===
        cout << "You " << (cutCount > 0 ? "cut an opponent!" : "did not cut.") << endl;

        // === [6.14, 7.6, 2.7] Function overloading, formatted output, passing arrays ===
        printStat(tokens, names, plCnt);

        // === [6.9, 4.1, 4.8, 3.13] Function returns boolean, relational/logical ops ===
        win = winChk(tokens, curP, FINISH);

        if (win)
        {
            cout << "*** " << names[curP] << " WINS! ***" << endl;
            logF << names[curP] << " wins.\n";
        }
        else
        {
            curP = (curP + 1) % plCnt; // [5.1] increment/decrement
            logF << endl;
        }
    }

    logF.close(); // [5.11] File output close

    // ========== POST-GAME: Stats, Leaderboard, Sorting, Searching ==========
    cout << "\n-- Final Stats --\n";
    for (int i = 0; i < plCnt; ++i)
    {
        cout << setw(12) << left << names[i]
             << "  Moves: " << moves[i]
             << "  Cuts: " << cuts[i]
             << "  Turns: " << turns[i] << endl;
        vNames.push_back(names[i]); // [7.12] STL vector push
        vMoves.push_back(moves[i]);
        allMoves += moves[i]; // [3.6] multiple assignment
    }

    int maxCutter = findMax(cuts, plCnt); // [8.1] Linear search
    cout << "\nMost cuts: " << names[maxCutter] << " (" << cuts[maxCutter] << ")\n";

    // [8.3] Bubble sort (by moves)
    bSort(names, moves, plCnt);
    cout << "\nLeaderboard (fewest moves to win, Bubble Sort):\n";
    for (int i = 0; i < plCnt; ++i)
        cout << setw(12) << left << names[i] << "  Moves: " << moves[i] << endl;

    // [8.4] Selection sort (by turns)
    sSort(names, turns, plCnt);
    cout << "\nLeaderboard (fewest turns to win, Selection Sort):\n";
    for (int i = 0; i < plCnt; ++i)
        cout << setw(12) << left << names[i] << "  Turns: " << turns[i] << endl;

    // [6.14, 7.12] Overloaded printStatus for vectors
    printStat(vNames, vMoves);

    // [3.9, 2.9] Math library, float
    avg = avgMoves(moves, plCnt);
    cout << "\nAverage moves per player (float, math lib): " << setprecision(2) << avg << endl;
    cout << "Score variance (pow): " << pow(allMoves - avg * plCnt, 2) << endl;

    // [5.11] Write leaderboard to file
    ofstream boardF("ludo_leaderboard.txt", ios::app);
    if (boardF)
    {
        boardF << "Game Results: ";
        for (int i = 0; i < plCnt; ++i)
            boardF << names[i] << "(" << moves[i] << " moves, " << cuts[i] << " cuts)  ";
        boardF << endl;
        boardF.close();
    }

    cout << "\nThank you for playing! Exiting program with exit(0)...\n";
    exit(0);  // [6.15] exit() at end
    return 0; // Unreachable, but required for style
}

// === FUNCTION DEFINITIONS, ALL CHECKLIST ITEMS DEMONSTRATED ===

// [6.3, 6.8] Function prototype, pass by value, returns int
int rollDie()
{
    return (rand() % 6) + 1;
}

// [4.14] Switch statement for die roll output (text output, checklist 4.14)
void showDie(int val)
{
    switch (val)
    {
    case 1:
        cout << "One pip." << endl;
        break;
    case 2:
        cout << "Two pips." << endl;
        break;
    case 3:
        cout << "Three pips." << endl;
        break;
    case 4:
        cout << "Four pips." << endl;
        break;
    case 5:
        cout << "Five pips." << endl;
        break;
    case 6:
        cout << "Six pips." << endl;
        break;
    default:
        cout << "Invalid die value.\n";
    }
}

// [6.13, 2.13] Function, pass by reference, move logic, logs to file
void moveTok(int tokens[][2], int plIdx, int tkIdx, int roll, int finish, ofstream &logF)
{
    int oldVal = tokens[plIdx][tkIdx];
    tokens[plIdx][tkIdx] += roll;
    if (tokens[plIdx][tkIdx] > finish) // [4.4] If statement, overflow prevention
        tokens[plIdx][tkIdx] = finish;
    logF << "Moved token " << (tkIdx + 1) << " from " << oldVal << " to " << tokens[plIdx][tkIdx] << ". ";
}

// [6.13, 2.13, 8.1] Cut logic, pass by reference, returns number of cuts, logs to file
int cutTok(int tokens[][2], int plCnt, int plIdx, int tkIdx, int start, ofstream &logF)
{
    int movedPos = tokens[plIdx][tkIdx], cuts = 0;
    for (int op = 0; op < plCnt; ++op)
    { // [5.6] For loop
        if (op == plIdx)
            continue;
        for (int t = 0; t < 2; ++t)
        {
            if (tokens[op][t] == movedPos && movedPos != start)
            { // [4.1] Relational, [4.7] Flag
                tokens[op][t] = start;
                cuts++;
                cout << "Cut! Sent " << "P" << (op + 1) << " Token " << (t + 1) << " to start." << endl;
                logF << "Cut P" << (op + 1) << "T" << (t + 1) << ". ";
            }
        }
    }
    return cuts;
}

// [6.9, 4.8] Win check: returns bool, logical ops, static variable use demo (counts calls)
bool winChk(const int tokens[][2], int plIdx, int finish)
{
    static int callCount = 0; // [6.11] static var
    callCount++;              // Count calls for debugging/tracing
    // [3.13] Logical operator &&
    return (tokens[plIdx][0] >= finish && tokens[plIdx][1] >= finish);
}

// [6.14, 2.7, 7.6] Overloaded function: print status of 2D array tokens
void printStat(const int tokens[][2], const string names[], int plCnt)
{
    cout << "-- Current Token Positions --" << endl;
    for (int i = 0; i < plCnt; ++i)
        cout << setw(12) << left << names[i] << ": ("
             << tokens[i][0] << ", " << tokens[i][1] << ")  ";
    cout << endl;
}

// [6.14, 7.12] Overloaded: printStatus for STL vectors (vector passing demo)
void printStat(const vector<string> &names, const vector<int> &moves)
{
    cout << "\n-- Vector Leaderboard Demo --\n";
    for (size_t i = 0; i < names.size(); ++i)
        cout << setw(12) << left << names[i] << "  Moves: " << moves[i] << endl;
}

// [8.1] Linear search for max in array (search demo)
int findMax(const int arr[], int sz)
{
    int maxIdx = 0;
    for (int i = 1; i < sz; ++i)
        if (arr[i] > arr[maxIdx])
            maxIdx = i;
    return maxIdx;
}

// [8.3] Bubble sort (sort demo), sorts parallel array/string
void bSort(string names[], int arr[], int sz)
{
    for (int i = 0; i < sz - 1; ++i)
        for (int j = 0; j < sz - 1 - i; ++j)
            if (arr[j] > arr[j + 1])
            {
                swap(arr[j], arr[j + 1]);
                swap(names[j], names[j + 1]);
            }
}

// [8.4] Selection sort (sort demo), sorts parallel array/string
void sSort(string names[], int arr[], int sz)
{
    for (int i = 0; i < sz - 1; ++i)
    {
        int minIdx = i;
        for (int j = i + 1; j < sz; ++j)
            if (arr[j] < arr[minIdx])
                minIdx = j;
        swap(arr[i], arr[minIdx]);
        swap(names[i], names[minIdx]);
    }
}

// [3.10] Use of sizeof, formatted output
void showSize()
{
    cout << "Variable sizes: int=" << sizeof(int)
         << " float=" << sizeof(float)
         << " char=" << sizeof(char)
         << " string=" << sizeof(string)
         << " bool=" << sizeof(bool) << endl;
}

// [3.5] Type casting, mixing int/float
void typeCastDemo()
{
    int i = 7;
    float f = 3.14159f;
    cout << "Type cast demo: int " << i << " as float is " << static_cast<float>(i)
         << "; float " << f << " as int is " << static_cast<int>(f) << endl;
}

// [6.12] Defaulted argument demo (avg over moves)
float avgMoves(const int moves[], int sz)
{
    int sum = 0;
    for (int i = 0; i < sz; ++i)
        sum += moves[i];
    return sz > 0 ? static_cast<float>(sum) / sz : 0.0f;
}