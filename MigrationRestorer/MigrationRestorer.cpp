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

#include "MigrationRestorer.h"
#include "cJSON.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>

#include "UtilsCStr.h"
#include "UtilsIarm.h"
#include "UtilsJsonRpc.h"
#include "UtilsString.h"
#include "UtilsisValidInt.h"
#include "UtilsSynchroIarm.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "exception.hpp"
#include "dsMgr.h"
#include "dsUtl.h"
#include "dsError.h"
#include "dsDisplay.h"
#include "host.hpp"
#include "videoOutputPort.hpp"
#include "videoOutputPortType.hpp"
#include "videoOutputPortConfig.hpp"
#include "videoResolution.hpp"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

#define MIGRATION_DATA_STORE_PATH "/opt/secure/migration/migration_data_store.json"
#define MIGARTION_SCHEMA_PATH "/opt/secure/migration/entos_settings_migration.schema.json"

// TODO: remove this
#define registerMethod(...) for (uint8_t i = 1; GetHandler(i); i++) GetHandler(i)->Register<JsonObject, JsonObject>(__VA_ARGS__)
//#define registerMethodLockedApi(...) for (uint8_t i = 1; GetHandler(i); i++) Utils::Synchro::RegisterLockedApiForHandler(GetHandler(i), __VA_ARGS__)

namespace WPEFramework
{

    namespace {

        static Plugin::Metadata<Plugin::MigrationRestorer> metadata(
            // Version (Major, Minor, Patch)
            API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {}
        );
    }

    namespace Plugin
    {

