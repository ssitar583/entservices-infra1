# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2024 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(PLUGIN_NAME MigrationRestorer)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

set(PLUGIN_MIGRATIONRESTORER_AUTOSTART true CACHE STRING "Automatically start plugin")
set(PLUGIN_MIGRATIONRESTORER_STARTUPORDER "" CACHE STRING "To configure startup order of plugin")
set(PLUGIN_MIGRATIONRESTORER_MODE "Off" CACHE STRING "Controls if the plugin should run in process or out of process")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)

add_library(${MODULE_NAME} SHARED
    MigrationRestorer.cpp
    Module.cpp)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)
        
set_source_files_properties(MigrationRestorer.cpp PROPERTIES COMPILE_FLAGS "-fexceptions")        

target_link_libraries(${MODULE_NAME}
    PRIVATE
        CompileSettingsDebug::CompileSettingsDebug
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${NAMESPACE}Definitions::${NAMESPACE}Definitions)

install(TARGETS ${MODULE_NAME}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${STORAGE_DIRECTORY}/plugins)


target_include_directories(${MODULE_NAME} PRIVATE ../helpers)

set(MIGRATIONRESTORER_INCLUDES $ENV{MIGRATIONRESTORER_INCLUDES})
separate_arguments(MIGRATIONRESTORER_INCLUDES)
include_directories(BEFORE ${MIGRATIONRESTORER_INCLUDES})   

install(TARGETS ${PLUGIN_IMPLEMENTATION}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
