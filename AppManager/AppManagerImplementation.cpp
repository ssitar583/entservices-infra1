/*
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2024 RDK Management
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
*/

#include "AppManagerImplementation.h"

namespace WPEFramework {
namespace Plugin {

SERVICE_REGISTRATION(AppManagerImplementation, 1, 0);
AppManagerImplementation* AppManagerImplementation::_instance = nullptr;

AppManagerImplementation::AppManagerImplementation()
: mAdminLock()
, mAppManagerNotification()
, mLifecycleInterfaceConnector(nullptr)
, mPersistentStoreRemoteStoreObject(nullptr)
, mPackageManagerHandlerObject(nullptr)
, mPackageManagerInstallerObject(nullptr)
, mCurrentservice(nullptr)
{
    LOGINFO("Create AppManagerImplementation Instance");
    if (nullptr == AppManagerImplementation::_instance)
    {
        AppManagerImplementation::_instance = this;
    }
}

AppManagerImplementation* AppManagerImplementation::getInstance()
{
    return _instance;
}

AppManagerImplementation::~AppManagerImplementation()
{
    LOGINFO("Delete AppManagerImplementation Instance");
    _instance = nullptr;
    if (nullptr != mLifecycleInterfaceConnector)
    {
        mLifecycleInterfaceConnector->releaseLifecycleManagerRemoteObject();
        delete mLifecycleInterfaceConnector;
        mLifecycleInterfaceConnector = nullptr;
    }
    releasePersistentStoreRemoteStoreObject();
    releasePackageManagerObject();
    if (nullptr != mCurrentservice)
    {
       mCurrentservice->Release();
       mCurrentservice = nullptr;
    }
}

/**
 * Register a notification callback
 */
Core::hresult AppManagerImplementation::Register(Exchange::IAppManager::INotification *notification)
{
    ASSERT (nullptr != notification);

    mAdminLock.Lock();

    if (std::find(mAppManagerNotification.begin(), mAppManagerNotification.end(), notification) == mAppManagerNotification.end())
    {
        LOGINFO("Register notification");
        mAppManagerNotification.push_back(notification);
        notification->AddRef();
    }

    mAdminLock.Unlock();

    return Core::ERROR_NONE;
}

/**
 * Unregister a notification callback
 */
Core::hresult AppManagerImplementation::Unregister(Exchange::IAppManager::INotification *notification )
{
    Core::hresult status = Core::ERROR_GENERAL;

    ASSERT (nullptr != notification);

    mAdminLock.Lock();

    auto itr = std::find(mAppManagerNotification.begin(), mAppManagerNotification.end(), notification);
    if (itr != mAppManagerNotification.end())
    {
        (*itr)->Release();
        LOGINFO("Unregister notification");
        mAppManagerNotification.erase(itr);
        status = Core::ERROR_NONE;
    }
    else
    {
        LOGERR("notification not found");
    }

    mAdminLock.Unlock();

    return status;
}

void AppManagerImplementation::dispatchEvent(EventNames event, const JsonObject &params)
{
    Core::IWorkerPool::Instance().Submit(Job::Create(this, event, params));
}

void AppManagerImplementation::Dispatch(EventNames event, const JsonObject params)
{
    LOGINFO("Dispatch Entered");
}

uint32_t AppManagerImplementation::Configure(PluginHost::IShell* service)
{
    uint32_t result = Core::ERROR_GENERAL;

    if (service != nullptr)
    {
        mCurrentservice = service;
        mCurrentservice->AddRef();

        if (nullptr == (mLifecycleInterfaceConnector = new LifecycleInterfaceConnector(mCurrentservice)))
        {
            LOGERR("Failed to create LifecycleInterfaceConnector");
        }
        else if (Core::ERROR_NONE != mLifecycleInterfaceConnector->createLifecycleManagerRemoteObject())
        {
            LOGERR("Failed to create LifecycleInterfaceConnector");
        }
        else
        {
            LOGINFO("created LifecycleManagerRemoteObject");
        }

        if (Core::ERROR_NONE != createPersistentStoreRemoteStoreObject())
        {
            LOGERR("Failed to create createPersistentStoreRemoteStoreObject");
        }
        else
        {
            LOGINFO("created createPersistentStoreRemoteStoreObject");
        }

        if (Core::ERROR_NONE != createPackageManagerObject())
        {
            LOGERR("Failed to create createPackageManagerObject");
        }
        else
        {
            LOGINFO("created createPackageManagerObject");
        }

        result = Core::ERROR_NONE;
    }
    else
    {
        LOGERR("service is null \n");
    }

    return result;
}

Core::hresult AppManagerImplementation::createPersistentStoreRemoteStoreObject()
{
    Core::hresult status = Core::ERROR_GENERAL;

    if (nullptr == mCurrentservice)
    {
        LOGERR("mCurrentservice is null \n");
    }
    else if (nullptr == (mPersistentStoreRemoteStoreObject = mCurrentservice->QueryInterfaceByCallsign<WPEFramework::Exchange::IStore2>("org.rdk.PersistentStore")))
    {
        LOGERR("mPersistentStoreRemoteStoreObject is null \n");
    }
    else
    {
        LOGINFO("created PersistentStore Object\n");
        status = Core::ERROR_NONE;
    }
    return status;
}

void AppManagerImplementation::releasePersistentStoreRemoteStoreObject()
{
    ASSERT(nullptr != mPersistentStoreRemoteStoreObject);
    if(mPersistentStoreRemoteStoreObject)
    {
        mPersistentStoreRemoteStoreObject->Release();
        mPersistentStoreRemoteStoreObject = nullptr;
    }
}

Core::hresult AppManagerImplementation::createPackageManagerObject()
{
    Core::hresult status = Core::ERROR_GENERAL;

    if (nullptr == mCurrentservice)
    {
        LOGERR("mCurrentservice is null \n");
    }
    else if (nullptr == (mPackageManagerHandlerObject = mCurrentservice->QueryInterfaceByCallsign<WPEFramework::Exchange::IPackageHandler>("org.rdk.PackageManagerRDKEMS")))
    {
        LOGERR("mPackageManagerHandlerObject is null \n");
    }
    else if (nullptr == (mPackageManagerInstallerObject = mCurrentservice->QueryInterfaceByCallsign<WPEFramework::Exchange::IPackageInstaller>("org.rdk.PackageManagerRDKEMS")))
    {
        LOGERR("mPackageManagerInstallerObject is null \n");
    }
    else
    {
        LOGINFO("created PackageManager Object\n");
        status = Core::ERROR_NONE;
    }
    return status;
}

void AppManagerImplementation::releasePackageManagerObject()
{
    ASSERT(nullptr != mPackageManagerHandlerObject);
    if(mPackageManagerHandlerObject)
    {
        mPackageManagerHandlerObject->Release();
        mPackageManagerHandlerObject = nullptr;
    }
    ASSERT(nullptr != mPackageManagerInstallerObject);
    if(mPackageManagerInstallerObject)
    {
        mPackageManagerInstallerObject->Release();
        mPackageManagerInstallerObject = nullptr;
    }
}

bool AppManagerImplementation::createOrUpdatePackageInfoByAppId(const string& appId, PackageInfo &packageData)
{
    bool result = false;
    if( packageData.version.empty() || packageData.unpackedPath.empty())
    {
        LOGWARN("AppId[%s] unpacked path is empty! or version is empty ", appId.c_str());
    }
    else
    {
        /* Check if the appId exists in the map */
        auto it = mAppInfo.find(appId);
        if (it != mAppInfo.end())
        {
            /* Update existing entry (PackageInfo inside LoadedAppInfo) */
            it->second.packageInfo.version         = packageData.version;
            it->second.packageInfo.lockId          = packageData.lockId;
            it->second.packageInfo.unpackedPath    = packageData.unpackedPath;
            it->second.packageInfo.configMetadata  = packageData.configMetadata;
            it->second.packageInfo.appMetadata     = packageData.appMetadata;

            LOGINFO("Existing package entry updated for appId: %s " \
                    "version: %s lockId: %d unpackedPath: %s configMetadata: %s appMetadata: %s",
                    appId.c_str(), it->second.packageInfo.version.c_str(),
                    it->second.packageInfo.lockId, it->second.packageInfo.unpackedPath.c_str(),
                    it->second.packageInfo.configMetadata.c_str(), it->second.packageInfo.appMetadata.c_str());
        }
        else
        {
            /* Create new entry (PackageInfo inside LoadedAppInfo) */
            mAppInfo[appId].packageInfo.version         = packageData.version;
            mAppInfo[appId].packageInfo.lockId          = packageData.lockId;
            mAppInfo[appId].packageInfo.unpackedPath    = packageData.unpackedPath;
            mAppInfo[appId].packageInfo.configMetadata  = packageData.configMetadata;
            mAppInfo[appId].packageInfo.appMetadata     = packageData.appMetadata;

            LOGINFO("Created new package entry for appId: %s " \
                    "version: %s lockId: %d unpackedPath: %s configMetadata: %s appMetadata: %s",
                    appId.c_str(), mAppInfo[appId].packageInfo.version.c_str(),
                    mAppInfo[appId].packageInfo.lockId, mAppInfo[appId].packageInfo.unpackedPath.c_str(),
                    mAppInfo[appId].packageInfo.configMetadata.c_str(), mAppInfo[appId].packageInfo.appMetadata.c_str());
        }
        result = true;
    }

    return result;
}


bool AppManagerImplementation::fetchPackageInfoByAppId(const string& appId, PackageInfo &packageData)
{
        bool result = false;
        auto it = mAppInfo.find(appId);
        if (it != mAppInfo.end())
        {
            packageData.version           = it->second.packageInfo.version;
            packageData.lockId            = it->second.packageInfo.lockId;
            packageData.unpackedPath      = it->second.packageInfo.unpackedPath;
            packageData.configMetadata    = it->second.packageInfo.configMetadata;
            packageData.appMetadata       = it->second.packageInfo.appMetadata;
            LOGINFO("Fetching package entry updated for appId: %s " \
                    "version: %s lockId: %d unpackedPath: %s configMetadata: %s appMetadata: %s",
                    appId.c_str(), packageData.version.c_str(), packageData.lockId, packageData.unpackedPath.c_str(), packageData.configMetadata.c_str(), packageData.appMetadata.c_str());
            result = true;
        }
        return result;
}

bool AppManagerImplementation::removeAppInfoByAppId(const string &appId)
{
    bool result = false;

    /* Check if appId StorageInfo is found, erase it */
    auto it = mAppInfo.find(appId);
    if (it != mAppInfo.end())
    {
        LOGINFO("Existing package entry updated for appId: %s " \
                "version: %s lockId: %d unpackedPath: %s configMetadata: %s appMetadata: %s",
                appId.c_str(), it->second.packageInfo.version.c_str(), it->second.packageInfo.lockId, it->second.packageInfo.unpackedPath.c_str(), it->second.packageInfo.configMetadata.c_str(), it->second.packageInfo.appMetadata.c_str());
        mAppInfo.erase(appId);
        result = true;
    } 
    else
    {
        LOGWARN("AppId[%s] PackageInfo not found", appId.c_str());
    }
    return result;
}

Core::hresult AppManagerImplementation::packageLock(const string& appId, PackageInfo &packageData, Exchange::IPackageHandler::LockReason lockReason)
{
        Core::hresult status = Core::ERROR_GENERAL;
        bool result = false;
        Exchange::IPackageInstaller::IPackageIterator* packages;
        bool loaded = true;

        if (nullptr != mLifecycleInterfaceConnector)
        {
            status = mLifecycleInterfaceConnector->isAppLoaded(appId, loaded);
        }
        if(!loaded && status == Core::ERROR_NONE)
        {
            if (nullptr != mPackageManagerInstallerObject)
            {
                status = mPackageManagerInstallerObject->ListPackages(packages);
                if (status == Core::ERROR_NONE)
                {
                    while (packages != nullptr )
                    {
                        WPEFramework::Exchange::IPackageInstaller::Package package;
                        if (!packages->Next(package))
                        {
                            LOGERR("Failed to get next package");
                            break;
                        }

                        /* Check if the packageId matches the provided appId */
                        if (!package.packageId.empty() && package.packageId == appId)
                        {
                            /* If it matches, store the version */
                            packageData.version = std::string(package.version);
                            break;  /* Exit the loop after finding the matching package */
                        }
                    }
                    if ((nullptr != mPackageManagerHandlerObject) && (!packageData.version.empty()))
                    {
                        /*status = mPackageManagerHandlerObject->Lock(appId, packageData.version, lockReason, packageData.lockId, packageData.unpackedPath, packageData.configMetadata, packageData.appMetadata );
                        if(status == Core::ERROR_NONE)
                        {
                            LOGINFO("Fetching package entry updated for appId: %s " \
                            "version: %s lockId: %d unpackedPath: %s configMetadata: %s appMetadata: %s",
                            appId.c_str(), packageData.version.c_str(), packageData.lockId, packageData.unpackedPath.c_str(), packageData.configMetadata.c_str(), packageData.appMetadata.c_str());
                            result = createOrUpdatePackageInfoByAppId(appId, packageData);
                            if (false == result)
                            {
                                LOGERR("Failed to createOrUpdate the PackageInfo");
                                status = Core::ERROR_GENERAL;
                            }
                        }
                        else*/
                        {
                            LOGERR("Failed to PackageManager Lock");
                        }
                    }
                    else
                    {
                        LOGERR("PackageManager handler is %s", (nullptr != mPackageManagerHandlerObject) ? "valid, but package version is empty" : "null");
                        status = Core::ERROR_GENERAL;
                    }
                }
                else
                {
                    LOGERR("Failed to get the ListPackages");
                }
            }
            else
            {
                LOGERR("PackageManager Installer is %s", ((nullptr != mPackageManagerInstallerObject) ? "valid": "null"));
                status = Core::ERROR_GENERAL;
            }
        }
        return status;
}

Core::hresult AppManagerImplementation::packageUnLock(const string& appId)
{
        Core::hresult status = Core::ERROR_GENERAL;
        bool result = false;
        auto it = mAppInfo.find(appId);
        if (it != mAppInfo.end())
        {
            if (nullptr != mPackageManagerHandlerObject)
            {
                status = mPackageManagerHandlerObject->Unlock(appId, it->second.packageInfo.version);
                if(status == Core::ERROR_NONE)
                {
                    result = removeAppInfoByAppId(appId);
                    if(false == result)
                    {
                        LOGERR("Failed to remove the AppInfo");
                        status = Core::ERROR_GENERAL;
                    }
                }
                else
                {
                    LOGERR("Failed to PackageManager Unlock ");
                }
            }
            else
            {
                LOGERR("PackageManager handler is %s",((nullptr != mPackageManagerHandlerObject) ? "valid": "null"));
            }
        }
        else
        {
            LOGERR("AppId not found in map to get the version");
        }
        return status;
}
/*
 * @brief Launch App requested by client.
 * @Params[in]  : const string& appId , const string& intent, const string& launchArgs
 * @Params[out] : None
 * @return      : Core::hresult
 */
Core::hresult AppManagerImplementation::LaunchApp(const string& appId , const string& intent , const string& launchArgs)
{
    Core::hresult status = Core::ERROR_GENERAL;
    PackageInfo packageData;
    Exchange::IPackageHandler::LockReason lockReason = Exchange::IPackageHandler::LockReason::LAUNCH;
    LOGINFO(" LaunchApp enter with appId %s", appId.c_str());

    mAdminLock.Lock();
    if (nullptr != mLifecycleInterfaceConnector)
    {
        status = packageLock(appId, packageData, lockReason);
        if (status == Core::ERROR_NONE)
        {
            status = mLifecycleInterfaceConnector->launch(appId, intent, launchArgs);
        }
    }
    mAdminLock.Unlock();

    LOGINFO(" LaunchApp returns with status %d", status);
    return status;
}

/*
 * This function handles the process of closing an app by moving it from the ACTIVE state to the RUNNING state.
 * The state change is performed by the Lifecycle Manager.
 * If the app's state is not ACTIVE, the function does nothing.
 *
 * Input:
 * - const string& appId: The ID of the application to be closed.
 *
 * Output:
 * - Returns a Core::hresult status code:
 *     - Core::ERROR_NONE on success.
 *     - Core::ERROR_GENERAL on error.
 */
Core::hresult AppManagerImplementation::CloseApp(const string& appId)
{
    Core::hresult status = Core::ERROR_GENERAL;

    LOGINFO("CloseApp Entered with appId %s", appId.c_str());

    if (!appId.empty())
    {
        mAdminLock.Lock();
        if (nullptr != mLifecycleInterfaceConnector)
        {
            status = mLifecycleInterfaceConnector->closeApp(appId);
        }
        mAdminLock.Unlock();
    }
    else
    {
        LOGERR("appId is empty");
    }
    return status;
}

/*
 * This function Starts terminating a running app. Lifecycle Manager will move the state into
 * TERMINATE_REQUESTED
 *.This will trigger a clean shutdown
 *
 * Input:
 * - const string& appId: The ID of the application to be closed.
 *
 * Output:
 * - Returns a Core::hresult status code:
 *     - Core::ERROR_NONE on success.
 *     - Core::ERROR_GENERAL on error.
 */
Core::hresult AppManagerImplementation::TerminateApp(const string& appId )
{
    Core::hresult status = Core::ERROR_GENERAL;

    LOGINFO(" TerminateApp Entered with appId %s", appId.c_str());

    if (!appId.empty())
    {
        mAdminLock.Lock();
        if (nullptr != mLifecycleInterfaceConnector)
        {
            status = mLifecycleInterfaceConnector->terminateApp(appId);
            if(status == Core::ERROR_NONE)
            {
                status = packageUnLock(appId);
            }
        }
        mAdminLock.Unlock();
    }
    else
    {
        LOGERR("appId is empty");
    }
    return status;
}

Core::hresult AppManagerImplementation::KillApp(const string& appId)
{
    Core::hresult status = Core::ERROR_NONE;

    LOGINFO("KillApp entered appId: '%s'", appId.c_str());

    mAdminLock.Lock();
    if (nullptr != mLifecycleInterfaceConnector)
    {
        status = mLifecycleInterfaceConnector->killApp(appId);
        if(status == Core::ERROR_NONE)
        {
            status = packageUnLock(appId);
        }
    }
    mAdminLock.Unlock();

    LOGINFO("KillApp exited");
    return status;
}

Core::hresult AppManagerImplementation::GetLoadedApps(string& appData)
{
    Core::hresult status = Core::ERROR_GENERAL;
    LOGINFO("GetLoadedApps Entered");
    mAdminLock.Lock();
    if (nullptr != mLifecycleInterfaceConnector)
    {
        status = mLifecycleInterfaceConnector->getLoadedApps(appData);
    }
    mAdminLock.Unlock();

    LOGINFO(" GetLoadedApps returns with status %d", status);
    return status;
}

Core::hresult AppManagerImplementation::SendIntent(const string& appId , const string& intent)
{
    Core::hresult status = Core::ERROR_NONE;

    LOGINFO("SendIntent entered with appId '%s' and intent '%s'", appId.c_str(), intent.c_str());

    mAdminLock.Lock();
    if (nullptr != mLifecycleInterfaceConnector)
    {
        status = mLifecycleInterfaceConnector->sendIntent(appId, intent);
    }
    mAdminLock.Unlock();

    LOGINFO("SendIntent exited");
    return status;
}

/***
 * @brief Preloads an Application.
 * Preloads an Application and app will be in the RUNNING state (hidden).
 *
 * @param[in] appId     : App identifier for the application
 * @param[in] launchArgs(optional) : Additional parameters passed to the application
 * @param[out] error : if success = false it holds the appropriate error reason
 *
 * @return              : Core::<StatusCode>
 */
Core::hresult AppManagerImplementation::PreloadApp(const string& appId , const string& launchArgs ,string& error)
{
    Core::hresult status = Core::ERROR_GENERAL;
    PackageInfo packageData;
    Exchange::IPackageHandler::LockReason lockReason = Exchange::IPackageHandler::LockReason::LAUNCH;
    LOGINFO(" PreloadApp enter with appId %s", appId.c_str());

    mAdminLock.Lock();
    if (nullptr != mLifecycleInterfaceConnector)
    {
        status = packageLock(appId, packageData, lockReason);
        if (status == Core::ERROR_NONE)
        {
            status = mLifecycleInterfaceConnector->preLoadApp(appId, launchArgs, error);
        }
    }
    mAdminLock.Unlock();

    LOGINFO(" PreloadApp returns with status %d", status);
    return status;
}

/***
 * @brief Gets a property for a given app.
 * Gets a property for a given app based on the appId and key.
 *
 * @param[in] appId     : App identifier for the application
 * @param[in] key       : the name of the property to get
 * @param[out] value    : value of the key queried, this can be a boolean, number, string or object type
 *
 * @return              : Core::<StatusCode>
 */
Core::hresult AppManagerImplementation::GetAppProperty(const string& appId , const string& key , string& value )
{
    LOGINFO("GetAppProperty Entered");
    Core::hresult status = Core::ERROR_GENERAL;
    uint32_t ttl = 0;
    if(appId.empty())
    {
        LOGERR("Empty appId");
    }
    else if(key.empty())
    {
        LOGERR("Empty Key");
    }
    else
    {
        mAdminLock.Lock();
        /* Checking if mPersistentStoreRemoteStoreObject is not valid then create the object, not required to destroy this object as it is handled in Destructor */
        if (nullptr == mPersistentStoreRemoteStoreObject)
        {
            LOGINFO("Create PersistentStore Remote store object");
            if (Core::ERROR_NONE != createPersistentStoreRemoteStoreObject())
            {
                LOGERR("Failed to create createPersistentStoreRemoteStoreObject");
            }
        }
        ASSERT (nullptr != mPersistentStoreRemoteStoreObject);
        if (nullptr != mPersistentStoreRemoteStoreObject)
        {
            status = mPersistentStoreRemoteStoreObject->GetValue(Exchange::IStore2::ScopeType::DEVICE, appId, key, value, ttl);
            LOGINFO("Key[%s] value[%s] status[%d]", key.c_str(), value.c_str(), status);
            if (Core::ERROR_NONE != status)
            {
                LOGERR("GetValue Failed");
            }
        }
        else
        {
            LOGERR("PersistentStore object is not valid");
        }
        mAdminLock.Unlock();
    }
    LOGINFO("GetAppProperty Exited");

    return status;
}

/***
 * @brief Sets a property for a given app.
 * Sets a property for a given app based with given namespace, key and Value to persistentStore.
 *
 * @param[in] appId     : App identifier for the application
 * @param[in] key       : the name of the property to get
 * @param[in] value     : the property value(json string format) to set, this can be a boolean, number, string or object type
 *
 * @return              : Core::<StatusCode>
 */
Core::hresult AppManagerImplementation::SetAppProperty(const string& appId, const string& key, const string& value)
{
    Core::hresult status = Core::ERROR_GENERAL;

    if (appId.empty())
    {
        LOGERR("appId is empty");
    }
    else if (key.empty())
    {
        LOGERR("key is empty");
    }
    else
    {
        mAdminLock.Lock();

        /* Checking if mPersistentStoreRemoteStoreObject is not valid then create the object */
        if (nullptr == mPersistentStoreRemoteStoreObject)
        {
            LOGINFO("Create PersistentStore Remote store object");
            if (Core::ERROR_NONE != createPersistentStoreRemoteStoreObject())
            {
                LOGERR("Failed to create createPersistentStoreRemoteStoreObject");
            }
        }

        ASSERT (nullptr != mPersistentStoreRemoteStoreObject);
        if (nullptr != mPersistentStoreRemoteStoreObject)
        {
            status = mPersistentStoreRemoteStoreObject->SetValue(Exchange::IStore2::ScopeType::DEVICE, appId, key, value, 0);
        }
        else
        {
            LOGERR("PersistentStore object is not valid");
        }

        mAdminLock.Unlock();
    }
    return status;
}

Core::hresult AppManagerImplementation::GetInstalledApps(string& apps)
{
    Core::hresult status = Core::ERROR_NONE;

    LOGINFO("GetInstalledApps Entered");

    return status;
}

Core::hresult AppManagerImplementation::IsInstalled(const string& appId, bool& installed)
{
    Core::hresult status = Core::ERROR_NONE;

    LOGINFO("IsInstalled Entered");

    return status;
}

Core::hresult AppManagerImplementation::StartSystemApp(const string& appId)
{
    Core::hresult status = Core::ERROR_NONE;

    LOGINFO("StartSystemApp Entered");

    return status;
}

Core::hresult AppManagerImplementation::StopSystemApp(const string& appId)
{
    Core::hresult status = Core::ERROR_NONE;

    LOGINFO("StopSystemApp Entered");

    return status;
}

Core::hresult AppManagerImplementation::ClearAppData(const string& appId)
{
    Core::hresult status = Core::ERROR_NONE;

    LOGINFO("ClearAppData Entered");

    return status;
}

Core::hresult AppManagerImplementation::ClearAllAppData()
{
    Core::hresult status = Core::ERROR_NONE;

    LOGINFO("ClearAllAppData Entered");

    return status;
}

Core::hresult AppManagerImplementation::GetAppMetadata(const string& appId, const string& metaData, string& result)
{
    Core::hresult status = Core::ERROR_NONE;

    LOGINFO("GetAppMetadata Entered");

    return status;
}

Core::hresult AppManagerImplementation::GetMaxRunningApps(int32_t& maxRunningApps) const
{
    LOGINFO("GetMaxRunningApps Entered");

    maxRunningApps = -1;

    return Core::ERROR_NONE;
}

Core::hresult AppManagerImplementation::GetMaxHibernatedApps(int32_t& maxHibernatedApps) const
{
    LOGINFO("GetMaxHibernatedApps Entered");

    maxHibernatedApps = -1;

    return Core::ERROR_NONE;
}

Core::hresult AppManagerImplementation::GetMaxHibernatedFlashUsage(int32_t& maxHibernatedFlashUsage) const
{
    LOGINFO("GetMaxHibernatedFlashUsage Entered");

    maxHibernatedFlashUsage = -1;

    return Core::ERROR_NONE;
}

Core::hresult AppManagerImplementation::GetMaxInactiveRamUsage(int32_t& maxInactiveRamUsage) const
{
    LOGINFO("GetMaxInactiveRamUsage Entered");

    maxInactiveRamUsage = -1;

    return Core::ERROR_NONE;
}
} /* namespace Plugin */
} /* namespace WPEFramework */
