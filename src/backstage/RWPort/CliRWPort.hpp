#include "RWPort.hpp"
#include <stdio.h>

class CliRWPort : public RWPort
{
public:
    CliRWPort( const char* name = "undefined" ) : RWPort( name )
    {
        printf("CliRWPort constructor: %s\n", name);
    };
    virtual ~CliRWPort()
    {
        printf("CliRWPort destructor: %s\n", Name());
    };

    virtual uint32_t Write(uint8_t* data , uint32_t len) override{
        printf("CliRWPort(%s) Write (len%d):\r\n" , Name() , len );
        for( uint32_t temp = 0 ; temp < len ;  )
        {
            /* 打印8个字节，每行8个字节 */
            for( uint32_t i = 0 ; i < 8 ; i++ )
            {
                if( temp < len ){
                    printf("%02X ", data[temp]);
                    temp++;
                }
                else{
                    printf("  ");
                }
            }
            printf("\r\n");
        }
        printf("\r\n");
        return len;
    }

    virtual uint32_t Read(uint8_t* data , uint32_t len) override{
        return 0;
    }
};

