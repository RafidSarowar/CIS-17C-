/*
 * File:    Ludo_STL_V4_Final.cpp
 * Author:  Rafid Sarowar
 * Purpose: CIS 17C Project 1 - Final STL Ludo Version
 *
 * Project background:
 * This program rebuilds my CIS 5 Ludo game as an STL-based CIS 17C project.
 * The original version used arrays, manual sorting, searching, and a small vector
 * demonstration. This version removes vector completely and redesigns the data
 * storage with STL containers.
 *
 * Game summary:
 * Ludo is a well-known board game where players roll a die and move tokens
 * around a track. In this smaller console version, each player has two tokens.
 * The first player to move both tokens to the finish tile wins.
 *
 * Final version features:
 * 1. 2-4 players
 * 2. Two tokens per player
 * 3. Dice rolling
 * 4. Queue-based turn order
 * 5. Safe tiles
 * 6. Cut rule
 * 7. Triple-six penalty
 * 8. Move history
 * 9. File logging
 * 10. Final statistics
 * 11. Leaderboard
 * 12. STL algorithms
 *
 * Required STL containers demonstrated:
 * list    - each player's tokens and final event labels
 * map     - player name to Player object
 * set     - safe tiles and finished players
 * queue   - turn order
 * stack   - move history
 * deque   - sortable leaderboard without using vector
 *
 * Required iterator use:
 * The program uses iterators to traverse list, map, set, stack copies, and deque.
 *
 * Required STL algorithm categories:
 * Non-mutating:
 * find_if, count_if, for_each, max_element, min_element
 *
 * Mutating:
 * replace, remove_if, swap
 *
 * Organizing/sorting:
 * sort
 *
 * Important class rule:
 * No vector is used in this program.
 */

#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <string>

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
const Token *findTokenConst(const list<Token> &tokens, int tokenNumber);
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
void sortLeaderboard(deque<ScoreRow> &leaderboard);
void printLeaderboard(const deque<ScoreRow> &leaderboard);
void saveLeaderboard(const deque<ScoreRow> &leaderboard);
void runAlgorithmReport(const map<string, Player> &players,
                        const set<string> &finishedPlayers,
                        stack<Move> history);
list<string> buildEventLabels(const map<string, Player> &players,
                              const set<string> &finishedPlayers);
void cleanEventLabels(list<string> &labels);
int countFinishedTokens(const Player &player);
int countTokensAtStart(const Player &player);
int countTokensOnBoard(const Player &player);
int countAllCuts(const map<string, Player> &players);
int countAllMoves(const map<string, Player> &players);
float averageMoves(const map<string, Player> &players);
string bestCutterName(const map<string, Player> &players);
string mostEfficientName(const map<string, Player> &players);
void swapScoreRowsIfNeeded(ScoreRow &left, ScoreRow &right);

// Core game execution loop handling game state initialization, turns, and game over sequences
int main()
{
    srand(static_cast<unsigned int>(time(0)));

    map<string, Player> players;
    queue<string> turnOrder;
    stack<Move> moveHistory;
    set<int> safeTiles = buildSafeTiles();
    set<string> finishedPlayers;

    ofstream logFile("ludo_stl_v4_final_log.txt");
    if (!logFile)
    {
        cout << "Error: could not open ludo_stl_v4_final_log.txt\n";
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
            cout << "The token could not be moved.\n";
            turnOrder.push(currentPlayer);
            continue;
        }

        player.moves++;

        int cutsMade = applyCuts(players, currentPlayer, newPosition, safeTiles);
        player.cuts += cutsMade;

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
    sortLeaderboard(leaderboard);
    printLeaderboard(leaderboard);
    saveLeaderboard(leaderboard);

    runAlgorithmReport(players, finishedPlayers, moveHistory);
    writeFinalLog(logFile, players, finishedPlayers, moveHistory);

    cout << "\nFinal version complete. Results were saved to:\n";
    cout << "  ludo_stl_v4_final_log.txt\n";
    cout << "  ludo_stl_v4_leaderboard.txt\n";

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
    cout << "        CIS 17C PROJECT 1: STL LUDO FINAL\n";
    cout << "==================================================\n";
    cout << "This program transforms my CIS 5 Ludo project into\n";
    cout << "a Standard Template Library based game.\n";
    cout << "No vector is used. The final version uses list, map,\n";
    cout << "set, queue, stack, iterators, and STL algorithms.\n";
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
    cout << "\nSTL Checklist\n";
    cout << "-------------\n";
    cout << "list  : Player token lists and event labels\n";
    cout << "map   : Player name to Player object\n";
    cout << "set   : Safe tiles and finished players\n";
    cout << "queue : Player turn order\n";
    cout << "stack : Move history\n";
    cout << "deque : Sortable leaderboard without vector\n";
    cout << "Algorithms: find_if, count_if, for_each, replace, remove_if, sort\n";
}

