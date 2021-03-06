/*
 * mr222146.cpp
 *
 *  Created on: Oct 19, 2008
 *      Author: Mikolaj Radwan
 */

#include <iostream>
#include <string>

#include <gtpclient.hpp>
#include <gtphandlers.hpp>
#include <gamestate.hpp>
#include <invadersgtphandlers.hpp>

#ifndef NDEBUG
const bool DEBUG = true;
#else
const bool DEBUG = false;
#endif

int main(int argc, char* argv[])
{
    GtpClient gtpClient(std::cin, std::cout);

    SimpleStringGtpHandler nameGtpHandler(argv[0]);
    gtpClient.addHandler("name", &nameGtpHandler);
    SimpleStringGtpHandler authorGtpHandler("mr222146");
    gtpClient.addHandler("author", &authorGtpHandler);

    GameState gameState;
    SetboardGtpHandler setboardGtpHandler(std::cin, gameState);
    gtpClient.addHandler("setboard", &setboardGtpHandler);
    PlaceGtpHandler placeGtpHandler(gameState);
    gtpClient.addHandler("place", &placeGtpHandler);
    MoveGtpHandler moveGtpHandler(gameState);
    gtpClient.addHandler("move", &moveGtpHandler);
    TimeLeftGtpHandler timeLeftGtpHandler(gameState);
    gtpClient.addHandler("time_left", &timeLeftGtpHandler);
    GenmoveGtpHandler genmoveGtpHandler(gameState);
    gtpClient.addHandler("genmove", &genmoveGtpHandler);
    GenplaceGtpHandler genplaceGtpHandler(gameState);
    gtpClient.addHandler("genplace", &genplaceGtpHandler);

    while(true)
    {
        try
        {
            if(gtpClient.handle())
                break;

            if(DEBUG)
            {
                std::cerr << gameState << std::endl;
            }
        }
        catch(const std::string& e)
        {
            std::cerr << "Caught exception in main: " << e << "\n";
        }
    }

    return 0;
}
