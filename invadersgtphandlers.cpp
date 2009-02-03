/*
 * invadersgtphandlers.cpp
 *
 *  Created on: Jan 25, 2009
 *      Author: mikosz
 */

#include <invadersgtphandlers.hpp>

#include <cassert>
#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>

namespace
{
const Position NULL_POS = Position(MAX_SIZE_T, MAX_SIZE_T);

const size_t MAX_BEST_MOVES = 10;

const size_t MAX_BEST_PLACES = 10;

struct Move
{
    Move() :
        pawn(0)
    {
    }

    Move(Position move, Position block, Pawn* pawn) :
        move(move), block(block), pawn(pawn)
    {
    }

    Position move, block;
    Pawn* pawn;
};

template<class First, class Second>
struct SecondOfPairGreaterComp
{
    bool operator()(const std::pair<First, Second>& lhs, const std::pair<First, Second>& rhs) const
    {
        return rhs.second < lhs.second;
    }
};

std::pair<Move, StateValue> bestMove(std::vector<Field>& board, std::map<char, Pawn>& pawns,
        std::map<char, Pawn>& opponentPawns, std::vector<size_t>& proximity, std::vector<size_t>& opponentProximity,
        Field::Type pawnType, Field::Type opponentPawnType, unsigned int maxDepth, bool checkAllSeparated)
{
    typedef std::vector<std::pair<Move, StateValue> > MovesContainer;
    MovesContainer moves;
    bool allSeparated = true;

    std::map<char, Pawn>::iterator pawnIt, pawnsEnd = pawns.end();
    for(pawnIt = pawns.begin(); pawnIt != pawnsEnd; ++pawnIt)
    {
        if(checkAllSeparated && opponentProximity[pawnIt->second.pos] == MAX_SIZE_T)
            continue;
        else
            allSeparated = false;

        for(Position::Iterator moveIt = Position::Iterator(pawnIt->second.pos); !moveIt.atEnd(); ++moveIt)
        {
            if(board[*moveIt].type == Field::FREE)
            {
                Position startingPawnPos = pawnIt->second.pos;
                movePawn(board, pawnIt->second, *moveIt, pawnType);
                for(Position::Iterator blockIt = Position::Iterator(*moveIt); !blockIt.atEnd(); ++blockIt)
                {
                    if(board[*blockIt].type == Field::FREE)
                    {
                        blockPosition(board, *blockIt);

                        calculateProximity(board, pawns, proximity);
                        calculateProximity(board, opponentPawns, opponentProximity);
                        StateValue value = stateValue(board, proximity, opponentProximity);

                        Move move(*moveIt, *blockIt, &pawnIt->second);
                        MovesContainer::iterator place = std::lower_bound(moves.begin(), moves.end(), std::make_pair(
                                move, value), SecondOfPairGreaterComp<Move, StateValue> ());
                        if(static_cast<size_t> (place - moves.begin()) < MAX_BEST_MOVES)
                            moves.insert(place, std::make_pair(move, value));
                        if(moves.size() > MAX_BEST_MOVES)
                            moves.pop_back();

                        unblockPosition(board, *blockIt);
                    }
                }
                movePawn(board, pawnIt->second, startingPawnPos, pawnType);
            }
        }
    }

    if(allSeparated && checkAllSeparated)
    {
        return bestMove(board, pawns, opponentPawns, proximity, opponentProximity, pawnType, opponentPawnType, 0, false);
    }
    else if(moves.size() && maxDepth)
    {
        MovesContainer movesRecalculated;
        MovesContainer::iterator end = moves.end();
        for(MovesContainer::iterator move = moves.begin(); move != end; ++move)
        {
            Position startingPawnPos = move->first.pawn->pos;
            movePawn(board, *move->first.pawn, move->first.move, pawnType);
            blockPosition(board, move->first.block);
            move->second -= bestMove(board, opponentPawns, pawns, opponentProximity, proximity, opponentPawnType,
                    pawnType, maxDepth - 1, true).second;
            MovesContainer::iterator place = std::lower_bound(movesRecalculated.begin(), movesRecalculated.end(),
                    *move, SecondOfPairGreaterComp<Move, StateValue> ());
            movesRecalculated.insert(place, *move);

            unblockPosition(board, move->first.block);
            movePawn(board, *move->first.pawn, startingPawnPos, pawnType);
        }
        moves.swap(movesRecalculated);
    }

    if(moves.size())
    {
        return std::make_pair(moves.front().first, moves.front().second);
    }
    else
    {
        return std::make_pair(Move(Position(), Position(), 0), StateValue());
    }
}

}

