/*
 * invadersgtphandlers.hpp
 *
 *  Created on: Dec 8, 2008
 *      Author: mikosz
 */

#ifndef INVADERSGTPHANDLERS_HPP_
#define INVADERSGTPHANDLERS_HPP_

#include <gtphandlers.hpp>
#include <gamestate.hpp>
#include <istream>
#include <stdexcept>
#include <string>
#include <cstdlib>
#include <util.hpp>
#include <ctime>
#include <cstdlib>

#include <iostream>

class SetboardGtpHandler: public GtpHandler
{
public:

    SetboardGtpHandler(std::istream& istream, GameState& gameState) :
        istream_(istream), gameState_(gameState)
    {
    }

    virtual result_type operator()(argument_type arguments)
    {
        std::string line;

        std::getline(istream_, line);
        size_t boardSize = atoi(line.c_str());

        std::getline(istream_, line);
        size_t pawnsPerPlayer = atoi(line.c_str());
        gameState_.pawnsPerPlayer = pawnsPerPlayer;

        gameState_.init(boardSize + 2, pawnsPerPlayer);

        for(size_t i = 0; i < boardSize + 2; ++i)
        {
            gameState_.board[i].type = GameState::Field::OBSTACLE;
            gameState_.board[i + ((boardSize + 2) * (boardSize + 1))].type
                    = GameState::Field::OBSTACLE;
        }

        for(size_t row = 1; row <= boardSize; ++row)
        {
            gameState_.board[Position(row, 0)].type = GameState::Field::OBSTACLE;

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

                gameState_.board[Position(row, boardSize + 1)].type = GameState::Field::OBSTACLE;
            }
        }

        return "=";
    }

private:

    std::istream& istream_;

    GameState& gameState_;

};

class PlaceGtpHandler: public GtpHandler
{
public:

    PlaceGtpHandler(GameState& gameState) :
        gameState_(gameState)
    {
    }

    virtual result_type operator ()(argument_type arguments)
    {
        Position pos = coordToPos(arguments[2]);

        GameState::Field field;
        if(arguments[0] == "al")
        {
            field.type = GameState::Field::ALPHA;
            gameState_.alphaPawns.insert(std::make_pair(arguments[1][0], GameState::Pawn(pos)));
            field.id = arguments[1][0];
            gameState_.board[pos] = field;

            gameState_.calculateProximity(gameState_.alphaPawns, gameState_.alphaProximity);
        }
        else
        {
            field.type = GameState::Field::NUM;
            gameState_.numPawns.insert(std::make_pair(arguments[1][0], GameState::Pawn(pos)));
            field.id = arguments[1][0];
            gameState_.board[pos] = field;

            gameState_.calculateProximity(gameState_.numPawns, gameState_.numProximity);
        }

        return "=";
    }

private:

    GameState& gameState_;

};

class MoveGtpHandler: public GtpHandler
{
public:

    MoveGtpHandler(GameState& gameState) :
        gameState_(gameState)
    {
    }

    virtual result_type operator ()(argument_type arguments)
    {
        if(arguments[0] == "al")
        {
            Position currentPos = gameState_.alphaPawns[arguments[1][0]].pos;
            Position movePos = moveToPos(currentPos, arguments[2]);
            Position blockPos = moveToPos(movePos, arguments[3]);

            gameState_.board[movePos].id = arguments[1][0];
            gameState_.board[currentPos].type = GameState::Field::FREE;
            gameState_.board[movePos].type = GameState::Field::ALPHA;
            gameState_.alphaPawns[arguments[1][0]].pos = movePos;
            gameState_.board[blockPos].type = GameState::Field::OBSTACLE;

            gameState_.calculateProximity(gameState_.alphaPawns, gameState_.alphaProximity);
        }
        else
        {
            Position currentPos = gameState_.numPawns[arguments[1][0]].pos;
            Position movePos = moveToPos(currentPos, arguments[2]);
            Position blockPos = moveToPos(movePos, arguments[3]);

            gameState_.board[movePos].id = arguments[1][0];
            gameState_.board[currentPos].type = GameState::Field::FREE;
            gameState_.board[movePos].type = GameState::Field::NUM;
            gameState_.numPawns[arguments[1][0]].pos = movePos;
            gameState_.board[blockPos].type = GameState::Field::OBSTACLE;

            gameState_.calculateProximity(gameState_.numPawns, gameState_.numProximity);
        }

        return "=";
    }

private:

    GameState& gameState_;

};

class TimeLeftGtpHandler: public GtpHandler
{
public:

    TimeLeftGtpHandler(GameState& gameState) :
        gameState_(gameState)
    {
    }

    result_type operator ()(argument_type arguments)
    {
        gameState_.timeLeft = atoi(arguments[0].c_str());
        return "=";
    }

private:

    GameState& gameState_;

};

class GenplaceGtpHandler: public GtpHandler
{
public:

