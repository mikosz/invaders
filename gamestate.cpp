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

void setFieldFree(Field& field)
{
    field.type = Field::FREE;
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

bool StateValue::operator<(const StateValue& rhs) const
{
    return fieldsOwned < rhs.fieldsOwned;
}

StateValue stateValue(std::vector<Field>& board, std::vector<size_t>& proximity, std::vector<size_t>& opponentProximity)
{
    StateValue value;

    for(size_t i = 0; i < board.size(); ++i)
    {
        if(board[i].type != Field::OBSTACLE)
        {
            if(proximity[i] < opponentProximity[i])
            {
                ++value.fieldsOwned;
            }
            else if(opponentProximity[i] < proximity[i])
            {
                --value.fieldsOwned;
            }
            // na razie ignoruje rowne odleglosci
        }
    }

    return value;
}

void calculateProximity(const std::vector<Field>& board, std::map<char, Pawn>& pawns, std::vector<size_t>& proximity)
{
    size_t size = board.size();

    std::vector<bool> visited;
    visited.resize(size, false);
    proximity.assign(size, MAX_SIZE_T);

    VisitEntry entry;

    std::deque<VisitEntry> toVisit;

    for(std::map<char, Pawn>::iterator it = pawns.begin(); it != pawns.end(); ++it)
    {
        entry.pos = it->second.pos;
        entry.distance = 0;
        toVisit.push_back(entry);
    }

    while(toVisit.size())
    {
        VisitEntry front = toVisit.front();
        toVisit.pop_front();
        visited[front.pos] = true;

        if(front.distance < proximity[front.pos])
        {
            proximity[front.pos] = front.distance;
            entry.distance = front.distance + 1;

            for(Position::Iterator it(front.pos); !it.atEnd(); ++it)
            {
                Position pos = *it;
                if(!visited[pos] && board[pos].type != Field::OBSTACLE)
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

std::ostream& operator<<(std::ostream& os, const Field& field)
{
    if(field.type == Field::OBSTACLE)
        os << '#';
    else if(field.type == Field::FREE)
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
                * Position::boardSize), std::ostream_iterator<Field>(os));
        os << '\n';
    }

    return os;
}

void movePawn(std::vector<Field>& board, Pawn& pawn, const Position& toPosition, Field::Type pawnType)
{
    assert(board[toPosition].type == Field::FREE);
    assert(max(abs(pawn.pos.row() - toPosition.row()), abs(pawn.pos.col() - toPosition.col())) == 1);
    assert(pawnType == Field::ALPHA || pawnType == Field::NUM);

    board[pawn.pos].type = Field::FREE;
    pawn.pos = toPosition;
    board[toPosition].type = pawnType;
    board[toPosition].id = pawn.id;
}

void blockPosition(std::vector<Field>& board, const Position& target)
{
    assert(board[target].type == Field::FREE);

    board[target].type = Field::OBSTACLE;
}

void unblockPosition(std::vector<Field>& board, const Position& target)
{
    assert(board[target].type == Field::OBSTACLE);

    board[target].type = Field::FREE;
}
