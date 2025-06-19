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

#pragma once
#include "cJSON.h"
#include "Module.h"
#include "UtilsLogging.h"
#include "tracing/Logging.h"
#include "UtilsJsonRpc.h"
#include <mutex>



namespace WPEFramework {
namespace Plugin {

    class MigrationRestorer: public PluginHost::IPlugin, public PluginHost::JSONRPC
    {         
        public:
            // We do not allow this plugin to be copied !!
            MigrationRestorer(const MigrationRestorer&) = delete;
            MigrationRestorer& operator=(const MigrationRestorer&) = delete;

            MigrationRestorer();
            virtual ~MigrationRestorer();

            BEGIN_INTERFACE_MAP(MigrationRestorer)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            END_INTERFACE_MAP

            //  IPlugin methods
            // -------------------------------------------------------------------------------------------------------
            const string Initialize(PluginHost::IShell* service) override;
            void Deinitialize(PluginHost::IShell* service) override;
            string Information() const override;
            void RegisterAll();
            void UnregisterAll();
            void PopulateMigrationDataStore();
            void PopulateMigrationDataStoreSchema();
            cJSON* resolveRef( const std::string& ref);
            bool isInEnum(cJSON* enumNode, const std::string& value);
            bool validateValue(cJSON* value, cJSON* schema);
           

       
            uint32_t ApplyDisplaySettings(const JsonObject& parameters, JsonObject& response);
            
        private:
            PluginHost::IShell* _service{};
            uint32_t _connectionId{};

            std::map<std::string, cJSON*> inputMap;
            std::map<std::string, cJSON*> schemaMap;
            cJSON* schemaRoot = nullptr;
    };

} // namespace Plugin
} // namespace WPEFramework
