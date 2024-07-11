#include "JsonSingleton.hpp"
#include <fstream>
#include <iostream>

JsonSingleton::JsonSingleton()
{
    _path = "./config.json";
    readData();
    // initDefaultConfig();
}

void JsonSingleton::writeData(nlohmann::json data)
{
    std::ofstream file(_path);
    file << std::setw(4) << data << std::endl;
}

nlohmann::json JsonSingleton::readData()
{
    std::ifstream file(_path);
    try
    {
        file >> _readData;
    }
    catch (nlohmann::json::parse_error &e)
    {
        LOG(INFO) << "JSON parse error: " << e.what() << std::endl;
    }
    return _readData;
}

std::vector<std::string> JsonSingleton::getLibsVec(std::string appName)
{
    std::vector<std::string> retVec;
    nlohmann::json libs = _readData[appName]["libs"];
    for (auto &item : libs.items())
    {
        retVec.push_back(item.key());
    }
    return retVec;
}

nlohmann::json JsonSingleton::getCodesInfos(std::string appName)
{
    return _readData[appName]["codes"];
}

std::string JsonSingleton::getCodesAddress(std::string appName)
{
    LOG(INFO) << "getCodesAddress " << appName << std::endl;
    nlohmann::json json = _readData;
    return json[appName]["address"];
}

nlohmann::json JsonSingleton::getCodeItem(std::string url, std::string version)
{
    nlohmann::json item;
    item["url"] = url;
    item["version"] = version;
    return item;
}

void JsonSingleton::initDefaultConfig()
{
    nlohmann::json apps;
    nlohmann::json roots;
    nlohmann::json libs;
    nlohmann::json codes;
    libs["postgresql"] = "7.9.0";
    libs["mimalloc"] = "2.1";
    libs["libunwind"] = "1.8.1";
    libs["libzmq"] = "4.3.5";

    codes["hiredis"] = getCodeItem("https://github.com/redis/hiredis.git", "1.2.0");
    codes["libczmq"] = getCodeItem("https://github.com/zeromq/czmq.git", "4.2.1");
    codes["glog"] = getCodeItem("https://github.com/google/glog.git", "0.7.0");

    roots["os"] = "archLinux";
    roots["compiler"] = "g++";
    roots["version"] = "1.0.0";
    roots["address"] = "https://github.com/ZXOUD-OWNER/zlink.git";
    roots["libs"] = libs;
    roots["codes"] = codes;

    apps["zlink"] = roots;
    writeData(apps);
}

void JsonSingleton::setFilePath(std::string path)
{
    _path = path;
}
