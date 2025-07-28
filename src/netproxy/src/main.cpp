//
// Created by ywj on 2025/7/29.
//
#include<string.h>
#include<unistd.h>
#include"config.h"
#include"log.h"
const static char* version="1.0.0";

static void usage( const char* prog )
{
    LOG( LOG_INFO,  "usage: %s [-h] [-v] [-f config_file]", prog );
}

int main(int argc,char**argv)
{
    char cfg_file[1024];
    memset( cfg_file, '\0', 100 );
    int option;
    while ( ( option = getopt( argc, argv, "f:xvh" ) ) != -1 )
    {
        switch ( option )
        {
        case 'x':
            {
                set_loglevel( LOG_DEBUG );
                break;
            }
        case 'v':
            {
                log( LOG_INFO, __FILE__, __LINE__, "%s %s", argv[0], version );
                return 0;
            }
        case 'h':
            {
                usage( basename( argv[ 0 ] ) );
                return 0;
            }
        case 'f':
            {
                memcpy( cfg_file, optarg, strlen( optarg ) );
                break;
            }
        case '?':
            {
                log( LOG_ERR, __FILE__, __LINE__, "un-recognized option %c", option );
                usage( basename( argv[ 0 ] ) );
                return 1;
            }
        }
    }
    if( cfg_file[0] == '\0' )
    {
        log( LOG_ERR, __FILE__, __LINE__, "%s", "please specifiy the config file" );
        return 1;
    }
    if (!config::loadConfig(std::string(cfg_file)))
    {
        return 1;
    }


    return 0;
}