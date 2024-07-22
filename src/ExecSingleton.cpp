#include <sstream>
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
    {
        LOG(ERROR) << "sshChannel == NULL";
        return result;
    }
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
    LOG(ERROR) << cmd;
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

void ExecSingleton::installYudb()
{
    execCmd2Host("pacman -Syu --noconfirm");
    execCmd2Host("pacman -Sy");
    installComponent("python");
    installComponent("wget");
    installComponent("curl");
    std::string cmd = "wget https://downloads.yugabyte.com/releases/2.21.1.0/yugabyte-2.21.1.0-b271-el8-aarch64.tar.gz;";
    std::string tarCmd = "tar xvfz yugabyte-2.21.1.0-b271-el8-aarch64.tar.gz;";
    tarCmd += "rm -rf yugabyte-2.21.1.0-b271-el8-aarch64.tar.gz;";
    std::string system = execCmd2Host("uname -m");
    if (system.find("x86") == 0)
    {
        LOG(INFO) << system;
        cmd = "wget https://downloads.yugabyte.com/releases/2.21.1.0/yugabyte-2.21.1.0-b271-linux-x86_64.tar.gz;";
        tarCmd = "tar xvfz yugabyte-2.21.1.0-b271-linux-x86_64.tar.gz;";
        tarCmd += "rm -rf yugabyte-2.21.1.0-b271-linux-x86_64.tar.gz;";
    }
    cmd += tarCmd;
    execCmd2Host(cmd.c_str());
    cmd = "cd /root/yugabyte-2.21.1.0/ && ./bin/post_install.sh;";
    execCmd2Host(cmd.c_str());
    cmd = "mkdir /home/centos && mkdir /home/centos/disk1;";
    cmd += "mkdir /home/centos/disk2;";
    execCmd2Host(cmd.c_str());
}

void ExecSingleton::installKeydb()
{
    execCmd2Host("pacman -Syu --noconfirm");
    execCmd2Host("pacman -Sy");
    // compile original code
    installComponent("base-devel");
    installComponent("git");
    std::string cmd = "git config --global http.version HTTP/1.1 && ";
    cmd += "git config --global http.postBuffer 524288000 && ";
    cmd += "git clone  https://github.com/EQ-Alpha/KeyDB.git && ";
    cmd += "cd KeyDB && make && make install";
    execCmd2Host(cmd.c_str());
}

void ExecSingleton::setMasterIp(std::string master)
{
    _masterYuDB = master;
}

void ExecSingleton::yugabyteDeploy(std::string ip, bool bTserver)
{
    LOG(INFO) << (bTserver ? "tserver" : "master") << " ip is " << ip;
    if (_masterYuDB != ip)
        installYudb();
    std::string cmd;
    if (!bTserver)
    {
        // create and start master node
        cmd = std::format("cd /root/yugabyte-2.21.1.0/ && ./bin/yb-master --master_addresses {}:7100 --rpc_bind_addresses {}  --fs_data_dirs \"/home/centos/disk1,/home/centos/disk2\" --placement_cloud aws --placement_region us-west --placement_zone us-west-2a --leader_failure_max_missed_heartbeat_periods 10 >& /home/centos/disk1/yb-master.out &",
                          ip, ip);
        _masterYuDB = ip;
    }
    else
    {
        cmd = std::format("cd /root/yugabyte-2.21.1.0/ && ./bin/yb-tserver\
         --tserver_master_addrs {}:7100\
         --rpc_bind_addresses {}\
         --enable_ysql\
         --pgsql_proxy_bind_address {}:5433\
         --cql_proxy_bind_address {}:9042\
         --fs_data_dirs \"/home/centos/disk1,/home/centos/disk2\"\
         --placement_cloud aws\
         --placement_region us-west\
         --placement_zone us-west-2a\
         --leader_failure_max_missed_heartbeat_periods 10 >& /home/centos/disk1/yb-tserver.out &",
                          _masterYuDB, ip, ip, ip);
    }
    LOG(INFO) << cmd;
    execCmd2Host(cmd.c_str());
    if (!bTserver)
        yugabyteReplica();
}

