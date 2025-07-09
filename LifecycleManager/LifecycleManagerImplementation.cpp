/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
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
**/

#include "LifecycleManagerImplementation.h"
#include "RequestHandler.h"
#include "UtilsJsonRpc.h"
#include <interfaces/json/JsonData_LifecycleManagerState.h>
#include <interfaces/json/JLifecycleManagerState.h>
#include <semaphore.h>

namespace WPEFramework
{
    namespace Plugin
    {
        SERVICE_REGISTRATION(LifecycleManagerImplementation, 1, 0);

        LifecycleManagerImplementation::LifecycleManagerImplementation(): mLifecycleManagerNotification(), mLifecycleManagerStateNotification(), mLoadedApplications(), mService(nullptr)
        {
            LOGINFO("Create LifecycleManagerImplementation Instance");
        }

        LifecycleManagerImplementation::~LifecycleManagerImplementation()
        {
            LOGINFO("Delete LifecycleManagerImplementation Instance");
            terminate();
        }

        bool LifecycleManagerImplementation::initialize(PluginHost::IShell* service)
        {
            bool ret = RequestHandler::getInstance()->initialize(service, this);
	    return ret;
        }

        void LifecycleManagerImplementation::terminate()
        {
            try
            {
                RequestHandler::getInstance()->terminate();
            }
            catch(const std::exception& e)
            {
                LOGERR("Exception in LifecycleManagerImplementation::terminate: %s", e.what());
            }
            if (mService != nullptr)
            {
               mService->Release();
               mService = nullptr;
            }
        }

        Core::hresult LifecycleManagerImplementation::Register(Exchange::ILifecycleManager::INotification *notification)
        {
            ASSERT (nullptr != notification);
        
            mAdminLock.Lock();
        
            /* Make sure we can't register the same notification callback multiple times */
            if (std::find(mLifecycleManagerNotification.begin(), mLifecycleManagerNotification.end(), notification) == mLifecycleManagerNotification.end())
            {
                LOGINFO("Register notification");
                mLifecycleManagerNotification.push_back(notification);
                notification->AddRef();
            }
        
            mAdminLock.Unlock();
        
            return Core::ERROR_NONE;
        }
        
        Core::hresult LifecycleManagerImplementation::Unregister(Exchange::ILifecycleManager::INotification *notification )
        {
            Core::hresult status = Core::ERROR_GENERAL;
        
            ASSERT (nullptr != notification);
        
            mAdminLock.Lock();
        
            /* Make sure we can't unregister the same notification callback multiple times */
            auto itr = std::find(mLifecycleManagerNotification.begin(), mLifecycleManagerNotification.end(), notification);
            if (itr != mLifecycleManagerNotification.end())
            {
                (*itr)->Release();
                LOGINFO("Unregister notification");
                mLifecycleManagerNotification.erase(itr);
                status = Core::ERROR_NONE;
            }
            else
            {
                LOGERR("notification not found");
            }
        
            mAdminLock.Unlock();
        
            return status;
        }
        
        void LifecycleManagerImplementation::dispatchEvent(EventNames event, const JsonValue &params)
        {
            Core::IWorkerPool::Instance().Submit(Job::Create(this, event, params));
        }
        
        void LifecycleManagerImplementation::Dispatch(EventNames event, const JsonValue params)
        {
             JsonObject obj = params.Object();

             //below is for ILifecycleManager notification
             string appId(obj["appId"].String());
             string errorReason(obj["errorReason"].String());
             uint32_t newLifecycleState(obj["newLifecycleState"].Number());

             //below is additional for ILifecycleManagerState notification
             string appInstanceId(obj["appInstanceId"].String());
             uint32_t oldLifecycleState(obj["oldLifecycleState"].Number());
             string navigationIntent(obj["navigationIntent"].String());

             mAdminLock.Lock();
        
             std::list<Exchange::ILifecycleManager::INotification*>::const_iterator index(mLifecycleManagerNotification.begin());
             std::list<Exchange::ILifecycleManagerState::INotification*>::const_iterator stateNotificationIndex(mLifecycleManagerStateNotification.begin());
        
             switch(event)
             {
                 case LIFECYCLE_MANAGER_EVENT_APPSTATECHANGED:
                     handleStateChangeEvent(obj);
                     while (index != mLifecycleManagerNotification.end())
                     {
                         (*index)->OnAppStateChanged(appId, (LifecycleState)newLifecycleState, errorReason);
                         ++index;
                     }
                     while (stateNotificationIndex != mLifecycleManagerStateNotification.end())
                     {
                         (*stateNotificationIndex)->OnAppLifecycleStateChanged(appId, appInstanceId, (LifecycleState)oldLifecycleState, (LifecycleState)newLifecycleState, navigationIntent);
                         ++stateNotificationIndex;
                     }
                     break;
                 case LIFECYCLE_MANAGER_EVENT_RUNTIME:
                     handleRuntimeManagerEvent(obj);
                     break;
                 case LIFECYCLE_MANAGER_EVENT_WINDOW:
                      handleWindowManagerEvent(obj);
                      break;
                 default:
                     LOGWARN("Event[%u] not handled", event);
                     break;
             }
        
             mAdminLock.Unlock();
        }
        
