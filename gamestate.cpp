/*
 * gamestate.cpp
 *
 *  Created on: Jan 19, 2009
 *      Author: mikosz
 */

#include <gamestate.hpp>
#include <util.hpp>

#include <algorithm>
#include <cassert>
#include <iostream>

size_t Position::boardSize = DEFAULT_BOARD_SIZE;

void setFieldFree(GameState::Field& field)
{
    field.type = GameState::Field::FREE;
}

void GameState::init(size_t boardSize, size_t pawnsPerPlayer)
{
    Position::boardSize = boardSize;
    board.resize(boardSize * boardSize);
    alphaProximity.resize(board.size());
    numProximity.resize(board.size());

    std::for_each(board.begin(), board.end(), setFieldFree);

    for(size_t i = 0; i < boardSize; ++i)
    {
        board[i].type = Field::OBSTACLE;
        board[Position(boardSize - 1, i)].type = Field::OBSTACLE;
        board[Position(i, 0)].type = Field::OBSTACLE;
        board[Position(i, boardSize - 1)].type = Field::OBSTACLE;
    }
}

std::pair<size_t, size_t> stateValue(std::vector<GameState::Field>& board, std::vector<size_t>& proximity, std::vector<
        size_t>& opponentProximity)
{
    size_t value = 0;
    size_t avgDistanceSum = 0, avgDistanceCount = 0;

    for(size_t i = 0; i < board.size(); ++i)
    {
        if(board[i].type != GameState::Field::OBSTACLE)
        {
            if(proximity[i] < opponentProximity[i])
            {
                avgDistanceSum += proximity[i];
                ++avgDistanceCount;
                ++value;
            }
            else if(opponentProximity[i] < proximity[i])
            {
                --value;
            }
            // na razie ignoruje rowne odleglosci
        }
    }

    return std::make_pair(value, avgDistanceCount ? avgDistanceSum / avgDistanceCount : 0);
}

void calculateProximity(const std::vector<GameState::Field>& board, std::map<char, GameState::Pawn>& pawns,
        std::vector<size_t>& proximity)
{
    size_t size = board.size();

    std::vector<bool> visited;
    visited.resize(size, false);
    proximity.assign(size, MAX_SIZE_T);

    GameState::VisitEntry entry;

    std::deque<GameState::VisitEntry> toVisit;

    for(std::map<char, GameState::Pawn>::iterator it = pawns.begin(); it != pawns.end(); ++it)
    {
        entry.pos = it->second.pos;
        entry.distance = 0;
        toVisit.push_back(entry);
    }

    while(toVisit.size())
    {
        GameState::VisitEntry front = toVisit.front();
        toVisit.pop_front();
        visited[front.pos] = true;

        if(front.distance < proximity[front.pos])
        {
            proximity[front.pos] = front.distance;
            entry.distance = front.distance + 1;

            for(Position::Iterator it(front.pos); !it.atEnd(); ++it)
            {
                Position pos = *it;
                if(!visited[pos] && board[pos].type != GameState::Field::OBSTACLE)
                {
                    visited[pos] = true;
                    entry.pos = pos;
                    toVisit.push_back(entry);
                }
            }
        }
    }
}

Position moveToPos(const Position& pos, const std::string& move)
{
    if(move == "nw")
    {
        return Position(pos.row() - 1, pos.col() - 1);
    }
    else if(move == "n")
    {
        return Position(pos.row() - 1, pos.col());
    }
    else if(move == "ne")
    {
        return Position(pos.row() - 1, pos.col() + 1);
    }
    else if(move == "w")
    {
        return Position(pos.row(), pos.col() - 1);
    }
    else if(move == "e")
    {
        return Position(pos.row(), pos.col() + 1);
    }
    else if(move == "sw")
    {
        return Position(pos.row() + 1, pos.col() - 1);
    }
    else if(move == "s")
    {
        return Position(pos.row() + 1, pos.col());
    }
    else if(move == "se")
    {
        return Position(pos.row() + 1, pos.col() + 1);
    }

    throw std::runtime_error("Wrong move: " + move);
}

std::string posToCoord(const Position& pos)
{
    std::stringstream result;
    result << static_cast<char> ('a' + pos.col() - 1) << Position::boardSize - pos.row() - 1;
    return result.str();
}

std::string posToMove(const Position& start, const Position& end)
{
    std::stringstream result;
    if(start.row() < end.row())
        result << 's';
    else if(start.row() > end.row())
        result << 'n';

    if(start.col() < end.col())
        result << 'e';
    else if(start.col() > end.col())
        result << 'w';

    return result.str();
}

std::ostream& operator<<(std::ostream& os, const GameState::Field& field)
{
    if(field.type == GameState::Field::OBSTACLE)
        os << '#';
    else if(field.type == GameState::Field::FREE)
        os << '.';
    else
        os << field.id;
    return os;
}

std::ostream& operator<<(std::ostream& os, const GameState& gameState)
{
    for(size_t i = 0; i < Position::boardSize; ++i)
    {
        std::copy(gameState.board.begin() + (i * Position::boardSize), gameState.board.begin() + ((i + 1)
                * Position::boardSize), std::ostream_iterator<GameState::Field>(os));
        os << '\n';
    }

    return os;
}

void movePawn(std::vector<GameState::Field>& board, GameState::Pawn& pawn, const Position& toPosition,
        GameState::Field::Type pawnType)
{
    assert(board[toPosition].type == GameState::Field::FREE);
    assert(max(abs(pawn.pos.row() - toPosition.row()), abs(pawn.pos.col() - toPosition.col())) == 1);
    assert(pawnType == GameState::Field::ALPHA || pawnType == GameState::Field::NUM);

    board[pawn.pos].type = GameState::Field::FREE;
    pawn.pos = toPosition;
    board[toPosition].type = pawnType;
    board[toPosition].id = pawn.id;
}

void blockPosition(std::vector<GameState::Field>& board, const Position& target)
{
    assert(board[target].type == GameState::Field::FREE);

    board[target].type = GameState::Field::OBSTACLE;
}

void unblockPosition(std::vector<GameState::Field>& board, const Position& target)
{
    assert(board[target].type == GameState::Field::OBSTACLE);

    board[target].type = GameState::Field::FREE;
}
