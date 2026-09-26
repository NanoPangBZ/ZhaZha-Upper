#pragma once

#include <stdint.h>
#include <string>

class RWPort
{
public:
    RWPort( const char* name = "undefined" )
    {
        m_name = name;
    }
    virtual ~RWPort() = default;

    const char* Name()
    {
        return m_name.c_str();
    };

    virtual uint32_t Write(uint8_t* data , uint32_t len) = 0;
    virtual uint32_t Read(uint8_t* data , uint32_t len) = 0;

private:
    std::string m_name;
};