        Core::hresult LifecycleManagerImplementation::GetLoadedApps(const bool verbose, string& apps)
        {
            Core::hresult status = Core::ERROR_NONE;
            JsonArray appsInformation;
            std::list<ApplicationContext*>::iterator iter = mLoadedApplications.end();
	    for (iter = mLoadedApplications.begin(); iter != mLoadedApplications.end(); iter++)
	    {
                if (nullptr != *iter)
                {
                    JsonObject appData;
                    ApplicationContext* context = (*iter);
                    appData["appInstanceID"] = context->getAppInstanceId();
                    appData["appId"] = context->getAppId();
		    struct timespec lastStateChangeTime = context->getLastLifecycleStateChangeTime();
                    char timeData[200];
                    char timeDataExpanded[1000];
		    memset(timeData, 0, sizeof(timeData));
		    memset(timeDataExpanded, 0, sizeof(timeDataExpanded));
                    strftime(timeData, sizeof timeData, "%D %T", gmtime(&lastStateChangeTime.tv_sec));
                    sprintf(timeDataExpanded, "%s.%09ld", timeData, lastStateChangeTime.tv_nsec);
                    appData["currentLifecycleState"] = (uint32_t) context->getCurrentLifecycleState();
                    appData["timeOfLastLifecycleStateChange"] = string(timeDataExpanded);
                    appData["activeSessionId"] = context->getActiveSessionId();
                    appData["targetLifecycleState"] = (uint32_t) context->getTargetLifecycleState();
                    appData["mostRecentIntent"] = context->getMostRecentIntent();
                    if (verbose)
		    {	    
                        RuntimeManagerHandler* runtimeManagerHandler = RequestHandler::getInstance()->getRuntimeManagerHandler();
			if (nullptr != runtimeManagerHandler)
			{
                            string runtimeStats("");
                            Core::hresult runtimeStatsResult = runtimeManagerHandler->getRuntimeStats(context->getAppInstanceId(), runtimeStats);
                            if (Core::ERROR_NONE == runtimeStatsResult)
                            {
                                appData["runtimeStats"] = runtimeStats;
                            }
                            else
                            {
                                printf("unable to get runtime status of application\n");
                                fflush(stdout);
                            }
                        }
                    }
                    appsInformation.Add(appData);
                }
	    }
            appsInformation.ToString(apps);
            return status;
        }
        
        Core::hresult LifecycleManagerImplementation::IsAppLoaded(const string& appId, bool& loaded) const
        {
            Core::hresult status = Core::ERROR_NONE;
            loaded = false;
            ApplicationContext* context = getContext("", appId);
            if (nullptr != context)
	    {
                loaded = true;
            }
            return status;
        }
        
        Core::hresult LifecycleManagerImplementation::SpawnApp(const string& appId, const string& launchIntent, const LifecycleState targetLifecycleState, const WPEFramework::Exchange::RuntimeConfig& runtimeConfigObject, const string& launchArgs, string& appInstanceId, string& errorReason, bool& success)
        {
	    // Launches an app.  This will be an asynchronous call.
            // Notifies appropriate API Gateway when an app is about to be loaded
            // Lifecycle manager will create the appInstanceId once the app is loaded.  Ripple is responsible for creating a token. 
            Core::hresult status = Core::ERROR_NONE;
            ApplicationContext* context = getContext("", appId);
            bool firstLaunch = false;
            if (nullptr == context)
	    {
                context = new ApplicationContext(appId);
                context->setApplicationLaunchParams(appId, launchIntent, launchArgs, targetLifecycleState, runtimeConfigObject);
		mLoadedApplications.push_back(context);
                firstLaunch = true;
	    }
            context->setTargetLifecycleState(targetLifecycleState);
            context->setMostRecentIntent(launchIntent);
            success = RequestHandler::getInstance()->launch(context, launchIntent, targetLifecycleState, errorReason);
            if (!success)
	    {
                status = Core::ERROR_GENERAL;
	    }
            else
	    {
                if (firstLaunch)
		{
                    sem_wait(&context->mReachedLoadingStateSemaphore);
		}
                appInstanceId = context->getAppInstanceId();
            }
            return status;
        }
        
