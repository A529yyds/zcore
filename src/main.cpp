#include "CLISingleton.hpp"
#include <string>

int main(int argc, char **argv)
{
    CLISingleton::getInstance().setCmdsParse(argc, argv);
    return 0;
}