char GenplaceGtpHandler::dummyPawnId = '9' < 'z' ? 'z' + 1 : '9' + 1;

SetboardGtpHandler::result_type SetboardGtpHandler::operator()(SetboardGtpHandler::argument_type arguments)
{
    std::string line;

    std::getline(istream_, line);
    size_t boardSize = atoi(line.c_str());

    std::getline(istream_, line);
    size_t pawnsPerPlayer = atoi(line.c_str());
    gameState_.pawnsPerPlayer = pawnsPerPlayer;

    gameState_.init(boardSize + 2, pawnsPerPlayer);

    for(size_t row = 1; row <= boardSize; ++row)
    {
        std::getline(istream_, line);
        for(size_t col = 1; col <= boardSize; ++col)
        {
            char c = line[col - 1];
            if(c == '.')
            {
                gameState_.board[Position(row, col)].type = Field::FREE;
            }
            else if(c == '#')
            {
                gameState_.board[Position(row, col)].type = Field::OBSTACLE;
            }
            else
            {
                throw std::runtime_error(std::string("Invalid field descriptor: \'") + c + "\'");
            }
        }
    }

    return "=";
}

PlaceGtpHandler::result_type PlaceGtpHandler::operator ()(PlaceGtpHandler::argument_type arguments)
{
    Position pos = coordToPos(arguments[2]);

    Field field;
    if(arguments[0] == "al")
    {
        field.type = Field::ALPHA;
        gameState_.alphaPawns.insert(std::make_pair(arguments[1][0], Pawn(arguments[1][0], pos)));
        field.id = arguments[1][0];
        gameState_.board[pos] = field;

        calculateProximity(gameState_.board, gameState_.alphaPawns, gameState_.alphaProximity);
    }
    else
    {
        field.type = Field::NUM;
        gameState_.numPawns.insert(std::make_pair(arguments[1][0], Pawn(arguments[1][0], pos)));
        field.id = arguments[1][0];
        gameState_.board[pos] = field;

        calculateProximity(gameState_.board, gameState_.numPawns, gameState_.numProximity);
    }

    return "=";
}

MoveGtpHandler::result_type MoveGtpHandler::operator ()(MoveGtpHandler::argument_type arguments)
{
    if(arguments[0] == "al")
    {
        Position currentPos = gameState_.alphaPawns[arguments[1][0]].pos;
        Position movePos = moveToPos(currentPos, arguments[2]);
        Position blockPos = moveToPos(movePos, arguments[3]);

        movePawn(gameState_.board, gameState_.alphaPawns[arguments[1][0]], movePos, Field::ALPHA);
        blockPosition(gameState_.board, blockPos);
        calculateProximity(gameState_.board, gameState_.alphaPawns, gameState_.alphaProximity);
    }
    else
    {
        Position currentPos = gameState_.numPawns[arguments[1][0]].pos;
        Position movePos = moveToPos(currentPos, arguments[2]);
        Position blockPos = moveToPos(movePos, arguments[3]);

        movePawn(gameState_.board, gameState_.numPawns[arguments[1][0]], movePos, Field::NUM);
        blockPosition(gameState_.board, blockPos);
        calculateProximity(gameState_.board, gameState_.numPawns, gameState_.numProximity);
    }

    return "=";
}

