#include "CLISingleton.hpp"
#include "ExecSingleton.hpp"

CLISingleton::CLISingleton()
{
    _app.description("Zcore start");
    _app.footer("Zcore finished");
    _hostAddr = "";
    _hostKeyPath = "";
    _hostPwd = "";
    _hostUserName = "";
}

int CLISingleton::setCmdsParse(int argc, char **argv)
{
    std::string path, appName = "";
    std::function<void(std::string)> funcApp = std::bind(&CLISingleton::installCallback, this, std::placeholders::_1);
    std::function<void(std::string)> funcPath = std::bind(&CLISingleton::pathCallback, this, std::placeholders::_1);
    _app.add_option("-a,--address", _hostAddr, "connect host address for installation")->required();
    _app.add_option("-w,--password", _hostPwd, "connect host password for installation");
    _app.add_option("-k,--keypath", _hostKeyPath, "connect host API key path for installation");
    _app.add_option("-u,--username", _hostUserName, "connect host user name for installation")->required();
    _app.add_option("-i,--installapp", appName, "install application")->each(funcApp);
    _app.add_option("-p,--path", path, "select application install path")->each(funcPath);
    // nlohmann::json cntInfos;
    // cntInfos["ip"] = "172.17.0.3";
    // cntInfos["path"] = "";
    // cntInfos["password"] = "123";
    // cntInfos["userName"] = "root";
    // ExecSingleton::getInstance().connect(cntInfos);
    CLI11_PARSE(_app, argc, argv);
    return 0;
}

void CLISingleton::installCallback(std::string app)
{
    nlohmann::json cntInfos;
    cntInfos["ip"] = _hostAddr;
    cntInfos["path"] = _hostKeyPath;
    cntInfos["password"] = _hostPwd;
    cntInfos["userName"] = _hostUserName;
    ExecSingleton::getInstance().connect(cntInfos);
    ExecSingleton::getInstance().installCallback(app);
}

void CLISingleton::pathCallback(std::string path)
{
    ExecSingleton::getInstance().pathCallback(path);
}