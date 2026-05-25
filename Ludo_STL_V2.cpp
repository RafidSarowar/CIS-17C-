/*
 * File:    Ludo_STL_V2.cpp
 * Author:  Rafid Sarowar
 * Purpose: CIS 17C Project 1 - Version 2
 *
 * Version 2 improvements over Version 1:
 * 1. Adds queue<string> to manage the turn order.
 * 2. Adds set<int> to store safe Ludo tiles.
 * 3. Adds map<string, int> for turn statistics.
 * 4. Keeps iterator-based traversal.
 *
 * STL shown in this version:
 * list, map, set, queue, iterators
 *
 * No vector is used in this file.
 */

#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <string>

using namespace std;

const int START = 0;
const int FINISH = 40;
const int TOKENS = 2;

int rollDie();
void showIntro();
void buildPlayers(map<string, list<int>> &players,
                  map<string, int> &turns,
                  queue<string> &turnOrder);
set<int> buildSafeTiles();
void printSafeTiles(const set<int> &safeTiles);
void printBoard(const map<string, list<int>> &players,
                const map<string, int> &turns);
void moveToken(list<int> &tokens, int tokenNumber, int roll);
bool isSafeTile(const set<int> &safeTiles, int position);
bool hasWon(const list<int> &tokens);

// Main gameplay loop managing turn order recycling, input pacing, and goal tracking
int main()
{
    srand(static_cast<unsigned int>(time(0)));

    map<string, list<int>> players;
    map<string, int> turns;
    queue<string> turnOrder;
    set<int> safeTiles = buildSafeTiles();

    showIntro();
    printSafeTiles(safeTiles);
    buildPlayers(players, turns, turnOrder);

    bool gameOver = false;
    string winner = "";

    while (!gameOver)
    {
        // Dequeue the next player up and increment their turn counter
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

        moveToken(players[currentPlayer], tokenChoice, roll);

        // Verify if the active token just arrived on a protected board tile
        list<int> &tokens = players[currentPlayer];
        int tokenNumber = 1;
        for (list<int>::iterator tokenIt = tokens.begin(); tokenIt != tokens.end(); ++tokenIt)
        {
            if (tokenNumber == tokenChoice && isSafeTile(safeTiles, *tokenIt))
            {
                cout << "Token " << tokenChoice << " landed on safe tile " << *tokenIt << ".\n";
            }
            ++tokenNumber;
        }

        printBoard(players, turns);

        // Check victory status; recycle the player name to the back of the queue if the game continues
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
    cout << "Version 2 complete: added queue turn order and safe tile set.\n";
    return 0;
}

// Simulates a standard 6-sided die roll
int rollDie()
{
    return rand() % 6 + 1;
}

void showIntro()
{
    cout << "============================================\n";
    cout << "      LUDO STL VERSION 2 - TURN QUEUE\n";
    cout << "============================================\n";
    cout << "This version adds queue-based turns and set-based safe tiles.\n";
}

// Prompts for active participant sizes and constructs nested token lists
void buildPlayers(map<string, list<int>> &players,
                  map<string, int> &turns,
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

        list<int> tokens;
        for (int t = 0; t < TOKENS; ++t)
        {
            tokens.push_back(START);
        }

        players[name] = tokens;
        turns[name] = 0;
        turnOrder.push(name);
    }
}

// Stores custom tile integer targets inside a unique ordered search set
set<int> buildSafeTiles()
{
    set<int> safeTiles;
    safeTiles.insert(START);
    safeTiles.insert(8);
    safeTiles.insert(16);
    safeTiles.insert(24);
    safeTiles.insert(32);
    safeTiles.insert(FINISH);
    return safeTiles;
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

// Traverses player data maps using constant iterators to display positions and turn tallies
void printBoard(const map<string, list<int>> &players,
                const map<string, int> &turns)
{
    cout << "\nCurrent Token Positions\n";
    cout << "-----------------------\n";

    for (map<string, list<int>>::const_iterator playerIt = players.begin();
         playerIt != players.end();
         ++playerIt)
    {
        cout << setw(12) << left << playerIt->first << ": ";

        int tokenNumber = 1;
        for (list<int>::const_iterator tokenIt = playerIt->second.begin();
             tokenIt != playerIt->second.end();
             ++tokenIt)
        {
            cout << "T" << tokenNumber << "=" << *tokenIt << " ";
            ++tokenNumber;
        }

        map<string, int>::const_iterator turnIt = turns.find(playerIt->first);
        if (turnIt != turns.end())
        {
            cout << " Turns=" << turnIt->second;
        }

        cout << "\n";
    }
}

// Increments the target piece index by the roll value and clamps it at the finish mark
void moveToken(list<int> &tokens, int tokenNumber, int roll)
{
    int currentToken = 1;
    for (list<int>::iterator tokenIt = tokens.begin();
         tokenIt != tokens.end();
         ++tokenIt)
    {
        if (currentToken == tokenNumber)
        {
            *tokenIt += roll;
            if (*tokenIt > FINISH)
            {
                *tokenIt = FINISH;
            }
        }

        ++currentToken;
    }
}

// Queries the safe tracking container to verify spot safety parameters
bool isSafeTile(const set<int> &safeTiles, int position)
{
    return safeTiles.find(position) != safeTiles.end();
}

// Loops over constant values to verify if all piece track metrics match the finish cap
bool hasWon(const list<int> &tokens)
{
    for (list<int>::const_iterator tokenIt = tokens.begin();
         tokenIt != tokens.end();
         ++tokenIt)
    {
        if (*tokenIt < FINISH)
        {
            return false;
        }
    }

    return true;
}