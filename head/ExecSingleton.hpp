/*
 * This file is part of the software and assets of HK ZXOUD LIMITED.
 * @copyright (c) HK ZXOUD LIMITED https://www.zxoud.com
 * Author: A529yyds(email:1041389196@qq.com)
 * create: 20240711
 * FilePath: /zcore/head/ExecSingleton.hpp
 * Description: a singleton of sending an execution command to a remote host by Secure Shell
 */

#pragma once
#include "JsonSingleton.hpp"
#include <libssh/libssh.h>

class ExecSingleton
{
public:
    /**
     * @description: get a ExecSingleton instance
     * @return {ExecSingleton} instance
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
    /**
     * @description: deploy yugabyteDB
     * @param {string} masterIp - master ip
     * @param {vector<std::string>} tservers' ip
     * @return {*}
     */
    void setMasterIp(std::string master);
    /**
     * @description: deploy YugabyteDB dependence and start YugabyteDB server
     * @param {string} ip - host ip
     * @param {bool} bTserver - select whether to deploy tserver node
     */
    void yugabyteDeploy(std::string ip, bool bTserver = false);
    /**
     * @description: deploy YugabyteDB dependence and start YugabyteDB server in a host
     * @param {string} masterIp - deploy master ip
     * @param {vector<std::string>} tserverIps - tservers ip
     */
    void yugabyteDeploy(std::string masterIp, std::vector<std::string> tserverIps);
    /**
     * @description: deploy KeyDB master node
     * @param {string} port - deploy port
     */
    void keydbDeploy(std::string port);
    /**
     * @description: deploy KeyDB cluster node
     * @param {vector<std::string>} ipPort - configure port
     */
    void keydbClusterSet(std::vector<std::string> ipPort);
    /**
     * @description: free ssh_session source
     */
    void freeSession();

private:
    bool _bRemote;
    std::string _appName;
    std::string _masterYuDB;
    ssh_session _sshSession;

private:
    ExecSingleton();
    ~ExecSingleton()
    {
    }
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
    /**
     * @description: install YugabyteDB dependence libraries in archlinux
     */
    void installYudb();
    /**
     * @description: deploy YugabyteDB replica placement policy
     */
    void yugabyteReplica();
    /**
     * @description: install KeyDB dependence libraries  in archlinux
     */
    void installKeydb();
};