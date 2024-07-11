/*
 * This file is part of the software and assets of HK ZXOUD LIMITED.
 * @copyright (c) HK ZXOUD LIMITED https://www.zxoud.com
 * Author: A529yyds(email:1041389196@qq.com)
 * create: 20240711
 * FilePath: /zcore/head/ExecSingleton.hpp
 * Description: a singleton of sending an execution command to a remote host by Secure Shell
 */
#include "JsonSingleton.hpp"
#include <libssh/libssh.h>

class ExecSingleton
{
public:
    /**
     * @description: a singleton of ExecSingleton
     * @return {ExecSingleton} a singleton
     */
    inline static ExecSingleton &getInstance()
    {
        static ExecSingleton instance;
        return instance;
    }
    /**
     * @description: execute Secure Shell connection
     * @param {json} infomation of host
     * @return {int} connect result 0: success, -1: fail
     */
    int connect(nlohmann::json infos);
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
    bool _bRemote;
    std::string _appName;
    ssh_session _sshSession;

private:
    ExecSingleton();
    ~ExecSingleton()
    {
        // disconnect and release the SSH session
        if (_sshSession)
        {
            try
            {
                ssh_disconnect(_sshSession);
                ssh_free(_sshSession);
            }
            catch (const std::exception &e)
            {
                LOG(ERROR) << e.what() << '\n';
            }
        }
    }
    ExecSingleton(const ExecSingleton &copy) = delete;
    ExecSingleton &operator=(const ExecSingleton &other) = delete;
    /**
     * @description: execute command to host by Secure Shell
     * @param {char} *cmd - command
     * @return {string} execution result
     */
    std::string execCmd2Host(const char *cmd);
    /**
     * @description: execute command to local
     * @param {char} *cmd - command
     * @param {bool} bInPath - indicates whether to enter the specified path
     * @return {string} execution result
     */
    std::string execCmd2Local(const char *cmd, bool bInPath = false);
    /**
     * @description: determines whether a library named lib exists
     * @param {string} lib - library name
     * @return {bool} exist state
     */
    bool isLibExist(std::string lib);
    /**
     * @description: compile the source code with cmake
     * @param {string} url - source URL
     * @param {string} version - source version
     */
    void cmakeOrgCode(std::string url, std::string version = "");
    /**
     * @description: install a component named cpn
     * @param {string} cpn - component name
     */
    void installComponent(std::string cpn);
    /**
     * @description: install component by compiling the source code with cmake
     * @param {string} name - component name
     * @param {string} url - source URL
     * @param {string} version - source version
     */
    void cmakeComponent(std::string name, std::string url, std::string version = "");
};