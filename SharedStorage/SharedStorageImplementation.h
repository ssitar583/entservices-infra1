/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
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

#pragma once

#include "Module.h"
#include <interfaces/ISharedStorage.h>
#include <interfaces/IStore2.h>
#include <interfaces/IStoreCache.h>
#include <interfaces/IConfiguration.h>

namespace WPEFramework {
namespace Plugin {

    class SharedStorageImplementation : public Exchange::ISharedStorage,
                                        public Exchange::ISharedStorageInspector,
                                        public Exchange::ISharedStorageLimit,
                                        public Exchange::ISharedStorageCache,
                                        public Exchange::IConfiguration {
    private:
        SharedStorageImplementation(const SharedStorageImplementation&) = delete;
        SharedStorageImplementation& operator=(const SharedStorageImplementation&) = delete;

    private:
        class Store2Notification : public Exchange::IStore2::INotification {
        private:
            Store2Notification(const Store2Notification&) = delete;
            Store2Notification& operator=(const Store2Notification&) = delete;

        public:
            explicit Store2Notification(SharedStorageImplementation& parent)
                : _parent(parent)
            {
            }
            ~Store2Notification() override = default;

        public:
            void ValueChanged(const Exchange::IStore2::ScopeType scope, const string& ns, const string& key, const string& value) override
            {
                _parent.ValueChanged((ISharedStorage::ScopeType)scope, ns, key, value);
            }

            BEGIN_INTERFACE_MAP(Store2Notification)
            INTERFACE_ENTRY(Exchange::IStore2::INotification)
            END_INTERFACE_MAP

        private:
        SharedStorageImplementation& _parent;
        };

    public:
        SharedStorageImplementation();
        ~SharedStorageImplementation() override;
        static SharedStorageImplementation* instance(SharedStorageImplementation *SharedStorageImpl = nullptr);
        BEGIN_INTERFACE_MAP(SharedStorageImplementation)
        INTERFACE_ENTRY(Exchange::ISharedStorage)
        INTERFACE_ENTRY(ISharedStorageInspector)
        INTERFACE_ENTRY(ISharedStorageLimit)
        INTERFACE_ENTRY(ISharedStorageCache)
        INTERFACE_ENTRY(Exchange::IConfiguration)
        END_INTERFACE_MAP

    private:
        // IConfiguration interface
        Core::hresult Configure(PluginHost::IShell* service) override;
        Core::hresult Register(ISharedStorage::INotification* notification) override;
        Core::hresult Unregister(ISharedStorage::INotification* notification) override;
        //ISharedStorage APIs
        Core::hresult SetValue(const ISharedStorage::ScopeType eScope, const string& ns, const string& key, const string& value, const uint32_t ttl, Success& success) override;
        Core::hresult GetValue(const ISharedStorage::ScopeType eScope, const string& ns, const string& key, string& value, uint32_t& ttl, bool& success) override;
        Core::hresult DeleteKey(const ISharedStorage::ScopeType eScope, const string& ns, const string& key, Success& success) override;
        Core::hresult DeleteNamespace(const ISharedStorage::ScopeType eScope, const string& ns, Success& success) override;

        //ISharedStorageInspector APIs
        Core::hresult GetKeys(const ISharedStorageInspector::ScopeType eScope, const string& ns, RPC::IStringIterator*& keys, bool& success) override;
        Core::hresult GetNamespaces(const ISharedStorageInspector::ScopeType eScope, RPC::IStringIterator*& namespaces, bool& success) override;
        Core::hresult GetStorageSizes(const ISharedStorageInspector::ScopeType eScope, INamespaceSizeIterator*& storageList, bool& success) override;

        //ISharedStorageLimit APIs
        Core::hresult SetNamespaceStorageLimit(const ISharedStorageLimit::ScopeType eScope, const string& ns, const uint32_t size, bool& success) override;
        Core::hresult GetNamespaceStorageLimit(const ISharedStorageLimit::ScopeType eScope, const string& ns, StorageLimit& storageLimit, bool& success) override;

        //ISharedStorageCache APIs
        Core::hresult FlushCache() override;

        Exchange::IStore2* getRemoteStoreObject(const Exchange::ISharedStorageLimit::ScopeType eScope);
        void ValueChanged(const Exchange::ISharedStorage::ScopeType eScope, const string& ns, const string& key, const string& value);

    private:
        mutable Core::CriticalSection _adminLock;
        PluginHost::IShell* _service;
        Core::Sink<Store2Notification> _storeNotification;
        Exchange::IStore2* _psObject;
        Exchange::IStoreInspector* _psInspector;
        Exchange::IStoreLimit* _psLimit;
        Exchange::IStoreCache* _psCache;
        Exchange::IStore2* _csObject;
};

} // namespace Plugin
} // namespace WPEFramework
