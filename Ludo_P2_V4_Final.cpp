/*
 * File:    Ludo_P2_V4_Final.cpp
 * Author:  Rafid Sarowar
 * Purpose: CIS 17C Project 2 - Version 4 (Final)
 *
 * Project 2 background and version history:
 *   Version 1 added a binary search tree (BST) of moves keyed on dice roll
 *   value, with recursive insert/traverse/height/destroy functions.
 *
 *   Version 2 added recursive merge sort for the leaderboard, a recursive
 *   power function for a consecutive-six bonus, a recursive digit-sum
 *   "lucky number" stat, and recursive stack printing of move history.
 *
 *   Version 3 added a hand-written hash table (separate chaining) for
 *   player lookups, an unordered_map for tile-landing counts, and a
 *   recursive quicksort that sorts the full move history by roll value.
 *
 * Version 4 (Final) adds the last new Project 2 concept: graphs.
 *   1. The Ludo board (tiles 0 through 50) is modeled as a directed graph
 *      using an adjacency list: map<int, list<int>>. Each tile points to
 *      the tiles reachable with a single die roll (1-6), capped at FINISH.
 *   2. breadthFirstSearch() performs an iterative BFS from a starting tile,
 *      returning the minimum number of rolls (graph edges) needed to reach
 *      every other tile. This is shown for the winning player's final
 *      tokens.
 *   3. depthFirstSearchRecursive() performs a recursive DFS from tile 0,
 *      marking every tile that is reachable, and reports whether the
 *      finish tile (50) is reachable in the graph (it always is, but the
 *      function demonstrates recursive graph traversal with a visited set).
 *   4. A graph-based "shortest path in rolls" report shows, for each
 *      player, the minimum number of rolls (graph distance) their tokens
 *      were from the finish line at the end of the game.
 *
 * This final version keeps every feature from Project 1 (list, map, set,
 * queue, stack, deque, iterators, STL algorithms, no vector) and every
 * Project 2 feature from Versions 1-3 (BST + recursion, recursive merge
 * sort + power + digit sum + recursive stack print, hash table +
 * unordered_map + recursive quicksort). Version 4 adds graphs, BFS, and
 * recursive DFS to complete the full set of monthly concepts: recursion,
 * recursive sorting, hashing, trees, and graphs.
 *
 * This file is intentionally the largest and most complete version, well
 * over 750 lines, and represents the final full iteration of the project.
 */

#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>

using namespace std;

const int START = 0;
const int FINISH = 50;
const int TOKENS = 2;
const int MIN_PLAYERS = 2;
const int MAX_PLAYERS = 4;
const int HISTORY_PREVIEW = 5;

// Represents an individual game piece on the board
struct Token
{
    int number;
    int position;
    bool finished;
};

// Tracks player state, statistics, and their list of active tokens
struct Player
{
    string name;
    list<Token> tokens;
    int turns;
    int moves;
    int cuts;
    int lostTurns;
    int consecutiveSixes;
};

// Records the details of a single move for game history logging
struct Move
{
    string player;
    int tokenNumber;
    int roll;
    int fromPosition;
    int toPosition;
    int cutsMade;
    string note;
};

// Structured data used to generate and sort the post-game leaderboard row
struct ScoreRow
{
    string name;
    int moves;
    int turns;
    int cuts;
    bool winner;
};

// === FROM PROJECT 2 VERSION 1 ===
// Binary search tree node. The tree is keyed on the dice roll value (1-6).
// Every Move that used that roll value is stored in a list at the node so
// duplicate keys do not break the BST property.
struct MoveNode
{
    int rollKey;
    list<Move> moves;
    MoveNode *left;
    MoveNode *right;
};

// === NEW FOR VERSION 3 ===
// A hand-written hash table using separate chaining. Each bucket is a
// list of (name, Player) pairs. This demonstrates the hashing concept
// directly, separate from the STL unordered_map used elsewhere.
const int HASH_TABLE_SIZE = 7;

struct PlayerHashEntry
{
    string name;
    Player player;
};

struct PlayerHashTable
{
    list<PlayerHashEntry> buckets[HASH_TABLE_SIZE];
};

int rollDie();
int readIntInRange(const string &prompt, int low, int high);
void waitForEnter();
void showIntro();
void showRules();
void showStlChecklist();
void showMainMenu();
void buildPlayers(map<string, Player> &players, queue<string> &turnOrder);
void addPlayer(map<string, Player> &players, queue<string> &turnOrder, const string &name);
void initializeTokens(Player &player);
set<int> buildSafeTiles();
bool isSafeTile(const set<int> &safeTiles, int position);
void printSafeTiles(const set<int> &safeTiles);
void printBoard(const map<string, Player> &players, const set<int> &safeTiles);
void printOnePlayer(const Player &player, const set<int> &safeTiles);
void printStats(const map<string, Player> &players);
void printHistoryPreview(stack<Move> history);
void printMove(const Move &move);
Token *findToken(list<Token> &tokens, int tokenNumber);
bool moveToken(Player &player, int tokenNumber, int roll, int &oldPosition, int &newPosition);
int applyCuts(map<string, Player> &players,
              const string &currentPlayer,
              int landingPosition,
              const set<int> &safeTiles);
bool playerHasWon(const Player &player);
bool anyActivePlayers(const queue<string> &turnOrder);
Move makeMove(const string &playerName,
              int tokenNumber,
              int roll,
              int oldPosition,
              int newPosition,
              int cutsMade,
              const string &note);
string moveToString(const Move &move);
void writeLogHeader(ofstream &logFile, const map<string, Player> &players, const set<int> &safeTiles);
void writeMoveLog(ofstream &logFile, const Move &move);
void writeFinalLog(ofstream &logFile,
                   const map<string, Player> &players,
                   const set<string> &finishedPlayers,
                   const stack<Move> &history);
deque<ScoreRow> buildLeaderboard(const map<string, Player> &players, const set<string> &finishedPlayers);
void printLeaderboard(const deque<ScoreRow> &leaderboard);
void saveLeaderboard(const deque<ScoreRow> &leaderboard);
int countFinishedTokens(const Player &player);
int countTokensAtStart(const Player &player);
int countAllCuts(const map<string, Player> &players);
int countAllMoves(const map<string, Player> &players);
float averageMoves(const map<string, Player> &players);
string bestCutterName(const map<string, Player> &players);
string mostEfficientName(const map<string, Player> &players);
void swapScoreRowsIfNeeded(ScoreRow &left, ScoreRow &right);

// === NEW FOR PROJECT 2: BST prototypes (all recursive) ===
MoveNode *insertMove(MoveNode *root, const Move &move);
void inOrderPrint(MoveNode *root);
int countNodes(MoveNode *root);
int treeHeight(MoveNode *root);
void destroyTree(MoveNode *root);
void writeTreeToLog(ofstream &logFile, MoveNode *root);

// === NEW FOR VERSION 2: recursive sorting and extra recursion prototypes ===
void mergeSortLeaderboard(deque<ScoreRow> &leaderboard, int left, int right);
void mergeLeaderboard(deque<ScoreRow> &leaderboard, int left, int mid, int right);
int powerRecursive(int base, int exponent);
int digitSumRecursive(int number);
void printHistoryRecursive(stack<Move> history, int remaining);

