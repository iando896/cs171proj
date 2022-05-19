// ======================================================================
// FILE:        MyAI.hpp
//
// AUTHOR:      Jian Li
//
// DESCRIPTION: This file contains your agent class, which you will
//              implement. You are responsible for implementing the
//              'getAction' function and any helper methods you feel you
//              need.
//
// NOTES:       - If you are having trouble understanding how the shell
//                works, look at the other parts of the code, as well as
//                the documentation.
//
//              - You are only allowed to make changes to this portion of
//                the code. Any changes to other portions of the code will
//                be lost when the tournament runs your code.
// ======================================================================

#ifndef MINE_SWEEPER_CPP_SHELL_MYAI_HPP
#define MINE_SWEEPER_CPP_SHELL_MYAI_HPP

#include "Agent.hpp"
#include <iostream> // temporary use
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <functional>

using namespace std;

class MyAI : public Agent
{
public:
    MyAI ( int _rowDimension, int _colDimension, int _totalMines, int _agentX, int _agentY );

    Action getAction ( int number ) override;

    // ======================================================================
    // YOUR CODE BEGINS
    // ======================================================================

    class Coord {
        public:
        Coord() : x(0), y(0) {}
        Coord(int x, int y) : x(x), y(y) {}
        Coord operator+(const Coord& right) const {
            return Coord(x + right.x, y + right.y);
        }
        bool operator<(const Coord& right) const {
            return x < right.x || (x == right.x && y < right.y);
        }
        int x;
        int y;
    };

    class NumberTile {
    public:
        NumberTile() : effectiveValue(0) { range.min = 0; range.max = 0;}
        NumberTile(int x) : NumberTile() { effectiveValue = x; range.max = x; }
        struct Range {
            int min;
            int max;
        };
        int effectiveValue;
        Range range;
        set<Coord> areaTiles;
    };
    
    void addToZeroFrontier(Coord coord);
    bool validCoord(Coord coord); 
    void record(Coord coord);
    void decrementSurrounding(Coord coord);
    set<Coord> surroundingCovered(Coord coord, bool flag = false);
    void addFlag(Coord c);


    /*
    class label() {
    public:
        label(int x) : actual(x), delta(0) {}
        int getEffective() { return actual - delta; }
        int increment() { delta++; }
        int decrement() { delta--; }
    private:
        int actual;
        int delta;
    }*/

private:
    Coord lastMove;

    vector<Coord> zeroFrontier;
    set<Coord> moves;
    map<Coord, int> tileMap;
    set<Coord> flags;
    vector<Coord> flagFrontier;
    vector<Coord> toErase;

    vector<Coord> tileDeltas;

    // ======================================================================
    // YOUR CODE ENDS
    // ======================================================================
};

#endif //MINE_SWEEPER_CPP_SHELL_MYAI_HPP
