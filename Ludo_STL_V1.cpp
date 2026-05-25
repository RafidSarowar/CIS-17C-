/*
 * File:    Ludo_STL_V1.cpp
 * Author:  Rafid Sarowar
 * Purpose: CIS 17C Project 1 - Version 1
 *
 * Version 1 goal:
 *   Convert the original CIS 5 Ludo idea from arrays into basic STL containers.
 *   This first version keeps the game small on purpose so the improvement path is clear.
 *
 * STL shown in this version:
 *   1. list<int> for each player's token positions
 *   2. map<string, list<int>> for player name -> token list
 *   3. iterators for printing and updating token positions
 *
 * No vector is used in this file.
 */

#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <string>

using namespace std;

const int START = 0;
const int FINISH = 30;
const int TOKENS = 2;

int rollDie();
void showIntro();
void buildPlayers(map<string, list<int>> &players);
void printBoard(const map<string, list<int>> &players);
void moveToken(list<int> &tokens, int tokenNumber, int roll);
bool hasWon(const list<int> &tokens);

int main()
{
    srand(static_cast<unsigned int>(time(0)));

    map<string, list<int>> players;
    showIntro();
    buildPlayers(players);

    bool gameOver = false;
    string winner = "";

    while (!gameOver)
    {
        for (map<string, list<int>>::iterator playerIt = players.begin();
             playerIt != players.end() && !gameOver;
             ++playerIt)
        {
            cout << "\n" << playerIt->first << "'s turn. Press Enter to roll.";
            cin.ignore(1000, '\n');

            int roll = rollDie();
            cout << playerIt->first << " rolled " << roll << ".\n";

            int tokenChoice = 0;
            do
            {
                cout << "Choose token 1 or 2: ";
                cin >> tokenChoice;
            } while (tokenChoice < 1 || tokenChoice > TOKENS);
            cin.ignore(1000, '\n');

            moveToken(playerIt->second, tokenChoice, roll);
            printBoard(players);

            if (hasWon(playerIt->second))
            {
                gameOver = true;
                winner = playerIt->first;
            }
        }
    }

    cout << "\nWinner: " << winner << "\n";
    cout << "Version 1 complete: basic STL Ludo using map, list, and iterators.\n";
    return 0;
}

int rollDie()
{
    return rand() % 6 + 1;
}

void showIntro()
{
    cout << "============================================\n";
    cout << "        LUDO STL VERSION 1 - BASIC\n";
    cout << "============================================\n";
    cout << "This version demonstrates a simple STL rewrite.\n";
    cout << "Each player has two tokens. First player to get both\n";
    cout << "tokens to " << FINISH << " wins.\n";
}

void buildPlayers(map<string, list<int>> &players)
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
    }
}

void printBoard(const map<string, list<int>> &players)
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

        cout << "\n";
    }
}

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
