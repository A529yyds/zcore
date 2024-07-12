#include "ExecSingleton.hpp"

ExecSingleton::ExecSingleton()
{
    _appName = "";
    _bRemote = false;
    _sshSession = ssh_new();
}

std::string ExecSingleton::execCmd2Host(const char *cmd)
{
    std::string result = "";
    ssh_channel sshChannel;
    sshChannel = ssh_channel_new(_sshSession);
    if (sshChannel == NULL)
        return result;
    int rc = ssh_channel_open_session(sshChannel);
    if (rc != SSH_OK)
    {
        ssh_channel_free(sshChannel);
        LOG(ERROR) << "ssh channel open session error, result is " << (rc == SSH_OK ? "OK" : "not OK");
        return result;
    }
    // exec cmd context to aim host for getting remote host info
    rc = ssh_channel_request_exec(sshChannel, cmd);
    if (rc != SSH_OK)
    {
        LOG(ERROR) << "ssh channel exec" << cmd << "error, result is " << (rc == SSH_OK ? "OK" : "not OK");
        ssh_channel_close(sshChannel);
        ssh_channel_free(sshChannel);
        return result;
    }
    char buf[256];
    unsigned int iBytes;
    iBytes = ssh_channel_read(sshChannel, buf, sizeof(buf), 0);
    while (iBytes > 0)
    {
        fwrite(buf, 1, iBytes, stdout);
        LOG(INFO) << "ssh read info " << buf;
        std::string str(buf);
        result += buf;
        iBytes = ssh_channel_read(sshChannel, buf, sizeof(buf), 0);
    }
    ssh_channel_send_eof(sshChannel);
    ssh_channel_close(sshChannel);
    ssh_channel_free(sshChannel);
    return result;
}