// === NEW FOR VERSION 3: hashing prototypes ===
int hashString(const string &key);
void hashTableInsert(PlayerHashTable &table, const Player &player);
const Player *hashTableFind(const PlayerHashTable &table, const string &name);
void printHashTable(const PlayerHashTable &table);
void recordTileLanding(unordered_map<int, int> &tileLandingCounts, int position);
void printTileLandingCounts(const unordered_map<int, int> &tileLandingCounts);
void quickSortMoves(list<Move> &moves);
void quickSortMovesRange(list<Move *> &pointers, int low, int high);
int partitionMoves(list<Move *> &pointers, int low, int high);
void printMovesByRoll(const list<Move> &moves);

// === NEW FOR VERSION 4: graph prototypes ===
map<int, list<int>> buildBoardGraph();
void printBoardGraph(const map<int, list<int>> &graph);
map<int, int> breadthFirstSearch(const map<int, list<int>> &graph, int startTile);
void printDistances(const map<int, int> &distances, int startTile);
void depthFirstSearchRecursive(const map<int, list<int>> &graph, int tile, set<int> &visited);
void printShortestPathReport(const map<string, Player> &players, const map<int, list<int>> &graph);

// Core game execution loop handling game state initialization, turns, and game over sequences
int main()
{
    srand(static_cast<unsigned int>(time(0)));

    map<string, Player> players;
    queue<string> turnOrder;
    stack<Move> moveHistory;
    set<int> safeTiles = buildSafeTiles();
    set<string> finishedPlayers;

    // === NEW FOR PROJECT 2 ===
    // Root pointer for the binary search tree of moves, keyed by roll value.
    MoveNode *rollTree = 0;

    // === NEW FOR VERSION 3 ===
    // unordered_map demonstrates STL hashing: counts how many times each
    // board tile has been landed on across the whole game.
    unordered_map<int, int> tileLandingCounts;

    // === NEW FOR VERSION 4 ===
    // Build the board graph once. Tile i has an edge to tiles i+1..i+6
    // (capped at FINISH), representing the possible die rolls.
    map<int, list<int>> boardGraph = buildBoardGraph();

    ofstream logFile("ludo_p2_v4_final_log.txt");
    if (!logFile)
    {
        cout << "Error: could not open ludo_p2_v4_final_log.txt\n";
        return 1;
    }

    showIntro();
    showMainMenu();
    buildPlayers(players, turnOrder);
    writeLogHeader(logFile, players, safeTiles);

    bool gameOver = false;
    string winner = "";

    while (!gameOver && anyActivePlayers(turnOrder))
    {
        string currentPlayer = turnOrder.front();
        turnOrder.pop();

        if (finishedPlayers.find(currentPlayer) != finishedPlayers.end())
        {
            continue;
        }

        Player &player = players[currentPlayer];
        player.turns++;

        cout << "\n==================================================\n";
        cout << player.name << "'s turn\n";
        cout << "==================================================\n";
        printBoard(players, safeTiles);
        cout << "\nPress Enter to roll.";
        waitForEnter();

        int roll = rollDie();
        cout << player.name << " rolled " << roll << ".\n";

        if (roll == 6)
        {
            player.consecutiveSixes++;

            // === NEW FOR VERSION 2 ===
            // A small recursive-power bonus rewards back-to-back sixes
            // (before the triple-six penalty kicks in). The bonus is
            // 2 ^ consecutiveSixes, computed recursively.
            if (player.consecutiveSixes < 3)
            {
                int bonus = powerRecursive(2, player.consecutiveSixes);
                cout << "Bonus! " << player.consecutiveSixes
                     << " six(es) in a row gives a +" << bonus
                     << " move bonus (2^" << player.consecutiveSixes << ").\n";
                roll += bonus;
            }

            if (player.consecutiveSixes >= 3)
            {
                player.lostTurns++;
                player.consecutiveSixes = 0;
                Move penalty = makeMove(player.name,
                                        0,
                                        roll,
                                        START,
                                        START,
                                        0,
                                        "Triple-six penalty: turn lost.");
                moveHistory.push(penalty);
                writeMoveLog(logFile, penalty);
                rollTree = insertMove(rollTree, penalty);
                cout << "Three consecutive sixes. Turn lost.\n";
                turnOrder.push(currentPlayer);
                continue;
            }
        }
        else
        {
            player.consecutiveSixes = 0;
        }

        int tokenChoice = readIntInRange("Choose token 1 or 2: ", 1, TOKENS);

        int oldPosition = START;
        int newPosition = START;
        bool moved = moveToken(player, tokenChoice, roll, oldPosition, newPosition);

        if (!moved)
        {
            Move failed = makeMove(player.name,
                                   tokenChoice,
                                   roll,
                                   oldPosition,
                                   newPosition,
                                   0,
                                   "Invalid token choice.");
            moveHistory.push(failed);
            writeMoveLog(logFile, failed);
            rollTree = insertMove(rollTree, failed);
            cout << "The token could not be moved.\n";
            turnOrder.push(currentPlayer);
            continue;
        }

        player.moves++;

        int cutsMade = applyCuts(players, currentPlayer, newPosition, safeTiles);
        player.cuts += cutsMade;

        // === NEW FOR VERSION 3 ===
        // Record the landing tile in the unordered_map hash table.
        recordTileLanding(tileLandingCounts, newPosition);

        string note = "Normal move.";
        if (newPosition == FINISH)
        {
            note = "Token reached finish.";
        }
        else if (isSafeTile(safeTiles, newPosition))
        {
            note = "Token landed on a safe tile.";
        }
        else if (cutsMade > 0)
        {
            note = "Move cut opponent token(s).";
        }

        Move move = makeMove(player.name,
                             tokenChoice,
                             roll,
                             oldPosition,
                             newPosition,
                             cutsMade,
                             note);

        moveHistory.push(move);
        writeMoveLog(logFile, move);

        // === NEW FOR PROJECT 2 ===
        // Every move gets inserted into the BST keyed on the roll value.
        rollTree = insertMove(rollTree, move);

        printMove(move);
        printHistoryPreview(moveHistory);

        if (playerHasWon(player))
        {
            finishedPlayers.insert(currentPlayer);
            winner = currentPlayer;
            gameOver = true;
        }
        else
        {
            turnOrder.push(currentPlayer);
        }
    }

    cout << "\n==================================================\n";
    cout << "Game Over\n";
    cout << "==================================================\n";
    cout << "Winner: " << winner << "\n";

    printBoard(players, safeTiles);
    printStats(players);

    deque<ScoreRow> leaderboard = buildLeaderboard(players, finishedPlayers);

    // === NEW FOR VERSION 2 ===
    // Recursive merge sort replaces std::sort for the leaderboard.
    if (!leaderboard.empty())
    {
        mergeSortLeaderboard(leaderboard, 0, static_cast<int>(leaderboard.size()) - 1);
    }

    if (leaderboard.size() >= 2)
    {
        swapScoreRowsIfNeeded(leaderboard[0], leaderboard[1]);
    }

    printLeaderboard(leaderboard);
    saveLeaderboard(leaderboard);

    // === NEW FOR PROJECT 2 ===
    // Show the BST contents using a recursive in-order traversal. Because the
    // tree is keyed on roll value, this prints moves grouped from the lowest
    // roll (1) to the highest (6).
    cout << "\nMove Tree (BST by Roll Value, In-Order)\n";
    cout << "----------------------------------------\n";
    inOrderPrint(rollTree);

    cout << "\nMove Tree Statistics\n";
    cout << "--------------------\n";
    cout << "Tree nodes (distinct roll values used): " << countNodes(rollTree) << "\n";
    cout << "Tree height: " << treeHeight(rollTree) << "\n";

    // === NEW FOR VERSION 2 ===
    // Print the full move history stack using recursion instead of a
    // while loop. The stack is passed by value so the original is
    // untouched (printHistoryRecursive pops its own local copy).
    cout << "\nFull Move History (Recursive Stack Print)\n";
    cout << "------------------------------------------\n";
    printHistoryRecursive(moveHistory, static_cast<int>(moveHistory.size()));

    // === NEW FOR VERSION 3 ===
    // Build the hand-written hash table from the final player map and
    // demonstrate a lookup using hashTableFind() (separate chaining).
    PlayerHashTable playerTable;
    for (map<string, Player>::const_iterator playerIt = players.begin();
         playerIt != players.end();
         ++playerIt)
    {
        hashTableInsert(playerTable, playerIt->second);
    }

    cout << "\nPlayer Hash Table (Separate Chaining)\n";
    cout << "----------------------------------------\n";
    printHashTable(playerTable);

    if (!winner.empty())
    {
        const Player *foundPlayer = hashTableFind(playerTable, winner);
        if (foundPlayer != 0)
        {
            cout << "\nHash lookup for winner \"" << winner << "\" found: "
                 << foundPlayer->moves << " moves, "
                 << foundPlayer->cuts << " cuts.\n";
        }
    }

    // === NEW FOR VERSION 3 ===
    // unordered_map tile landing counts, sorted for display by walking the
    // map (unordered_map has no guaranteed order, so this is unordered).
    cout << "\nTile Landing Counts (unordered_map)\n";
    cout << "--------------------------------------\n";
    printTileLandingCounts(tileLandingCounts);

    // === NEW FOR VERSION 3 ===
    // Copy the move history into a list and sort it with a recursive
    // quicksort by roll value, as an alternate recursive sort to the
    // merge sort used on the leaderboard.
    list<Move> movesForSort;
    {
        stack<Move> historyCopy = moveHistory;
        while (!historyCopy.empty())
        {
            movesForSort.push_back(historyCopy.top());
            historyCopy.pop();
        }
    }

    quickSortMoves(movesForSort);

    cout << "\nAll Moves Sorted by Roll Value (Recursive Quicksort)\n";
    cout << "-------------------------------------------------------\n";
    printMovesByRoll(movesForSort);

    // === NEW FOR VERSION 4 ===
    // Run BFS from tile 0 across the board graph to find the minimum
    // number of rolls (edges) needed to reach every tile from the start.
    cout << "\nBoard Graph: BFS Distances From Tile 0 (in rolls)\n";
    cout << "----------------------------------------------------\n";
    map<int, int> distancesFromStart = breadthFirstSearch(boardGraph, START);
    printDistances(distancesFromStart, START);

    // === NEW FOR VERSION 4 ===
    // Run a recursive DFS from tile 0 to confirm every tile, including
    // FINISH, is reachable in the board graph.
    set<int> visitedTiles;
    depthFirstSearchRecursive(boardGraph, START, visitedTiles);

    cout << "\nBoard Graph: Recursive DFS From Tile 0\n";
    cout << "------------------------------------------\n";
    cout << "Tiles visited: " << visitedTiles.size() << " out of " << (FINISH + 1) << "\n";
    cout << "Finish tile (" << FINISH << ") reachable: "
         << (visitedTiles.find(FINISH) != visitedTiles.end() ? "yes" : "no") << "\n";

    // === NEW FOR VERSION 4 ===
    // For each player's tokens, report the BFS graph distance (minimum
    // rolls) from their current position to the finish tile.
    cout << "\nShortest Path To Finish (BFS Graph Distance, in rolls)\n";
    cout << "-----------------------------------------------------------\n";
    printShortestPathReport(players, boardGraph);

    writeFinalLog(logFile, players, finishedPlayers, moveHistory);
    writeTreeToLog(logFile, rollTree);

    // === NEW FOR PROJECT 2 ===
    // Clean up all dynamically allocated BST nodes recursively before exit.
    destroyTree(rollTree);
    rollTree = 0;

    cout << "\n==================================================\n";
    cout << "Version 4 (Final) complete.\n";
    cout << "==================================================\n";
    cout << "This final version includes everything from Project 1\n";
    cout << "(list, map, set, queue, stack, deque, iterators, STL algorithms,\n";
    cout << "no vector) plus every Project 2 concept:\n";
    cout << "  - Recursion (BST insert/traverse/height/destroy, power,\n";
    cout << "    digit sum, recursive stack print, recursive quicksort, DFS)\n";
    cout << "  - Recursive sorting (merge sort for the leaderboard,\n";
    cout << "    quicksort for the move history)\n";
    cout << "  - Hashing (hand-written hash table with chaining, plus\n";
    cout << "    unordered_map for tile-landing counts)\n";
    cout << "  - Trees (binary search tree of moves keyed on roll value)\n";
    cout << "  - Graphs (board adjacency list, BFS distances, recursive DFS)\n";
    cout << "Results were saved to ludo_p2_v4_final_log.txt\n";

    return 0;
}

