/*
 * gamestate.hpp
 *
 *  Created on: Oct 19, 2008
 *      Author: mikosz
 */

#ifndef GAMESTATE_HPP_
#define GAMESTATE_HPP_

#include <iterator>
#include <ctime>
#include <map>
#include <sstream>
#include <cstdlib>
#include <deque>
#include <limits>
#include <stdexcept>
#include <vector>

#include <util.hpp>

const size_t DEFAULT_BOARD_SIZE = 19;
const size_t DEFAULT_PAWNS_PER_PLAYER = 5;

class Position
{
public:

    static size_t boardSize; // zakladam, ze tylko jedna gra odbywa sie na raz

    Position()
    {
    }

    Position(size_t pos) :
        pos_(pos)
    {
    }

    Position(const std::pair<size_t, size_t>& pos) :
        pos_(pos.first * boardSize + pos.second)
    {
    }

    Position(size_t row, size_t col) :
        pos_(row * boardSize + col)
    {
    }

    operator size_t() const
    {
        return pos_;
    }

    const size_t row() const
    {
        return pos_ / boardSize;
    }

    const size_t col() const
    {
        return pos_ % boardSize;
    }

    typedef class PosIterator Iterator;

    friend bool operator==(const Position& lhs, const Position& rhs);

    Position& operator=(size_t rhs)
    {
        pos_ = rhs;
        return *this;
    }

private:

    friend class PosIterator;

    size_t pos_;

};

inline bool operator==(const Position& lhs, const Position& rhs)
{
    return lhs.pos_ == rhs.pos_;
}

// iteruje po polach wokol pola centre
class PosIterator
{
public:

    PosIterator(const Position& centre) :
        centre_(centre), current_(centre.row() - 1, centre.col() - 1)
    {
    }

    PosIterator& operator++()
    {
        if(current_.col() == centre_.col() + 1)
        {
            current_.pos_ += Position::boardSize - 2;
        }
        else
        {
            ++current_.pos_;
            if(current_.pos_ == centre_.pos_)
            {
                return ++(*this);
            }
        }

        return *this;
    }

    Position& operator*()
    {
        return current_;
    }

    Position* operator->()
    {
        return &current_;
    }

    bool atEnd() const
    {
        return current_.pos_ > Position(centre_.row() + 1, centre_.col() + 1);
    }

private:

    Position centre_;

    Position current_;

};

struct GameState
{
public:

    struct Field
    {
        enum Type
        {
            OBSTACLE, ALPHA, NUM, FREE
        } type;

        char id;
    };

    struct VisitEntry
    {
        Position pos;
        size_t distance;
    };

    struct PathEntry
    {
        PathEntry()
        {
        }

        PathEntry(Position move, Position block) :
            move(move), block(block)
        {
        }

        bool operator==(const PathEntry& rhs) const
        {
            return move == rhs.move && block == rhs.block;
        }

        Position move, block;
    };

    struct Pawn
    {
        Pawn()
        {
        }

        explicit Pawn(char id, const Position& pos) :
            id(id), pos(pos)
        {
        }

        char id;

        Position pos;

    };

    GameState()
    {
        init(DEFAULT_BOARD_SIZE + 2, DEFAULT_PAWNS_PER_PLAYER);
    }

    void init(size_t boardSize, size_t pawnsPerPlayer);

    std::vector<Field> board;

    std::map<char, Pawn> alphaPawns, numPawns;

    std::vector<size_t> alphaProximity, numProximity;

    size_t pawnsPerPlayer;

    clock_t timeLeft;

};

// zwraca pare: <wartosc ruchu, srednia odleglosc od wlasnego pola>
std::pair<size_t, size_t> stateValue(std::vector<GameState::Field>& board, std::vector<size_t>& proximity, std::vector<
        size_t>& opponentProximity);

void calculateProximity(const std::vector<GameState::Field>& board, std::map<char, GameState::Pawn>& pawns,
        std::vector<size_t>& proximity);

void movePawn(std::vector<GameState::Field>& board, GameState::Pawn& pawn, const Position& toPosition,
        GameState::Field::Type pawnType);

void blockPosition(std::vector<GameState::Field>& board, const Position& target);

void unblockPosition(std::vector<GameState::Field>& board, const Position& target);

Position moveToPos(const Position& pos, const std::string& move);

std::string posToCoord(const Position& pos);

std::string posToMove(const Position& start, const Position& end);

std::ostream& operator<<(std::ostream& os, const GameState::Field& field);

std::ostream& operator<<(std::ostream& os, const GameState& gameState);

inline size_t coordToRow(const std::string& coord)
{
    return Position::boardSize - atoi(coord.substr(1).c_str()) - 1;
}

inline size_t coordToCol(const std::string& coord)
{
    return coord[0] - 'a' + 1;
}

inline Position coordToPos(const std::string& coord)
{
    return Position(coordToRow(coord), coordToCol(coord));
}

#endif /* GAMESTATE_HPP_ */