void ExecSingleton::yugabyteReplica()
{
    /*
     * set master stategy:
     * ./bin/yb-admin: a YugabyteDB manage tool
     * --master_addresses: specifies the address and port of the master node
     * modify_placement_info: example Modify the cluster placement policy
     * this line defines the new placement policy. Each entry is composed of cloud providers, regions, and availability zones
     * this parameter defines the number of copies required for each placement area
     */
    std::string cmd = std::format("cd /root/yugabyte-2.21.1.0/ && ./bin/yb-admin \
    --master_addresses {}:7100 \
    modify_placement_info \
    aws.us-west.us-west-2a, aws.us-east-1.us-east-1a, aws.ap-northeast-1.ap-northeast-1a \
    3",
                                  _masterYuDB);
    execCmd2Host(cmd.c_str());
}

void ExecSingleton::yugabyteDeploy(std::string masterIp, std::vector<std::string> tserverIps)
{
    LOG(INFO) << masterIp << " yugabyteDeploy start deploy";
    execCmd2Host("pacman -Syu --noconfirm");
    execCmd2Host("pacman -Sy");
    installComponent("python");
    installComponent("wget");
    installComponent("curl");
    std::string cmd = "wget https://downloads.yugabyte.com/releases/2.21.1.0/yugabyte-2.21.1.0-b271-el8-aarch64.tar.gz;";
    std::string tarCmd = "tar xvfz yugabyte-2.21.1.0-b271-el8-aarch64.tar.gz;";
    std::string system = execCmd2Host("uname -m");
    LOG(INFO) << masterIp << " system is " << system;
    if (system.find("x86") == 0)
    {
        LOG(INFO) << system;
        cmd = "wget https://downloads.yugabyte.com/releases/2.21.1.0/yugabyte-2.21.1.0-b271-linux-x86_64.tar.gz;";
        tarCmd = "tar xvfz yugabyte-2.21.1.0-b271-linux-x86_64.tar.gz;";
    }
    cmd += tarCmd;
    execCmd2Host(cmd.c_str());
    cmd = "cd /root/yugabyte-2.21.1.0/ && ./bin/post_install.sh;";
    execCmd2Host(cmd.c_str());
    cmd = "mkdir /home/centos && mkdir /home/centos/disk1;";
    cmd += "mkdir /home/centos/disk2;";
    execCmd2Host(cmd.c_str());
    LOG(INFO) << "yugabyteDeploy create and start master node";
    // create and start master node
    cmd = std::format("cd /root/yugabyte-2.21.1.0/ && ./bin/yb-master --master_addresses {}:7100 --rpc_bind_addresses {}  --fs_data_dirs \" /home/centos/disk1, /home/centos/disk2\" --placement_cloud aws --placement_region us-west --placement_zone us-west-2a --leader_failure_max_missed_heartbeat_periods 10 >& /home/centos/disk1/yb-master.out &",
                      masterIp, masterIp);
    execCmd2Host(cmd.c_str());
    // create and start tserver nodes
    LOG(INFO) << "yugabyteDeploy create and start tserver node";
    for (auto &ip : tserverIps)
    {
        LOG(INFO) << "start tserver node ip is " << ip;
        cmd = std::format("cd /root/yugabyte-2.21.1.0/ && ./bin/yb-tserver\
         --tserver_master_addrs {}:7100\
         --rpc_bind_addresses {}\
         --enable_ysql\
         --pgsql_proxy_bind_address {}:5433\
         --cql_proxy_bind_address {}:9042\
         --fs_data_dirs \"/home/centos/disk1,/home/centos/disk2\"\
         --placement_cloud aws\
         --placement_region us-west\
         --placement_zone us-west-2a\
         --leader_failure_max_missed_heartbeat_periods 10 >& /home/centos/disk1/yb-tserver.out &",
                          masterIp, ip, ip, ip);
        execCmd2Host(cmd.c_str());
    }
    /*
     * set master stategy:
     * ./bin/yb-admin: a YugabyteDB manage tool
     * --master_addresses: specifies the address and port of the master node
     * modify_placement_info: example Modify the cluster placement policy
     * this line defines the new placement policy. Each entry is composed of cloud providers, regions, and availability zones
     * this parameter defines the number of copies required for each placement area
     */
    cmd = std::format("cd /root/yugabyte-2.21.1.0/ && ./bin/yb-admin \
    --master_addresses {}:7100 \
    modify_placement_info \
    aws.us-west.us-west-2a, aws.us-east-1.us-east-1a, aws.ap-northeast-1.ap-northeast-1a \
    3",
                      masterIp);
    execCmd2Host(cmd.c_str());
    LOG(INFO) << "yugabyteDB deploy finished";
}

