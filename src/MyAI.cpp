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
        tileMap.insert(std::pair<const Coord, int>(lastMove, number));
    }

    //label
    start:
    
    //
    if (toErase.size()) {
        tileMap.erase(toErase.back());
        toErase.pop_back();
    }
    
    
    //flag frontier
    if (flagFrontier.size()) {
        Coord coord = flagFrontier.back();
        flagFrontier.pop_back();
        //return coordinate we just popped
        return {FLAG, coord.x, coord.y};
    }

    //push the 0 zeroFrontier
    if (zeroFrontier.size()) {
        Coord move = zeroFrontier.back();
        zeroFrontier.pop_back();
        record(move);
        cout << move.x << ", " << move.y << endl;
        return {UNCOVER, move.x, move.y};
    }

    //Check winning condition
    if (flags.size() == totalMines) {
        cout << "FOUND ALL FLAGS, LEFT THE GAME\n";
        return {LEAVE, lastMove.x, lastMove.y};
    }

    /*
    //FIND FLAGS
    //OP strat
    for_each(tileMap.begin(), tileMap.end(), [&](pair<Coord, int> symbol ) { 
        Coord coord = symbol.first;
        int effectiveLabel = symbol.second;

        set<Coord> covered = surroundingCovered(coord, true);

        if (covered.size() == effectiveLabel) {
            
            for (Coord c : covered) {
                if (!flags.count(c)) {
                    addFlag(c);
                    cout << "Added " << c.x << ", " << c.y << " to the flagFrontier!\n";
                }
            }
        }
    });*/

    
    //model checking
    if (!flagFrontier.size()) {
        cout << "MODEL CHECKING: " << endl;

        //for each local area
        for_each(tileMap.begin(), tileMap.end(), [&](pair<Coord, int> symbol ) { 
            Coord coord = symbol.first;
            int effectiveLabel = symbol.second;

            //STEP 0: PRECOMPUTE
            set<Coord> localTiles = surroundingCovered(coord, true);

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

            map<Coord, NumberTile> numberTiles;
            vector<Coord> toPrune;

            //insert middle number tile
            numberTiles.insert(pair<Coord, NumberTile>(coord, NumberTile(effectiveLabel)));

            //insert all other number tiles
            for_each(tileDeltas.begin(), tileDeltas.end(), [&](Coord& delta){
                Coord toInsert = coord + delta;
                if (tileMap.count(toInsert)) {
                    numberTiles.insert(pair<Coord, NumberTile>(toInsert, NumberTile(tileMap.at(toInsert))));
                }
            });

            //set number tile information
            for (pair<const Coord, NumberTile>& p : numberTiles) {
                Coord tileCoord = p.first;
                NumberTile& info = p.second;

                //set surroundingTiles
                info.areaTiles = surroundingCovered(tileCoord, true);
                //cout << info.areaTiles.size() << endl;
                
                //set range
                int nonOverlappingTiles = 0;
                for (const Coord& c : info.areaTiles) {
                    if (!localTiles.count(c)) {
                        nonOverlappingTiles++;
                    }
                }
                //local tiles = 4

                //min = total surrounding minus non-overlap
                info.range.min = info.effectiveValue - nonOverlappingTiles;
                if (info.range.min < 0) {
                    info.range.min = 0;
                }
                //if no influence
                if (info.areaTiles.size() == 0) {
                    //prune this tile in the future
                    toPrune.push_back(tileCoord);
                }
            }
            //remove number tiles that will not influence the possible worlds
            while (toPrune.size()) {
                numberTiles.erase(toPrune.back());
                toPrune.pop_back();
            }
            //no tiles, no info
            if (numberTiles.empty()) {
                return;
            }
            
            //arbitrarily ordered local tiles
            map<Coord, bool> localTilesAndBomb;
            vector<bool*> order; //use pointer to change variable in localTilesAndBomb
            for (Coord c : localTiles) {
                localTilesAndBomb.insert(pair<Coord, bool>(c, false));
                order.push_back(&localTilesAndBomb.at(c));
            }

            vector<map<Coord, bool>> possibleWorlds;

            //STEP 1: PERMUTE
            //use binary representation to go through all possible boolean sets
            for (int permutation = 0; permutation < pow(2, localTiles.size()); permutation++) {
                //cout << "permuation: " << permutation << endl;

                for (int i = 0; i < order.size(); i++) {
                    *order[i] = (permutation >> i) & 1; //change using pointer
                } 

                //STEP 2: Check permutations against numbered tiles
                bool valid = true;
                //for each number tile
                for (pair<Coord, NumberTile> p : numberTiles) { 
                    Coord tileCoord = p.first;
                    NumberTile& info = p.second;

                    //count number of bombs in tiles surrounding number tile
                    int numBombs = 0;
                   
                    //cout << info.areaTiles.size() << endl;
                    for (Coord c : info.areaTiles) {
                        
                        //if tile is in the local area and is a bomb
                        if (localTilesAndBomb.count(c) && localTilesAndBomb.at(c)) {
                            numBombs++;
                        }
                    }
                    
                    //if number of bombs does not obey tile number
                    if (numBombs > info.range.max || numBombs < info.range.min) {
                        //cout << "DISCARDED\n";

                        //world is not a possible world
                        valid = false;
                        //stop checking tiles
                        break;
                    }

                }

                //valid world
                if (valid) {
                    //record it
                    possibleWorlds.push_back(localTilesAndBomb);
                }

            }
            
            //STEP 3: Conclusions
            for (Coord tile : localTiles) { //for each local tile
                //test if the tile is the same in all worlds (e.g. a bomb)
                bool first = true;
                bool bomb = false;
                bool valid = true;

                if (possibleWorlds.empty()) {
                    break;
                }

                for (map<Coord, bool> world : possibleWorlds) {
                    if (first) {
                        bomb = world.at(tile);
                        first = false;
                    }
                    else {
                        if (bomb != world.at(tile)) {
                            valid = false;
                            break;
                        }
                    }
                }
                if (valid) { //all worlds agreed on this tile
                    if (bomb) {
                        if (!flags.count(tile)) {
                            addFlag(tile);
                            cout << "Added " << tile.x << ", " << tile.y << " to the flagFrontier!\n";
                        }
                    }
                    else {
                        if (!moves.count(tile) && !flags.count(tile)) {
                            cout << "Added " << tile.x << ", " << tile.y << " to the zeroFrontier!\n";
                            zeroFrontier.push_back(tile);
                            moves.insert(tile);
                        }
                    }
                }
            }

        });

    }

    if (flagFrontier.size() || zeroFrontier.size()) {
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
                toErase.push_back(coord);
            }

        }
    };

    for_each(tileDeltas.begin(), tileDeltas.end(), [&](Coord& delta){ decrement(coord + delta); });
}

set<MyAI::Coord> MyAI::surroundingCovered(Coord coord, bool flag) {
    set<Coord> covered;

    for_each(tileDeltas.begin(), tileDeltas.end(), [&](Coord& delta){ 
        Coord toTry = coord + delta;
        if (validCoord(toTry) && !moves.count(toTry) && (!flag || !flags.count(toTry))) {
            covered.insert(toTry);
        }

    });

    return covered;
}

void MyAI::addFlag(Coord c) {
    flagFrontier.push_back(c);
    flags.insert(c);
    decrementSurrounding(c);
}

// ======================================================================
// YOUR CODE ENDS
// ======================================================================