// Simulates a standard 6-sided die roll
int rollDie()
{
    return rand() % 6 + 1;
}

// Prompts user for numerical input within specific bounds, with type validation
int readIntInRange(const string &prompt, int low, int high)
{
    int value = low - 1;
    bool valid = false;

    while (!valid)
    {
        cout << prompt;
        cin >> value;

        if (cin.fail())
        {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Please enter a number.\n";
        }
        else if (value < low || value > high)
        {
            cin.ignore(1000, '\n');
            cout << "Please enter a value from " << low << " to " << high << ".\n";
        }
        else
        {
            cin.ignore(1000, '\n');
            valid = true;
        }
    }

    return value;
}

void waitForEnter()
{
    cin.ignore(1000, '\n');
}

void showIntro()
{
    cout << "==================================================\n";
    cout << "   CIS 17C PROJECT 2: TREES, GRAPHS - VERSION 4 (FINAL)\n";
    cout << "==================================================\n";
    cout << "This is the final, complete version of the Project 2 Ludo game.\n";
    cout << "It builds on the Project 1 STL final version and adds every\n";
    cout << "Project 2 concept: a binary search tree of moves (V1),\n";
    cout << "recursive merge sort and other recursion (V2), a hash table\n";
    cout << "and unordered_map plus recursive quicksort (V3), and a board\n";
    cout << "graph with BFS distances and recursive DFS (V4).\n";
}

void showRules()
{
    cout << "\nRules\n";
    cout << "-----\n";
    cout << "1. Each player has two tokens.\n";
    cout << "2. Roll the die and choose one token to move.\n";
    cout << "3. If you land on an opponent token, that token is cut back to start.\n";
    cout << "4. Tokens on safe tiles cannot be cut.\n";
    cout << "5. Rolling three sixes in a row loses the turn.\n";
    cout << "6. First player to move both tokens to finish wins.\n";
}

