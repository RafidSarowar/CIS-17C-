/*
 * File:    Ludo_P2_V3.cpp
 * Author:  Rafid Sarowar
 * Purpose: CIS 17C Project 2 - Version 3
 *
 * Project 2 background:
 * Version 1 added a binary search tree (BST) of moves keyed on dice roll
 * value, with recursive insert/traverse/height/destroy functions.
 * Version 2 added recursive merge sort for the leaderboard, plus a
 * recursive power function, recursive digit sum, and recursive stack
 * printing.
 *
 * Version 3 improvements over Version 2:
 *   1. Hashing: a hand-written hash table (PlayerHashTable) is added that
 *      stores Player records using separate chaining. It demonstrates:
 *        - a simple string hash function (hashString)
 *        - insertion, lookup, and bucket-count display
 *      The hash table is built once per turn loop from the existing
 *      map<string, Player> and used to look up the current player in O(1)
 *      average time, separate from the map lookup.
 *   2. unordered_map<int, int> tileLandingCounts tracks how many times each
 *      board tile has been landed on, demonstrating STL hashing via
 *      unordered_map.
 *   3. A recursive quicksort, quickSortMoves(), sorts a list of Move
 *      objects copied from the move history by roll value (ascending),
 *      shown at game end as an alternate recursive sort to the merge sort
 *      used for the leaderboard.
 *
 * All Version 1 and Version 2 features (BST, recursive merge sort, power,
 * digit sum, recursive stack printing) are kept unchanged.
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

    ofstream logFile("ludo_p2_v3_log.txt");
    if (!logFile)
    {
        cout << "Error: could not open ludo_p2_v3_log.txt\n";
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

    writeFinalLog(logFile, players, finishedPlayers, moveHistory);
    writeTreeToLog(logFile, rollTree);

    // === NEW FOR PROJECT 2 ===
    // Clean up all dynamically allocated BST nodes recursively before exit.
    destroyTree(rollTree);
    rollTree = 0;

    cout << "\nVersion 3 (Project 2) complete: added a hand-written hash table,\n";
    cout << "an unordered_map tile-landing tracker, and a recursive quicksort.\n";
    cout << "Results were saved to ludo_p2_v3_log.txt\n";

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
    cout << "     CIS 17C PROJECT 2: TREES, GRAPHS - VERSION 3\n";
    cout << "==================================================\n";
    cout << "This version builds on the Project 1 final STL Ludo game.\n";
    cout << "New this version: a recursive binary search tree (BST) that\n";
    cout << "stores every move keyed on the dice roll value.\n";
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
    cout << "list  : Player token lists\n";
    cout << "map   : Player name to Player object\n";
    cout << "set   : Safe tiles and finished players\n";
    cout << "queue : Player turn order\n";
    cout << "stack : Move history\n";
    cout << "deque : Sortable leaderboard without vector\n";
    cout << "tree  : Binary search tree of moves keyed on roll value (NEW)\n";
    cout << "recursion: insertMove, inOrderPrint, countNodes, treeHeight, destroyTree (NEW)\n";
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
        cout << "3. Start game\n";

        int choice = readIntInRange("Choose 1-3: ", 1, 3);

        if (choice == 1)
        {
            showRules();
        }
        else if (choice == 2)
        {
            showStlChecklist();
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
    logFile << "CIS 17C Project 2 - Version 3 Log\n";
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
    ofstream outFile("ludo_p2_v3_leaderboard.txt", ios::app);

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
