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

const size_t DEFAULT_BOARD_SIZE = 19;
const size_t DEFAULT_PAWNS_PER_PLAYER = 5;

class Position
{
public:

    static size_t boardSize; // zakladam, ze tylko jedna gra odbywa sie na raz

    Position()
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

    operator size_t()
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

private:

    friend class PosIterator;

    size_t pos_;

};

inline bool operator==(const Position& lhs, const Position& rhs)
{
    return lhs.pos_ == rhs.pos_;
}

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
            current_.pos_ += Position::boardSize;
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

    Position operator*()
    {
        return current_;
    }

    bool atEnd() const
    {
        return current_.pos_ >= Position(centre_.row() + 1, centre_.col() + 1);
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
        Pawn() :
            pathCalculated(false)
        {
        }

        explicit Pawn(const Position& pos) :
            pos(pos), pathCalculated(false)
        {
        }

        Position pos;

        bool pathCalculated;

        std::deque<PathEntry> path;
    };

    GameState()
    {
        init(DEFAULT_BOARD_SIZE, DEFAULT_PAWNS_PER_PLAYER);
        for(size_t i = 0; i < board.size(); ++i)
        {
            board[i].type = Field::FREE;
        }
    }

    void init(size_t boardSize, size_t pawnsPerPlayer)
    {
        Position::boardSize = boardSize;
        board.resize(boardSize * boardSize);
        alphaProximity.resize(boardSize * boardSize);
        numProximity.resize(boardSize * boardSize);
    }

    std::vector<Field> board;

    std::map<char, Pawn> alphaPawns, numPawns;

    std::vector<size_t> alphaProximity, numProximity;

    size_t pawnsPerPlayer;

    clock_t timeLeft;

    void calculateProximity(std::map<char, Pawn>& pawns,
            std::vector<size_t>& proximity)
    {
        size_t size = board.size();

        std::vector<bool> visited;
        visited.resize(size, false);
        proximity.assign(size, std::numeric_limits<size_t>::max());

        VisitEntry entry;

        std::deque<VisitEntry> toVisit;

        for(std::map<char, Pawn>::iterator it = pawns.begin(); it
                != pawns.end(); ++it)
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

};

inline size_t row(const std::string& coord, size_t height)
{
    return height - atoi(coord.substr(1).c_str());
}

inline size_t column(const std::string& coord)
{
    return coord[0] - 'a';
}

inline Position coordToPos(const std::string& coord, size_t height)
{
    return std::make_pair(row(coord, height), column(coord));
}

inline std::string posToCoord(const Position& pos, size_t height)
{
    std::stringstream result;
    result << static_cast<char> ('a' + pos.col()) << height - pos.row();
    return result.str();
}

inline Position moveToPos(const Position& pos, const std::string& move)
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
        return Position(pos.row() - 1, pos.col() - 1);
    }
    else if(move == "s")
    {
        return Position(pos.row() - 1, pos.col());
    }
    else if(move == "se")
    {
        return Position(pos.row() - 1, pos.col() + 1);
    }

    throw std::runtime_error("Wrong move: " + move);
}

inline std::string posToMove(const Position& start, const Position& end)
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

inline std::ostream& operator<<(std::ostream& os, const GameState::Field& field)
{
    if(field.type == GameState::Field::OBSTACLE)
        os << '#';
    else if(field.type == GameState::Field::FREE)
        os << '.';
    else
        os << field.id;
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const GameState& gameState)
{
    for(size_t row = 0; row < gameState.board.size(); ++row)
    {
        std::copy(gameState.board.begin() + (row * Position::boardSize),
                gameState.board.begin() + ((row + 1) * Position::boardSize),
                std::ostream_iterator<GameState::Field>(os));
        os << '\n';
    }

    return os;
}

#endif /* GAMESTATE_HPP_ */