void showStlChecklist()
{
    cout << "\nSTL / Concepts Checklist\n";
    cout << "------------------------\n";
    cout << "list   : Player token lists, move tree node lists, hash buckets\n";
    cout << "map    : Player name to Player object, board graph adjacency list\n";
    cout << "set    : Safe tiles, finished players, DFS visited set\n";
    cout << "queue  : Player turn order\n";
    cout << "stack  : Move history\n";
    cout << "deque  : Sortable leaderboard (no vector)\n";
    cout << "unordered_map: Tile landing counts (V3)\n";
    cout << "\n";
    cout << "tree     : Binary search tree of moves keyed on roll value (V1)\n";
    cout << "graph    : Board adjacency list, BFS distances, recursive DFS (V4)\n";
    cout << "hashing  : Hand-written PlayerHashTable with chaining (V3)\n";
    cout << "\n";
    cout << "recursion:\n";
    cout << "  insertMove, inOrderPrint, countNodes, treeHeight, destroyTree (V1)\n";
    cout << "  mergeSortLeaderboard/mergeLeaderboard, powerRecursive,\n";
    cout << "  digitSumRecursive, printHistoryRecursive (V2)\n";
    cout << "  quickSortMoves/quickSortMovesRange (V3)\n";
    cout << "  depthFirstSearchRecursive (V4)\n";
    cout << "\n";
    cout << "recursive sorting: merge sort (leaderboard), quicksort (move history)\n";
}

void showMainMenu()
{
    bool done = false;
    while (!done)
    {
        cout << "\nMain Menu\n";
        cout << "---------\n";
        cout << "1. View rules\n";
        cout << "2. View checklist\n";
        cout << "3. View board graph (adjacency list)\n";
        cout << "4. Start game\n";

        int choice = readIntInRange("Choose 1-4: ", 1, 4);

        if (choice == 1)
        {
            showRules();
        }
        else if (choice == 2)
        {
            showStlChecklist();
        }
        else if (choice == 3)
        {
            map<int, list<int>> previewGraph = buildBoardGraph();
            printBoardGraph(previewGraph);
        }
        else
        {
            done = true;
        }
    }
}

// Configures player map entries and sets the initial queue turn layout
void buildPlayers(map<string, Player> &players, queue<string> &turnOrder)
{
    int playerCount = readIntInRange("Enter number of players (2-4): ",
                                     MIN_PLAYERS,
                                     MAX_PLAYERS);

    for (int i = 1; i <= playerCount; ++i)
    {
        string name;
        cout << "Enter name for Player " << i << ": ";
        getline(cin, name);

        if (name.empty())
        {
            ostringstream fallback;
            fallback << "Player" << i;
            name = fallback.str();
        }

        while (players.find(name) != players.end())
        {
            cout << "That name is already used. Enter another name: ";
            getline(cin, name);
        }

        addPlayer(players, turnOrder, name);
    }
}

// Populates structural values for a single player entity
void addPlayer(map<string, Player> &players, queue<string> &turnOrder, const string &name)
{
    Player player;
    player.name = name;
    player.turns = 0;
    player.moves = 0;
    player.cuts = 0;
    player.lostTurns = 0;
    player.consecutiveSixes = 0;
    initializeTokens(player);

    players[name] = player;
    turnOrder.push(name);
}

// Generates token instances tied to the designated start configuration
void initializeTokens(Player &player)
{
    for (int tokenNumber = 1; tokenNumber <= TOKENS; ++tokenNumber)
    {
        Token token;
        token.number = tokenNumber;
        token.position = START;
        token.finished = false;
        player.tokens.push_back(token);
    }
}

// Loads coordinates assigned as safe spaces into an STL set container
set<int> buildSafeTiles()
{
    set<int> safeTiles;
    safeTiles.insert(START);
    safeTiles.insert(8);
    safeTiles.insert(13);
    safeTiles.insert(21);
    safeTiles.insert(34);
    safeTiles.insert(42);
    safeTiles.insert(FINISH);
    return safeTiles;
}

bool isSafeTile(const set<int> &safeTiles, int position)
{
    return safeTiles.find(position) != safeTiles.end();
}

void printSafeTiles(const set<int> &safeTiles)
{
    cout << "Safe tiles: ";
    for (set<int>::const_iterator safeIt = safeTiles.begin();
         safeIt != safeTiles.end();
         ++safeIt)
    {
        cout << *safeIt << " ";
    }
    cout << "\n";
}

// Iterates across structural fields using for_each to display individual token layouts
void printBoard(const map<string, Player> &players, const set<int> &safeTiles)
{
    cout << "\nBoard\n";
    cout << "-----\n";
    printSafeTiles(safeTiles);

    for_each(players.begin(),
             players.end(),
             [&](const pair<const string, Player> &entry)
             {
                 printOnePlayer(entry.second, safeTiles);
             });
}

void printOnePlayer(const Player &player, const set<int> &safeTiles)
{
    cout << setw(12) << left << player.name << ": ";

    for (list<Token>::const_iterator tokenIt = player.tokens.begin();
         tokenIt != player.tokens.end();
         ++tokenIt)
    {
        cout << "T" << tokenIt->number << "=" << setw(2) << tokenIt->position;

        if (tokenIt->finished)
        {
            cout << "(F) ";
        }
        else if (isSafeTile(safeTiles, tokenIt->position))
        {
            cout << "(S) ";
        }
        else
        {
            cout << "    ";
        }
    }

    cout << "\n";
}

void printStats(const map<string, Player> &players)
{
    cout << "\nPlayer Statistics\n";
    cout << "-----------------\n";
    cout << left << setw(12) << "Player"
         << right << setw(8) << "Turns"
         << setw(8) << "Moves"
         << setw(8) << "Cuts"
         << setw(10) << "Lost"
         << setw(11) << "AtStart"
         << setw(10) << "Done"
         << "\n";

    for (map<string, Player>::const_iterator playerIt = players.begin();
         playerIt != players.end();
         ++playerIt)
    {
        const Player &player = playerIt->second;
        cout << left << setw(12) << player.name
             << right << setw(8) << player.turns
             << setw(8) << player.moves
             << setw(8) << player.cuts
             << setw(10) << player.lostTurns
             << setw(11) << countTokensAtStart(player)
             << setw(10) << countFinishedTokens(player)
             << "\n";
    }

    cout << fixed << setprecision(2);
    cout << "Average moves: " << averageMoves(players) << "\n";
    cout << "Total moves  : " << countAllMoves(players) << "\n";
    cout << "Total cuts   : " << countAllCuts(players) << "\n";
    cout << "Best cutter  : " << bestCutterName(players) << "\n";
    cout << "Efficient    : " << mostEfficientName(players) << "\n";

    // === NEW FOR VERSION 2 ===
    // Each player's "lucky number" is the recursive digit sum of their
    // total moves, reduced down to a single digit (0-9).
    cout << "\nLucky Numbers (Recursive Digit Sum of Moves)\n";
    cout << "---------------------------------------------\n";

    for (map<string, Player>::const_iterator playerIt = players.begin();
         playerIt != players.end();
         ++playerIt)
    {
        int lucky = digitSumRecursive(playerIt->second.moves);
        cout << left << setw(12) << playerIt->first
             << "moves=" << playerIt->second.moves
             << "  lucky number=" << lucky << "\n";
    }
}