void ExecSingleton::keydbDeploy(std::string port)
{
    installKeydb();
    // open nodes
    std::string cmd = std::format("keydb-server --port {}\
         --cluster-enabled yes \
         --cluster-config-file node01.conf \
         --cluster-node-timeout 5000 \
         --appendonly yes \
         --protected-mode no \
         --daemonize yes",
                                  port);
    LOG(INFO) << cmd;
    execCmd2Host(cmd.c_str());
}

void ExecSingleton::keydbClusterSet(std::vector<std::string> ipPorts)
{
    std::string cmd = "keydb-cli --cluster create --cluster-yes";
    for (auto ipPort : ipPorts)
    {
        cmd += " " + ipPort;
    }
    execCmd2Host(cmd.c_str());
}

void ExecSingleton::installCallback(std::string app)
{
    LOG(INFO) << app << " installCallback, update system..." << std::endl;
    if (app.empty())
        return;
    _appName = app;
    execCmd2Host("pacman -Syu --noconfirm");
    execCmd2Host("pacman -Sy");
    installComponent("cmake");
    installComponent("make");
    installComponent("git");
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
        LOG(INFO) << "install current path" << std::endl;
    }
    else
    {
        std::string cmd = "mv /root/" + _appName + " " + path;
        execCmd2Host(cmd.c_str());
    }
}

void ExecSingleton::uninstall(std::string name, bool bSCodes)
{
    std::string cmd;
    if (!execCmd2Host("find / -wholename \"/var/lib/pacman/db.lck\"").empty())
    {
        cmd = "sudo rm /var/lib/pacman/db.lck";
    }
    cmd = "sudo pacman -Syu --noconfirm && pacman -Rs " + name + " --noconfirm";

    if (bSCodes)
    {
        cmd = std::format("find / -name \"{}\"", name);
        std::string reply = execCmd2Host(cmd.c_str());
        std::istringstream iss(reply);
        std::vector<std::string> paths;
        std::string line;
        while (std::getline(iss, line))
        {
            paths.push_back(line);
            LOG(INFO) << cmd;
        }
        for (const auto &path : paths)
        {
            cmd = "rm -rf " + path + ";";
            LOG(INFO) << cmd;
            execCmd2Host(cmd.c_str());
        }
    }
    else
    {
        LOG(INFO) << cmd;
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
    if (ssh_is_connected(_sshSession))
    {
        LOG(WARNING) << "ssh session is connected";
        return -1;
    }
    // set remote host ip
    std::string val = infos["ip"];
    if (val.empty())
    {
        _bRemote = false;
        LOG(WARNING) << "host ip is empty";
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
        freeSession();
    }
    // import private key path
    ssh_key privKey;
    val = infos["path"];
    if (!val.empty())
    {
        LOG(WARNING) << "match key file path " << val;
        rc = ssh_pki_import_privkey_file(val.c_str(), NULL, NULL, NULL, &privKey);
        if (rc != SSH_OK)
        {
            freeSession();
            LOG(ERROR) << "set private key path error, result is " << rc;
            return -1;
        }
        // identify by private key
        rc = ssh_userauth_publickey(_sshSession, NULL, privKey);
        if (rc != SSH_AUTH_SUCCESS)
        {
            ssh_key_free(privKey);
            freeSession();
            LOG(ERROR) << "ssh_userauth_publickey error, result is " << rc;
            return -1;
        }
    }
    val = infos["password"];
    if (!val.empty())
        rc = ssh_userauth_password(_sshSession, NULL, val.c_str());
    if (rc != SSH_AUTH_SUCCESS)
    {
        freeSession();
        LOG(ERROR) << "set user password error, result is " << (rc == SSH_AUTH_SUCCESS ? "success" : "fail");
    }
    return rc;
}
