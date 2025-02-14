#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mntent.h>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <cstdio>
#include "USBMassStorage.h"
#include "USBMassStorageImplementation.h"
#include "ServiceMock.h"
#include "USBDeviceMock.h"
#include "COMLinkMock.h"
#include "CommunicatorMock.h"
#include "WrapsMock.h"
#include "ThunderPortability.h"

using ::testing::NiceMock;
using namespace WPEFramework;

class USBMassStorageTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::USBMassStorage> plugin;
    Core::JSONRPC::Handler& handler;
    Core::JSONRPC::Context connection;
    NiceMock<ServiceMock> service;
    Core::JSONRPC::Message message;
    NiceMock<COMLinkMock> comLinkMock;
    Core::ProxyType<Plugin::USBMassStorageImplementation> USBMassStorageImpl;
    string response;
    USBDeviceMock *p_usbDeviceMock = nullptr;
    WrapsImplMock *p_wrapsImplMock   = nullptr;
    CommunicatorClientMock  *p_communicatorClientMock  = nullptr;
    ServiceMock  *p_serviceMock  = nullptr;
    USBMassStorageTest()
        : plugin(Core::ProxyType<Plugin::USBMassStorage>::Create())
        , handler(*plugin)
        , connection(1,0,"")
    {
        p_serviceMock = new NiceMock <ServiceMock>;

        p_usbDeviceMock = new NiceMock <USBDeviceMock>;

        p_wrapsImplMock  = new NiceMock <WrapsImplMock>;
        Wraps::setImpl(p_wrapsImplMock);

        p_communicatorClientMock = new NiceMock <CommunicatorClientMock>;
        WPEFramework::RPC::CommunicatorClient::setImpl(p_communicatorClientMock);
        
        EXPECT_CALL(*p_communicatorClientMock, Open(::testing::_, ::testing::_, ::testing::_))
            .WillOnce(testing::Return(p_serviceMock));

        EXPECT_CALL(*p_serviceMock, QueryInterface(::testing::_))
            .WillOnce(testing::Return(p_usbDeviceMock));

        EXPECT_CALL(*p_usbDeviceMock, AddRef())
            .WillOnce(testing::Return());

        ON_CALL(comLinkMock, Instantiate(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](const RPC::Object& object, const uint32_t waitTime, uint32_t& connectionId) {
                USBMassStorageImpl = Core::ProxyType<Plugin::USBMassStorageImplementation>::Create();
                return &USBMassStorageImpl;
                }));

        plugin->Initialize(&service);
    }

    virtual ~USBMassStorageTest() override
    {

        ON_CALL(*p_communicatorClientMock, Close(::testing::_))
            .WillByDefault(testing::Return(3000));

        plugin->Deinitialize(&service);

        WPEFramework::RPC::CommunicatorClient::setImpl(nullptr);
        if (p_communicatorClientMock != nullptr)
        {
            delete p_communicatorClientMock;
            p_communicatorClientMock = nullptr;
        }

        if (p_serviceMock != nullptr)
        {
            delete p_serviceMock;
            p_serviceMock = nullptr;
        }

        if (p_usbDeviceMock != nullptr)
        {
            delete p_usbDeviceMock;
            p_usbDeviceMock = nullptr;
        }

        Wraps::setImpl(nullptr);
        if (p_wrapsImplMock != nullptr)
        {
            delete p_wrapsImplMock;
            p_wrapsImplMock = nullptr;
        }
    }
};

TEST_F(USBMassStorageTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getDeviceList")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getMountPoints")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getPartitionInfo")));
}


TEST_F(USBMassStorageTest, getdevicelistFailed_devicelist_empty)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getDeviceList"), _T("{}"), response));
}

TEST_F(USBMassStorageTest, getdevicelistFailed_GetDeviceList_failed)
{
    std::list<Exchange::IUSBDevice::USBDevice> usbDeviceList;
    Exchange::IUSBDevice::USBDevice usbDevice1;
    usbDevice1.deviceClass = LIBUSB_CLASS_MASS_STORAGE;
    usbDevice1.deviceSubclass = 0x12;
    usbDevice1.deviceName = "Device1";
    usbDevice1.devicePath = "";
    usbDeviceList.emplace_back(usbDevice1);
    auto mockIterator = Core::Service<RPC::IteratorType<Exchange::IUSBDevice::IUSBDeviceIterator>>::Create<Exchange::IUSBDevice::IUSBDeviceIterator>(usbDeviceList);

    EXPECT_CALL(*p_usbDeviceMock, GetDeviceList(testing::_))
    .WillOnce([&](Exchange::IUSBDevice::IUSBDeviceIterator*& devices) {
        devices = mockIterator;
        return Core::ERROR_GENERAL;
    });

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getDeviceList"), _T("{}"), response));

}