        Core::hresult LifecycleManagerImplementation::SetTargetAppState(const string& appInstanceId, const LifecycleState targetLifecycleState, const string& launchIntent)
        {
            // Moves a currently loaded app between states
            Core::hresult status = Core::ERROR_NONE;
            ApplicationContext* context = getContext(appInstanceId, "");
            if (nullptr == context)
	    {
                status = Core::ERROR_GENERAL;
                return status;
	    }

            string errorReason("");
            context->setTargetLifecycleState(targetLifecycleState);
            context->setMostRecentIntent(launchIntent);

            bool success = RequestHandler::getInstance()->updateState(context, targetLifecycleState, errorReason);
            if (false == success)
            {
                status = Core::ERROR_GENERAL;
                return status;
            }
            return status;
        }
        
        Core::hresult LifecycleManagerImplementation::UnloadApp(const string& appInstanceId, string& errorReason, bool& success)
        {
            // Begins a graceful shutdown of the app.  Moves the app through the lifecycle states till it ultimately ends in app container being terminated.
            // This is an asynchronous call, clients should use the onAppStateChange event to determine when the app is actually terminated.
            // This moves an app to the unloaded state
            Core::hresult status = Core::ERROR_NONE;
            ApplicationContext* context = getContext(appInstanceId, "");
            if (nullptr == context)
	    {
                status = Core::ERROR_GENERAL;
                success = false;
                return status;
	    }
            context->setTargetLifecycleState(Exchange::ILifecycleManager::LifecycleState::TERMINATING);
            context->setApplicationKillParams(false);

            success = RequestHandler::getInstance()->terminate(context, false, errorReason);
            if (!success)
	    {
                status = Core::ERROR_GENERAL;
	    }
            return status;
        }
        
        Core::hresult LifecycleManagerImplementation::KillApp(const string& appInstanceId, string& errorReason, bool& success)
        {
            Core::hresult status = Core::ERROR_NONE;
            ApplicationContext* context = getContext(appInstanceId, "");
            if (nullptr == context)
	    {
                status = Core::ERROR_GENERAL;
                success = false;
                return status;
	    }
            context->setTargetLifecycleState(Exchange::ILifecycleManager::LifecycleState::TERMINATING);
            context->setApplicationKillParams(true);
            success = RequestHandler::getInstance()->terminate(context, true, errorReason);
            return status;
        }
        
        Core::hresult LifecycleManagerImplementation::SendIntentToActiveApp(const string& appInstanceId, const string& intent, string& errorReason, bool& success)
        {
            // Sends arguments to a launched app.  This API is used for sending deeplinks to an application.  This can only be sent to an active app.  This method does nothing if the app is not active, and an errorReason is returned.
            Core::hresult status = Core::ERROR_NONE;
            ApplicationContext* context = getContext(appInstanceId, "");
            if (nullptr == context)
	    {
                status = Core::ERROR_GENERAL;
                success = false;
                return status;
	    }
            success = RequestHandler::getInstance()->sendIntent(context, intent, errorReason);
            if (!success)
	    {
                status = Core::ERROR_GENERAL;
	    }
            return status;
        }

        Core::hresult LifecycleManagerImplementation::Register(Exchange::ILifecycleManagerState::INotification *notification)
        {
            ASSERT (nullptr != notification);
        
            mAdminLock.Lock();
        
            /* Make sure we can't register the same notification callback multiple times */
            if (std::find(mLifecycleManagerStateNotification.begin(), mLifecycleManagerStateNotification.end(), notification) == mLifecycleManagerStateNotification.end())
            {
                LOGINFO("Register notification");
                mLifecycleManagerStateNotification.push_back(notification);
                notification->AddRef();
            }
        
            mAdminLock.Unlock();
        
            return Core::ERROR_NONE;
        }
        
