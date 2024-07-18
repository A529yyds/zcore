/*
 * This file is part of the software and assets of HK ZXOUD LIMITED.
 * @copyright (c) HK ZXOUD LIMITED https://www.zxoud.com
 * Author: A529yyds(email:1041389196@qq.com)
 * create: 2024062
 * FilePath: /zcore/head/JsonSingleton.hpp
 * Description: a singleton of json file operation
 */

#pragma once
#include <glog/logging.h>
#include "nlohmann/json.hpp"

class JsonSingleton
{

public:
    /**
     * @description: get a JsonSingleton instance
     * @return {JsonSingleton} instance
     */
    inline static JsonSingleton &getInstance()
    {
        static JsonSingleton instance;
        return instance;
    }
    /**
     * @description: write json data to file
     * @param {json} data : add context
     */
    void writeData(nlohmann::json data);
    /**
     * @description: set configuration file's path
     * @param {string} path - set value
     */
    void setFilePath(std::string path);
    /**
     * @description: read json data from files
     * @return {json} file context
     */
    nlohmann::json readData();
    /**
     * @description: get names of libraries that can be installed directly as vector
     * @param {string} appName - application to be installed
     * @return {vector} the names of librariies
     */
    std::vector<std::string> getLibsVec(std::string appName);
    /**
     * @description: get libraries source infomation from "appName" json key is "codes"
     * @param {string} appName - the name of application to be installed
     * @return {json} return url and version as a json data
     */
    nlohmann::json getCodesInfos(std::string appName);
    /**
     * @description: get source URL from json data
     * @param {string} appName - name of the application to be installed
     * @return {string} the URL of application to be installed
     */
    std::string getCodesAddress(std::string appName);

protected:
    /**
     * @description: json data combined into pull source code
     * @param {string} url - source URL
     * @param {string} version - source version
     * @return {json} json data
     */
    nlohmann::json getCodeItem(std::string url, std::string version);

private:
    std::string _path;
    nlohmann::json _readData;

private:
    JsonSingleton();
    ~JsonSingleton()
    {
    }
    JsonSingleton(const JsonSingleton &copy) = delete;
    JsonSingleton &operator=(const JsonSingleton &other) = delete;
    /**
     * @description: init a example to json file
     */
    void initDefaultConfig();
};