TEST_F(USBMassStorageTest, getMountPointsFailed_devicename_empty)
{
    EXPECT_EQ(Core::ERROR_INVALID_PARAMETER, handler.Invoke(connection, _T("getMountPoints"), _T("{}"), response));
}

TEST_F(USBMassStorageTest, getMountPointsFailed_devicename_mismatch)
{
    std::list<Exchange::IUSBDevice::USBDevice> usbDeviceList;
    Exchange::IUSBDevice::USBDevice usbDevice1;
    usbDevice1.deviceClass = LIBUSB_CLASS_MASS_STORAGE;
    usbDevice1.deviceSubclass = 0x12;
    usbDevice1.deviceName = "001/002";
    usbDevice1.devicePath = "/dev/sda";
    usbDeviceList.emplace_back(usbDevice1); 
    auto mockIterator = Core::Service<RPC::IteratorType<Exchange::IUSBDevice::IUSBDeviceIterator>>::Create<Exchange::IUSBDevice::IUSBDeviceIterator>(usbDeviceList);

    EXPECT_CALL(*p_usbDeviceMock, GetDeviceList(testing::_))
    .WillOnce([&](Exchange::IUSBDevice::IUSBDeviceIterator*& devices) {
        devices = mockIterator;
        return Core::ERROR_NONE;
    });

    handler.Invoke(connection, _T("getDeviceList"), _T("{}"), response);

    EXPECT_EQ(Core::ERROR_INVALID_DEVICENAME, handler.Invoke(connection, _T("getMountPoints"), _T("{\"deviceName\": \"001\"}"), response));

}

TEST_F(USBMassStorageTest, getMountPointsFailed_umount_fail)
{
    std::list<Exchange::IUSBDevice::USBDevice> usbDeviceList;
    Exchange::IUSBDevice::USBDevice usbDevice1;
    usbDevice1.deviceClass = LIBUSB_CLASS_MASS_STORAGE;
    usbDevice1.deviceSubclass = 0x12;
    usbDevice1.deviceName = "001/002";
    usbDevice1.devicePath = "/dev/sda";
    usbDeviceList.emplace_back(usbDevice1);
    auto mockIterator = Core::Service<RPC::IteratorType<Exchange::IUSBDevice::IUSBDeviceIterator>>::Create<Exchange::IUSBDevice::IUSBDeviceIterator>(usbDeviceList);

    EXPECT_CALL(*p_usbDeviceMock, GetDeviceList(testing::_))
    .WillOnce([&](Exchange::IUSBDevice::IUSBDeviceIterator*& devices) {
        devices = mockIterator;
        return Core::ERROR_NONE;
    });

    handler.Invoke(connection, _T("getDeviceList"), _T("{}"), response);

    EXPECT_CALL(*p_wrapsImplMock, umount(::testing::_))
    .WillRepeatedly([](const char* path) {
        return -1;
    });

    EXPECT_EQ(Core::ERROR_INVALID_DEVICENAME, handler.Invoke(connection, _T("getMountPoints"), _T("{\"deviceName\": \"001\"}"), response));

}