std::string ExecSingleton::execCmd2Local(const char *cmd, bool bInPath)
{
    std::array<char, 256> buffer;
    std::string result;
    // "r" indicates that you want to read the output of the command
    // popen: execute a process and provide a flow
    // pclose: deleter
    if (!bInPath)
    {
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
        if (!pipe)
        {
            throw std::runtime_error("popen() failed!");
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        {
            result += buffer.data();
        }
    }
    else
    {
        chdir(cmd);
    }
    return result;
}

bool ExecSingleton::isLibExist(std::string lib)
{
    bool bExist = false;
    if (lib.empty())
        LOG(ERROR) << "lib name is empty";
    else
    {
        std::string cmd = "pacman -Qs " + lib;
        std::string ret = execCmd2Host(cmd.c_str());
        if (!ret.empty())
            bExist = true;
    }
    return bExist;
}

void ExecSingleton::cmakeOrgCode(std::string url, std::string version)
{
    std::string cmd;
    cmd = "cd;";
    cmd += "git clone " + url + ";";
    int startCutPos = url.find_last_of("/") + 1;
    int cutLen = url.size() - startCutPos - std::string(".git").size();
    std::string cName = url.substr(startCutPos, cutLen);
    cmd += "cd " + cName + ";";
    if (!version.empty())
    {
        cmd += "git checkout v" + version + ";";
    }
    cmd += "mkdir build;";
    cmd += "cd build;";
    cmd += "cmake ..;";
    cmd += "make -j8;";
    cmd += "make install;";
    execCmd2Host(cmd.c_str());
}

void ExecSingleton::installComponent(std::string cpn)
{
    if (isLibExist(cpn))
    {
        LOG(ERROR) << cpn << " already exists" << std::endl;
    }
    else
    {
        LOG(INFO) << cpn << " doesn't exist " << std::endl;
        if (cpn == "libzmq")
            cpn = "zeromq";
        std::string cmd = "pacman -S " + cpn + " --noconfirm";
        execCmd2Host(cmd.c_str());
    }
}

void ExecSingleton::cmakeComponent(std::string name, std::string url, std::string version)
{
    std::string cmd = "find / -name " + name + ".pc";
    if (!execCmd2Host(cmd.c_str()).empty())
    {
        LOG(ERROR) << name << " already exists" << std::endl;
    }
    else
    {
        LOG(INFO) << name << " doesn't exist, " << std::endl;
        // pull original code
        cmakeOrgCode(url, version);
        LOG(INFO) << name << " finished installation" << std::endl;
    }
}

void ExecSingleton::installCallback(std::string app)
{
    LOG(INFO) << app << " installCallback, update system..." << std::endl;
    if (app.empty())
        return;
    _appName = app;
    execCmd2Host("pacman -Syu --noconfirm");
    execCmd2Host("pacman -Sy");
    installComponent("cmake --noconfirm");
    installComponent("make --noconfirm");
    installComponent("git --noconfirm");
    for (auto cpn : JsonSingleton::getInstance().getLibsVec(app))
    {
        // install related component of application which name is 'app'
        installComponent(cpn);
    }
    nlohmann::json codes = JsonSingleton::getInstance().getCodesInfos(app);
    for (auto &cpn : codes.items())
    {
        std::string cpnName = cpn.key();
        cmakeComponent(cpn.key(), cpn.value()["url"], cpn.value()["version"]);
    }
    // cmake original program codes from github.com which name is 'app'
    cmakeOrgCode(JsonSingleton::getInstance().getCodesAddress(app));
    LOG(INFO) << app << " finished installation" << std::endl;
}

void ExecSingleton::pathCallback(std::string path)
{
    if (path.empty())
    {
        LOG(ERROR) << "install current path" << std::endl;
    }
    else
    {
        std::string cmd = "mv /root/" + _appName + " " + path;
        execCmd2Host(cmd.c_str());
    }
}

void ExecSingleton::freeSession()
{
    // disconnect and release the SSH session
    if (_sshSession)
    {
        try
        {
            if (ssh_is_connected(_sshSession))
            {
                ssh_disconnect(_sshSession);
            }
            ssh_free(_sshSession);
            _sshSession = nullptr;
        }
        catch (const std::exception &e)
        {
            LOG(ERROR) << e.what() << '\n';
        }
    }
}

int ExecSingleton::connect(nlohmann::json infos)
{
    int rc = -1;
    if (!_sshSession)
    {
        LOG(ERROR) << rc;
        return rc;
    }
    // set remote host ip
    std::string val = infos["ip"];
    if (val.empty())
    {
        _bRemote = false;
        return -1;
    }
    rc = ssh_options_set(_sshSession, SSH_OPTIONS_HOST, val.c_str());
    if (rc < 0)
        LOG(ERROR) << "set host ip error, result is " << rc;
    // set user name
    val = infos["userName"];
    rc = ssh_options_set(_sshSession, SSH_OPTIONS_USER, val.c_str());
    if (rc < 0)
        LOG(ERROR) << "set user name error, result is " << rc;
    // connect to remote host
    rc = -1;
    rc = ssh_connect(_sshSession);
    if (rc != SSH_OK)
    {
        ssh_free(_sshSession);
        _sshSession = nullptr;
    }
    // import private key path
    ssh_key privKey;
    val = infos["path"];
    if (!val.empty())
    {
        rc = ssh_pki_import_privkey_file(val.c_str(), NULL, NULL, NULL, &privKey);
        if (rc != SSH_OK)
        {
            ssh_disconnect(_sshSession);
            ssh_free(_sshSession);
            _sshSession = nullptr;
            LOG(ERROR) << "set private key path error, result is " << rc;
            return -1;
        }
        // identify by private key
        rc = ssh_userauth_publickey(_sshSession, NULL, privKey);
        if (rc != SSH_AUTH_SUCCESS)
        {
            ssh_key_free(privKey);
            ssh_disconnect(_sshSession);
            ssh_free(_sshSession);
            _sshSession = nullptr;
            return -1;
        }
    }
    val = infos["password"];
    if (!val.empty())
        rc = ssh_userauth_password(_sshSession, NULL, val.c_str());
    if (rc != SSH_AUTH_SUCCESS)
    {
        ssh_disconnect(_sshSession);
        ssh_free(_sshSession);
        _sshSession = nullptr;
        LOG(ERROR) << "set user password error, result is " << (rc == SSH_AUTH_SUCCESS ? "success" : "fail");
    }
    // ssh_disconnect(_sshSession);
    return rc;
}
