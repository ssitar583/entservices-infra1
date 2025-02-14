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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../PersistentStore.h"
#include "ServiceMock.h"

using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::Test;
using ::WPEFramework::Plugin::PersistentStore;
using ::WPEFramework::PluginHost::IPlugin;

const auto kFile1 = "/tmp/persistentstore/l0test/persistentstoretest1";
const auto kFile2 = "/tmp/persistentstore/l0test/persistentstoretest2";
const uint8_t kFileContent[12]{ 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0xFE, 0x03, 0x01, 0xC1, 0x00, 0x01 };
const auto kFileContentSize = sizeof(kFileContent) / sizeof(uint8_t);

class APersistentStore : public Test {
protected:
    NiceMock<ServiceMock>* service;
    IPlugin* plugin;
    APersistentStore()
        : service(WPEFramework::Core::Service<NiceMock<ServiceMock>>::Create<NiceMock<ServiceMock>>())
        , plugin(WPEFramework::Core::Service<PersistentStore>::Create<IPlugin>())
    {
    }
    ~APersistentStore() override
    {
        plugin->Release();
        service->Release();
    }
};

TEST_F(APersistentStore, MovesFileWhenInitializedWithNewAndPreviousPath)
{
    WPEFramework::Core::File file1(kFile1);
    WPEFramework::Core::File file2(kFile2);
    file1.Destroy();
    file2.Destroy();
    WPEFramework::Core::Directory(file1.PathName().c_str()).CreatePath();
    ASSERT_THAT(file1.Create(), IsTrue());
    file1.Write(kFileContent, kFileContentSize);
    JsonObject config;
    config["path"] = kFile2;
    config["legacypath"] = kFile1;
    string configJsonStr;
    config.ToString(configJsonStr);
    ON_CALL(*service, ConfigLine())
        .WillByDefault(Return(configJsonStr));
    plugin->Initialize(service);
    plugin->Deinitialize(service);
    ASSERT_THAT(WPEFramework::Core::File(kFile1).Exists(), IsFalse());
    ASSERT_THAT(file2.Open(true), IsTrue());
    uint8_t buffer[1024];
    ASSERT_THAT(kFileContentSize, file2.Read(buffer, 1024));
    EXPECT_THAT(memcmp(buffer, kFileContent, kFileContentSize), Eq(0));
}
