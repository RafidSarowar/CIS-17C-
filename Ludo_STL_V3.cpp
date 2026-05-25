/*
 * File:    Ludo_STL_V3.cpp
 * Author:  Rafid Sarowar
 * Purpose: CIS 17C Project 1 - Version 3
 *
 * Version 3 improvements over Version 2:
 * 1. Adds the Ludo cut rule.
 * 2. Adds stack<string> for move history.
 * 3. Adds file logging.
 * 4. Adds STL algorithms from <algorithm>.
 *
 * STL shown in this version:
 * list, map, set, queue, stack, iterators, find_if, count_if, for_each
 *
 * No vector is used in this file.
 */

#include <algorithm>
#include <ctime>
#include <cstdlib>
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

// Individual tracking structure for token numbers and board locations
struct Token
{
    int number;
    int position;
};

int rollDie();
void showIntro();
void buildPlayers(map<string, list<Token>> &players,
                  map<string, int> &turns,
                  map<string, int> &moves,
                  map<string, int> &cuts,
                  queue<string> &turnOrder);
set<int> buildSafeTiles();
void printBoard(const map<string, list<Token>> &players,
                const map<string, int> &turns,
                const map<string, int> &moves,
                const map<string, int> &cuts);
bool isSafeTile(const set<int> &safeTiles, int position);
bool moveToken(list<Token> &tokens, int tokenNumber, int roll, int &oldPosition, int &newPosition);
int applyCuts(map<string, list<Token>> &players,
              const string &currentPlayer,
              int landingPosition,
              const set<int> &safeTiles,
              ofstream &logFile);
bool hasWon(const list<Token> &tokens);
void printHistoryPreview(stack<string> history);
string makeMoveMessage(const string &name, int tokenNumber, int oldPosition, int newPosition, int roll);

// Main gameplay sequence managing engine states, turn shifts, and file handling
int main()
{
    srand(static_cast<unsigned int>(time(0)));

    map<string, list<Token>> players;
    map<string, int> turns;
    map<string, int> moves;
    map<string, int> cuts;
    queue<string> turnOrder;
    stack<string> moveHistory;
    set<int> safeTiles = buildSafeTiles();

    ofstream logFile("ludo_stl_v3_log.txt");
    if (!logFile)
    {
        cout << "Could not open log file.\n";
        return 1;
    }

    showIntro();
    buildPlayers(players, turns, moves, cuts, turnOrder);

    bool gameOver = false;
    string winner = "";

    while (!gameOver)
    {
        string currentPlayer = turnOrder.front();
        turnOrder.pop();
        turns[currentPlayer]++;

        cout << "\n" << currentPlayer << "'s turn. Press Enter to roll.";
        cin.ignore(1000, '\n');

        int roll = rollDie();
        cout << currentPlayer << " rolled " << roll << ".\n";

        int tokenChoice = 0;
        do
        {
            cout << "Choose token 1 or 2: ";
            cin >> tokenChoice;
        } while (tokenChoice < 1 || tokenChoice > TOKENS);
        cin.ignore(1000, '\n');

        int oldPosition = START;
        int newPosition = START;
        bool moved = moveToken(players[currentPlayer], tokenChoice, roll, oldPosition, newPosition);

        if (moved)
        {
            moves[currentPlayer]++;
            string message = makeMoveMessage(currentPlayer, tokenChoice, oldPosition, newPosition, roll);
            moveHistory.push(message);
            logFile << message << "\n";

            int cutCount = applyCuts(players, currentPlayer, newPosition, safeTiles, logFile);
            cuts[currentPlayer] += cutCount;
            if (cutCount > 0)
            {
                cout << currentPlayer << " cut " << cutCount << " token(s).\n";
            }
        }

        printBoard(players, turns, moves, cuts);
        printHistoryPreview(moveHistory);

        if (hasWon(players[currentPlayer]))
        {
            gameOver = true;
            winner = currentPlayer;
        }
        else
        {
            turnOrder.push(currentPlayer);
        }
    }

    cout << "\nWinner: " << winner << "\n";
    logFile << "Winner: " << winner << "\n";
    cout << "Version 3 complete: added cuts, stack history, logging, and algorithms.\n";
    return 0;
}

// Generates a random number from 1 to 6
int rollDie()
{
    return rand() % 6 + 1;
}

void showIntro()
{
    cout << "============================================\n";
    cout << "      LUDO STL VERSION 3 - CUT RULE\n";
    cout << "============================================\n";
    cout << "This version adds opponent cuts, move history, and file logging.\n";
}

