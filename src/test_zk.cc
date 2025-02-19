#include "zookeeperutil.h"
#include <iostream>

int main()
{

    ZKClient zk;

    zk.Start();

    std::string path = "/demo";
    std::string data = "helloZK, this is a test data!";
    int dataLen = sizeof(data.c_str());

    zk.Create(path.c_str(), data.c_str(), dataLen, 0);
    std::string ret = zk.GetData(path.c_str());
    std::cout << ret << std::endl;

    return 0;
}