    GenplaceGtpHandler(GameState& gameState) :
        gameState_(gameState)
    {
    }

    result_type operator ()(argument_type arguments)
    {
        Position pos;

        srand(time(0));
        while(1)
        {
            pos = Position(rand() % (Position::boardSize - 1) + 1, rand() % (Position::boardSize - 1) + 1);
            if(gameState_.board[pos].type == GameState::Field::FREE)
                break;
        }

        if(arguments[0] == "al")
        {
            gameState_.board[pos].type = GameState::Field::ALPHA;
            gameState_.alphaPawns.insert(std::make_pair(arguments[1][0], pos));
        }
        else
        {
            gameState_.board[pos].type = GameState::Field::NUM;
            gameState_.numPawns.insert(std::make_pair(arguments[1][0], pos));
        }
        gameState_.board[pos].id = arguments[1][0];

        return "= " + posToCoord(pos);
    }

private:

    GameState& gameState_;

};

class GenmoveGtpHandler: public GtpHandler
{
public:

    GenmoveGtpHandler(GameState& gameState) :
        gameState_(gameState)
    {
    }

    result_type operator ()(argument_type arguments)
    {
        std::stringstream result;
        result << "= ";

        Position moveTarget, blockTarget;
        char id;
        if(arguments[0] == "al")
        {
            gameState_.calculateProximity(gameState_.numPawns, gameState_.numProximity);

            if(findBestMove(gameState_.alphaPawns, gameState_.numProximity, moveTarget,
                    blockTarget, id))
            {
                Position from = gameState_.alphaPawns[id].pos;
                gameState_.board[from].type = GameState::Field::FREE;
                gameState_.board[moveTarget].type = GameState::Field::ALPHA;
                gameState_.board[moveTarget].id = id;
                result << id << " " << posToMove(from, moveTarget) << " ";
                gameState_.alphaPawns[id].pos = moveTarget;
                gameState_.board[blockTarget].type = GameState::Field::OBSTACLE;
                result << posToMove(moveTarget, blockTarget);
                return result.str();
            }
        }
        else
        {
            gameState_.calculateProximity(gameState_.alphaPawns, gameState_.alphaProximity);

            if(findBestMove(gameState_.numPawns, gameState_.alphaProximity, moveTarget,
                    blockTarget, id))
            {
                Position from = gameState_.numPawns[id].pos;
                gameState_.board[from].type = GameState::Field::FREE;
                gameState_.board[moveTarget].type = GameState::Field::NUM;
                gameState_.board[moveTarget].id = id;
                result << id << " " << posToMove(from, moveTarget) << " ";
                gameState_.numPawns[id].pos = moveTarget;
                gameState_.board[blockTarget].type = GameState::Field::OBSTACLE;
                result << posToMove(moveTarget, blockTarget);
                return result.str();
            }
        }

        result << "pass";
        return result.str();
    }

private:

    GameState& gameState_;

    std::pair<bool, size_t> closestAdjacentField(const Position& position,
            const std::vector<size_t>& proximity, size_t distance, Position& field) const
    {
        bool found = false;
        size_t bestDistance = MAX_SIZE_T;

        for(Position::Iterator it(position); !it.atEnd(); ++it)
        {
            size_t currentDistance = absDiff(proximity[*it], distance);
            if(gameState_.board[*it].type == GameState::Field::FREE && currentDistance
                    < bestDistance)
            {
                field = *it;
                bestDistance = currentDistance;
                found = true;
            }
        }

        return std::make_pair(found, bestDistance);
    }

    bool findMoveAround(const Position& pos, const std::vector<GameState::Field>& board,
            GameState::PathEntry& move)
    {
        for(size_t moveRow = move.move.row(); moveRow < min(pos.row() + 2, board.size()); ++moveRow)
        {
            size_t moveCol;
            if(moveRow == move.move.row())
                moveCol = move.move.col();
            else
                moveCol = decrement(pos.col());

            for(; moveCol < min(pos.col() + 2, board.size()); ++moveCol)
            {
                if(pos.row() == moveRow && pos.col() == moveCol)
                    continue;

                if(board[Position(moveRow, moveCol)].type == GameState::Field::FREE)
                {
                    size_t blockRow;
                    if(moveRow == move.move.row() && moveCol == move.move.col())
                        blockRow = move.block.row();
                    else
                        blockRow = decrement(moveRow);

                    for(; blockRow < min(moveRow + 2, board.size()); ++blockRow)
                    {
                        size_t blockCol;
                        if(moveRow == move.move.row() && moveCol == move.move.col() && blockRow
                                == move.block.row())
                            blockCol = move.block.col();
                        else
                            blockCol = decrement(moveCol);

                        for(; blockCol < min(moveCol + 2, board.size()); ++blockCol)
                        {
                            if(blockRow == moveRow && blockCol == moveCol)
                                continue;

                            if(board[Position(blockRow, blockCol)].type == GameState::Field::FREE)
                            {
                                move.move = Position(moveRow, moveCol);
                                move.block = Position(blockRow, blockCol);
                                return true;
                            }
                        }
                    }
                }
            }
        }
        return false;
    }

