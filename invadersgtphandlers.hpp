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

    result_type operator()(argument_type arguments);

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

    result_type operator ()(argument_type arguments);

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

    result_type operator ()(argument_type arguments);

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

    result_type operator ()(argument_type arguments);

private:

    GameState& gameState_;

    static char dummyPawnId;

    // zwraca pare <wybrane miejsce, <wartosc ruchu, srednia odleglosc od wlasnego pola> >
    std::pair<Position, std::pair<size_t, size_t> > bestPlace(std::vector<GameState::Field>& board, std::map<char,
            GameState::Pawn>& pawns, std::map<char, GameState::Pawn>& opponentPawns, std::vector<size_t>& proximity,
            std::vector<size_t>& opponentProximity, GameState::Field::Type pawnType,
            GameState::Field::Type opponentPawnType);

};

class GenmoveGtpHandler: public GtpHandler
{
public:

    GenmoveGtpHandler(GameState& gameState) :
        gameState_(gameState)
    {
    }

    result_type operator ()(argument_type arguments);

private:

    std::pair<GameState::Pawn*, std::pair<Position, Position> > bestMove(std::vector<GameState::Field>& board,
            std::map<char, GameState::Pawn>& pawns, std::map<char, GameState::Pawn>& opponentPawns,
            std::vector<size_t>& proximity, std::vector<size_t>& opponentProximity, GameState::Field::Type pawnType,
            GameState::Field::Type opponentPawnType);

    GameState& gameState_;

};

#endif /* INVADERSGTPHANDLERS_HPP_ */
