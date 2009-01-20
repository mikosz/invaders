/*
 * gtphandlers.hpp
 *
 *  Created on: Oct 19, 2008
 *      Author: Mikolaj Radwan
 */

#ifndef GTPHANDLERS_HPP_
#define GTPHANDLERS_HPP_

#include <string>

class GtpHandler : public std::unary_function<const std::vector<std::string>&, std::string>
{
public:

    virtual ~GtpHandler()
    {}

    virtual result_type operator()(argument_type arguments) = 0;

};

class SimpleStringGtpHandler : public GtpHandler
{
public:

    SimpleStringGtpHandler(const std::string& value) : value_(value)
    {}

    result_type operator()(argument_type arguments)
    {
        return "= " + value_;
    }

private:

    std::string value_;

};

#endif /* GTPHANDLERS_HPP_ */
