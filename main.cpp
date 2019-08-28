#include <iostream> // cout ...

#include <unistd.h> // getopt
#include <stdlib.h> // exit

#include "c_server.h"

static void printUsage(std::ostream& os, const std::string& programName){
  os << "Usage: " << programName << " [Options...]\n"
     << "Options:\n"
     << "    -h          Display this help message\n"
     << "    -p <port>   Listening port"
     << std::endl;
}

int main(int argc, char * argv[]){
    std::string programName(argv[0]);
    if(argc < 2){
        printUsage(std::cerr, programName);
        exit(1);
    }

    int listenport = 0;
    int opt;
    while((opt = getopt(argc, argv, "hp:")) != -1){
        switch (opt) {
        case 'h':
            printUsage(std::cout, programName);
            exit(0);
        case 'p':
            listenport = std::stoi(optarg);
            break;
        default:
            printUsage(std::cerr, programName);
            exit(1);
        }
    }

    Server server(listenport);
    server.run();

    return 0;
}