std::pair<Position, StateValue> GenplaceGtpHandler::bestPlace(std::vector<Field>& board, std::map<char, Pawn>& pawns,
        std::map<char, Pawn>& opponentPawns, std::vector<size_t>& proximity, std::vector<size_t>& opponentProximity,
        Field::Type pawnType, Field::Type opponentPawnType, unsigned int maxDepth)
{
    typedef std::vector<std::pair<Position, StateValue> > PlacesContainer;
    PlacesContainer places;

    for(size_t i = 0; i < board.size(); ++i)
    {
        if(board[i].type == Field::FREE)
        {
            board[i].type = pawnType;
            board[i].id = dummyPawnId;
            bool inserted = pawns.insert(std::make_pair(dummyPawnId, Pawn(dummyPawnId, i))).second;
            ++dummyPawnId;
            assert(inserted);
            calculateProximity(board, pawns, proximity);
            calculateProximity(board, opponentPawns, opponentProximity);
            StateValue value = stateValue(board, proximity, opponentProximity);

            std::pair<Position, StateValue> place(i, value);
            PlacesContainer::iterator placeIt = std::lower_bound(places.begin(), places.end(), place,
                    SecondOfPairGreaterComp<Position, StateValue> ());
            if(static_cast<size_t> (placeIt - places.begin()) < MAX_BEST_PLACES)
                places.insert(placeIt, place);
            if(places.size() > MAX_BEST_PLACES)
                places.pop_back();

            --dummyPawnId;
            size_t erased = pawns.erase(dummyPawnId);
            assert(erased);
            board[i].type = Field::FREE;
        }
    }

    if(maxDepth)
    {
        PlacesContainer placesRecalculated;
        PlacesContainer::iterator end = places.end();
        for(PlacesContainer::iterator place = places.begin(); place != end; ++place)
        {
            board[place->first].type = pawnType;
            board[place->first].id = dummyPawnId;
            bool inserted = pawns.insert(std::make_pair(dummyPawnId, Pawn(dummyPawnId, place->first))).second;
            ++dummyPawnId;
            assert(inserted);

            if(opponentPawns.size() < gameState_.pawnsPerPlayer)
                place->second -= bestPlace(board, opponentPawns, pawns, opponentProximity, proximity, opponentPawnType,
                        pawnType, maxDepth - 1).second;
            else
                place->second -= bestMove(board, opponentPawns, pawns, opponentProximity, proximity, opponentPawnType,
                        pawnType, maxDepth - 1, true).second;

            PlacesContainer::iterator placeIt = std::lower_bound(placesRecalculated.begin(), placesRecalculated.end(),
                    *place, SecondOfPairGreaterComp<Position, StateValue> ());
            placesRecalculated.insert(placeIt, *place);

            --dummyPawnId;
            size_t erased = pawns.erase(dummyPawnId);
            assert(erased);
            board[place->first].type = Field::FREE;
        }
        places.swap(placesRecalculated);
    }

    return places.front();
}

GenplaceGtpHandler::result_type GenplaceGtpHandler::operator ()(GenplaceGtpHandler::argument_type arguments)
{
    GameState testState(gameState_);
    Position bestPos = NULL_POS;

    if(arguments[0] == "al")
    {
        bestPos = bestPlace(testState.board, testState.alphaPawns, testState.numPawns, testState.alphaProximity,
                testState.numProximity, Field::ALPHA, Field::NUM, 1).first;
        if(bestPos != NULL_POS)
        {
            gameState_.board[bestPos].type = Field::ALPHA;
            gameState_.alphaPawns.insert(std::make_pair(arguments[1][0], Pawn(arguments[1][0], bestPos)));
        }
    }
    else
    {
        bestPos = bestPlace(testState.board, testState.numPawns, testState.alphaPawns, testState.numProximity,
                testState.alphaProximity, Field::NUM, Field::ALPHA, 1).first;
        if(bestPos != NULL_POS)
        {
            gameState_.board[bestPos].type = Field::NUM;
            gameState_.numPawns.insert(std::make_pair(arguments[1][0], Pawn(arguments[1][0], bestPos)));
        }
    }
    gameState_.board[bestPos].id = arguments[1][0];

    return "= " + posToCoord(bestPos);
}

GenmoveGtpHandler::result_type GenmoveGtpHandler::operator ()(GenmoveGtpHandler::argument_type arguments)
{
    std::stringstream result;
    result << "= ";

    Move bestMov;
    if(arguments[0] == "al")
    {
        bestMov = bestMove(gameState_.board, gameState_.alphaPawns, gameState_.numPawns, gameState_.alphaProximity,
                gameState_.numProximity, Field::ALPHA, Field::NUM, 2, true).first;
        if(bestMov.pawn != 0)
        {
            result << bestMov.pawn->id << " " << posToMove(bestMov.pawn->pos, bestMov.move) << " " << posToMove(
                    bestMov.move, bestMov.block);
            movePawn(gameState_.board, *(bestMov.pawn), bestMov.move, Field::ALPHA);
            blockPosition(gameState_.board, bestMov.block);
            return result.str();
        }
    }
    else
    {
        bestMov = bestMove(gameState_.board, gameState_.numPawns, gameState_.alphaPawns, gameState_.numProximity,
                gameState_.alphaProximity, Field::NUM, Field::ALPHA, 2, true).first;
        if(bestMov.pawn != 0)
        {
            result << bestMov.pawn->id << " " << posToMove(bestMov.pawn->pos, bestMov.move) << " " << posToMove(
                    bestMov.move, bestMov.block);
            movePawn(gameState_.board, *(bestMov.pawn), bestMov.move, Field::NUM);
            blockPosition(gameState_.board, bestMov.block);
            return result.str();
        }
    }

    result << "pass";
    return result.str();
}