    /*
     *Register MigrationPreparer module as wpeframework plugin
     **/
    SERVICE_REGISTRATION(MigrationRestorer, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    MigrationRestorer::MigrationRestorer() : _service(nullptr)
    {
        SYSLOG(Logging::Startup, (_T("MigrationRestorer Constructor")));
        LOGINFO("MigrationRestorer's Constructor called");
        printf("MigrationRestorer's Constructor called");

        PopulateMigrationDataStore();
        PopulateMigrationDataStoreSchema();
        
    }

    MigrationRestorer::~MigrationRestorer()
    {
        SYSLOG(Logging::Shutdown, (string(_T("MigrationRestorer Destructor"))));
        LOGINFO("MigrationRestorer's destructor called");
        printf("MigrationRestorer's destructor called");        

        
    }


    void MigrationRestorer :: PopulateMigrationDataStore()
    {
        printf("MigrationRestorer'S PopulateMigrationDataStore Method called");
        char *jsonDoc = NULL;
        FILE *file = fopen(MIGRATION_DATA_STORE_PATH, "r");

        if (!file)
            return ;

        fseek(file, 0, SEEK_END);
        long numbytes = ftell(file);
        jsonDoc = (char*)malloc(sizeof(char)*(numbytes + 1));
        if(jsonDoc == NULL) {
            fclose(file);
            return ;
        }

        fseek(file, 0, SEEK_SET);
        if(numbytes >= 0) {
            (void)fread(jsonDoc, numbytes, 1, file);
        }
        fclose(file);
        file =NULL;
        jsonDoc[numbytes] = '\0';

        cJSON *inputRoot = cJSON_Parse(jsonDoc);
        free(jsonDoc);
        jsonDoc = NULL;
        if (!inputRoot) {
            std::cout << "Failed to parse data.json\n";
            return ;
        }
        // ParseInputJson(parserRoot, "");  
        // Store all input data in memory
        cJSON* item = nullptr;
        cJSON_ArrayForEach(item, inputRoot) {
            inputMap[item->string] = item;
        }

        cJSON_Delete(inputRoot);
        

    }

    void MigrationRestorer :: PopulateMigrationDataStoreSchema()
    {
        printf("MigrationRestorer'S PopulateMigrationDataStoreSchema Method called");
        char *jsonDoc = NULL;
        std::string retVal = "";
        FILE *file = fopen(MIGARTION_SCHEMA_PATH, "r");

        if (!file)
            return ;

        fseek(file, 0, SEEK_END);
        long numbytes = ftell(file);
        jsonDoc = (char*)malloc(sizeof(char)*(numbytes + 1));
        if(jsonDoc == NULL) {
            fclose(file);
            return ;
        }

        fseek(file, 0, SEEK_SET);
        if(numbytes >= 0) {
            (void)fread(jsonDoc, numbytes, 1, file);
        }
        fclose(file);
        file = NULL;
        jsonDoc[numbytes] = '\0';

        schemaRoot = cJSON_Parse(jsonDoc);
        free(jsonDoc);
        jsonDoc = NULL;


        if (!schemaRoot) {
        std::cout << "Failed to parse schema.json\n";
        // cJSON_Delete(schemaRoot);
        return ;
        }

        // extractFromSchema(schema, "");
        // Store all schema properties in memory
        cJSON* properties = cJSON_GetObjectItem(schemaRoot, "properties");
        if (properties) {
            cJSON* prop = nullptr;
            cJSON_ArrayForEach(prop, properties) {
                schemaMap[prop->string] = prop;
            }

        // cJSON_Delete(schemaRoot);
     }
    }	

    const std::string MigrationRestorer::Initialize(PluginHost::IShell* service)
    {
        LOGINFO("MigrationRestorer's Initialize method called");
        printf("MigrationRestorer's Initialize method called");
        string message="";

        ASSERT(nullptr != service);
        ASSERT(nullptr == _service);

        SYSLOG(Logging::Startup, (_T("MigrationRestorer::Initialize: PID=%u"), getpid()));

        _service = service;
        _service->AddRef();
        
        RegisterAll();
        
        if (0 != message.length())
        {
           Deinitialize(service);
        }

        return message;
    }

    void MigrationRestorer::Deinitialize(PluginHost::IShell* service)
    {
        LOGINFO("MigrationRestorer'S Deinitialize Method called");
        printf("MigrationRestorer'S Deinitialize Method called");
        ASSERT(_service == service);

        SYSLOG(Logging::Shutdown, (string(_T("MigrationRestorer::Deinitialize"))));

       
        UnregisterAll();
        _connectionId = 0;
        _service->Release();
        _service = nullptr;
        SYSLOG(Logging::Shutdown, (string(_T("MigrationRestorer de-initialised"))));
    }

    std::string MigrationRestorer::Information() const
    {
       return (string("{\"service\": \"") + string("org.rdk.MigrationRestorer") + string("\"}"));
    }

    void MigrationRestorer::RegisterAll()
    {
        printf("MigrationRestorer'S RegisterAll Method called");
        registerMethod("ApplyDisplaySettings", &MigrationRestorer::ApplyDisplaySettings, this);
    }

    void MigrationRestorer::UnregisterAll()
    {
        printf("MigrationRestorer'S UnregisterAll Method called");
    }

    // bool MigrationRestorer :: validateKey(const string& key,cJSON* value)


    // Helper to resolve $ref in schema, including /items/N
    cJSON* MigrationRestorer :: resolveRef( const std::string& ref) {
        if (ref.rfind("#/definitions/", 0) == 0) {
            std::string path = ref.substr(strlen("#/definitions/"));
            size_t slash = path.find('/');
            std::string defName = path.substr(0, slash);
            cJSON* node = cJSON_GetObjectItem(cJSON_GetObjectItem(schemaRoot, "definitions"), defName.c_str());
            while (slash != std::string::npos && node) {
                path = path.substr(slash + 1);
                slash = path.find('/');
                std::string key = path.substr(0, slash);
                if (key == "items") {
                    node = cJSON_GetObjectItem(node, "items");
                } else {
                    int idx = std::atoi(key.c_str());
                    node = cJSON_GetArrayItem(node, idx);
                }
            }
            return node;
        }
        return nullptr;
    }


    // Helper to check if a value is in an enum array
    bool MigrationRestorer :: isInEnum(cJSON* enumNode, const std::string& value) {
        if (!enumNode || !cJSON_IsArray(enumNode)) return false;
        cJSON* item = nullptr;
        cJSON_ArrayForEach(item, enumNode) {
            if (cJSON_IsString(item) && value == item->valuestring) return true;
        }
        return false;
    }

    // Validate a value against a schema node
    bool MigrationRestorer :: validateValue(cJSON* value, cJSON* schema) {
        if (!schema) return false;

        // Handle $ref
        cJSON* refNode = cJSON_GetObjectItem(schema, "$ref");
        if (refNode && cJSON_IsString(refNode)) {
            cJSON* resolved = resolveRef(refNode->valuestring);
            if (!resolved) return false;
            return validateValue(value, resolved);
        }

        // Handle type
        cJSON* typeNode = cJSON_GetObjectItem(schema, "type");
        if (typeNode && cJSON_IsString(typeNode)) {
            std::string type = typeNode->valuestring;
            if (type == "string") {
                if (!cJSON_IsString(value)) return false;
                cJSON* enumNode = cJSON_GetObjectItem(schema, "enum");
                if (enumNode && !isInEnum(enumNode, value->valuestring)) return false;
                return true;
            }
            if (type == "number" || type == "integer") {
                if (!cJSON_IsNumber(value)) return false;
                cJSON* minNode = cJSON_GetObjectItem(schema, "minimum");
                cJSON* maxNode = cJSON_GetObjectItem(schema, "maximum");
                if (minNode && value->valuedouble < minNode->valuedouble) return false;
                if (maxNode && value->valuedouble > maxNode->valuedouble) return false;
                return true;
            }
            if (type == "array") {
                if (!cJSON_IsArray(value)) return false;
                cJSON* itemsNode = cJSON_GetObjectItem(schema, "items");
                if (!itemsNode) return false;
                // If items is an array, validate each item by index
                if (cJSON_IsArray(itemsNode)) {
                    int arrSize = cJSON_GetArraySize(itemsNode);
                    for (int i = 0; i < cJSON_GetArraySize(value); ++i) {
                        cJSON* v = cJSON_GetArrayItem(value, i);
                        cJSON* s = cJSON_GetArrayItem(itemsNode, i % arrSize);
                        if (!validateValue(v, s)) return false;
                    }
                    return true;
                } else {
                    // items is a schema, validate each element
                    cJSON* arrItem = nullptr;
                    cJSON_ArrayForEach(arrItem, value) {
                        if (!validateValue(arrItem, itemsNode)) return false;
                    }
                    return true;
                }
            }
            if (type == "object") {
                // Not handled in this minimal example
                return false;
            }
        }
        // Handle enum at root
        cJSON* enumNode = cJSON_GetObjectItem(schema, "enum");
        if (enumNode && cJSON_IsString(value) && !isInEnum(enumNode, value->valuestring)) return false;

        return true;
    }

    
    uint32_t MigrationRestorer :: ApplyDisplaySettings(const JsonObject& parameters, JsonObject& response)
    {
        printf("MigrationRestorer'S ApplyDisplaySettings Method called\n");
        string key;
        for(const auto& pair : inputMap)
        {
            key = pair.first;
            auto itInput = inputMap.find(key);
            auto itSchema = schemaMap.find(key);
            if (itInput == inputMap.end() || itSchema == schemaMap.end()) 
            {
                std::cout << key << " : NOT FOUND" << std::endl;
                printf("The key is not found inside json file\n");
                return false;
            }

            
            if( key=="picture/resolution")
            {
                printf("Entered inside picture/resolution block\n" );
                bool valid = validateValue(itInput->second, itSchema->second);
                if(valid)
                {
                    printf("picture/resolution validation success\n" );
                    cJSON* values = itInput->second;
                    string resolution;
                    if (cJSON_IsString(values))
                    {
                      resolution = values->valuestring;

                    }
                    string videoDisplay = "HDMI0";
                    bool persist = true;
                    bool isIgnoreEdid = true;
                    bool success = true;
                    try
                    {
                        printf("Entered inside resolution try block\n" );
                        device::VideoOutputPort &vPort = device::Host::getInstance().getVideoOutputPort(videoDisplay);
                        vPort.setResolution(resolution, persist, isIgnoreEdid);
                    }
                    catch (const device::Exception& err)
                    {
                        printf("Entered inside resolution catch block\n" );
                       // LOG_DEVICE_EXCEPTION2(videoDisplay, resolution);
                        success = false;
                    }
                    returnResponse(success);
                }

            }


	        else if( key=="sound/dolbyvolume")
            {
                printf("Entered inside sound/dolbyvolume block\n" );
                bool valid = validateValue(itInput->second, itSchema->second);
                if(valid)
                {
                    printf("sound/dolbyvolume validation success\n" );
                    //apply key
                }
            }
            else if(key == "sound/enhancespeech")
            {
                printf("Entered inside sound/enhancespeech block\n" );
                bool valid = validateValue(itInput->second, itSchema->second);
                if(valid)
                {
                    printf("sound/enhancespeech validation success\n" );
                    //apply settings
                }
            }
            else if (key == "sound/opticalformat")
            {
                printf("Entered inside sound/opticalformat block\n" );
                bool valid = validateValue(itInput->second, itSchema->second);
                if(valid)
                {
                    printf("sound/opticalformat validation success\n" );
                    //apply settings
                }
            }
            else if (key == "sound/hdmi/earc/audioformat")
            {
                bool valid = validateValue(itInput->second, itSchema->second);
                if(valid)
                {
                    printf("sound/hdmi/earc/audioformat validation success\n" );
                    //apply settings
                }
            }
            else if (key == "system/timezone")
            {
                printf("Entered inside system/timezone block\n" );
                bool valid = validateValue(itInput->second, itSchema->second);
                if(valid)
                {
                    printf("system/timezone validation success\n" );
                    //apply settings
                }
            }
            
        }
	return 0;
    }

 } // namespace Plugin
  
 }// namespace WPEFramework
