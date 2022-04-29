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
    if (number == 0) {
        addToFrontier(lastMove);
    }
    else {
        tileMap.insert(std::pair<Coord, int>(lastMove, number));
    }

    //push the 0 frontier
    if (frontier.size()) {
        Coord move = frontier.back();
        frontier.pop_back();
        record(move);
        cout << move.x << ", " << move.y << endl;
        return {UNCOVER, move.x, move.y};
    }

    return {LEAVE, lastMove.x, lastMove.y};
    // ======================================================================
    // YOUR CODE ENDS
    // ======================================================================
}


// ======================================================================
// YOUR CODE BEGINS
// ======================================================================
void MyAI::addToFrontier(Coord coord) {

    function<void(Coord)> checkAndAdd = [&](Coord coord){
        if (validCoord(coord) && !moves.count(coord)) {
            frontier.push_back(coord);
            moves.insert(coord);
            cout << "Added " << coord.x << ", " << coord.y << " to the frontier!\n";
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

            if (tileMap.at(coord) == 0) { //add to frontier
                addToFrontier(coord);
                tileMap.erase(coord);
            }

        }
    };

    for_each(tileDeltas.begin(), tileDeltas.end(), [&](Coord& delta){ decrement(coord + delta); });

}


// ======================================================================
// YOUR CODE ENDS
// ======================================================================
