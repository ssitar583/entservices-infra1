/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2025 RDK Management
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
**/

#include "DobbySpecGenerator.h"
#include "ApplicationConfiguration.h"
#include "UtilsLogging.h"
#include <sys/mount.h>
#include <fstream>
#include <sstream>
#include <bitset>

namespace WPEFramework
{
namespace Plugin
{

namespace 
{
    const std::string WESTEROS_SOCKET_MOUNT_POINT = "/tmp/westeros";
    const std::string RUNTIME_PATH_MOUNT_POINT = "/runtime";

    const int NONHOMEAPP_MEM_LIMIT = 524288000;
    const int NONHOMEAPP_GPUMEM_LIMIT = 367001600;

    const size_t CONTAINER_LOG_CAP = 65536;
}

DobbySpecGenerator::DobbySpecGenerator()
{
    LOGINFO("DobbySpecGenerator()");
}

DobbySpecGenerator::~DobbySpecGenerator()
{
    LOGINFO("~DobbySpecGenerator()");
}

bool DobbySpecGenerator::generate(const ApplicationConfiguration& config, RuntimeConfig& runtimeConfig, string& resultSpec)
{
    LOGINFO("DobbySpecGenerator::generate()");
    resultSpec = "";

    std::ifstream inFile("/tmp/specchange");
    if (inFile.good())
    {
        inFile.open("/tmp/specchange"); //open the input file
        std::stringstream strStream;
        strStream << inFile.rdbuf(); //read the file
        resultSpec = strStream.str(); //str holds the content of the file
        std::cout << resultSpec << "\n"; //you can do anything with the string!!!
        inFile.close();
        JsonObject parameters;
        parameters.FromString(resultSpec.c_str());
        return true;
    }
   
    Json::Value spec;
    spec["version"] = "1.1";
    spec["memLimit"] = getSysMemoryLimit(config);

    Json::Value args(Json::arrayValue);
    args.append(runtimeConfig.command);
    //TODO : What if more args?
    /*
    for (const string& arg : config.mArgs)
        args.append(arg);
    */
    spec["args"] = std::move(args);
    spec["cwd"] = runtimeConfig.appPath;
    // if app uses graphics enable gpu and gpu mem limit
    if (shouldEnableGpu(config))
    {
        Json::Value gpuObj(Json::objectValue);
        gpuObj["enable"] = true;
        gpuObj["memLimit"] = getGPUMemoryLimit();
        spec["gpu"] = std::move(gpuObj);
    }
    spec["restartOnCrash"] = false;

    Json::Value vpuObj;
    vpuObj["enable"] = getVpuEnabled();
    spec["vpu"] = std::move(vpuObj);
    
    Json::Value cpuObj;
    cpuObj["cores"] = getCpuCores();
    spec["cpu"] = std::move(cpuObj);

    Json::Value consoleObj;
    //TODO: limit, path properties are retrieved from app package and need to be populated here
    consoleObj["limit"] = CONTAINER_LOG_CAP;
    consoleObj["path"] = "/tmp/container.log";
    spec["console"] = std::move(consoleObj);

    Json::Value etcObj;
    Json::Value hostsArray(Json::arrayValue);
    Json::Value servicesArray(Json::arrayValue);
    // always add a local host entry
    hostsArray.append("127.0.0.1\tlocalhost");

    // add some common services for the app
    servicesArray.append("ftp\t\t21/tcp");
    servicesArray.append("domain\t\t53/tcp");
    servicesArray.append("domain\t\t53/udp");
    servicesArray.append("http\t\t80/tcp\t\twww");
    servicesArray.append("http\t\t80/udp");
    servicesArray.append("ntp\t\t123/udp");
    servicesArray.append("https\t\t443/tcp");
    servicesArray.append("https\t\t443/udp");
    //TODO Read for mapi capability from package and populate below for every mapi ports
    //servicesArray.append("mapi\t\t" + std::to_string(port) + "/tcp");

    //TODO: Based on soc and if broadcom, populate wayland-client and wayland-egl
    Json::Value preloadsArray(Json::arrayValue);
    etcObj["hosts"] = hostsArray;
    etcObj["services"] = servicesArray;
    etcObj["ld-preload"] = preloadsArray;
    spec["etc"] = std::move(etcObj);

    // TODO: verify if we need EthanLog plugin, it seems it works in conjunction with AI 1.0
    // standard plugins are EthanLog and OCDM, OCDM is enabled if an app does not user rialto nad requires drm
    // TODO: not for Q1 vpu, dbus, seccomp

    // TODO: network field should not be needed, in dobby it just forces adding network plugin
    // verify, if we can safely remove it, as we always add network plugin
    if (runtimeConfig.wanLanAccess)
    {
        spec["network"] = "nat";
    }
    else
    {
        spec["network"] = "private";
    }
    Json::Value userObj;
    userObj["uid"] = config.mUserId;
    userObj["gid"] = config.mGroupId;
    spec["user"] = std::move(userObj);

    spec["plugins"] = populateClassicPlugins(config, runtimeConfig);

    // PENDING BELOW
    // populate rdkPlugins   
    // populate mounts   
    // populate env
    spec["rdkPlugins"] = creteRdkPlugins(config);
    spec["mounts"] = createMounts(config);
    spec["env"] = createEnvVars(config);

    Json::FastWriter writer;
    resultSpec = writer.write(spec);
    LOGINFO("spec: '%s'\n", resultSpec.c_str());
    return true;
}

Json::Value DobbySpecGenerator::createEnvVars(const ApplicationConfiguration& config) const
{
    Json::Value env(Json::arrayValue);
    
    for (const std::string& str : config.mEnvVars)
    {
        env.append(str);
    }

    if (!config.mWesterosSocketPath.empty())
    {
		env.append("XDG_RUNTIME_DIR=/tmp");
		env.append("WAYLAND_DISPLAY=westeros");
    }

    return env;
}

Json::Value DobbySpecGenerator::createMounts(const ApplicationConfiguration& config, RuntimeConfig& runtimeConfig) const
{
    Json::Value mounts(Json::arrayValue);

    if (!config.mAppPath.empty())
    {
        mounts.append(createBindMount(config.mAppPath, runtimeConfig.appPath, MS_BIND | MS_RDONLY | MS_NOSUID | MS_NODEV));
    }

    if (!config.mRuntimePath.empty())
    {
        mounts.append(createBindMount(config.mRuntimePath, runtimeConfig.runtimePath, MS_BIND | MS_RDONLY | MS_NOSUID | MS_NODEV));
    }

    // this is not needed, Dobby adds this mount
    // if (!config.mWesterosSocketPath.empty())
    // {
    //     mounts.append(createBindMount(config.mWesterosSocketPath, WESTEROS_SOCKET_MOUNT_POINT, MS_BIND | MS_NOSUID | MS_NODEV | MS_NOEXEC));
    // }

    // TODO add extra mounts from config

    return mounts;
}

Json::Value DobbySpecGenerator::createBindMount(const std::string& source,
                                                const std::string& destination,
                                                unsigned long mountFlags) const
{
    Json::Value mount(Json::objectValue);

    mount["source"] = source;
    mount["destination"] = destination;
    mount["type"] = "bind";

    Json::Value mountOptions(Json::arrayValue);;

    static const std::vector<std::pair<unsigned long, std::string>> mountFlagsNames =
    {
        {   MS_BIND | MS_REC,   "rbind"         },
        {   MS_BIND,            "bind"          },
        {   MS_SILENT,          "silent"        },
        {   MS_RDONLY,          "ro"            },
        {   MS_SYNCHRONOUS,     "sync"          },
        {   MS_NOSUID,          "nosuid"        },
        {   MS_DIRSYNC,         "dirsync"       },
        {   MS_NODIRATIME,      "nodiratime"    },
        {   MS_RELATIME,        "relatime"      },
        {   MS_NOEXEC,          "noexec"        },
        {   MS_NODEV,           "nodev"         },
        {   MS_NOATIME,         "noatime"       },
        {   MS_STRICTATIME,     "strictatime"   },
    };

    // convert the mount flags to their string equivalents
    for (const auto &entry : mountFlagsNames)
    {
        const unsigned long mountFlag = entry.first;
        if ((mountFlag & mountFlags) == mountFlag)
        {
            mountOptions.append(entry.second);

            // clear the mount flags bit such that we can display a warning
            // if the caller supplies a flag we don't support
            mountFlags &= ~mountFlag;
        }
    }

    // if there was a mount flag we didn't support display a warning
    if (mountFlags != 0)
    {
        LOGWARN("unsupported mount flag(s) 0x%04lx", mountFlags);
    }

    mount["options"] = std::move(mountOptions);
    
    return mount;
}

bool DobbySpecGenerator::shouldEnableGpu(const ApplicationConfiguration& config) const
{
    return !config.mWesterosSocketPath.empty();
}

ssize_t DobbySpecGenerator::getSysMemoryLimit(const ApplicationConfiguration& config, RuntimeConfig& runtimeConfig) const
{
    ssize_t memoryLimit = runtimeConfig.systemMemoryLimit;
    if (memoryLimit <= 0)
    {
        if (runtimeConfig.appType == ApplicationType::INTERACTIVE)
	{
            memoryLimit = NONHOMEAPP_MEM_LIMIT;  
        }
        //TODO: Add other application types
    }
    return memoryLimit;
}

ssize_t DobbySpecGenerator::getGPUMemoryLimit(const ApplicationConfiguration& config, RuntimeConfig& runtimeConfig) const
{
    ssize_t gpuMemoryLimit = runtimeConfig.gpuMemoryLimit;
    if (gpuMemoryLimit <= 0)
    {
        if (runtimeConfig.appType == ApplicationType::INTERACTIVE)
	{
            gpuMemoryLimit = NONHOMEAPP_GPUMEM_LIMIT;
        }
        //TODO: Add other application types
    }
    return gpuMemoryLimit;
}

bool DobbySpecGenerator::getVpuEnabled(const ApplicationConfiguration& config) const
{
    //TODO: To return true, if below conditions are met:
    // check if app is not system app
    // rialto is not enabled
    // vpus blacklist is not having app
    
    return true;
}

std::string DobbySpecGenerator::getCpuCores()
{
    //TODO: Populate cpu bitmask from config (aisettings.json)
    
    //std::bitset<32> cpuSetBitmask = mConfig->getAppsCpuSet();
    //cpuSetBitmask &= ((0x1U << nCores) - 1);

    // check the bitmask so that we have at least one core enabled
    //if (cpuSetBitmask.none())
    //{
    //    cpuSetBitmask = ((0x1U << nCores) - 1);
    //}

    std::bitset<32> cpuSetBitmask = ((0x1U << nCores) - 1);
    const int nCores = std::min(32, get_nprocs());
    // create the json string for the cores to enable
    std::ostringstream coresStream;
    for (int core = 0; core < nCores; core++)
    {
        if (cpuSetBitmask.test(core))
        {
            coresStream << core << ",";
        }
    }

    // get the string and remove trailing ','
    std::string coresStr = coresStream.str();
    if (!coresStr.empty())
        coresStr.pop_back();

    return coreStr;
}

Json::Value DobbySpecGenerator::creteRdkPlugins(const ApplicationConfiguration& config) const
{
    Json::Value rdkPluginsObj(Json::objectValue);

/*
    // add ION memory limiting (ION memory is used for GPU and media buffers)
    pluginsArray.append(createIonMemoryPlugin(true));

    //TODO: Appsservice plugin is not needed for sure?
    
    if (runtimeConfig.thunder)
    {
        pluginsArray.append(createThunderPlugin(true, config, runtimeConfig));
    }

    //TODO: Nat holepuncher, multicast socket plugin, credentialmanager plugin, httpproxy
*/
    if (!config.mPorts.empty())
    {
        rdkPluginsObj["appservicesrdk"] = createAppServiceSDKPlugin(config);
    }

    rdkPluginsObj["minidump"] = createMinidumpPlugin(config);

    // TODO create ionplugin on xione

    rdkPluginsObj["networking"] = createNetworkPlugin(config);

    return rdkPluginsObj;
}

Json::Value DobbySpecGenerator::createMinidumpPlugin(const ApplicationConfiguration& config) const
{
    Json::Value pluginObj(Json::objectValue);

    pluginObj["required"] = false;
    pluginObj["data"]["destinationPath"] = "/opt/minidumps";

    return pluginObj;
}

Json::Value DobbySpecGenerator::createAppServiceSDKPlugin(const ApplicationConfiguration& config, RuntimeConfig& runtimeConfig) const
{
    //TODO: May need to check for any local services, dial port
    Json::Value pluginObj(Json::objectValue);

    pluginObj["required"] = false;

    Json::Value dependencies(Json::arrayValue);
    dependencies.append("networking");

    pluginObj["dependsOn"] = std::move(dependencies);

    if (runtimeConfig.dial)
    {
        //TODO: Need to populate dial ports	    
    }
    Json::Value ports(Json::arrayValue);
    for (auto port : config.mPorts)
        ports.append(port);

    pluginObj["data"]["additionalPorts"] = std::move(ports);

    return pluginObj;
}

Json::Value DobbySpecGenerator::createNetworkPlugin(const ApplicationConfiguration& config) const
{
    Json::Value pluginObj(Json::objectValue);

    pluginObj["required"] = true;

    Json::Value dataObj(Json::objectValue);

    if (config.mWanLanAccess)
    {
        dataObj["type"] = "nat";
        dataObj["dnsmasq"] = true;
    }
    else
    {
        dataObj["type"] = "none";
        dataObj["dnsmasq"] = false;
    }

    dataObj["ipv4"] = true;
    dataObj["ipv6"] = false; // TODO need a switch for this

    pluginObj["data"] = std::move(dataObj);

    return pluginObj;
}

void DobbySpecGenerator::populateClassicPlugins(const ApplicationConfiguration& config, RuntimeConfig& runtimeConfig, Json::Value& spec) const
{
    Json::Value pluginsArray(Json::arrayValue)
    // enable the logging plugin
    pluginsArray.append(createEthanLogPlugin(config, runtimeConfig));
    
    //TODO: if webapp or not needed rialto. how to get these info?
    pluginsArray.append(createOpenCDMPlugin(config, runtimeConfig));

    spec[plugins] = std::move(pluginsArray);
}

Json::Value DobbySpecGenerator::createEthanLogPlugin(const ApplicationConfiguration& config, RuntimeConfig& runtimeConfig) const
{
    Json::Value plugin(Json::objectValue);
    static const Json::StaticString name("name");
    static const Json::StaticString data("data");
    static const Json::StaticString pluginName("EthanLog");

    static const Json::StaticString loglevels("loglevels");
    static const Json::StaticString fatal("fatal");
    static const Json::StaticString error("error");
    static const Json::StaticString warning("warning");
    static const Json::StaticString milestone("milestone");
    static const Json::StaticString info("info");
    static const Json::StaticString debug("debug");

    Json::Value levels(Json::arrayValue);
    //TODO: Read logging mask from package
    /*
    unsigned logMask = package->loggingMask();
    if (logMask & unsigned(IPackage::LogLevel::Default))
        logMask |= mDefaultLoggingMask;

    if (logMask & unsigned(IPackage::LogLevel::Fatal))
        levels.append(fatal);
    if (logMask & unsigned(IPackage::LogLevel::Error))
        levels.append(error);
    if (logMask & unsigned(IPackage::LogLevel::Warning))
        levels.append(warning);
    if (logMask & unsigned(IPackage::LogLevel::Info))
        levels.append(info);
    if (logMask & unsigned(IPackage::LogLevel::Debug))
        levels.append(debug);
    if (logMask & unsigned(IPackage::LogLevel::Milestone))
        levels.append(milestone);
    */
    levels.append(fatal);
    levels.append(error);
    levels.append(warning);
    levels.append(info);
    levels.append(debug);
    levels.append(milestone);

    plugin[name] = pluginName;
    plugin[data][loglevels] = levels;

    return plugin;
}

Json::Value DobbySpecGenerator::createIonMemoryPlugin(bool enable) const
{
    Json::Value plugin(Json::objectValue);
    return plugin;
}

Json::Value DobbySpecGenerator::createThunderPlugin(bool enable, const ApplicationConfiguration& config, RuntimeConfig& runtimeConfig) const
{
    Json::Value plugin(Json::objectValue);
    return plugin;
}

Json::Value DobbySpecGenerator::createOpenCDMPlugin(const ApplicationConfiguration& config, RuntimeConfig& runtimeConfig) const
{
    static const Json::StaticString name("name");
    static const Json::StaticString data("data");
    static const Json::StaticString pluginName("OpenCDM");

    Json::Value plugin(Json::objectValue);
    plugin[name] = pluginName;
    plugin[data] = Json::Value::null;

    return plugin;
}

} // namespace Plugin
} // namespace WPEFramework