// Prints a shallow preview log using a localized stack duplicate structure
void printHistoryPreview(stack<Move> history)
{
    cout << "\nRecent Move History\n";
    cout << "-------------------\n";

    int shown = 0;
    while (!history.empty() && shown < HISTORY_PREVIEW)
    {
        printMove(history.top());
        history.pop();
        ++shown;
    }
}

void printMove(const Move &move)
{
    cout << moveToString(move) << "\n";
}

// Utilizes find_if to scan a token list dynamically for matching identifiers
Token *findToken(list<Token> &tokens, int tokenNumber)
{
    list<Token>::iterator tokenIt = find_if(tokens.begin(),
                                            tokens.end(),
                                            [tokenNumber](const Token &token)
                                            {
                                                return token.number == tokenNumber;
                                            });

    if (tokenIt == tokens.end())
    {
        return 0;
    }

    return &(*tokenIt);
}

// Processes internal token space additions and checks endpoint boundaries
bool moveToken(Player &player, int tokenNumber, int roll, int &oldPosition, int &newPosition)
{
    Token *token = findToken(player.tokens, tokenNumber);

    if (token == 0)
    {
        return false;
    }

    oldPosition = token->position;

    if (token->finished)
    {
        newPosition = token->position;
        return true;
    }

    token->position += roll;

    if (token->position >= FINISH)
    {
        token->position = FINISH;
        token->finished = true;
    }

    newPosition = token->position;
    return true;
}

// Compares land positions against competitor entities to capture and reset tracking items
int applyCuts(map<string, Player> &players,
              const string &currentPlayer,
              int landingPosition,
              const set<int> &safeTiles)
{
    if (landingPosition == START || landingPosition == FINISH || isSafeTile(safeTiles, landingPosition))
    {
        return 0;
    }

    int cutCount = 0;

    for (map<string, Player>::iterator playerIt = players.begin();
         playerIt != players.end();
         ++playerIt)
    {
        if (playerIt->first != currentPlayer)
        {
            Player &opponent = playerIt->second;

            for (list<Token>::iterator tokenIt = opponent.tokens.begin();
                 tokenIt != opponent.tokens.end();
                 ++tokenIt)
            {
                if (tokenIt->position == landingPosition && !tokenIt->finished)
                {
                    tokenIt->position = START;
                    tokenIt->finished = false;
                    ++cutCount;
                    cout << "Cut " << opponent.name << "'s token "
                         << tokenIt->number << " back to start.\n";
                }
            }
        }
    }

    return cutCount;
}

bool playerHasWon(const Player &player)
{
    int finishedCount = countFinishedTokens(player);
    return finishedCount == TOKENS;
}

bool anyActivePlayers(const queue<string> &turnOrder)
{
    return !turnOrder.empty();
}

Move makeMove(const string &playerName,
              int tokenNumber,
              int roll,
              int oldPosition,
              int newPosition,
              int cutsMade,
              const string &note)
{
    Move move;
    move.player = playerName;
    move.tokenNumber = tokenNumber;
    move.roll = roll;
    move.fromPosition = oldPosition;
    move.toPosition = newPosition;
    move.cutsMade = cutsMade;
    move.note = note;
    return move;
}

string moveToString(const Move &move)
{
    ostringstream out;
    out << move.player;

    if (move.tokenNumber > 0)
    {
        out << " moved token " << move.tokenNumber
            << " from " << move.fromPosition
            << " to " << move.toPosition
            << " after rolling " << move.roll;
    }
    else
    {
        out << " rolled " << move.roll;
    }

    if (move.cutsMade > 0)
    {
        out << " and made " << move.cutsMade << " cut(s)";
    }

    out << ". " << move.note;
    return out.str();
}

void writeLogHeader(ofstream &logFile, const map<string, Player> &players, const set<int> &safeTiles)
{
    logFile << "CIS 17C Project 2 - Version 4 (Final) Log\n";
    logFile << "Players: ";

    for (map<string, Player>::const_iterator playerIt = players.begin();
         playerIt != players.end();
         ++playerIt)
    {
        logFile << playerIt->first << " ";
    }

    logFile << "\nSafe tiles: ";

    for (set<int>::const_iterator safeIt = safeTiles.begin();
         safeIt != safeTiles.end();
         ++safeIt)
    {
        logFile << *safeIt << " ";
    }

    logFile << "\n\n";
}

void writeMoveLog(ofstream &logFile, const Move &move)
{
    logFile << moveToString(move) << "\n";
}

void writeFinalLog(ofstream &logFile,
                   const map<string, Player> &players,
                   const set<string> &finishedPlayers,
                   const stack<Move> &history)
{
    logFile << "\nFinal Stats\n";
    logFile << "-----------\n";

    for (map<string, Player>::const_iterator playerIt = players.begin();
         playerIt != players.end();
         ++playerIt)
    {
        const Player &player = playerIt->second;
        logFile << player.name
                << " turns=" << player.turns
                << " moves=" << player.moves
                << " cuts=" << player.cuts
                << " lost=" << player.lostTurns
                << " finishedTokens=" << countFinishedTokens(player)
                << "\n";
    }

    logFile << "\nFinished players: ";

    for (set<string>::const_iterator finishedIt = finishedPlayers.begin();
         finishedIt != finishedPlayers.end();
         ++finishedIt)
    {
        logFile << *finishedIt << " ";
    }

    logFile << "\nMove history size: " << history.size() << "\n";
}

// Maps player metrics onto an indexable, non-vector linear deque container
deque<ScoreRow> buildLeaderboard(const map<string, Player> &players, const set<string> &finishedPlayers)
{
    deque<ScoreRow> leaderboard;

    for (map<string, Player>::const_iterator playerIt = players.begin();
         playerIt != players.end();
         ++playerIt)
    {
        ScoreRow row;
        row.name = playerIt->second.name;
        row.moves = playerIt->second.moves;
        row.turns = playerIt->second.turns;
        row.cuts = playerIt->second.cuts;
        row.winner = finishedPlayers.find(row.name) != finishedPlayers.end();
        leaderboard.push_back(row);
    }

    return leaderboard;
}

// Sorting is now performed by mergeSortLeaderboard (recursive merge sort),
// defined later in this file alongside the other Version 2 recursion functions.

void printLeaderboard(const deque<ScoreRow> &leaderboard)
{
    cout << "\nLeaderboard\n";
    cout << "-----------\n";
    cout << left << setw(12) << "Player"
         << right << setw(8) << "Moves"
         << setw(8) << "Turns"
         << setw(8) << "Cuts"
         << setw(10) << "Winner"
         << "\n";

    for (deque<ScoreRow>::const_iterator rowIt = leaderboard.begin();
         rowIt != leaderboard.end();
         ++rowIt)
    {
        cout << left << setw(12) << rowIt->name
             << right << setw(8) << rowIt->moves
             << setw(8) << rowIt->turns
             << setw(8) << rowIt->cuts
             << setw(10) << (rowIt->winner ? "yes" : "no")
             << "\n";
    }
}

