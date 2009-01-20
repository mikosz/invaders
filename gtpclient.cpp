/*
 * gtpclient.cpp
 *
 *  Created on: Oct 19, 2008
 *      Author: Mikolaj Radwan
 */

#include <gtpclient.hpp>

#include <sstream>
#include <algorithm>
#include <iterator>
#include <iostream>

#ifndef NDEBUG
const bool DEBUG = true;
#else
const bool DEBUG = false;
#endif

bool GtpClient::handle()
{
    std::string line;
    getline(inputStream_, line);
    std::istringstream iss(line);

    if(DEBUG)
    {
        std::cerr << "<- " << line << std::endl;
    }

    std::string command;
    std::string argument;
    std::vector<std::string> arguments;

    iss >> command;
    while(iss.good())
    {
        iss >> argument;
        arguments.push_back(argument);
    }

    if(command == "quit")
        return true;

    std::map<std::string, GtpHandler*>::iterator handler = handlers_.find(command);
    if(handler == handlers_.end())
        throw "Command: \"" + command + "\" does not have a handler";

    std::string result = handler->second->operator()(arguments);

    if(DEBUG)
    {
        std::cerr << "-> " << result << std::endl;
    }

    outputStream_ << result << "\n";

    return false;
}

void GtpClient::addHandler(const std::string& command, GtpHandler* handler)
{
    handlers_.insert(std::make_pair(command, handler));
}
