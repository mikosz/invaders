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

namespace
{
const Position NULL_POS = Position(MAX_SIZE_T, MAX_SIZE_T);
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
                gameState_.board[Position(row, col)].type = GameState::Field::FREE;
            }
            else if(c == '#')
            {
                gameState_.board[Position(row, col)].type = GameState::Field::OBSTACLE;
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

    GameState::Field field;
    if(arguments[0] == "al")
    {
        field.type = GameState::Field::ALPHA;
        gameState_.alphaPawns.insert(std::make_pair(arguments[1][0], GameState::Pawn(arguments[1][0], pos)));
        field.id = arguments[1][0];
        gameState_.board[pos] = field;

        calculateProximity(gameState_.board, gameState_.alphaPawns, gameState_.alphaProximity);
    }
    else
    {
        field.type = GameState::Field::NUM;
        gameState_.numPawns.insert(std::make_pair(arguments[1][0], GameState::Pawn(arguments[1][0], pos)));
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

        movePawn(gameState_.board, gameState_.alphaPawns[arguments[1][0]], movePos, GameState::Field::ALPHA);
        blockPosition(gameState_.board, blockPos);
        calculateProximity(gameState_.board, gameState_.alphaPawns, gameState_.alphaProximity);
    }
    else
    {
        Position currentPos = gameState_.numPawns[arguments[1][0]].pos;
        Position movePos = moveToPos(currentPos, arguments[2]);
        Position blockPos = moveToPos(movePos, arguments[3]);

        movePawn(gameState_.board, gameState_.numPawns[arguments[1][0]], movePos, GameState::Field::NUM);
        blockPosition(gameState_.board, blockPos);
        calculateProximity(gameState_.board, gameState_.numPawns, gameState_.numProximity);
    }

    return "=";
}

std::pair<Position, std::pair<size_t, size_t> > GenplaceGtpHandler::bestPlace(std::vector<GameState::Field>& board,
        std::map<char, GameState::Pawn>& pawns, std::map<char, GameState::Pawn>& opponentPawns,
        std::vector<size_t>& proximity, std::vector<size_t>& opponentProximity, GameState::Field::Type pawnType,
        GameState::Field::Type opponentPawnType)
{
    Position bestPos = NULL_POS;
    std::pair<size_t, size_t> bestPosValue;

    for(size_t i = 0; i < board.size(); ++i)
    {
        if(board[i].type == GameState::Field::FREE)
        {
            board[i].type = pawnType;
            board[i].id = dummyPawnId;
            bool inserted = pawns.insert(std::make_pair(dummyPawnId, GameState::Pawn(dummyPawnId, i))).second;
            ++dummyPawnId;
            assert(inserted);
            calculateProximity(board, pawns, proximity);
            calculateProximity(board, opponentPawns, opponentProximity);
            std::pair<size_t, size_t> value = stateValue(board, proximity, opponentProximity);
            if(bestPos == NULL_POS || value.first > bestPosValue.first || (value.first == bestPosValue.first
                    && value.second < bestPosValue.second))
            {
                bestPos = i;
                bestPosValue = value;
            }
            --dummyPawnId;
            size_t erased = pawns.erase(dummyPawnId);
            assert(erased);
            board[i].type = GameState::Field::FREE;
        }
    }

    return std::make_pair(bestPos, bestPosValue);
}

GenplaceGtpHandler::result_type GenplaceGtpHandler::operator ()(GenplaceGtpHandler::argument_type arguments)
{
    GameState testState(gameState_);
    Position bestPos = NULL_POS;

    if(arguments[0] == "al")
    {
        bestPos = bestPlace(testState.board, testState.alphaPawns, testState.numPawns, testState.alphaProximity,
                testState.numProximity, GameState::Field::ALPHA, GameState::Field::NUM).first;
        if(bestPos != NULL_POS)
        {
            gameState_.board[bestPos].type = GameState::Field::ALPHA;
            gameState_.alphaPawns.insert(std::make_pair(arguments[1][0], GameState::Pawn(arguments[1][0], bestPos)));
        }
    }
    else
    {
        bestPos = bestPlace(testState.board, testState.numPawns, testState.alphaPawns, testState.numProximity,
                testState.alphaProximity, GameState::Field::NUM, GameState::Field::ALPHA).first;
        if(bestPos != NULL_POS)
        {
            gameState_.board[bestPos].type = GameState::Field::NUM;
            gameState_.numPawns.insert(std::make_pair(arguments[1][0], GameState::Pawn(arguments[1][0], bestPos)));
        }
    }
    gameState_.board[bestPos].id = arguments[1][0];

    return "= " + posToCoord(bestPos);
}