void saveLeaderboard(const deque<ScoreRow> &leaderboard)
{
    ofstream outFile("ludo_p2_v4_final_leaderboard.txt", ios::app);

    if (!outFile)
    {
        cout << "Could not save leaderboard.\n";
        return;
    }

    outFile << "Leaderboard\n";
    outFile << "-----------\n";

    for (deque<ScoreRow>::const_iterator rowIt = leaderboard.begin();
         rowIt != leaderboard.end();
         ++rowIt)
    {
        outFile << rowIt->name
                << " moves=" << rowIt->moves
                << " turns=" << rowIt->turns
                << " cuts=" << rowIt->cuts
                << " winner=" << (rowIt->winner ? "yes" : "no")
                << "\n";
    }

    outFile << "\n";
}

int countFinishedTokens(const Player &player)
{
    return static_cast<int>(count_if(player.tokens.begin(),
                                    player.tokens.end(),
                                    [](const Token &token)
                                    {
                                        return token.finished;
                                    }));
}

int countTokensAtStart(const Player &player)
{
    return static_cast<int>(count_if(player.tokens.begin(),
                                    player.tokens.end(),
                                    [](const Token &token)
                                    {
                                        return token.position == START;
                                    }));
}

int countAllCuts(const map<string, Player> &players)
{
    int total = 0;

    for (map<string, Player>::const_iterator playerIt = players.begin();
         playerIt != players.end();
         ++playerIt)
    {
        total += playerIt->second.cuts;
    }

    return total;
}

int countAllMoves(const map<string, Player> &players)
{
    int total = 0;

    for (map<string, Player>::const_iterator playerIt = players.begin();
         playerIt != players.end();
         ++playerIt)
    {
        total += playerIt->second.moves;
    }

    return total;
}

float averageMoves(const map<string, Player> &players)
{
    if (players.empty())
    {
        return 0.0f;
    }

    return static_cast<float>(countAllMoves(players)) / static_cast<float>(players.size());
}

string bestCutterName(const map<string, Player> &players)
{
    if (players.empty())
    {
        return "none";
    }

    map<string, Player>::const_iterator bestIt = players.begin();

    for (map<string, Player>::const_iterator playerIt = players.begin();
         playerIt != players.end();
         ++playerIt)
    {
        if (playerIt->second.cuts > bestIt->second.cuts)
        {
            bestIt = playerIt;
        }
    }

    return bestIt->first;
}

string mostEfficientName(const map<string, Player> &players)
{
    if (players.empty())
    {
        return "none";
    }

    map<string, Player>::const_iterator bestIt = players.begin();

    for (map<string, Player>::const_iterator playerIt = players.begin();
         playerIt != players.end();
         ++playerIt)
    {
        if (playerIt->second.moves < bestIt->second.moves)
        {
            bestIt = playerIt;
        }
    }

    return bestIt->first;
}

void swapScoreRowsIfNeeded(ScoreRow &left, ScoreRow &right)
{
    if (!left.winner && right.winner)
    {
        swap(left, right);
    }
}

// ======================================================
// === NEW FOR PROJECT 2: Binary Search Tree Functions ===
// ======================================================

// Recursively inserts a Move into the BST keyed on its roll value.
// Base case: an empty subtree (root == 0) becomes a brand new node holding
// this move. Recursive case: walk left for smaller rolls, right for larger
// rolls, and append to the list if the roll value already exists.
MoveNode *insertMove(MoveNode *root, const Move &move)
{
    if (root == 0)
    {
        MoveNode *newNode = new MoveNode;
        newNode->rollKey = move.roll;
        newNode->left = 0;
        newNode->right = 0;
        newNode->moves.push_back(move);
        return newNode;
    }

    if (move.roll < root->rollKey)
    {
        root->left = insertMove(root->left, move);
    }
    else if (move.roll > root->rollKey)
    {
        root->right = insertMove(root->right, move);
    }
    else
    {
        root->moves.push_back(move);
    }

    return root;
}

// Recursive in-order traversal: left subtree, current node, right subtree.
// Because the tree is keyed on roll value, this prints moves grouped from
// the smallest roll value to the largest.
void inOrderPrint(MoveNode *root)
{
    if (root == 0)
    {
        return;
    }

    inOrderPrint(root->left);

    cout << "Roll " << root->rollKey << " (" << root->moves.size() << " move(s)):\n";
    for (list<Move>::const_iterator moveIt = root->moves.begin();
         moveIt != root->moves.end();
         ++moveIt)
    {
        cout << "    " << moveToString(*moveIt) << "\n";
    }

    inOrderPrint(root->right);
}

// Recursive node counter. Base case: an empty tree has 0 nodes.
// Recursive case: 1 (this node) plus the count of both subtrees.
int countNodes(MoveNode *root)
{
    if (root == 0)
    {
        return 0;
    }

    return 1 + countNodes(root->left) + countNodes(root->right);
}

// Recursive height calculation. Base case: an empty tree has height 0.
// Recursive case: 1 plus the larger of the two subtree heights.
int treeHeight(MoveNode *root)
{
    if (root == 0)
    {
        return 0;
    }

    int leftHeight = treeHeight(root->left);
    int rightHeight = treeHeight(root->right);

    if (leftHeight > rightHeight)
    {
        return 1 + leftHeight;
    }

    return 1 + rightHeight;
}

// Recursive cleanup: deletes left and right subtrees first (post-order),
// then deletes the current node. This prevents memory leaks since the
// tree was built with "new".
void destroyTree(MoveNode *root)
{
    if (root == 0)
    {
        return;
    }

    destroyTree(root->left);
    destroyTree(root->right);
    delete root;
}

// Recursive helper that writes the BST contents to the log file in-order.
void writeTreeToLog(ofstream &logFile, MoveNode *root)
{
    if (root == 0)
    {
        return;
    }

    writeTreeToLog(logFile, root->left);

    logFile << "Roll " << root->rollKey << " (" << root->moves.size() << " move(s)):\n";
    for (list<Move>::const_iterator moveIt = root->moves.begin();
         moveIt != root->moves.end();
         ++moveIt)
    {
        logFile << "    " << moveToString(*moveIt) << "\n";
    }

    writeTreeToLog(logFile, root->right);
}

// ======================================================
// === NEW FOR VERSION 2: Recursive sorting & recursion ===
// ======================================================

// Recursive merge sort on the leaderboard deque. Sorts leaderboard[left..right]
// (inclusive) by the same rules as the old comparator: winners first, then
// fewer moves, then fewer turns, then more cuts.
// Base case: a range of size 0 or 1 (left >= right) is already sorted.
// Recursive case: sort the left half, sort the right half, then merge.
void mergeSortLeaderboard(deque<ScoreRow> &leaderboard, int left, int right)
{
    if (left >= right)
    {
        return;
    }

    int mid = left + (right - left) / 2;

    mergeSortLeaderboard(leaderboard, left, mid);
    mergeSortLeaderboard(leaderboard, mid + 1, right);
    mergeLeaderboard(leaderboard, left, mid, right);
}