TEST_F(USBMassStorageTest, getMountPointsFailed_mount_fail)
{
    std::list<Exchange::IUSBDevice::USBDevice> usbDeviceList;
    Exchange::IUSBDevice::USBDevice usbDevice1;
    usbDevice1.deviceClass = LIBUSB_CLASS_MASS_STORAGE;
    usbDevice1.deviceSubclass = 0x12;
    usbDevice1.deviceName = "001/002";
    usbDevice1.devicePath = "/dev/sda";
    usbDeviceList.emplace_back(usbDevice1); 
    auto mockIterator = Core::Service<RPC::IteratorType<Exchange::IUSBDevice::IUSBDeviceIterator>>::Create<Exchange::IUSBDevice::IUSBDeviceIterator>(usbDeviceList);

    EXPECT_CALL(*p_usbDeviceMock, GetDeviceList(testing::_))
    .WillOnce([&](Exchange::IUSBDevice::IUSBDeviceIterator*& devices) {
        devices = mockIterator;
        return Core::ERROR_NONE;
    });

    handler.Invoke(connection, _T("getDeviceList"), _T("{}"), response);

    static int index = 1;

    EXPECT_CALL(*p_wrapsImplMock, stat(::testing::_, ::testing::_))
    .WillRepeatedly([](const std::string& path, struct stat* info) {
        std::string expectedPath = "/tmp/media/usb" + std::to_string(index);

        if (path == expectedPath) {
            index++;
            return -1;
        }
        info->st_mode = S_IFDIR;
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, mkdir(::testing::_, ::testing::_))
    .WillRepeatedly([](const char* path, mode_t mode) {
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, mount(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
    .WillRepeatedly([](const char* source, const char* target, const char* filesystemtype, unsigned long mountflags, const void* data) {
        return -1;
    });

    EXPECT_CALL(*p_wrapsImplMock, umount(::testing::_))
    .WillRepeatedly([](const char* path) {
        return 0;
    });

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getMountPoints"), _T("{\"deviceName\": \"001/002\"}"), response));

    usbDeviceList.clear();

}

TEST_F(USBMassStorageTest, getMountPointsFailed_mkdir_fail)
{
    std::list<Exchange::IUSBDevice::USBDevice> usbDeviceList;
    Exchange::IUSBDevice::USBDevice usbDevice1;
    usbDevice1.deviceClass = LIBUSB_CLASS_MASS_STORAGE;
    usbDevice1.deviceSubclass = 0x12;
    usbDevice1.deviceName = "001/002";
    usbDevice1.devicePath = "/dev/sda";
    usbDeviceList.emplace_back(usbDevice1); 
    auto mockIterator = Core::Service<RPC::IteratorType<Exchange::IUSBDevice::IUSBDeviceIterator>>::Create<Exchange::IUSBDevice::IUSBDeviceIterator>(usbDeviceList);

    EXPECT_CALL(*p_usbDeviceMock, GetDeviceList(testing::_))
    .WillOnce([&](Exchange::IUSBDevice::IUSBDeviceIterator*& devices) {
        devices = mockIterator;
        return Core::ERROR_NONE;
    });

    handler.Invoke(connection, _T("getDeviceList"), _T("{}"), response);

    static int index = 1;

    EXPECT_CALL(*p_wrapsImplMock, stat(::testing::_, ::testing::_))
    .WillRepeatedly([](const std::string& path, struct stat* info) {
        std::string expectedPath = "/tmp/media/usb" + std::to_string(index);

        if (path == expectedPath) {
            index++;
            return -1;
        }
        info->st_mode = S_IFDIR;
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, mkdir(::testing::_, ::testing::_))
    .WillRepeatedly([](const char* path, mode_t mode) {
        return -1;
    });

    EXPECT_CALL(*p_wrapsImplMock, umount(::testing::_))
    .WillRepeatedly([](const char* path) {
        return 0;
    });

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getMountPoints"), _T("{\"deviceName\": \"001/002\"}"), response));

    usbDeviceList.clear();

}

TEST_F(USBMassStorageTest, getPartitionInfoFailed_mountpoints_empty)
{
    EXPECT_EQ(Core::ERROR_INVALID_PARAMETER, handler.Invoke(connection, _T("getPartitionInfo"), _T("{}"), response));
}

TEST_F(USBMassStorageTest, getPartitionInfoFail_statfs_fail)
{
    std::list<Exchange::IUSBDevice::USBDevice> usbDeviceList;
    Exchange::IUSBDevice::USBDevice usbDevice1;
    usbDevice1.deviceClass = LIBUSB_CLASS_MASS_STORAGE;
    usbDevice1.deviceSubclass = 0x12;
    usbDevice1.deviceName = "001/002";
    usbDevice1.devicePath = "/dev/sda";
    usbDeviceList.emplace_back(usbDevice1); 

    auto mockIterator = Core::Service<RPC::IteratorType<Exchange::IUSBDevice::IUSBDeviceIterator>>::Create<Exchange::IUSBDevice::IUSBDeviceIterator>(usbDeviceList);

    EXPECT_CALL(*p_usbDeviceMock, GetDeviceList(testing::_))
    .WillOnce([&](Exchange::IUSBDevice::IUSBDeviceIterator*& devices) {
        devices = mockIterator;
        return Core::ERROR_NONE;  // Simulate successful return
    });

    handler.Invoke(connection, _T("getDeviceList"), _T("{}"), response);

    static int index = 1;

    EXPECT_CALL(*p_wrapsImplMock, stat(::testing::_, ::testing::_))
    .WillRepeatedly([](const std::string& path, struct stat* info) {
        std::string expectedPath = "/tmp/media/usb" + std::to_string(index);

        if (path == expectedPath) {
            index++;
            return -1;
        }
        info->st_mode = S_IFDIR;
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, mkdir(::testing::_, ::testing::_))
    .WillRepeatedly([](const char* path, mode_t mode) {
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, mount(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
    .WillRepeatedly([](const char* source, const char* target, const char* filesystemtype, unsigned long mountflags, const void* data) {
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, umount(::testing::_))
    .WillRepeatedly([](const char* path) {
        return 0;
    });

    handler.Invoke(connection, _T("getMountPoints"), _T("{\"deviceName\": \"001/002\"}"), response);
    EXPECT_CALL(*p_wrapsImplMock, statfs(::testing::_, ::testing::_))
    .WillOnce([](const char* path, struct statfs* buf) -> int {
        return -1; 
    });

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getPartitionInfo"), _T("{\"mountPath\": \"/tmp/media/usb1\"}"), response));

    usbDeviceList.clear();

}

TEST_F(USBMassStorageTest, getPartitionInfoFail_statvfs_fail)
{
    std::list<Exchange::IUSBDevice::USBDevice> usbDeviceList;
    Exchange::IUSBDevice::USBDevice usbDevice1;
    usbDevice1.deviceClass = LIBUSB_CLASS_MASS_STORAGE;
    usbDevice1.deviceSubclass = 0x12;
    usbDevice1.deviceName = "001/002";
    usbDevice1.devicePath = "/dev/sda";
    usbDeviceList.emplace_back(usbDevice1); 

    auto mockIterator = Core::Service<RPC::IteratorType<Exchange::IUSBDevice::IUSBDeviceIterator>>::Create<Exchange::IUSBDevice::IUSBDeviceIterator>(usbDeviceList);

EXPECT_CALL(*p_usbDeviceMock, GetDeviceList(testing::_))
    .WillOnce([&](Exchange::IUSBDevice::IUSBDeviceIterator*& devices) {
        devices = mockIterator;
        return Core::ERROR_NONE;  // Simulate successful return
    });

    handler.Invoke(connection, _T("getDeviceList"), _T("{}"), response);

    static int index = 1;

    EXPECT_CALL(*p_wrapsImplMock, stat(::testing::_, ::testing::_))
    .WillRepeatedly([](const std::string& path, struct stat* info) {
        std::string expectedPath = "/tmp/media/usb" + std::to_string(index);

        if (path == expectedPath) {
            index++;
            return -1;
        }
        info->st_mode = S_IFDIR;
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, mkdir(::testing::_, ::testing::_))
    .WillRepeatedly([](const char* path, mode_t mode) {
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, mount(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
    .WillRepeatedly([](const char* source, const char* target, const char* filesystemtype, unsigned long mountflags, const void* data) {
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, umount(::testing::_))
    .WillRepeatedly([](const char* path) {
        return 0;
    });

    handler.Invoke(connection, _T("getMountPoints"), _T("{\"deviceName\": \"001/002\"}"), response);
    EXPECT_CALL(*p_wrapsImplMock, statfs(::testing::_, ::testing::_))
    .WillOnce([](const char* path, struct statfs* buf) -> int {
        if (buf != nullptr) {
            buf->f_type = 0x565a;
            return 0;
        }
        return -1; 
    });

    EXPECT_CALL(*p_wrapsImplMock, statvfs(::testing::_, ::testing::_))
    .WillOnce([](const char* path, struct statvfs* stat) -> int {
        return -1;
    });

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getPartitionInfo"), _T("{\"mountPath\": \"/tmp/media/usb1\"}"), response));

    usbDeviceList.clear();

}

TEST_F(USBMassStorageTest, getPartitionInfoFail_devicepath_empty)
{
    std::list<Exchange::IUSBDevice::USBDevice> usbDeviceList;
    Exchange::IUSBDevice::USBDevice usbDevice1;
    usbDevice1.deviceClass = LIBUSB_CLASS_MASS_STORAGE;
    usbDevice1.deviceSubclass = 0x12;
    usbDevice1.deviceName = "001/002";
    usbDevice1.devicePath = "/dev/sda";
    usbDeviceList.emplace_back(usbDevice1); 

    auto mockIterator = Core::Service<RPC::IteratorType<Exchange::IUSBDevice::IUSBDeviceIterator>>::Create<Exchange::IUSBDevice::IUSBDeviceIterator>(usbDeviceList);

    EXPECT_CALL(*p_usbDeviceMock, GetDeviceList(testing::_))
    .WillOnce([&](Exchange::IUSBDevice::IUSBDeviceIterator*& devices) {
        devices = mockIterator;
        return Core::ERROR_NONE;  // Simulate successful return
    });

    handler.Invoke(connection, _T("getDeviceList"), _T("{}"), response);

    static int index = 1;

    EXPECT_CALL(*p_wrapsImplMock, stat(::testing::_, ::testing::_))
    .WillRepeatedly([](const std::string& path, struct stat* info) {
        std::string expectedPath = "/tmp/media/usb" + std::to_string(index);

        if (path == expectedPath) {
            index++;
            return -1;
        }
        info->st_mode = S_IFDIR;
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, mkdir(::testing::_, ::testing::_))
    .WillRepeatedly([](const char* path, mode_t mode) {
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, mount(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
    .WillRepeatedly([](const char* source, const char* target, const char* filesystemtype, unsigned long mountflags, const void* data) {
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, umount(::testing::_))
    .WillRepeatedly([](const char* path) {
        return 0;
    });

    handler.Invoke(connection, _T("getMountPoints"), _T("{\"deviceName\": \"001/002\"}"), response);

    EXPECT_EQ(Core::ERROR_INVALID_MOUNTPOINT, handler.Invoke(connection, _T("getPartitionInfo"), _T("{\"mountPath\": \"/tmp/medis/usb1\"}"), response));

    usbDeviceList.clear();
}

TEST_F(USBMassStorageTest, getPartitionInfoFail_opencall_fail)
{
    std::list<Exchange::IUSBDevice::USBDevice> usbDeviceList;
    Exchange::IUSBDevice::USBDevice usbDevice1;
    usbDevice1.deviceClass = LIBUSB_CLASS_MASS_STORAGE;
    usbDevice1.deviceSubclass = 0x12;
    usbDevice1.deviceName = "001/002";
    usbDevice1.devicePath = "/dev/sda";
    usbDeviceList.emplace_back(usbDevice1); 

    auto mockIterator = Core::Service<RPC::IteratorType<Exchange::IUSBDevice::IUSBDeviceIterator>>::Create<Exchange::IUSBDevice::IUSBDeviceIterator>(usbDeviceList);

    EXPECT_CALL(*p_usbDeviceMock, GetDeviceList(testing::_))
    .WillOnce([&](Exchange::IUSBDevice::IUSBDeviceIterator*& devices) {
        devices = mockIterator;
        return Core::ERROR_NONE;  // Simulate successful return
    });

    handler.Invoke(connection, _T("getDeviceList"), _T("{}"), response);

    static int index = 1;

    EXPECT_CALL(*p_wrapsImplMock, stat(::testing::_, ::testing::_))
    .WillRepeatedly([](const std::string& path, struct stat* info) {
        std::string expectedPath = "/tmp/media/usb" + std::to_string(index);

        if (path == expectedPath) {
            index++;
            return -1;
        }
        info->st_mode = S_IFDIR;
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, mkdir(::testing::_, ::testing::_))
    .WillRepeatedly([](const char* path, mode_t mode) {
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, mount(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
    .WillRepeatedly([](const char* source, const char* target, const char* filesystemtype, unsigned long mountflags, const void* data) {
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, umount(::testing::_))
    .WillRepeatedly([](const char* path) {
        return 0;
    });

    handler.Invoke(connection, _T("getMountPoints"), _T("{\"deviceName\": \"001/002\"}"), response);

    EXPECT_CALL(*p_wrapsImplMock, statfs(::testing::_, ::testing::_))
    .WillOnce([](const char* path, struct statfs* buf) -> int {
        if (buf != nullptr) {
            buf->f_type = 0x565a;
            return 0;
        }
        return -1; 
    });

    EXPECT_CALL(*p_wrapsImplMock, statvfs(::testing::_, ::testing::_))
    .WillOnce([](const char* path, struct statvfs* stat) -> int {
        if (stat != nullptr) {
            stat->f_blocks = 1024000;
            stat->f_frsize = 4096;
            stat->f_bfree = 256000;
            return 0;
        }
        return -1;
    });
    EXPECT_CALL(*p_wrapsImplMock, open(::testing::_, ::testing::_, ::testing::_))
    .WillOnce([](const char* pathname, int flags, mode_t mode) {
        return -1;
    });

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getPartitionInfo"), _T("{\"mountPath\": \"/tmp/media/usb1\"}"), response));

    usbDeviceList.clear();

}

TEST_F(USBMassStorageTest, getPartitionInfoFail_PartitionSize_fails)
{
    std::list<Exchange::IUSBDevice::USBDevice> usbDeviceList;
    Exchange::IUSBDevice::USBDevice usbDevice1;
    usbDevice1.deviceClass = LIBUSB_CLASS_MASS_STORAGE;
    usbDevice1.deviceSubclass = 0x12;
    usbDevice1.deviceName = "001/002";
    usbDevice1.devicePath = "/dev/sda";
    usbDeviceList.emplace_back(usbDevice1); 

    auto mockIterator = Core::Service<RPC::IteratorType<Exchange::IUSBDevice::IUSBDeviceIterator>>::Create<Exchange::IUSBDevice::IUSBDeviceIterator>(usbDeviceList);

    EXPECT_CALL(*p_usbDeviceMock, GetDeviceList(testing::_))
    .WillOnce([&](Exchange::IUSBDevice::IUSBDeviceIterator*& devices) {
        devices = mockIterator;
        return Core::ERROR_NONE;  // Simulate successful return
    });

    handler.Invoke(connection, _T("getDeviceList"), _T("{}"), response);

    static int index = 1;

    EXPECT_CALL(*p_wrapsImplMock, stat(::testing::_, ::testing::_))
    .WillRepeatedly([](const std::string& path, struct stat* info) {
        std::string expectedPath = "/tmp/media/usb" + std::to_string(index);

        if (path == expectedPath) {
            index++;
            return -1;
        }
        info->st_mode = S_IFDIR;
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, mkdir(::testing::_, ::testing::_))
    .WillRepeatedly([](const char* path, mode_t mode) {
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, mount(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
    .WillRepeatedly([](const char* source, const char* target, const char* filesystemtype, unsigned long mountflags, const void* data) {
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, umount(::testing::_))
    .WillRepeatedly([](const char* path) {
        return 0;
    });

    handler.Invoke(connection, _T("getMountPoints"), _T("{\"deviceName\": \"001/002\"}"), response);

    EXPECT_CALL(*p_wrapsImplMock, statfs(::testing::_, ::testing::_))
    .WillOnce([](const char* path, struct statfs* buf) -> int {
        if (buf != nullptr) {
            buf->f_type = 0x565a;
            return 0;
        }
        return -1; 
    });

    EXPECT_CALL(*p_wrapsImplMock, statvfs(::testing::_, ::testing::_))
    .WillOnce([](const char* path, struct statvfs* stat) -> int {
        if (stat != nullptr) {
            stat->f_blocks = 1024000;
            stat->f_frsize = 4096;
            stat->f_bfree = 256000;
            return 0;
        }
        return -1;
    });
    EXPECT_CALL(*p_wrapsImplMock, open(::testing::_, ::testing::_, ::testing::_))
    .WillOnce([](const char* pathname, int flags, mode_t mode) {
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, ioctl(::testing::_, ::testing::_, ::testing::_))
    .WillOnce([](int fd, unsigned long request, void* argp) -> int {
        if (request == BLKGETSIZE64) {
            *(reinterpret_cast<uint64_t*>(argp)) = 50ULL * 1024 * 1024 *1024; 
        }
        return -1;
    });

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getPartitionInfo"), _T("{\"mountPath\": \"/tmp/media/usb1\"}"), response));

    usbDeviceList.clear();

}

TEST_F(USBMassStorageTest, getPartitionInfoFail_NumberOfSectors_fails)
{
    std::list<Exchange::IUSBDevice::USBDevice> usbDeviceList;
    Exchange::IUSBDevice::USBDevice usbDevice1;
    usbDevice1.deviceClass = LIBUSB_CLASS_MASS_STORAGE;
    usbDevice1.deviceSubclass = 0x12;
    usbDevice1.deviceName = "001/002";
    usbDevice1.devicePath = "/dev/sda";
    usbDeviceList.emplace_back(usbDevice1); 

    auto mockIterator = Core::Service<RPC::IteratorType<Exchange::IUSBDevice::IUSBDeviceIterator>>::Create<Exchange::IUSBDevice::IUSBDeviceIterator>(usbDeviceList);

    EXPECT_CALL(*p_usbDeviceMock, GetDeviceList(testing::_))
    .WillOnce([&](Exchange::IUSBDevice::IUSBDeviceIterator*& devices) {
        devices = mockIterator;
        return Core::ERROR_NONE;  // Simulate successful return
    });

    handler.Invoke(connection, _T("getDeviceList"), _T("{}"), response);

    static int index = 1;

    EXPECT_CALL(*p_wrapsImplMock, stat(::testing::_, ::testing::_))
    .WillRepeatedly([](const std::string& path, struct stat* info) {
        std::string expectedPath = "/tmp/media/usb" + std::to_string(index);

        if (path == expectedPath) {
            index++;
            return -1;
        }
        info->st_mode = S_IFDIR;
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, mkdir(::testing::_, ::testing::_))
    .WillRepeatedly([](const char* path, mode_t mode) {
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, mount(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
    .WillRepeatedly([](const char* source, const char* target, const char* filesystemtype, unsigned long mountflags, const void* data) {
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, umount(::testing::_))
    .WillRepeatedly([](const char* path) {
        return 0;
    });

    handler.Invoke(connection, _T("getMountPoints"), _T("{\"deviceName\": \"001/002\"}"), response);

    EXPECT_CALL(*p_wrapsImplMock, statfs(::testing::_, ::testing::_))
    .WillOnce([](const char* path, struct statfs* buf) -> int {
        if (buf != nullptr) {
            buf->f_type = 0x565a;
            return 0;
        }
        return -1; 
    });

    EXPECT_CALL(*p_wrapsImplMock, statvfs(::testing::_, ::testing::_))
    .WillOnce([](const char* path, struct statvfs* stat) -> int {
        if (stat != nullptr) {
            stat->f_blocks = 1024000;
            stat->f_frsize = 4096;
            stat->f_bfree = 256000;
            return 0;
        }
        return -1;
    });
    EXPECT_CALL(*p_wrapsImplMock, open(::testing::_, ::testing::_, ::testing::_))
    .WillOnce([](const char* pathname, int flags, mode_t mode) {
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, ioctl(::testing::_, BLKGETSIZE64, ::testing::_))
    .WillOnce([](int fd, unsigned long request, void* argp) -> int {
        *(reinterpret_cast<uint64_t*>(argp)) = 50ULL * 1024 * 1024 * 1024;
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, ioctl(::testing::_, BLKGETSIZE, ::testing::_))
    .WillOnce([](int fd, unsigned long request, void* argp) -> int {
        return -1;
    });

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getPartitionInfo"), _T("{\"mountPath\": \"/tmp/media/usb1\"}"), response));

    usbDeviceList.clear();

}

TEST_F(USBMassStorageTest, getPartitionInfoFail_SectorSize_fails)
{
    std::list<Exchange::IUSBDevice::USBDevice> usbDeviceList;
    Exchange::IUSBDevice::USBDevice usbDevice1;
    usbDevice1.deviceClass = LIBUSB_CLASS_MASS_STORAGE;
    usbDevice1.deviceSubclass = 0x12;
    usbDevice1.deviceName = "001/002";
    usbDevice1.devicePath = "/dev/sda";
    usbDeviceList.emplace_back(usbDevice1); 

    auto mockIterator = Core::Service<RPC::IteratorType<Exchange::IUSBDevice::IUSBDeviceIterator>>::Create<Exchange::IUSBDevice::IUSBDeviceIterator>(usbDeviceList);

    EXPECT_CALL(*p_usbDeviceMock, GetDeviceList(testing::_))
    .WillOnce([&](Exchange::IUSBDevice::IUSBDeviceIterator*& devices) {
        devices = mockIterator;
        return Core::ERROR_NONE;  // Simulate successful return
    });

    handler.Invoke(connection, _T("getDeviceList"), _T("{}"), response);

    static int index = 1;

    EXPECT_CALL(*p_wrapsImplMock, stat(::testing::_, ::testing::_))
    .WillRepeatedly([](const std::string& path, struct stat* info) {
        std::string expectedPath = "/tmp/media/usb" + std::to_string(index);

        if (path == expectedPath) {
            index++;
            return -1;
        }
        info->st_mode = S_IFDIR;
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, mkdir(::testing::_, ::testing::_))
    .WillRepeatedly([](const char* path, mode_t mode) {
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, mount(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
    .WillRepeatedly([](const char* source, const char* target, const char* filesystemtype, unsigned long mountflags, const void* data) {
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, umount(::testing::_))
    .WillRepeatedly([](const char* path) {
        return 0;
    });
    handler.Invoke(connection, _T("getMountPoints"), _T("{\"deviceName\": \"001/002\"}"), response);

    EXPECT_CALL(*p_wrapsImplMock, statfs(::testing::_, ::testing::_))
    .WillOnce([](const char* path, struct statfs* buf) -> int {
        if (buf != nullptr) {
            buf->f_type = 0x565a;
            return 0;
        }
        return -1; 
    });

    EXPECT_CALL(*p_wrapsImplMock, statvfs(::testing::_, ::testing::_))
    .WillOnce([](const char* path, struct statvfs* stat) -> int {
        if (stat != nullptr) {
            stat->f_blocks = 1024000;
            stat->f_frsize = 4096;
            stat->f_bfree = 256000;
            return 0;
        }
        return -1;
    });
    EXPECT_CALL(*p_wrapsImplMock, open(::testing::_, ::testing::_, ::testing::_))
    .WillOnce([](const char* pathname, int flags, mode_t mode) {
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, ioctl(::testing::_, BLKGETSIZE64, ::testing::_))
    .WillOnce([](int fd, unsigned long request, void* argp) -> int {
        *(reinterpret_cast<uint64_t*>(argp)) = 50ULL * 1024 * 1024 * 1024;
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, ioctl(::testing::_, BLKGETSIZE, ::testing::_))
    .WillOnce([](int fd, unsigned long request, void* argp) -> int {
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, ioctl(::testing::_, BLKSSZGET, ::testing::_))
    .WillOnce([](int fd, unsigned long request, void* argp) -> int {
        *(reinterpret_cast<uint32_t*>(argp)) = 512;
        return -1;
    });

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getPartitionInfo"), _T("{\"mountPath\": \"/tmp/media/usb1\"}"), response));

    usbDeviceList.clear();

}
/*Success case for getdevicelist, getMountPoints, getPartitionInfo*/
TEST_F(USBMassStorageTest, USBMassStorage_Succeed)
{
    std::list<Exchange::IUSBDevice::USBDevice> usbDeviceList;
    Exchange::IUSBDevice::USBDevice usbDevice1;
    usbDevice1.deviceClass = LIBUSB_CLASS_MASS_STORAGE;
    usbDevice1.deviceSubclass = 0x12;
    usbDevice1.deviceName = "001/002";
    usbDevice1.devicePath = "/dev/sda";
    usbDeviceList.emplace_back(usbDevice1); 

    auto mockIterator = Core::Service<RPC::IteratorType<Exchange::IUSBDevice::IUSBDeviceIterator>>::Create<Exchange::IUSBDevice::IUSBDeviceIterator>(usbDeviceList);

    EXPECT_CALL(*p_usbDeviceMock, GetDeviceList(testing::_))
    .WillOnce([&](Exchange::IUSBDevice::IUSBDeviceIterator*& devices) {
        devices = mockIterator;
        return Core::ERROR_NONE;  // Simulate successful return
    });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getDeviceList"), _T("{}"), response));

    static int index = 1;

    EXPECT_CALL(*p_wrapsImplMock, stat(::testing::_, ::testing::_))
    .WillRepeatedly([](const std::string& path, struct stat* info) {
        std::string expectedPath = "/tmp/media/usb" + std::to_string(index);

        if (path == expectedPath) {
            index++;
            return -1;
        }
        info->st_mode = S_IFDIR;
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, mkdir(::testing::_, ::testing::_))
    .WillRepeatedly([](const char* path, mode_t mode) {
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, umount(::testing::_))
    .WillRepeatedly([](const char* path) {
        return 0;
    });

    EXPECT_CALL(*p_wrapsImplMock, mount(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
    .WillRepeatedly([](const char* source, const char* target, const char* filesystemtype, unsigned long mountflags, const void* data) {
        return 0;
    });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getMountPoints"), _T("{\"deviceName\": \"001/002\"}"), response));

    EXPECT_CALL(*p_wrapsImplMock, statfs(::testing::_, ::testing::_))
    .WillOnce([](const char* path, struct statfs* buf) -> int {
        if (buf != nullptr) {
            buf->f_type = 0x565a;
            return 0;
        }
        return -1; 
        });

    EXPECT_CALL(*p_wrapsImplMock, statvfs(::testing::_, ::testing::_))
    .WillOnce([](const char* path, struct statvfs* stat) -> int {
        if (stat != nullptr) {
            stat->f_blocks = 1024000;
            stat->f_frsize = 4096;
            stat->f_bfree = 256000;
            return 0;
        }
        return -1;
        });
        EXPECT_CALL(*p_wrapsImplMock, open(::testing::_, ::testing::_, ::testing::_))
        .WillOnce([](const char* pathname, int flags, mode_t mode) {
            return 0;
        });

        EXPECT_CALL(*p_wrapsImplMock, ioctl(::testing::_, ::testing::_, ::testing::_))
        .WillRepeatedly([](int fd, unsigned long request, void* argp) -> int {
            if (request == BLKGETSIZE64) {
                *(reinterpret_cast<uint64_t*>(argp)) = 50ULL * 1024 * 1024 *1024; 
            }
            else if (request == BLKGETSIZE) {
                *(reinterpret_cast<unsigned long*>(argp)) = 1024;
            }
            else if (request == BLKSSZGET) {
                *(reinterpret_cast<uint32_t*>(argp)) = 512; 
            }
        return 0;
    });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getPartitionInfo"), _T("{\"mountPath\": \"/tmp/media/usb1\"}"), response));

    usbDeviceList.clear();

}