        Core::hresult LifecycleManagerImplementation::Unregister(Exchange::ILifecycleManagerState::INotification *notification )
        {
            Core::hresult status = Core::ERROR_GENERAL;
        
            ASSERT (nullptr != notification);
        
            mAdminLock.Lock();
        
            /* Make sure we can't unregister the same notification callback multiple times */
            auto itr = std::find(mLifecycleManagerStateNotification.begin(), mLifecycleManagerStateNotification.end(), notification);
            if (itr != mLifecycleManagerStateNotification.end())
            {
                (*itr)->Release();
                LOGINFO("Unregister notification");
                mLifecycleManagerStateNotification.erase(itr);
                status = Core::ERROR_NONE;
            }
            else
            {
                LOGERR("notification not found");
            }
        
            mAdminLock.Unlock();
        
            return status;
        }
        
	Core::hresult LifecycleManagerImplementation::AppReady(const string& appId)
        {
            Core::hresult status = Core::ERROR_NONE;
	    printf("[LifecycleManager] Received appReady event for [%s] \n", appId.c_str());
	    fflush(stdout);
            ApplicationContext* context = getContext("", appId);
            if (nullptr == context)
	    {
                status = Core::ERROR_GENERAL;
                return status;
	    }
            sem_post(&context->mAppReadySemaphore);
	    return status;
	}

        Core::hresult LifecycleManagerImplementation::StateChangeComplete(const string& appId, const uint32_t stateChangedId, const bool success)
        {
            Core::hresult status = Core::ERROR_NONE;
	    return status;
	}

	Core::hresult LifecycleManagerImplementation::CloseApp(const string& appId, const Exchange::ILifecycleManagerState::AppCloseReason closeReason)
        {
            Core::hresult status = Core::ERROR_NONE;
            ApplicationContext* context = getContext("", appId);
            if (nullptr == context)
	    {
                status = Core::ERROR_GENERAL;
                return status;
	    }
	    bool success = false;
            bool activate = false;
            string errorReason(""), appInstanceId("");
            status = KillApp(context->getAppInstanceId(), errorReason, success); 
            if (status != Core::ERROR_NONE)
	    {
                printf("Failed to close the app [%s]\n", appId.c_str());
		fflush(stdout);
                return status;
	    }
	    if ((closeReason != KILL_AND_RUN) && (closeReason != KILL_AND_ACTIVATE))
	    {
                return status;
	    }
            activate = (closeReason == KILL_AND_ACTIVATE);		    

            ApplicationLaunchParams& launchParams = context->getApplicationLaunchParams();
	    status = SpawnApp(launchParams.mAppId, launchParams.mLaunchIntent, activate?Exchange::ILifecycleManager::LifecycleState::ACTIVE:Exchange::ILifecycleManager::LifecycleState::PAUSED, launchParams.mRuntimeConfigObject, launchParams.mLaunchArgs, appInstanceId, errorReason, success);
	    return status;
	}

        uint32_t LifecycleManagerImplementation::Configure(PluginHost::IShell* service)
        {
            uint32_t result = Core::ERROR_GENERAL;
            if (service != nullptr)
            {
                mService = service;
                mService->AddRef();
            }
            else
            {
                LOGERR("service is null \n");
                return result;
            }
            bool ret = initialize(service);
	    if (ret)
	    {
                result = Core::ERROR_NONE;
	    }
            else
	    {
                printf("unable to configure lifecyclemanager \n");
		fflush(stdout);
	    }
            return result;
        }

        ApplicationContext* LifecycleManagerImplementation::getContext(const string& appInstanceId, const string& appId) const
	{
            ApplicationContext* context = nullptr;
            std::list<ApplicationContext*>::const_iterator iter = mLoadedApplications.end();
	    for (iter = mLoadedApplications.begin(); iter != mLoadedApplications.end(); iter++)
	    {
                if (nullptr != *iter)
		{
                    if ((!appInstanceId.empty()) && ((*iter)->getAppInstanceId() == appInstanceId))
                    {
		        context = *iter;
		        break;
                    }
		    else if ((!appId.empty()) && ((*iter)->getAppId() == appId))
                    {
		        context = *iter;
		        break;
                    }
		}
	    }
	    return context;
	}

	void LifecycleManagerImplementation::onRuntimeManagerEvent(JsonObject& data)
	{
            dispatchEvent(LifecycleManagerImplementation::EventNames::LIFECYCLE_MANAGER_EVENT_RUNTIME, data);
	}