void showMainMenu()
{
    bool done = false;
    while (!done)
    {
        cout << "\nMain Menu\n";
        cout << "---------\n";
        cout << "1. View rules\n";
        cout << "2. View STL checklist\n";
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

// Constant variant of token query algorithm using target pointer structures
const Token *findTokenConst(const list<Token> &tokens, int tokenNumber)
{
    list<Token>::const_iterator tokenIt = find_if(tokens.begin(),
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
    logFile << "CIS 17C STL Ludo Final Log\n";
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

// Sorts leaderboard rows by victory condition, efficiency, and aggressive metrics
void sortLeaderboard(deque<ScoreRow> &leaderboard)
{
    sort(leaderboard.begin(),
         leaderboard.end(),
         [](const ScoreRow &left, const ScoreRow &right)
         {
             if (left.winner != right.winner)
             {
                 return left.winner > right.winner;
             }

             if (left.moves != right.moves)
             {
                 return left.moves < right.moves;
             }

             if (left.turns != right.turns)
             {
                 return left.turns < right.turns;
             }

             return left.cuts > right.cuts;
         });

    if (leaderboard.size() >= 2)
    {
        swapScoreRowsIfNeeded(leaderboard[0], leaderboard[1]);
    }
}

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
    ofstream outFile("ludo_stl_v4_leaderboard.txt", ios::app);

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

// Performs complexity metrics validations using max_element and min_element variations
void runAlgorithmReport(const map<string, Player> &players,
                        const set<string> &finishedPlayers,
                        stack<Move> history)
{
    cout << "\nSTL Algorithm Report\n";
    cout << "--------------------\n";

    list<string> labels = buildEventLabels(players, finishedPlayers);
    cleanEventLabels(labels);

    cout << "Clean event labels: ";
    for (list<string>::const_iterator labelIt = labels.begin();
         labelIt != labels.end();
         ++labelIt)
    {
        cout << "[" << *labelIt << "] ";
    }
    cout << "\n";

    deque<ScoreRow> leaderboard = buildLeaderboard(players, finishedPlayers);

    deque<ScoreRow>::iterator maxCutIt = max_element(leaderboard.begin(),
                                                     leaderboard.end(),
                                                     [](const ScoreRow &left, const ScoreRow &right)
                                                     {
                                                         return left.cuts < right.cuts;
                                                     });

    deque<ScoreRow>::iterator minMoveIt = min_element(leaderboard.begin(),
                                                      leaderboard.end(),
                                                      [](const ScoreRow &left, const ScoreRow &right)
                                                      {
                                                          return left.moves < right.moves;
                                                      });

    if (maxCutIt != leaderboard.end())
    {
        cout << "max_element cuts : " << maxCutIt->name << "\n";
    }

    if (minMoveIt != leaderboard.end())
    {
        cout << "min_element moves: " << minMoveIt->name << "\n";
    }

    int cutMoves = 0;
    while (!history.empty())
    {
        if (history.top().cutsMade > 0)
        {
            ++cutMoves;
        }
        history.pop();
    }

    cout << "Move history entries with cuts: " << cutMoves << "\n";
}

// Maps conditional flags into custom dynamic string list elements
list<string> buildEventLabels(const map<string, Player> &players,
                              const set<string> &finishedPlayers)
{
    list<string> labels;

    for (map<string, Player>::const_iterator playerIt = players.begin();
         playerIt != players.end();
         ++playerIt)
    {
        labels.push_back(playerIt->second.moves > 0 ? "played" : "remove-empty");
        labels.push_back(playerIt->second.cuts > 0 ? "cutter" : "replace-none");

        if (finishedPlayers.find(playerIt->first) != finishedPlayers.end())
        {
            labels.push_back("winner");
        }
        else
        {
            labels.push_back("replace-none");
        }
    }

    return labels;
}

// Demonstrates mutation structures utilizing std::replace and std::remove_if pipelines
void cleanEventLabels(list<string> &labels)
{
    replace(labels.begin(), labels.end(), string("replace-none"), string("none"));

    list<string>::iterator newEnd = remove_if(labels.begin(),
                                             labels.end(),
                                             [](const string &label)
                                             {
                                                 return label == "remove-empty";
                                             });

    labels.erase(newEnd, labels.end());
}

// Runs count_if to tally total tokens that have reached the finish tile
int countFinishedTokens(const Player &player)
{
    return static_cast<int>(count_if(player.tokens.begin(),
                                    player.tokens.end(),
                                    [](const Token &token)
                                    {
                                        return token.finished;
                                    }));
}

// Runs count_if to tally total tokens currently sitting at the start tile
int countTokensAtStart(const Player &player)
{
    return static_cast<int>(count_if(player.tokens.begin(),
                                    player.tokens.end(),
                                    [](const Token &token)
                                    {
                                        return token.position == START;
                                    }));
}

// Runs count_if to tally total active board tokens between start and finish
int countTokensOnBoard(const Player &player)
{
    return static_cast<int>(count_if(player.tokens.begin(),
                                    player.tokens.end(),
                                    [](const Token &token)
                                    {
                                        return token.position > START && token.position < FINISH;
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

// Manually swaps element indices using std::swap utility functions
void swapScoreRowsIfNeeded(ScoreRow &left, ScoreRow &right)
{
    if (!left.winner && right.winner)
    {
        swap(left, right);
    }
}