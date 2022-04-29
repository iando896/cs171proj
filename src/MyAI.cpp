// ======================================================================
// FILE:        MyAI.cpp
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

#include "MyAI.hpp"

MyAI::MyAI ( int _rowDimension, int _colDimension, int _totalMines, int _agentX, int _agentY ) : Agent()
{
    // ======================================================================
    // YOUR CODE BEGINS
    // ======================================================================

    rowDimension = _rowDimension;
    colDimension = _colDimension;
    totalMines = _totalMines;
    agentX = _agentX;
    agentY = _agentY;

    //cout << "BOARD SIZE: " << rowDimension << ", " << colDimension << endl;
    cout << "STARTING MOVE: " << agentX << ", " << agentY << endl;

    lastMove = { agentX, agentY };
    moves.insert(lastMove);

    tileDeltas = { Coord(-1, -1), Coord(0, -1), Coord(1, -1), Coord(-1, 0), Coord(1, 0), Coord(-1, 1), Coord(0, 1), Coord(1, 1) };

    // ======================================================================
    // YOUR CODE ENDS
    // ======================================================================
};

Agent::Action MyAI::getAction( int number )
{
    // ======================================================================
    // YOUR CODE BEGINS
    // ======================================================================
    
 
    //use deltas to loop through flags, change effective value of tile
    for_each(tileDeltas.begin(), tileDeltas.end(), [&](Coord& delta){ 
        if (flags.count(lastMove + delta)) {
            number--;
        }
    });
    
    if (number == 0) {
        addToZeroFrontier(lastMove);
    }
    else {
        //check surrounding tiles for flags
        tileMap.insert(std::pair<Coord, int>(lastMove, number));
    }

    //label
    start:
    
    //flag frontier
    if (flagFrontier.size()) {
        Coord coord = flagFrontier.back();
        flagFrontier.pop_back();
        decrementSurrounding(coord);
        //return coordinate we just popped
        return {FLAG, coord.x, coord.y};
    }

    //Check winning condition
    if (flags.size() == totalMines) {
        cout << "FOUND ALL FLAGS, LEFT THE GAME\n";
        return {LEAVE, lastMove.x, lastMove.y};
    }

    //push the 0 zeroFrontier
    if (zeroFrontier.size()) {
        Coord move = zeroFrontier.back();
        zeroFrontier.pop_back();
        record(move);
        cout << move.x << ", " << move.y << endl;
        return {UNCOVER, move.x, move.y};
    }

    //find flags
    //do a sweep and find flags
    for_each(tileMap.begin(), tileMap.end(), [&](pair<Coord, int> symbol ) { 
        Coord coord = symbol.first;
        int effectiveLabel = symbol.second;

        set<Coord> uncovered = surroundingUncovered(coord);

        //OP strat
        if (uncovered.size() == effectiveLabel) {
            
            for (Coord c : uncovered) {
                if (!flags.count(c)) {
                    flagFrontier.push_back(c);
                    flags.insert(c);
                    cout << "Added " << c.x << ", " << c.y << " to the flagFrontier!\n";
                }
            }
        }

        /*
        vector<pair<Coord, int>> neighbors;
        for_each(tileDeltas.begin(), tileDeltas.end(), [&](Coord& delta){ 

            Coord toTry = coord + delta;
            if (tileMap.count(toTry)) {
                neighbors.push_back(pair<Coord, int>(toTry, tileMap.at(toTry)));
            }
            else if (moves.count(toTry)) {
                neighbors.push_back(pair<Coord, int>(toTry, 0);
            }
        });*/

    });

    //a third thing? fifty fifty CNF

    if (flagFrontier.size()) {
        goto start;
    }
    
    //lose
    cout << "OUT OF MOVES, LEFT THE GAME\n";
    return {LEAVE, lastMove.x, lastMove.y};
    // ======================================================================
    // YOUR CODE ENDS
    // ======================================================================
}


// ======================================================================
// YOUR CODE BEGINS
// ======================================================================
void MyAI::addToZeroFrontier(Coord coord) {

    function<void(Coord)> checkAndAdd = [&](Coord coord){
        if (validCoord(coord) && !moves.count(coord) && !flags.count(coord)) {
            zeroFrontier.push_back(coord);
            moves.insert(coord);
            cout << "Added " << coord.x << ", " << coord.y << " to the zeroFrontier!\n";
        }
    };

    for_each(tileDeltas.begin(), tileDeltas.end(), [&](Coord& delta){ checkAndAdd(coord + delta); });
}

bool MyAI::validCoord(Coord coord) {
    return coord.x >= 0 && coord.y >= 0 
        && coord.x < colDimension && coord.y < rowDimension;
}

void MyAI::record(Coord coord) {
    moves.insert(coord);
    lastMove = coord;
}

void MyAI::decrementSurrounding(Coord coord) {
    function<void(Coord)> decrement = [&](Coord coord){
        if (validCoord(coord) && tileMap.count(coord)) {
            tileMap.at(coord) -= 1;

            if (tileMap.at(coord) == 0) { //add to zeroFrontier
                addToZeroFrontier(coord);
                tileMap.erase(coord);
            }

        }
    };

    for_each(tileDeltas.begin(), tileDeltas.end(), [&](Coord& delta){ decrement(coord + delta); });
}

set<MyAI::Coord> MyAI::surroundingUncovered(Coord coord) {
    set<Coord> uncovered;

    for_each(tileDeltas.begin(), tileDeltas.end(), [&](Coord& delta){ 
        Coord toTry = coord + delta;
        if (validCoord(toTry) && !moves.count(toTry)) {
            uncovered.insert(toTry);
        }

    });

    return uncovered;
}

// ======================================================================
// YOUR CODE ENDS
// ======================================================================