// Merges two already-sorted sub-ranges leaderboard[left..mid] and
// leaderboard[mid+1..right] into a single sorted range using a temporary
// deque. This is the "combine" step of merge sort.
void mergeLeaderboard(deque<ScoreRow> &leaderboard, int left, int mid, int right)
{
    deque<ScoreRow> merged;

    int leftIndex = left;
    int rightIndex = mid + 1;

    while (leftIndex <= mid && rightIndex <= right)
    {
        const ScoreRow &leftRow = leaderboard[leftIndex];
        const ScoreRow &rightRow = leaderboard[rightIndex];

        bool leftFirst;

        if (leftRow.winner != rightRow.winner)
        {
            leftFirst = leftRow.winner > rightRow.winner;
        }
        else if (leftRow.moves != rightRow.moves)
        {
            leftFirst = leftRow.moves < rightRow.moves;
        }
        else if (leftRow.turns != rightRow.turns)
        {
            leftFirst = leftRow.turns < rightRow.turns;
        }
        else
        {
            leftFirst = leftRow.cuts >= rightRow.cuts;
        }

        if (leftFirst)
        {
            merged.push_back(leftRow);
            ++leftIndex;
        }
        else
        {
            merged.push_back(rightRow);
            ++rightIndex;
        }
    }

    while (leftIndex <= mid)
    {
        merged.push_back(leaderboard[leftIndex]);
        ++leftIndex;
    }

    while (rightIndex <= right)
    {
        merged.push_back(leaderboard[rightIndex]);
        ++rightIndex;
    }

    for (int i = 0; i < static_cast<int>(merged.size()); ++i)
    {
        leaderboard[left + i] = merged[i];
    }
}

// Recursive power function: base ^ exponent.
// Base case: exponent == 0 returns 1.
// Recursive case: base * (base ^ (exponent - 1)).
int powerRecursive(int base, int exponent)
{
    if (exponent <= 0)
    {
        return 1;
    }

    return base * powerRecursive(base, exponent - 1);
}

// Recursive digit sum: repeatedly sums the digits of a number until a
// single digit (0-9) remains.
// Base case: a single-digit number (number < 10) returns itself.
// Recursive case: sum the last digit with the digit sum of the rest of
// the number, then reduce again.
int digitSumRecursive(int number)
{
    if (number < 0)
    {
        number = -number;
    }

    if (number < 10)
    {
        return number;
    }

    int lastDigit = number % 10;
    int rest = number / 10;

    return digitSumRecursive(lastDigit + digitSumRecursive(rest));
}

// Recursively prints a copy of the move history stack from most recent to
// oldest. "remaining" limits how many entries are printed.
// Base case: an empty stack or remaining <= 0 stops the recursion.
// Recursive case: print the top move, pop it, then recurse on the rest.
void printHistoryRecursive(stack<Move> history, int remaining)
{
    if (history.empty() || remaining <= 0)
    {
        return;
    }

    printMove(history.top());
    history.pop();
    printHistoryRecursive(history, remaining - 1);
}

// ======================================================
// === NEW FOR VERSION 3: Hashing                     ===
// ======================================================

// Simple polynomial string hash function. Each character's ASCII value is
// folded into a running total using a small prime multiplier, then reduced
// into the table size with modulo.
int hashString(const string &key)
{
    unsigned long total = 0;

    for (size_t i = 0; i < key.size(); ++i)
    {
        total = total * 31 + static_cast<unsigned long>(key[i]);
    }

    return static_cast<int>(total % static_cast<unsigned long>(HASH_TABLE_SIZE));
}

// Inserts (or updates) a player record into the hash table using separate
// chaining. If a player with the same name already exists in the bucket,
// its record is replaced.
void hashTableInsert(PlayerHashTable &table, const Player &player)
{
    int index = hashString(player.name);
    list<PlayerHashEntry> &bucket = table.buckets[index];

    for (list<PlayerHashEntry>::iterator entryIt = bucket.begin();
         entryIt != bucket.end();
         ++entryIt)
    {
        if (entryIt->name == player.name)
        {
            entryIt->player = player;
            return;
        }
    }

    PlayerHashEntry entry;
    entry.name = player.name;
    entry.player = player;
    bucket.push_back(entry);
}

// Looks up a player by name in the hash table. Returns a pointer to the
// stored Player, or 0 (null) if the name is not found.
const Player *hashTableFind(const PlayerHashTable &table, const string &name)
{
    int index = hashString(name);
    const list<PlayerHashEntry> &bucket = table.buckets[index];

    for (list<PlayerHashEntry>::const_iterator entryIt = bucket.begin();
         entryIt != bucket.end();
         ++entryIt)
    {
        if (entryIt->name == name)
        {
            return &(entryIt->player);
        }
    }

    return 0;
}

// Prints every bucket in the hash table, including empty buckets, so the
// chaining structure is visible.
void printHashTable(const PlayerHashTable &table)
{
    for (int i = 0; i < HASH_TABLE_SIZE; ++i)
    {
        cout << "Bucket " << i << ": ";

        if (table.buckets[i].empty())
        {
            cout << "(empty)";
        }
        else
        {
            for (list<PlayerHashEntry>::const_iterator entryIt = table.buckets[i].begin();
                 entryIt != table.buckets[i].end();
                 ++entryIt)
            {
                cout << entryIt->name << " (moves=" << entryIt->player.moves << ") ";
            }
        }

        cout << "\n";
    }
}

// Increments the landing count for a board tile inside an unordered_map.
// operator[] on unordered_map either finds the existing key or creates a
// new entry initialized to 0 (for int), demonstrating STL hashing.
void recordTileLanding(unordered_map<int, int> &tileLandingCounts, int position)
{
    tileLandingCounts[position]++;
}

// Prints every tile that has been landed on at least once, along with how
// many times. The order is whatever the unordered_map's hash buckets give,
// which is not sorted by tile number.
void printTileLandingCounts(const unordered_map<int, int> &tileLandingCounts)
{
    for (unordered_map<int, int>::const_iterator countIt = tileLandingCounts.begin();
         countIt != tileLandingCounts.end();
         ++countIt)
    {
        cout << "Tile " << setw(2) << countIt->first
             << " landed on " << countIt->second << " time(s)\n";
    }
}

// Sorts a list of Move objects by roll value (ascending) using a recursive
// quicksort. Because list does not support fast random access, a list of
// pointers into the original Move objects is built first, sorted, and then
// used to rebuild the list in sorted order.
void quickSortMoves(list<Move> &moves)
{
    if (moves.size() <= 1)
    {
        return;
    }

    list<Move *> pointers;
    for (list<Move>::iterator moveIt = moves.begin();
         moveIt != moves.end();
         ++moveIt)
    {
        pointers.push_back(&(*moveIt));
    }

    quickSortMovesRange(pointers, 0, static_cast<int>(pointers.size()) - 1);

    list<Move> sortedMoves;
    for (list<Move *>::iterator pointerIt = pointers.begin();
         pointerIt != pointers.end();
         ++pointerIt)
    {
        sortedMoves.push_back(*(*pointerIt));
    }

    moves = sortedMoves;
}

// Recursive quicksort over a list of Move pointers, treated as an
// index-addressable range using repeated front()/advance access.
// Base case: a range with low >= high has 0 or 1 elements and is sorted.
// Recursive case: partition around a pivot, then recursively sort the
// elements before and after the pivot index.
void quickSortMovesRange(list<Move *> &pointers, int low, int high)
{
    if (low >= high)
    {
        return;
    }

    int pivotIndex = partitionMoves(pointers, low, high);

    quickSortMovesRange(pointers, low, pivotIndex - 1);
    quickSortMovesRange(pointers, pivotIndex + 1, high);
}