// Prompts for player counts, instantiates token sets, and maps match statistical tables
void buildPlayers(map<string, list<Token>> &players,
                  map<string, int> &turns,
                  map<string, int> &moves,
                  map<string, int> &cuts,
                  queue<string> &turnOrder)
{
    int playerCount = 0;
    do
    {
        cout << "Enter number of players (2-4): ";
        cin >> playerCount;
    } while (playerCount < 2 || playerCount > 4);
    cin.ignore(1000, '\n');

    for (int i = 1; i <= playerCount; ++i)
    {
        string name;
        cout << "Enter name for Player " << i << ": ";
        getline(cin, name);

        list<Token> tokens;
        for (int t = 1; t <= TOKENS; ++t)
        {
            Token token;
            token.number = t;
            token.position = START;
            tokens.push_back(token);
        }

        players[name] = tokens;
        turns[name] = 0;
        moves[name] = 0;
        cuts[name] = 0;
        turnOrder.push(name);
    }
}

// populates safe board positions into a searchable STL set
set<int> buildSafeTiles()
{
    set<int> safeTiles;
    safeTiles.insert(START);
    safeTiles.insert(8);
    safeTiles.insert(13);
    safeTiles.insert(21);
    safeTiles.insert(34);
    safeTiles.insert(FINISH);
    return safeTiles;
}

// Loops over players via std::for_each to print names, locations, and history details
void printBoard(const map<string, list<Token>> &players,
                const map<string, int> &turns,
                const map<string, int> &moves,
                const map<string, int> &cuts)
{
    cout << "\nCurrent Token Positions\n";
    cout << "-----------------------\n";

    for_each(players.begin(), players.end(),
             [&](const pair<const string, list<Token>> &player)
             {
                 cout << setw(12) << left << player.first << ": ";

                 for (list<Token>::const_iterator tokenIt = player.second.begin();
                      tokenIt != player.second.end();
                      ++tokenIt)
                 {
                     cout << "T" << tokenIt->number << "=" << tokenIt->position << " ";
                 }

                 cout << " Turns=" << turns.at(player.first)
                      << " Moves=" << moves.at(player.first)
                      << " Cuts=" << cuts.at(player.first) << "\n";
             });
}

// Validates space properties by executing key search matches on the safe set
bool isSafeTile(const set<int> &safeTiles, int position)
{
    return safeTiles.find(position) != safeTiles.end();
}

// Finds the specified token via std::find_if and updates its location tracking index
bool moveToken(list<Token> &tokens, int tokenNumber, int roll, int &oldPosition, int &newPosition)
{
    list<Token>::iterator tokenIt = find_if(tokens.begin(), tokens.end(),
                                           [tokenNumber](const Token &token)
                                           {
                                               return token.number == tokenNumber;
                                           });

    if (tokenIt == tokens.end())
    {
        return false;
    }

    oldPosition = tokenIt->position;
    tokenIt->position += roll;
    if (tokenIt->position > FINISH)
    {
        tokenIt->position = FINISH;
    }
    newPosition = tokenIt->position;
    return true;
}

// Scans opponent pieces and resets overlapping units to START if they are not on safe spaces
int applyCuts(map<string, list<Token>> &players,
              const string &currentPlayer,
              int landingPosition,
              const set<int> &safeTiles,
              ofstream &logFile)
{
    if (landingPosition == START || isSafeTile(safeTiles, landingPosition))
    {
        return 0;
    }

    int cutCount = 0;

    for (map<string, list<Token>>::iterator playerIt = players.begin();
         playerIt != players.end();
         ++playerIt)
    {
        if (playerIt->first != currentPlayer)
        {
            int matches = static_cast<int>(count_if(playerIt->second.begin(),
                                                    playerIt->second.end(),
                                                    [landingPosition](const Token &token)
                                                    {
                                                        return token.position == landingPosition;
                                                    }));

            if (matches > 0)
            {
                for (list<Token>::iterator tokenIt = playerIt->second.begin();
                     tokenIt != playerIt->second.end();
                     ++tokenIt)
                {
                    if (tokenIt->position == landingPosition)
                    {
                        tokenIt->position = START;
                        ++cutCount;
                        cout << "Cut " << playerIt->first << "'s token "
                             << tokenIt->number << " back to start.\n";
                        logFile << "Cut " << playerIt->first << " token "
                                << tokenIt->number << ".\n";
                    }
                }
            }
        }
    }

    return cutCount;
}

// Tallies finish indicators with std::count_if to check if victory goals are achieved
bool hasWon(const list<Token> &tokens)
{
    int finished = static_cast<int>(count_if(tokens.begin(), tokens.end(),
                                            [](const Token &token)
                                            {
                                                return token.position >= FINISH;
                                            }));
    return finished == TOKENS;
}

// Outputs up to three recent entries by destructively reading a copied value stack
void printHistoryPreview(stack<string> history)
{
    cout << "\nRecent Move History\n";
    cout << "-------------------\n";

    int shown = 0;
    while (!history.empty() && shown < 3)
    {
        cout << history.top() << "\n";
        history.pop();
        ++shown;
    }
}

// Packs parameter info into a clean output description log string
string makeMoveMessage(const string &name, int tokenNumber, int oldPosition, int newPosition, int roll)
{
    ostringstream out;
    out << name << " moved token " << tokenNumber
        << " from " << oldPosition
        << " to " << newPosition
        << " after rolling " << roll << ".";
    return out.str();
}