/*
 * gtpclient.hpp
 *
 *  Created on: Oct 19, 2008
 *      Author: Mikolaj Radwan
 */

#ifndef GTPCLIENT_HPP_
#define GTPCLIENT_HPP_

#include <map>
#include <string>
#include <functional>
#include <vector>
#include <iostream>

#include <gtphandlers.hpp>

class GtpClient
{
public:

    GtpClient(std::istream& inputStream, std::ostream& outputStream) :
        inputStream_(inputStream),
        outputStream_(outputStream)
    {}

    bool handle();

    void addHandler(const std::string& command, GtpHandler* handler);

private:

    std::istream& inputStream_;

    std::ostream& outputStream_;

    std::map<std::string, GtpHandler*> handlers_;

};

#endif /* GTPCLIENT_HPP_ */