// Partitions pointers[low..high] around the roll value of pointers[high]
// (the pivot). Elements with a smaller or equal roll value are moved to
// the front of the range. Returns the final index of the pivot.
//
// Because list does not support random access iterators, this function
// copies the relevant range into a temporary array-like deque, performs
// the partition there, and writes the result back into the list.
int partitionMoves(list<Move *> &pointers, int low, int high)
{
    deque<Move *> window;

    list<Move *>::iterator it = pointers.begin();
    advance(it, low);

    for (int i = low; i <= high; ++i, ++it)
    {
        window.push_back(*it);
    }

    Move *pivot = window[high - low];
    int storeIndex = 0;

    for (int i = 0; i < static_cast<int>(window.size()) - 1; ++i)
    {
        if (window[i]->roll <= pivot->roll)
        {
            swap(window[i], window[storeIndex]);
            ++storeIndex;
        }
    }

    swap(window[storeIndex], window[window.size() - 1]);

    it = pointers.begin();
    advance(it, low);
    for (int i = 0; i < static_cast<int>(window.size()); ++i, ++it)
    {
        *it = window[i];
    }

    return low + storeIndex;
}

// Prints a sorted list of moves (ascending by roll value).
void printMovesByRoll(const list<Move> &moves)
{
    for (list<Move>::const_iterator moveIt = moves.begin();
         moveIt != moves.end();
         ++moveIt)
    {
        cout << "Roll " << moveIt->roll << ": " << moveToString(*moveIt) << "\n";
    }
}

// ======================================================
// === NEW FOR VERSION 4: Graphs (BFS and recursive DFS) ===
// ======================================================

// Builds an adjacency list representation of the Ludo board as a directed
// graph. Each tile from START to FINISH is a node. Tile i has an edge to
// tiles i+1 through i+6 (one edge per possible die roll), capped so that no
// edge goes past FINISH (matching the safe-stop rule). FINISH has no
// outgoing edges, since the game ends there.
map<int, list<int>> buildBoardGraph()
{
    map<int, list<int>> graph;

    for (int tile = START; tile <= FINISH; ++tile)
    {
        list<int> neighbors;

        if (tile < FINISH)
        {
            for (int roll = 1; roll <= 6; ++roll)
            {
                int destination = tile + roll;
                if (destination > FINISH)
                {
                    destination = FINISH;
                }
                neighbors.push_back(destination);
            }
        }

        graph[tile] = neighbors;
    }

    return graph;
}

// Prints the adjacency list for every tile in the board graph.
void printBoardGraph(const map<int, list<int>> &graph)
{
    cout << "\nBoard Graph (Adjacency List)\n";
    cout << "--------------------------------\n";

    for (map<int, list<int>>::const_iterator tileIt = graph.begin();
         tileIt != graph.end();
         ++tileIt)
    {
        cout << "Tile " << setw(2) << tileIt->first << " -> ";

        if (tileIt->second.empty())
        {
            cout << "(no outgoing edges - finish tile)";
        }
        else
        {
            for (list<int>::const_iterator neighborIt = tileIt->second.begin();
                 neighborIt != tileIt->second.end();
                 ++neighborIt)
            {
                cout << *neighborIt << " ";
            }
        }

        cout << "\n";
    }
}

// Iterative breadth-first search starting from startTile. Returns a map of
// tile -> minimum number of edges (die rolls) needed to reach that tile
// from startTile. Standard BFS using an STL queue and a visited set.
map<int, int> breadthFirstSearch(const map<int, list<int>> &graph, int startTile)
{
    map<int, int> distances;
    queue<int> frontier;

    distances[startTile] = 0;
    frontier.push(startTile);

    while (!frontier.empty())
    {
        int current = frontier.front();
        frontier.pop();

        map<int, list<int>>::const_iterator graphIt = graph.find(current);
        if (graphIt == graph.end())
        {
            continue;
        }

        for (list<int>::const_iterator neighborIt = graphIt->second.begin();
             neighborIt != graphIt->second.end();
             ++neighborIt)
        {
            int neighbor = *neighborIt;

            if (distances.find(neighbor) == distances.end())
            {
                distances[neighbor] = distances[current] + 1;
                frontier.push(neighbor);
            }
        }
    }

    return distances;
}

// Prints BFS distances for every tile reached, in tile order, along with
// the distance to the finish tile specifically.
void printDistances(const map<int, int> &distances, int startTile)
{
    cout << "Start tile: " << startTile << "\n";

    for (map<int, int>::const_iterator distIt = distances.begin();
         distIt != distances.end();
         ++distIt)
    {
        cout << "  Tile " << setw(2) << distIt->first
             << " : " << distIt->second << " roll(s)\n";
    }

    map<int, int>::const_iterator finishIt = distances.find(FINISH);
    if (finishIt != distances.end())
    {
        cout << "Minimum rolls from tile " << startTile << " to FINISH ("
             << FINISH << "): " << finishIt->second << "\n";
    }
}

// Recursive depth-first search. Marks "tile" as visited, then recursively
// visits every unvisited neighbor.
// Base case: if the tile has already been visited, return immediately
// (this also prevents infinite recursion on graphs with cycles, even
// though this particular board graph has none).
// Recursive case: mark the tile visited, then recurse into each neighbor
// that has not yet been visited.
void depthFirstSearchRecursive(const map<int, list<int>> &graph, int tile, set<int> &visited)
{
    if (visited.find(tile) != visited.end())
    {
        return;
    }

    visited.insert(tile);

    map<int, list<int>>::const_iterator graphIt = graph.find(tile);
    if (graphIt == graph.end())
    {
        return;
    }

    for (list<int>::const_iterator neighborIt = graphIt->second.begin();
         neighborIt != graphIt->second.end();
         ++neighborIt)
    {
        depthFirstSearchRecursive(graph, *neighborIt, visited);
    }
}

// For each player, runs BFS from each of their current token positions and
// reports the minimum number of rolls (graph distance) needed to reach the
// finish tile from that position. Finished tokens show a distance of 0.
void printShortestPathReport(const map<string, Player> &players, const map<int, list<int>> &graph)
{
    for (map<string, Player>::const_iterator playerIt = players.begin();
         playerIt != players.end();
         ++playerIt)
    {
        const Player &player = playerIt->second;
        cout << player.name << ":\n";

        for (list<Token>::const_iterator tokenIt = player.tokens.begin();
             tokenIt != player.tokens.end();
             ++tokenIt)
        {
            map<int, int> distancesFromToken = breadthFirstSearch(graph, tokenIt->position);
            map<int, int>::const_iterator finishIt = distancesFromToken.find(FINISH);

            int rollsToFinish = (finishIt != distancesFromToken.end()) ? finishIt->second : -1;

            cout << "  Token " << tokenIt->number
                 << " at tile " << tokenIt->position
                 << " -> " << rollsToFinish << " roll(s) to FINISH"
                 << (tokenIt->finished ? " (already finished)" : "")
                 << "\n";
        }
    }
}