    // brute force
    void calculateFillingPath(GameState::Pawn& pawn, clock_t maxTime)
    {
        static const std::pair<size_t, size_t> NULL_POS = std::make_pair(MAX_SIZE_T, MAX_SIZE_T);
        static const GameState::PathEntry NULL_ENTRY(NULL_POS, NULL_POS);

        pawn.pathCalculated = true;
        std::deque<GameState::PathEntry> path;
        GameState::PathEntry lastTry = NULL_ENTRY;
        std::vector<GameState::Field> board = gameState_.board;

        GameState::PathEntry entry;

        entry.move = pawn.pos;
        entry.block = NULL_POS;
        path.push_back(entry);
        board[pawn.pos].type = GameState::Field::FREE;

        clock_t start = clock();

        while(true)
        {
            if(((clock() - start) / (CLOCKS_PER_SEC / 1000)) > maxTime)
                return;

            if(lastTry == NULL_ENTRY)
            {
                entry.move = Position(decrement(path.back().move.row()), decrement(
                        path.back().move.col()));
                entry.block = Position(decrement(entry.move.row()), decrement(entry.move.col()));
            }
            else
            {
                entry = lastTry;
                entry.block = Position(entry.block.row(), entry.block.col() + 1);
            }

            if(findMoveAround(path.back().move, board, entry))
            {
                board[entry.block].type = GameState::Field::OBSTACLE;
                lastTry = NULL_ENTRY;
                path.push_back(entry);
            }
            else
            {
                if(path.size() == 1)
                    break;

                if(pawn.path.size() + 1 < path.size())
                {
                    pawn.path = path;
                    pawn.path.pop_front();
                }

                board[path.back().block].type = GameState::Field::FREE;
                lastTry = path.back();
                path.pop_back();
            }
        }
    }

    bool findBestMove(std::map<char, GameState::Pawn>& pawns, const std::vector<size_t>& proximity,
            Position& moveTarget, Position& blockTarget, char& id)
    {
        // za najlepszy ruch uwazam taki, ktory ustawi pionek w odleglosci 2 od
        // pewnego pionka przeciwnika i zablokuje pole w odleglosci 1 od przeciwnika
        bool found = false;
        size_t bestDistance = MAX_SIZE_T, bestBlockDistance = MAX_SIZE_T;

        std::map<char, GameState::Pawn>::iterator it, end = pawns.end();
        for(it = pawns.begin(); it != end; ++it)
        {
            // czy ten pionek jest oddzielony od przeciwnika?
            if(proximity[it->second.pos] == MAX_SIZE_T)
            {
                continue;
            }

            Position target;
            std::pair<bool, size_t> currentResult = closestAdjacentField(it->second.pos, proximity,
                    2, target);
            if(currentResult.first)
            {
                found = true;
                size_t currentDistance = currentResult.second;
                if(currentDistance < bestDistance)
                {
                    bestDistance = currentDistance;
                    moveTarget = target;
                    // troszke brzydkie...
                    GameState::Field::Type tmpType = gameState_.board[it->second.pos].type;
                    gameState_.board[it->second.pos].type = GameState::Field::FREE;
                    bestBlockDistance
                            = closestAdjacentField(moveTarget, proximity, 1, blockTarget).second;
                    gameState_.board[it->second.pos].type = tmpType;
                    id = it->first;
                }
                else if(currentDistance == bestDistance)
                {
                    Position tmpBlockTarget;
                    GameState::Field::Type tmpType = gameState_.board[it->second.pos].type;
                    gameState_.board[it->second.pos].type = GameState::Field::FREE;
                    size_t blockDistance = closestAdjacentField(moveTarget, proximity, 1,
                            tmpBlockTarget).second;
                    gameState_.board[it->second.pos].type = tmpType;
                    if(blockDistance < bestBlockDistance)
                    {
                        bestBlockDistance = blockDistance;
                        moveTarget = target;
                        blockTarget = tmpBlockTarget;
                        id = it->first;
                    }
                }
            }
        }

        if(!found)
        {
            // poruszam pionkami, ktore sa oddzielone
            for(it = pawns.begin(); it != end; ++it)
            {
                if(proximity[it->second.pos] == MAX_SIZE_T)
                {
                    if(!it->second.pathCalculated)
                    {
                        // TODO: to nie powinno byc hardcodowane
                        calculateFillingPath(it->second, 5000);
                    }

                    if(it->second.path.size())
                    {
                        moveTarget = it->second.path.front().move;
                        blockTarget = it->second.path.front().block;
                        it->second.path.pop_front();
                        id = it->first;
                        return true;
                    }
                }
            }
        }

        return found;
    }

};

#endif /* INVADERSGTPHANDLERS_HPP_ */