        void LifecycleManagerImplementation::onWindowManagerEvent(JsonObject& data)
	{
            dispatchEvent(LifecycleManagerImplementation::EventNames::LIFECYCLE_MANAGER_EVENT_WINDOW, data);
	}

        void LifecycleManagerImplementation::onRippleEvent(string name, JsonObject& data)
	{
            //mLifecycleManager->dispatchEvent(LifecycleManagerImplementation::EventNames::LIFECYCLE_MANAGER_EVENT_APPSTATECHANGED, JsonValue(minutes));
	}

        void LifecycleManagerImplementation::onStateChangeEvent(JsonObject& data)
	{
            dispatchEvent(LifecycleManagerImplementation::EventNames::LIFECYCLE_MANAGER_EVENT_APPSTATECHANGED, data);
	}

        void LifecycleManagerImplementation::handleRuntimeManagerEvent(const JsonObject &data)
        {
            string eventName = data["name"];
            if (eventName.compare("onTerminated") == 0)
            {
                LOGINFO("Received onterminated event from runtime manager");
                string appInstanceId = data["appInstanceId"];
                ApplicationContext* context = getContext(appInstanceId, "");
                if (nullptr == context)
                {
                    LOGERR("Received termination event for app which is not available");
                }
                else
                {
                    LOGINFO("Received state change event for app Onterminated event ");
                    sem_post(&context->mAppTerminatingSemaphore);
                }
            }
            else if (eventName.compare("onStateChanged") == 0)
            {
                string appInstanceId = data["appInstanceId"];
                uint32_t runtimeState = data["state"].Number();
                if (Exchange::IRuntimeManager::RuntimeState::RUNTIME_STATE_RUNNING == runtimeState)
                {
                    ApplicationContext* context = getContext(appInstanceId, "");
                    if (nullptr == context)
                    {
                        LOGERR("Received state change event for app which is not available");
                    }
                    else
                    {
                        LOGINFO("Received state change event for app which is available ie running");
                        sem_post(&context->mAppRunningSemaphore);
                    }
                }
            }
            else if (eventName.compare("onFailure") == 0)
            {
                string appInstanceId = data["appInstanceId"];
                string errorCode = data["errorCode"];
                LOGINFO("Received container failure event from runtime manager for app[%s] error[%s]", appInstanceId.c_str(), errorCode.c_str());
            }
            else if (eventName.compare("onStarted") == 0)
            {
                string appInstanceId = data["appInstanceId"];
                LOGINFO("Received container started event from runtime manager for app[%s]", appInstanceId.c_str());
            }
        }

	void LifecycleManagerImplementation::handleStateChangeEvent(const JsonObject &data)
    {
            string appInstanceId = data["appInstanceId"];
	    uint32_t stateInput = data["newLifecycleState"].Number();
            Exchange::ILifecycleManager::LifecycleState state = (Exchange::ILifecycleManager::LifecycleState) stateInput;
            if (state != Exchange::ILifecycleManager::LifecycleState::UNLOADED)
	    {
                return;
	    }
            ApplicationContext* context = nullptr;
            std::list<ApplicationContext*>::iterator iter = mLoadedApplications.end();
	    for (iter = mLoadedApplications.begin(); iter != mLoadedApplications.end(); iter++)
	    {
                context = *iter;
                if (nullptr == context)
		{
                    continue;
		}
                if (context->getAppInstanceId() == appInstanceId)
	        {
	            break;	    
                }
	    }
	    if ((iter != mLoadedApplications.end()) && (nullptr != context))
	    {
                delete context;
                mLoadedApplications.erase(iter);
	    }
    }


    void LifecycleManagerImplementation::handleWindowManagerEvent(const JsonObject &data)
    {
        string eventName = data["name"];
        if (eventName.compare("onUserInactivity") == 0)
        {
            printf("Received onUserInactivity event from window manager \n");
            fflush(stdout);
        }
        else if (eventName.compare("onDisconnect") == 0)
        {
            printf("Received onDisconnect event from window manager \n");
            fflush(stdout);
	}
        else if (eventName.compare("onReady") == 0)
        {
            printf("Received onReady event from window manager \n");
            fflush(stdout);
            std::string appInstanceId = data["appInstanceId"];
            ApplicationContext* context = getContext(appInstanceId, "");
            if (nullptr != context)
	    {
                sem_post(&context->mFirstFrameSemaphore);
	    }
	}
    }

    } /* namespace Plugin */
} /* namespace WPEFramework */
