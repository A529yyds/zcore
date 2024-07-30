#include "Zcore.hpp"
#include "ExecSingleton.hpp"

Zcore::Zcore()
{
    _app.description("Zcore start");
    _app.footer("Zcore finished");
    _dbTServers.clear();
}

Zcore::~Zcore()
{
    ExecSingleton::getInstance().freeSession();
}

int Zcore::setCmdsParse(int argc, char **argv)
{
    std::string path, appName, keydbIpPort = "";
    _app.add_option("-a,--address", _hostIps, "connect host address for installation")->required();
    _app.add_option("-w,--password", _hostPwds, "connect host password for installation");
    _app.add_option("-k,--keypath", _hostKeyPaths, "connect host API key path for installation");
    _app.add_option("-u,--username", _hostUserNames, "connect host user name for installation")->required();

    std::function<void(std::string)> funcApp = std::bind(&Zcore::installCallback, this, std::placeholders::_1);
    _app.add_option("-i,--installapp", appName, "install application")->each(funcApp);
    std::function<void(std::string)> funcPath = std::bind(&Zcore::pathCallback, this, std::placeholders::_1);
    _app.add_option("-p,--path", path, "select application install path")->each(funcPath);
    std::function<void(std::string)> funcUninstall = std::bind(&Zcore::uninstallCallback, this, std::placeholders::_1);
    _app.add_option("--uninstall", appName, "uninstall application or library")->each(funcUninstall);
    std::function<void(std::string)> funcRemove = std::bind(&Zcore::removeCallback, this, std::placeholders::_1);
    _app.add_option("--remove", appName, "uninstall application or library from compiled by original code")->each(funcRemove);

    bool flag = false;
    std::string master = "";
    std::function<void(std::string)> funcYudb = std::bind(&Zcore::yudbMasterDeploy, this, std::placeholders::_1);
    _app.add_flag("--ymaster", flag, "deploy YugabyteDB and create in master node")->each(funcYudb);
    funcYudb = std::bind(&Zcore::yudbTServerDeploy, this, std::placeholders::_1);
    _app.add_option("--tserver", master, "deploy YugabyteDB and create in tserver node, and must need to input master IP")->each(funcYudb);
    funcYudb = std::bind(&Zcore::addMaster2Cluster, this, std::placeholders::_1);
    _app.add_flag("--addmaster", master, "add master to cluster")->each(funcYudb);
    funcYudb = std::bind(&Zcore::removeMasterFromCluster, this, std::placeholders::_1);
    _app.add_flag("--rmvmaster", master, "remove master to cluster")->each(funcYudb);
    funcYudb = std::bind(&Zcore::yudbDirectDeploy, this, std::placeholders::_1);
    _app.add_option("--ybdeploy", master, "remove master to cluster")->each(funcYudb);

    flag = false;
    std::function<void(std::string)> funcKeydb = std::bind(&Zcore::keydbDeploy, this, std::placeholders::_1);
    _app.add_option("--keydb", keydbIpPort, "deploy KeyDB and start node one by one")->each(funcKeydb);
    _app.add_option("--keydbclusters", _keydbClusters, "run on a host that has KeyDB nodes enabled, \
    place all KeyDB nodes in the same cluster, \
    and enter all the IP:port that enable the cluster");
    std::function<void(std::string)> funcKeydbCluster = std::bind(&Zcore::keydbClusterSet, this, std::placeholders::_1);
    _app.add_flag("-d", flag, "the command deployed in the same cluster starts")->each(funcKeydbCluster);
    CLI11_PARSE(_app, argc, argv);
    return 0;
}

void Zcore::installCallback(std::string app)
{
    connectHost();
    ExecSingleton::getInstance().installCallback(app);
}

void Zcore::uninstallCallback(std::string name)
{
    connectHost();
    ExecSingleton::getInstance().uninstall(name);
}

void Zcore::removeCallback(std::string name)
{
    connectHost();
    ExecSingleton::getInstance().uninstall(name, true);
}

void Zcore::pathCallback(std::string path)
{
    ExecSingleton::getInstance().pathCallback(path);
    // the session needs to be released because of pathCallback execution after install
}

void Zcore::yudbMasterDeploy(std::string master)
{
    connectHost();
    ExecSingleton::getInstance().yugabyteDeploy(_hostIps[0]);
    ExecSingleton::getInstance().freeSession();
}

void Zcore::yudbTServerDeploy(std::string master)
{
    LOG(INFO) << "yudb TServer Deploy";
    connectHost();
    ExecSingleton::getInstance().setMasterIp(master);
    ExecSingleton::getInstance().yugabyteDeploy(_hostIps[0], true);
    ExecSingleton::getInstance().freeSession();
}

void Zcore::addMaster2Cluster(std::string master)
{
    connectHost();
    ExecSingleton::getInstance().addMaster2Cluster(_hostIps[0]);
    ExecSingleton::getInstance().freeSession();
}

void Zcore::removeMasterFromCluster(std::string master)
{
    connectHost();
    ExecSingleton::getInstance().removeMasterFromCluster(_hostIps[0]);
    ExecSingleton::getInstance().freeSession();
}

void Zcore::yudbDirectDeploy(std::string master)
{
    connectHost();
    ExecSingleton::getInstance().yudbDirectDeploy(master);
}

void Zcore::keydbDeploy(std::string port)
{
    connectHost();
    ExecSingleton::getInstance().keydbDeploy(port);
    ExecSingleton::getInstance().freeSession();
}

void Zcore::keydbClusterSet(std::string port)
{
    connectHost();
    ExecSingleton::getInstance().keydbClusterSet(_keydbClusters);
    ExecSingleton::getInstance().freeSession();
}

void Zcore::connectHost()
{
    std::vector<nlohmann::json> hostInfos;
    nlohmann::json hostInfo;
    for (int i = 0; i < _hostIps.size(); i++)
    {
        hostInfo["ip"] = _hostIps[i];
        hostInfo["password"] = i < _hostPwds.size() ? _hostPwds[i] : "";
        hostInfo["path"] = i < _hostKeyPaths.size() ? _hostKeyPaths[i] : "";
        hostInfo["userName"] = i < _hostUserNames.size() ? _hostUserNames[i] : "";
        LOG(ERROR) << hostInfo;
        hostInfos.push_back(hostInfo);
    }
    ExecSingleton::getInstance().setHostInfos(hostInfos);
    if (hostInfos.size() <= 1)
    {
        int ret = ExecSingleton::getInstance().connect(hostInfos.size() < 1 ? "" : hostInfos[0]);
        LOG(INFO) << "connect status " << ret;
    }
}
