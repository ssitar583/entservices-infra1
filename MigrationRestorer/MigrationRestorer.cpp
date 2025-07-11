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

#include <interfaces/IUserSettings.h>
#include "UtilsController.h"
#define USERSETTINGS_CALLSIGN "org.rdk.UserSettings"

// #include <interfaces/IHdmiCecSource.h>
#include <interfaces/IHdmiCecSink.h>
#define HDMICECSINK_CALLSIGN "org.rdk.HdmiCecSink"

#define TR181_SYSTEM_FRIENDLY_NAME "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.SystemServices.FriendlyName"
#include "rfcapi.h"

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
     *Register MigrationRestorer module as wpeframework plugin
     **/
    SERVICE_REGISTRATION(MigrationRestorer, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    MigrationRestorer::MigrationRestorer() : _service(nullptr) ,_userSettingsPlugin(nullptr)
    {
        SYSLOG(Logging::Startup, (_T("MigrationRestorer Constructor")));
        LOGINFO("MigrationRestorer's Constructor called");
        printf("MigrationRestorer's Constructor called\n");

        PopulateMigrationDataStore();
        PopulateMigrationDataStoreSchema();
        ValidateMigrationDataStore();
        
    }

    MigrationRestorer::~MigrationRestorer()
    {
        SYSLOG(Logging::Shutdown, (string(_T("MigrationRestorer Destructor"))));
        LOGINFO("MigrationRestorer's destructor called");
        printf("MigrationRestorer's destructor called\n");        
	
        if (_userSettingsPlugin) {
                 _userSettingsPlugin->Release();
                 _userSettingsPlugin = nullptr;
        }
    }


    void MigrationRestorer :: PopulateMigrationDataStore()
    {
        printf("MigrationRestorer'S PopulateMigrationDataStore Method called\n");
        char *jsonDoc = NULL;
        FILE *file = fopen(MIGRATION_DATA_STORE_PATH, "r");

        if (!file)
        {
            printf("The migration_data_store.json file failed to open\n");
            return ;
        }

        fseek(file, 0, SEEK_END);
        long numbytes = ftell(file);
        printf("The numbytes value is : %ld\n",numbytes);
        jsonDoc = (char*)malloc(sizeof(char)*(numbytes + 1));
        if(jsonDoc == NULL) {
            printf("Failed to allocated memory for jsonDoc inside PopulateMigrationDataStore function\n");
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

        //cJSON_Delete(inputRoot);
        

    }

    void MigrationRestorer :: PopulateMigrationDataStoreSchema()
    {
        printf("MigrationRestorer'S PopulateMigrationDataStoreSchema Method called\n");
        char *jsonDoc = NULL;
        std::string retVal = "";
        FILE *file = fopen(MIGARTION_SCHEMA_PATH, "r");

        if (!file)
        {
            printf("The entos_settings_migration.schema.json file failed to open\n");
            return ;
        }

        fseek(file, 0, SEEK_END);
        long numbytes = ftell(file);
        printf("The numbytes value is : %ld\n",numbytes);
        jsonDoc = (char*)malloc(sizeof(char)*(numbytes + 1));
        if(jsonDoc == NULL) {
            printf("Failed to allocated memory for jsonDoc inside PopulateMigrationDataStoreSchema function\n");
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

    void MigrationRestorer :: ValidateMigrationDataStore()
    {
        for(const auto& pair : inputMap)
        {
            string key = pair.first;
            auto itInput = inputMap.find(key);
            auto itSchema = schemaMap.find(key);

            if (itSchema == schemaMap.end()) 
            {
                std::cout << key << " : NOT FOUND" << std::endl;
                printf("The key is not found inside schema json file\n");
                continue;
            }

            cJSON* inputVal = itInput->second;
            cJSON* schemaVal = itSchema->second;

            if (!inputVal || !schemaVal) {
                printf("Null JSON pointers for key: %s\n", key.c_str());
                continue;
            }
            bool valid = validateValue(inputVal, schemaVal);
            if(valid)
            {
                validatedkeys.push_back(key);
		std::cout << key << " : Schema Validation Success" << std::endl;
            }
	    else{
		    std::cout << key << " : Schema Validation Failed" << std::endl;
	    }
        }
    }
	
    const std::string MigrationRestorer::Initialize(PluginHost::IShell* service)
    {
        LOGINFO("MigrationRestorer's Initialize method called");
        printf("MigrationRestorer's Initialize method called\n");
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
        printf("MigrationRestorer'S Deinitialize Method called\n");
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
            printf("Entered resolveRef function if block\n");
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
        printf("Enterning NULL in resolveRef function\n");
        return nullptr;
    }

    // Validate a value against a schema node
    bool MigrationRestorer::validateValue(cJSON* value, cJSON* schema) 
    {

        if (!schema) {
            printf("The schema cJSON object is NULL, returning false\n");
            return false;
        }

        // Handle $ref
        cJSON* refNode = cJSON_GetObjectItem(schema, "$ref");
        if (refNode && cJSON_IsString(refNode)) {
            printf("Entered $ref block for ref: %s\n", refNode->valuestring);
            cJSON* resolved = resolveRef(refNode->valuestring);
            if (!resolved) {
                printf("Failed to resolve $ref: %s\n", refNode->valuestring);
                return false;
            }
            return validateValue(value, resolved);
        }

        // Handle "type"
        cJSON* typeNode = cJSON_GetObjectItem(schema, "type");
        if (typeNode && cJSON_IsString(typeNode)) {
            std::string type = typeNode->valuestring;
            printf("Detected type: %s\n", type.c_str());

            if (type == "string") {
                if (!cJSON_IsString(value)) {
                    printf("Validation failed: value is not a string\n");
                    return false;
                }

                printf("Validating string value: %s\n", value->valuestring);

                cJSON* enumNode = cJSON_GetObjectItem(schema, "enum");
                if (enumNode) {
                    cJSON* item = nullptr;
                    cJSON_ArrayForEach(item, enumNode) {
                        if (cJSON_IsString(item)) {
                            if (strcasecmp(value->valuestring, item->valuestring) == 0) {
                                return true;
                            }
                        }
                    }
                    printf("String '%s' not found in enum (case-insensitive)\n", value->valuestring);
                    return false;
                }

                return true; // No enum to restrict, string is valid
            }

            if (type == "number" || type == "integer") {
                if (!cJSON_IsNumber(value)) {
                    printf("Validation failed: value is not a number\n");
                    return false;
                }

                cJSON* minNode = cJSON_GetObjectItem(schema, "minimum");
                cJSON* maxNode = cJSON_GetObjectItem(schema, "maximum");

                if (minNode && value->valuedouble < minNode->valuedouble) {
                    printf("Number %f is less than minimum %f\n", value->valuedouble, minNode->valuedouble);
                    return false;
                }
                if (maxNode && value->valuedouble > maxNode->valuedouble) {
                    printf("Number %f is greater than maximum %f\n", value->valuedouble, maxNode->valuedouble);
                    return false;
                }

                return true;
            }

            if (type == "array") {
                if (!cJSON_IsArray(value)) {
                    printf("Validation failed: value is not an array\n");
                    return false;
                }

                cJSON* itemsNode = cJSON_GetObjectItem(schema, "items");
                if (!itemsNode) {
                    printf("Schema missing 'items' definition for array\n");
                    return false;
                }

                if (cJSON_IsArray(itemsNode)) {
                    int arrSize = cJSON_GetArraySize(itemsNode);
                    for (int i = 0; i < cJSON_GetArraySize(value); ++i) {
                        cJSON* v = cJSON_GetArrayItem(value, i);
                        cJSON* s = cJSON_GetArrayItem(itemsNode, i % arrSize);
                        if (!validateValue(v, s)) {
                            printf("Validation failed for array item %d\n", i);
                            return false;
                        }
                    }
                    return true;
                } else {
                    int index = 0;
                    cJSON* arrItem = nullptr;
                    cJSON_ArrayForEach(arrItem, value) {
                        if (!validateValue(arrItem, itemsNode)) {
                            printf("Validation failed for array element %d\n", index);
                            return false;
                        }
                        index++;
                    }
                    return true;
                }
            }

            if (type == "object") {
                printf("Skipping object validation for now (type == object)\n");
                return true;
            }

            printf("Unknown type in schema: %s\n", type.c_str());
            return false;
        }

        // Root-level enum fallback (if no type specified)
        cJSON* enumNode = cJSON_GetObjectItem(schema, "enum");
        if (enumNode && cJSON_IsString(value)) {
            cJSON* item = nullptr;
            cJSON_ArrayForEach(item, enumNode) {
                if (cJSON_IsString(item)) {
                    if (strcasecmp(value->valuestring, item->valuestring) == 0) {
                        return true;
                    }
                }
            }
            printf("Root enum match failed for value: %s\n", value->valuestring);
            return false;
        }

        // No constraints, default to true
        printf("No type or enum found in schema — returning true\n");
        return true;
    }


    
    uint32_t MigrationRestorer :: ApplyDisplaySettings(const JsonObject& parameters, JsonObject& response)
    {
        printf("MigrationRestorer'S ApplyDisplaySettings Method called\n");
        for(const string& key : validatedkeys)
        {
            auto itInput = inputMap.find(key);

            cJSON* inputVal = itInput->second;
            
            if( key=="picture/resolution")
            {
                printf("Entered inside picture/resolution block\n" );
                    string resolution;
                    if (cJSON_IsString(inputVal))
                    {
                      resolution = inputVal->valuestring;

                    }
                    std::cout << "The value of resolution is :  " << resolution << std::endl ;
                    string videoDisplay = "Internal0";
                    bool persist = true;
                    bool isIgnoreEdid = true;
           //         bool success = true;
                    try
                    {
                        printf("Entered inside resolution try block\n" );
                        device::VideoOutputPort &vPort = device::Host::getInstance().getVideoOutputPort(videoDisplay);
                        vPort.setResolution(resolution, persist, isIgnoreEdid);
                    }
                    catch (const device::Exception& err)
                    {
                        printf("Entered inside resolution catch block\n" );
                        LOG_DEVICE_EXCEPTION2(videoDisplay, resolution);
             //           success = false;
                    }
            }


	        else if( key=="sound/dolbyvolume")
            {
                printf("Entered inside sound/dolbyvolume block\n" );

                    string audioPort = "SPEAKER0";
                    bool dolbyVolumeMode;
                    string sDolbyVolumeMode;
                    if (cJSON_IsString(inputVal))
                    {
                      sDolbyVolumeMode = inputVal->valuestring;
                      dolbyVolumeMode = (sDolbyVolumeMode == "on") ? true : false;
                    }
		            std::cout << "The value of dolbyVolumeMode is :  " << dolbyVolumeMode << std::endl ;
                    try
                    {
                        printf("Entered inside dolbyvolume try block\n" );
                        device::AudioOutputPort aPort = device::Host::getInstance().getAudioOutputPort(audioPort);
                        aPort.setDolbyVolumeMode (dolbyVolumeMode);
                    }
                    catch (const device::Exception& err)
                    {
                        printf("Entered inside dolbyvolume catch block\n" );
                        LOG_DEVICE_EXCEPTION2(audioPort, sDolbyVolumeMode);
                     //   success = false;
                    }                    
            }
            
            else if(key == "accessibility/voiceguidance")
            {
                printf("Entered inside accessibility/voiceguidance block\n" );
                
	            bool voiceguidancevalue = false;
                if (cJSON_IsString(inputVal))
                { 
                    printf("Entered inside voice guidance if-block\n" );
                    string set = inputVal->valuestring;
                    voiceguidancevalue = (set == "on") ? true : false;
                }

                PluginHost::IShell::state state;

                if ((Utils::getServiceState(_service, USERSETTINGS_CALLSIGN, state) == Core::ERROR_NONE) && (state != PluginHost::IShell::state::ACTIVATED))
                    Utils::activatePlugin(_service, USERSETTINGS_CALLSIGN);
                    
                if ((Utils::getServiceState(_service, USERSETTINGS_CALLSIGN, state) == Core::ERROR_NONE) && (state == PluginHost::IShell::state::ACTIVATED))
                {
                    printf("the usersettings plugin activated successfully\n");
                    ASSERT(_service != nullptr);

                    _userSettingsPlugin = _service->QueryInterfaceByCallsign<WPEFramework::Exchange::IUserSettings>(USERSETTINGS_CALLSIGN);
                    if (_userSettingsPlugin)
                    {
                        printf("Entered inside the _userSettingsPlugin block\n");   
                        if (_userSettingsPlugin->SetVoiceGuidance(voiceguidancevalue) == Core::ERROR_NONE)
                        {
                            printf("The set voiceguidance method executed successfully\n");
                        }
                        else
                        {
                            LOGERR("Failed to set voice guidance");
                        }
                    }
                }
                else
                {
                    LOGERR("Failed to activate %s", USERSETTINGS_CALLSIGN);
                }               
            }

            else if(key == "accessibility/closedcaption")
            {
                printf("Entered inside accessibility/closedcaption block\n" );
	            bool closedCaption = false;
                if (cJSON_IsString(inputVal))
                {
                    printf("Entered inside closedcaption if-block\n" );
                    string set = inputVal->valuestring;
                    closedCaption = (set == "on") ? true : false;
                }

                PluginHost::IShell::state state;

                if ((Utils::getServiceState(_service, USERSETTINGS_CALLSIGN, state) == Core::ERROR_NONE) && (state != PluginHost::IShell::state::ACTIVATED))
                    Utils::activatePlugin(_service, USERSETTINGS_CALLSIGN);

                if ((Utils::getServiceState(_service, USERSETTINGS_CALLSIGN, state) == Core::ERROR_NONE) && (state == PluginHost::IShell::state::ACTIVATED))
                {
                    printf("the usersettings plugin activated successfully\n");
                    ASSERT(_service != nullptr);

                    _userSettingsPlugin = _service->QueryInterfaceByCallsign<WPEFramework::Exchange::IUserSettings>(USERSETTINGS_CALLSIGN);
                    if (_userSettingsPlugin)
                    {
                        printf("Entered inside the _userSettingsPlugin block\n");
                        if (_userSettingsPlugin->SetCaptions(closedCaption) == Core::ERROR_NONE)
                        {
                            printf("The set closedcaption method executed successfully\n");
                        }
                        else
                        {
                            LOGERR("Failed to set closedcaption");
                        }
                    }
                }
                else
                {
                    LOGERR("Failed to activate %s", USERSETTINGS_CALLSIGN);
                }
            }

            else if(key == "system/presentationlanguage")
            {
                printf("Entered inside system/presentationlanguage block\n" );
	            std::string presentationLanguage ;
                if (cJSON_IsString(inputVal))
                {
                    printf("Entered inside presentationlanguage if-block\n" );
                    presentationLanguage = inputVal->valuestring;
                }

                PluginHost::IShell::state state;

                if ((Utils::getServiceState(_service, USERSETTINGS_CALLSIGN, state) == Core::ERROR_NONE) && (state != PluginHost::IShell::state::ACTIVATED))
                    Utils::activatePlugin(_service, USERSETTINGS_CALLSIGN);

                if ((Utils::getServiceState(_service, USERSETTINGS_CALLSIGN, state) == Core::ERROR_NONE) && (state == PluginHost::IShell::state::ACTIVATED))
                {
                    printf("the usersettings plugin activated successfully\n");
                    ASSERT(_service != nullptr);

                    _userSettingsPlugin = _service->QueryInterfaceByCallsign<WPEFramework::Exchange::IUserSettings>(USERSETTINGS_CALLSIGN);
                    if (_userSettingsPlugin)
                    {
                        printf("Entered inside the _userSettingsPlugin block\n");
                        if (_userSettingsPlugin->SetPresentationLanguage(presentationLanguage) == Core::ERROR_NONE)
                        {
                            printf("The set presentationlanguage method executed successfully\n");
                        }
                        else
                        {
                            LOGERR("Failed to set presentationlanguage");
                        }
                    }
                }
                else
                {
                    LOGERR("Failed to activate %s", USERSETTINGS_CALLSIGN);
                }
            }

            else if(key == "parental/parentalctrlpin")
            {
                printf("Entered inside parental/parentalctrlpin block\n" );
	            bool pinControl = false;
                if (cJSON_IsString(inputVal))
                { 
                    printf("Entered inside parentalctrlpin if-block\n" );
                    string set = inputVal->valuestring;
                    pinControl = (set == "on") ? true : false;
                }

                PluginHost::IShell::state state;

                if ((Utils::getServiceState(_service, USERSETTINGS_CALLSIGN, state) == Core::ERROR_NONE) && (state != PluginHost::IShell::state::ACTIVATED))
                    Utils::activatePlugin(_service, USERSETTINGS_CALLSIGN);

                if ((Utils::getServiceState(_service, USERSETTINGS_CALLSIGN, state) == Core::ERROR_NONE) && (state == PluginHost::IShell::state::ACTIVATED))
                {
                    printf("the usersettings plugin activated successfully\n");
                    ASSERT(_service != nullptr);

                    _userSettingsPlugin = _service->QueryInterfaceByCallsign<WPEFramework::Exchange::IUserSettings>(USERSETTINGS_CALLSIGN);
                    if (_userSettingsPlugin)
                    {
                        printf("Entered inside the _userSettingsPlugin block\n");
                        if (_userSettingsPlugin->SetPinControl(pinControl) == Core::ERROR_NONE)
                        {
                            printf("The set parentalctrlpin method executed successfully\n");
                        }
                        else
                        {
                            LOGERR("Failed to set parentalctrlpin");
                        }
                    }
                }
                else
                {
                    LOGERR("Failed to activate %s", USERSETTINGS_CALLSIGN);
                }
            }

            else if(key == "parental/pinonpurchase")
            {
                printf("Entered inside parental/pinonpurchase block\n" );
	            bool pinOnPurchase = false;
                if (cJSON_IsString(inputVal))
                { 
                    printf("Entered inside pinonpurchase if-block\n" );
                    string set = inputVal->valuestring;
                    pinOnPurchase = (set == "on") ? true : false;
                }

                PluginHost::IShell::state state;

                if ((Utils::getServiceState(_service, USERSETTINGS_CALLSIGN, state) == Core::ERROR_NONE) && (state != PluginHost::IShell::state::ACTIVATED))
                    Utils::activatePlugin(_service, USERSETTINGS_CALLSIGN);

                if ((Utils::getServiceState(_service, USERSETTINGS_CALLSIGN, state) == Core::ERROR_NONE) && (state == PluginHost::IShell::state::ACTIVATED))
                {
                    printf("the usersettings plugin activated successfully\n");
                    ASSERT(_service != nullptr);

                    _userSettingsPlugin = _service->QueryInterfaceByCallsign<WPEFramework::Exchange::IUserSettings>(USERSETTINGS_CALLSIGN);
                    if (_userSettingsPlugin)
                    {
                        printf("Entered inside the _userSettingsPlugin block\n");
                        if (_userSettingsPlugin->SetPinOnPurchase(pinOnPurchase) == Core::ERROR_NONE)
                        {
                            printf("The set pinonpurchase method executed successfully\n");
                        }
                        else
                        {
                            LOGERR("Failed to set pinonpurchase");
                        }
                    }
                }
                else
                {
                    LOGERR("Failed to activate %s", USERSETTINGS_CALLSIGN);
                }
            }

            else if(key == "hdmi/hdmicontrol")
            {
                printf("Entered inside hdmi/hdmicontrol block\n" );
	            bool enabled = false;
                Exchange::IHdmiCecSink::HdmiCecSinkSuccess success;
                if (cJSON_IsString(inputVal))
                { 
                    printf("Entered inside hdmi/hdmicontrol if-block\n" );
                    string set = inputVal->valuestring;
                    enabled = (set == "on") ? true : false;
                }

                PluginHost::IShell::state state;

                if ((Utils::getServiceState(_service, HDMICECSINK_CALLSIGN, state) == Core::ERROR_NONE) && (state != PluginHost::IShell::state::ACTIVATED))
                    Utils::activatePlugin(_service, HDMICECSINK_CALLSIGN);

                if ((Utils::getServiceState(_service, HDMICECSINK_CALLSIGN, state) == Core::ERROR_NONE) && (state == PluginHost::IShell::state::ACTIVATED))
                {
                    printf("the usersettings plugin activated successfully\n");
                    ASSERT(_service != nullptr);

                    _hdmiCecSinkPlugin = _service->QueryInterfaceByCallsign<WPEFramework::Exchange::IHdmiCecSink>(HDMICECSINK_CALLSIGN);
                    if (_hdmiCecSinkPlugin)
                    {
                        printf("Entered inside the _hdmiCecSink Plugin block\n");
                        if (_hdmiCecSinkPlugin->SetEnabled(enabled,success) == Core::ERROR_NONE)
                        {
                            printf("The set hdmi/hdmicontrol method executed successfully\n");
                        }
                        else
                        {
                            LOGERR("Failed to set hdmi/hdmicontrol");
                        }
                    }
                }
                else
                {
                    LOGERR("Failed to activate %s", HDMICECSINK_CALLSIGN);
                }
            }

            else if(key == "system/devicelocation")
            {
                std::string friendlyName;

                if (cJSON_IsString(inputVal))
                {
                    printf("Entered inside devicelocation if-block\n" );
                    friendlyName  = inputVal->valuestring;
                }

                JsonObject params;
                params["friendlyName"] = friendlyName;
                sendNotify("onFriendlyNameChanged", params);
                WDMP_STATUS status = WDMP_FAILURE;
                //write to persistence storage
                status = setRFCParameter((char*)"thunderapi",
                       TR181_SYSTEM_FRIENDLY_NAME,friendlyName.c_str(),WDMP_STRING);
                if (WDMP_SUCCESS == status){
                    LOGINFO("Success Setting the friendly name value\n");
                }
                else {
                    LOGINFO("Failed Setting the friendly name value %s\n",getRFCErrorString(status));
                }
            }

            else if(key == "picture/zoomsetting")
            {
                printf("Entered inside picture/zoomsetting block\n" );
                    string zoomSetting;
                    if (cJSON_IsString(inputVal))
                    {
                      zoomSetting = inputVal->valuestring;
                    }
                    try
                    {
                        printf("Entered inside picture/zoomsetting try block\n" );
                        if (device::Host::getInstance().getVideoDevices().size() < 1)
                        {
                            LOGINFO("DSMGR_NOT_RUNNING");
                            //returnResponse(false);
                        }

                        // TODO: why is this always the first one in the list?
                        device::VideoDevice &decoder = device::Host::getInstance().getVideoDevices().at(0);
                        decoder.setDFC(zoomSetting);
                        printf("picture/zoomsetting try block executed\n" );
                    }
                    catch(const device::Exception& err)
                    {
                        printf("Entered inside picture/zoomsetting catch block\n" );
                        LOG_DEVICE_EXCEPTION1(zoomSetting);
                        //success = false;
                    }
            }  
        }
	    return 0;
    }

 } // namespace Plugin
  
}// namespace WPEFramework
