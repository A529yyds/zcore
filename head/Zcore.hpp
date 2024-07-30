/*
 * This file is part of the software and assets of HK ZXOUD LIMITED.
 * @copyright (c) HK ZXOUD LIMITED https://www.zxoud.com
 * Author: A529yyds(email:1041389196@qq.com)
 * create: 20240711
 * FilePath: /zcore/head/Zcore.hpp
 * Description: command-line tools developed using CLI11
 */

#pragma once
#include "CLI/CLI.hpp"

class CmdExecution;

class Zcore
{
public:
    Zcore();
    ~Zcore();
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
     * @description: uninstall application named 'name'
     * @param {string} name - applicaion name
     */
    void uninstallCallback(std::string name);
    /**
     * @description: remove files of original codes
     * @param {string} name - original codes
     */
    void removeCallback(std::string name);
    /**
     * @description: callback function for option -p
     * @param {string} path - installed path
     */
    void pathCallback(std::string path);
    /**
     * @description: deploy YugabyteDB master dependence and start server
     * @param {string} master - master host ip
     */
    void yudbMasterDeploy(std::string master);
    /**
     * @description: deploy YugabyteDB tserver dependence and start server
     * @param {string} master - master host ip
     */
    void yudbTServerDeploy(std::string master);
    /**
     * @description: add master node to cluster
     * @param {string} master - master IP
     */
    void addMaster2Cluster(std::string master);
    /**
     * @description: remove master node to cluster
     * @param {string} master - master IP
     */
    void removeMasterFromCluster(std::string master);
    /**
     * @description: deploy over one master node in same cluster directly
     * @param {string} masterIp - master IP
     */
    void yudbDirectDeploy(std::string master);
    /**
     * @description: deploy KeyDB node dependence and start server
     * @param {string} port - configure port
     */
    void keydbDeploy(std::string port);
    /**
     * @description:  put the nodes of keydb into a cluster
     * @param {string} port - insignificance value
     */
    void keydbClusterSet(std::string port);
    /**
     * @description: connect host ip
     */
    void connectHost();

private:
    CLI::App _app;
    std::vector<std::string> _hostIps;
    std::vector<std::string> _hostPwds;
    std::vector<std::string> _hostKeyPaths;
    std::vector<std::string> _hostUserNames;
    std::vector<std::string> _dbTServers;
    std::vector<std::string> _keydbClusters;
};
