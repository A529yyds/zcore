/*
 * This file is part of the software and assets of HK ZXOUD LIMITED.
 * @copyright (c) HK ZXOUD LIMITED https://www.zxoud.com
 * Author: A529yyds(email:1041389196@qq.com)
 * create: 20240711
 * FilePath: /zcore/head/CLISingleton.hpp
 * Description: command-line tools developed using CLI11
 */

#include "CLI/CLI.hpp"
class CLISingleton
{
public:
    inline static CLISingleton &getInstance()
    {
        static CLISingleton instance;
        return instance;
    }
    /**
     * @description: set options for CLI::APP and parse
     * @param {int} argc - argc from main
     * @param {char} *argv -  argv from main
     * @return {int} result
     */
    int setCmdsParse(int argc, char **argv);

protected:
    /**
     * @description: callback function for option -i
     * @param {string} app - application name
     */
    void installCallback(std::string app);
    /**
     * @description: callback function for option -p
     * @param {string} path - installed path
     */
    void pathCallback(std::string path);

private:
    CLISingleton();
    ~CLISingleton()
    {
    }
    CLISingleton(const CLISingleton &copy) = delete;
    CLISingleton &operator=(const CLISingleton &other) = delete;

private:
    CLI::App _app;
    std::string _hostAddr;
    std::string _hostPwd;
    std::string _hostUserName;
    std::string _hostKeyPath;
};
