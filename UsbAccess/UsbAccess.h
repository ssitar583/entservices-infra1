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

#pragma once

#include <set>
#include "Module.h"
#include <interfaces/Ids.h>
#include <interfaces/IUSBMassStorage.h>
#include <thread>
using ::WPEFramework::Exchange::IUSBMassStorage;
namespace WPEFramework {
namespace Plugin {

    class UsbAccess :  public PluginHost::IPlugin, public PluginHost::JSONRPC {

    class USBMassStorageNotification : public Exchange::IUSBMassStorage::INotification {
    private:
        USBMassStorageNotification(const USBMassStorageNotification&) = delete;
        USBMassStorageNotification& operator=(const USBMassStorageNotification&) = delete;

    public:
        explicit USBMassStorageNotification(UsbAccess& parent)
            : _parent(parent)
        {
        }
        ~USBMassStorageNotification() override = default;

    public:
        void OnDeviceMounted(const Exchange::IUSBMassStorage::USBStorageDeviceInfo &deviceInfo , Exchange::IUSBMassStorage::IUSBStorageMountInfoIterator* const mountPoints) override
        {
            _parent.onDeviceMounted(deviceInfo, mountPoints);
        }
        void OnDeviceUnmounted(const Exchange::IUSBMassStorage::USBStorageDeviceInfo &deviceInfo , Exchange::IUSBMassStorage::IUSBStorageMountInfoIterator* const mountPoints) override
        {
            _parent.onDeviceUnmounted(deviceInfo, mountPoints);
        }

        BEGIN_INTERFACE_MAP(USBMassStorageNotification)
        INTERFACE_ENTRY(Exchange::IUSBMassStorage::INotification)
        END_INTERFACE_MAP

    private:
        UsbAccess& _parent;
    };

    public:
    class EXTERNAL Job : public Core::IDispatch {
    protected:
        Job(UsbAccess *usbAccess, bool &ismounted, string &mountpath)
         : _usbAccess(usbAccess)
         , _ismounted(ismounted)
         , _mountpath(mountpath)
        {
         if (_usbAccess != nullptr)
         {
             _usbAccess->AddRef();
         }
        }
 
    public:
        Job() = delete;
        Job(const Job&) = delete;
        Job& operator=(const Job&) = delete;
        ~Job()
        {
         if (_usbAccess != nullptr)
         {
             ASSERT(nullptr != _usbAccess);
             _usbAccess->Release();
         }
        }

        static Core::ProxyType<Core::IDispatch> Create(UsbAccess *usbAccess, bool ismounted, string mountpath)
        {
#ifndef USE_THUNDER_R4
         return (Core::proxy_cast<Core::IDispatch>(Core::ProxyType<Job>::Create(usbAccess, ismounted, mountpath)));
#else
         return (Core::ProxyType<Core::IDispatch>(Core::ProxyType<Job>::Create(usbAccess, ismounted, mountpath)));
#endif
        }

        virtual void Dispatch()
        {
            ASSERT(nullptr != _usbAccess);
            _usbAccess->Dispatch(_ismounted, _mountpath);
        }

     private:
        UsbAccess *_usbAccess;
        const bool _ismounted;
        const string _mountpath;
    };

    public:
        UsbAccess();
        virtual ~UsbAccess();
        virtual const string Initialize(PluginHost::IShell* service) override;
        virtual void Deinitialize(PluginHost::IShell* service) override;
        virtual string Information() const override;

        void registerEventHandlers();
        void onDeviceMounted(const Exchange::IUSBMassStorage::USBStorageDeviceInfo &deviceInfo, Exchange::IUSBMassStorage::IUSBStorageMountInfoIterator* const mountPoints);
        void onDeviceUnmounted(const Exchange::IUSBMassStorage::USBStorageDeviceInfo &deviceInfo, Exchange::IUSBMassStorage::IUSBStorageMountInfoIterator* const mountPoints);

        BEGIN_INTERFACE_MAP(UsbAccess)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
        END_INTERFACE_MAP

    public/*members*/:
        static UsbAccess* _instance;

    public /*constants*/:
        static const string SERVICE_NAME;
        //methods
        static const string METHOD_GET_FILE_LIST;
        static const string METHOD_CREATE_LINK;
        static const string METHOD_CLEAR_LINK;
        static const string METHOD_GET_AVAILABLE_FIRMWARE_FILES;
        static const string METHOD_GET_MOUNTED;
        static const string METHOD_UPDATE_FIRMWARE;
        static const string METHOD_ARCHIVE_LOGS;
        //events
        static const string EVT_ON_USB_MOUNT_CHANGED;
        static const string EVT_ON_ARCHIVE_LOGS;
        //other
        static const string LINK_URL_HTTP;
        static const string LINK_PATH;
        static const string REGEX_BIN;
        static const string REGEX_FILE;
        static const string REGEX_PATH;
        static const string PATH_DEVICE_PROPERTIES;
        static const std::list<string> ADDITIONAL_FW_PATHS;
        static const string ARCHIVE_LOGS_SCRIPT;
        enum ArchiveLogsError
        {
            ScriptError = -1,
            None,
            Locked,
            NoUSB,
            WritingError,
        };
        typedef std::map<ArchiveLogsError, string> ArchiveLogsErrorMap;
        static const ArchiveLogsErrorMap ARCHIVE_LOGS_ERRORS;

    private/*registered methods (wrappers)*/:

        //methods ("parameters" here is "params" from the curl request)
        uint32_t getFileListWrapper(const JsonObject& parameters, JsonObject& response);
        uint32_t createLinkWrapper(const JsonObject& parameters, JsonObject& response);
        uint32_t clearLinkWrapper(const JsonObject& parameters, JsonObject& response);
        uint32_t getLinksWrapper(const JsonObject& parameters, JsonObject& response);
        uint32_t getAvailableFirmwareFilesWrapper(const JsonObject& parameters, JsonObject& response);
        uint32_t getMountedWrapper(const JsonObject& parameters, JsonObject& response);
        uint32_t updateFirmware(const JsonObject& parameters, JsonObject& response);
        uint32_t archiveLogs(const JsonObject& parameters, JsonObject& response);

    private:
        void InitializeUSBMassStorage();
        void onUSBMountChanged(bool mounted, string mount_path);

    private/*internal methods*/:
        UsbAccess(const UsbAccess&) = delete;
        UsbAccess& operator=(const UsbAccess&) = delete;

        struct FileEnt
        {
            char fileType; // 'f' or 'd'
            string filename;
        };
        typedef std::list<FileEnt> FileList;

        static bool getFileList(const string& path, FileList& files, const string& fileRegex, bool includeFolders);
        bool getMounted(std::list<string>& paths);
        void Dispatch(const bool ismounted, string mount_path);

        void archiveLogsInternal();
        void onArchiveLogs(ArchiveLogsError error, const string& filePath);
        std::thread archiveLogsThread;
        std::map<int, std::string> m_CreatedLinkIds;
        JsonObject m_oArchiveParams;

    private:
        std::list <std::string> m_paths;
        mutable Core::CriticalSection _adminLock;

        Core::ProxyType<RPC::InvokeServerType<1, 0, 4>> _engine;
        Core::ProxyType<RPC::CommunicatorClient> _communicatorClient;
        PluginHost::IShell *_controller;
        Exchange::IUSBMassStorage* _usbMassStorageObject;
        Core::Sink<USBMassStorageNotification> _usbMassStorageNotification;
        bool _registeredEventHandlers;
    };

} // namespace Plugin
}// namespace WPEFramework