std::pair<GameState::Pawn*, std::pair<Position, Position> > GenmoveGtpHandler::bestMove(
        std::vector<GameState::Field>& board, std::map<char, GameState::Pawn>& pawns,
        std::map<char, GameState::Pawn>& opponentPawns, std::vector<size_t>& proximity,
        std::vector<size_t>& opponentProximity, GameState::Field::Type pawnType,
        GameState::Field::Type opponentPawnType)
{
    GameState::Pawn* bestPawn = 0;
    std::pair<Position, Position> bestMove = std::make_pair(NULL_POS, NULL_POS);
    std::pair<size_t, size_t> bestMoveValue;

    std::map<char, GameState::Pawn>::iterator pawnIt, pawnsEnd = pawns.end();
    for(pawnIt = pawns.begin(); pawnIt != pawnsEnd; ++pawnIt)
    {
        for(Position::Iterator moveIt = Position::Iterator(pawnIt->second.pos); !moveIt.atEnd(); ++moveIt)
        {
            if(board[*moveIt].type == GameState::Field::FREE)
            {
                Position startingPawnPos = pawnIt->second.pos;
                movePawn(board, pawnIt->second, *moveIt, pawnType);
                for(Position::Iterator blockIt = Position::Iterator(*moveIt); !blockIt.atEnd(); ++blockIt)
                {
                    if(board[*blockIt].type == GameState::Field::FREE)
                    {
                        blockPosition(board, *blockIt);

                        calculateProximity(board, pawns, proximity);
                        calculateProximity(board, opponentPawns, opponentProximity);
                        std::pair<size_t, size_t> value = stateValue(board, proximity, opponentProximity);

                        if(bestPawn == 0 || bestMoveValue.first < value.first || (bestMoveValue.first == value.first
                                && bestMoveValue.second > value.second))
                        {
                            bestMoveValue = value;
                            bestMove = std::make_pair(*moveIt, *blockIt);
                            bestPawn = &(pawnIt->second);
                        }

                        unblockPosition(board, *blockIt);
                    }
                }
                movePawn(board, pawnIt->second, startingPawnPos, pawnType);
            }
        }
    }

    return std::make_pair(bestPawn, bestMove);
}

GenmoveGtpHandler::result_type GenmoveGtpHandler::operator ()(GenmoveGtpHandler::argument_type arguments)
{
    std::stringstream result;
    result << "= ";

    std::pair<GameState::Pawn*, std::pair<Position, Position> > bestMov;
    if(arguments[0] == "al")
    {
        bestMov = bestMove(gameState_.board, gameState_.alphaPawns, gameState_.numPawns, gameState_.alphaProximity,
                gameState_.numProximity, GameState::Field::ALPHA, GameState::Field::NUM);
        if(bestMov.first != 0)
        {
            result << bestMov.first->id << " " << posToMove(bestMov.first->pos, bestMov.second.first) << " "
                    << posToMove(bestMov.second.first, bestMov.second.second);
            movePawn(gameState_.board, *(bestMov.first), bestMov.second.first, GameState::Field::ALPHA);
            blockPosition(gameState_.board, bestMov.second.second);
            return result.str();
        }
    }
    else
    {
        bestMov = bestMove(gameState_.board, gameState_.numPawns, gameState_.alphaPawns, gameState_.numProximity,
                gameState_.alphaProximity, GameState::Field::NUM, GameState::Field::ALPHA);
        if(bestMov.first != 0)
        {
            result << bestMov.first->id << " " << posToMove(bestMov.first->pos, bestMov.second.first) << " "
                    << posToMove(bestMov.second.first, bestMov.second.second);
            movePawn(gameState_.board, *(bestMov.first), bestMov.second.first, GameState::Field::NUM);
            blockPosition(gameState_.board, bestMov.second.second);
            return result.str();
        }
    }

    result << "pass";
    return result.str();
}
