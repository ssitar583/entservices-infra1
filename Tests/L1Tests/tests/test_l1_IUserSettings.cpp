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
distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
    
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdio.h>

#include "IUserSettings.h"

using namespace WPEFramework::Exchange; 

/**
* @brief Test to verify that GetAudioDescription returns ERROR_NONE with valid value
*
* This test checks the GetAudioDescription method to ensure it returns ERROR_NONE and sets the enabled flag correctly.@n
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 001@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Call GetAudioDescription with valid InterfacePointer | enabled = false | result = Core::ERROR_NONE, enabled = true or false | Should Pass |
*/
TEST(UserSettingsTest, GetAudioDescription_ReturnsErrorNone_WithValidValue) {…}

/**
* @brief Test to verify the GetBlockNotRatedContent function returns the enabled state successfully
*
* This test checks if the GetBlockNotRatedContent function correctly retrieves the enabled state of blocking unrated content. The function is expected to return a success result and the state should be either true or false.@n
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 002@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Call GetBlockNotRatedContent function | blockNotRatedContent = false | result = Core::ERROR_NONE, blockNotRatedContent = true or false | Should Pass |
*/
TEST(UserSettingsTest, GetBlockNotRatedContentReturnsEnabledStateSuccessfully) {…}

/**
* @brief Test to verify that the GetCaptions function returns the enabled state successfully.
*
* This test checks if the GetCaptions function correctly retrieves the enabled state of captions and returns the expected result code. This is important to ensure that the captions feature can be correctly queried for its state.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 003@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Call GetCaptions to retrieve the enabled state | enabled = false | result = Core::ERROR_NONE, enabled = true | Should Pass |
*/
TEST(UserSettingsTest, GetCaptionsReturnsEnabledStateSuccessfully) {…}

/**
* @brief Test to verify the GetHighContrast function returns the enabled state successfully.
*
* This test checks if the GetHighContrast function correctly retrieves the high contrast enabled state from the InterfacePointer. It ensures that the function returns a successful result and that the enabled state is either true or false.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 004@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Enter the test function | None | None | Should be successful |
* | 02| Initialize enabled to false | enabled = false | None | Should be successful |
* | 03| Call GetHighContrast function | enabled = false | result = Core::ERROR_NONE, enabled = true or false | Should Pass |
* | 04| Check the result of GetHighContrast | result = Core::ERROR_NONE | result = Core::ERROR_NONE | Should Pass |
* | 05| Verify the enabled state | enabled = true or false | enabled = true or false | Should Pass |
* | 06| Exit the test function | None | None | Should be successful |
*/
TEST(UserSettingsTest, GetHighContrastReturnsEnabledStateSuccessfully) {…}

/**
* @brief Test to verify the successful retrieval of live watershed status
*
* This test checks the functionality of the GetLiveWatershed method to ensure it correctly retrieves the live watershed status. The test verifies that the method returns a success result and that the liveWatershed variable is set to either true or false.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 005@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Call GetLiveWatershed method | liveWatershed = false | result = Core::ERROR_NONE, liveWatershed = true or false | Should Pass |
* | 02| Check result value | result = Core::ERROR_NONE | result = Core::ERROR_NONE | Should be successful |
* | 03| Validate liveWatershed value | liveWatershed = true or false | liveWatershed = true or false | Should be successful |
*/
TEST(UserSettingsTest, GetLiveWatershedReturnsLiveWatershedSuccessfully) {…}

/**
* @brief Test to verify that GetPinControl method returns the pin control status successfully.
*
* This test checks if the GetPinControl method of the InterfacePointer object returns the correct pin control status and ensures that the result is either true or false.@n
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 006@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Call GetPinControl method | pinControl = false | result = Core::ERROR_NONE, pinControl = true or false | Should Pass |
*/
TEST(UserSettingsTest, GetPinControlReturnsPinControlSuccessfully) {…}

/**
* @brief Test to verify the GetPinOnPurchase method returns the pin on purchase status successfully.
*
* This test checks if the GetPinOnPurchase method of the InterfacePointer correctly retrieves the pin on purchase status. The test ensures that the method returns a successful result and that the pinOnPurchase variable is set to either true or false.@n
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 007@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Enter the test function | None | None | Should be successful |
* | 02| Initialize pinOnPurchase to false | pinOnPurchase = false | None | Should be successful |
* | 03| Call GetPinOnPurchase method | pinOnPurchase = false | result = Core::ERROR_NONE, pinOnPurchase = true/false | Should Pass |
* | 04| Check the result of GetPinOnPurchase | result = Core::ERROR_NONE, pinOnPurchase = true/false | result == Core::ERROR_NONE, pinOnPurchase == true/false | Should Pass |
* | 05| Exit the test function | None | None | Should be successful |
*/
TEST(UserSettingsTest, GetPinOnPurchaseReturnsPinOnPurchaseSuccessfully) {…}

/**
* @brief Test to verify the GetPlaybackWatershed function returns ERROR_NONE with a valid playbackWatershed value.
*
* This test checks if the GetPlaybackWatershed function correctly returns ERROR_NONE and sets the playbackWatershed value to either true or false.@n
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 008@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Call GetPlaybackWatershed function | playbackWatershed = false | result = Core::ERROR_NONE, playbackWatershed = true or false | Should Pass |
*/
TEST(UserSettingsTest, GetPlaybackWatershed_ReturnsErrorNone_WithValidPlaybackWatershed) {…}

/**
* @brief Test to validate the preferred languages string returned by the GetPreferredAudioLanguages method.
*
* This test checks if the GetPreferredAudioLanguages method correctly retrieves the preferred audio languages string and verifies that it matches the expected value "eng". This ensures that the method is functioning correctly and returning the correct language preference.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 009@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Initialize preferredLanguages string and call GetPreferredAudioLanguages | preferredLanguages = "" | result = Core::ERROR_NONE, preferredLanguages = "eng" | Should Pass |
*/
TEST(UserSettingsTest, ValidPreferredLanguagesString) {…}

/**
* @brief Test to validate the retrieval of preferred languages
*
* This test checks if the GetPreferredCaptionsLanguages function correctly retrieves the preferred languages set by the user. The test ensures that the function returns the expected result and the preferred languages string matches the expected value.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 010@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Call GetPreferredCaptionsLanguages API | preferredLanguages = "" | result = Core::ERROR_NONE, preferredLanguages = "eng,fra" | Should Pass |
*/
TEST(UserSettingsTest, ValidPreferredLanguages) {…}

/**
* @brief Test to verify that a valid service name is returned by the GetPreferredClosedCaptionService method.
*
* This test checks if the GetPreferredClosedCaptionService method returns a valid closed caption service name. 
* The valid service names include "CC1", "CC2", "CC3", "CC4", "TEXT1", "TEXT2", "TEXT3", "TEXT4", or any 
* service name starting with "SERVICE" followed by a number between 1 and 64.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 011@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Call GetPreferredClosedCaptionService method | service = "" | result should be Core::ERROR_NONE, service should be one of the valid names | Should Pass |
* | 02| Check if the service name is valid | service = returned value | service should be "CC1", "CC2", "CC3", "CC4", "TEXT1", "TEXT2", "TEXT3", "TEXT4", or "SERVICE" followed by a number between 1 and 64 | Should Pass |
*/
TEST(UserSettingsTest, ValidServiceNameReturned) {…}

/**
* @brief Test to validate the presentation language retrieved by the GetPresentationLanguage method.
*
* This test checks if the GetPresentationLanguage method returns a valid presentation language code. The valid codes are "en-US", "es-US", "en-CA", and "fr-CA". The test ensures that the method returns one of these valid codes and that the result is successful.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 012@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Call GetPresentationLanguage method | presentationLanguage = "" | result = Core::ERROR_NONE, presentationLanguage = "en-US" or "es-US" or "en-CA" or "fr-CA" | Should Pass |
*/
TEST(UserSettingsTest, ValidPresentationLanguage) {…}

/**
* @brief Test to validate the retrieval of viewing restrictions
*
* This test checks if the GetViewingRestrictions method correctly retrieves the viewing restrictions and ensures that the returned string is not empty.@n
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 013@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Entering ValidViewingRestrictions test | - | - | Should be successful |
* | 02 | Call GetViewingRestrictions method | viewingRestrictions = "" | result = Core::ERROR_NONE, viewingRestrictions is not empty | Should Pass |
* | 03 | Exiting ValidViewingRestrictions test | - | - | Should be successful |
*/
TEST(UserSettingsTest, ValidViewingRestrictions) {…}

/**
* @brief Test to validate the viewing restrictions window
*
* This test checks if the GetViewingRestrictionsWindow function correctly retrieves the viewing restrictions window and verifies that it matches the expected value "ALWAYS". This ensures that the function behaves as expected under normal conditions.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 014@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Entering the test function | None | None | Should be successful |
* | 02| Call GetViewingRestrictionsWindow API | viewingRestrictionsWindow = "" | result = Core::ERROR_NONE, viewingRestrictionsWindow = "ALWAYS" | Should Pass |
* | 03| Verify the result | result = Core::ERROR_NONE | result = Core::ERROR_NONE | Should Pass |
* | 04| Verify the viewingRestrictionsWindow value | viewingRestrictionsWindow = "ALWAYS" | viewingRestrictionsWindow = "ALWAYS" | Should Pass |
* | 05| Exiting the test function | None | None | Should be successful |
*/
TEST(UserSettingsTest, ValidViewingRestrictionsWindow) {…}

/**
* @brief Test to verify the GetVoiceGuidance function returns ERROR_NONE with a valid enabled value.
*
* This test checks if the GetVoiceGuidance function of the InterfacePointer returns the expected result of ERROR_NONE and sets the enabled variable to a valid boolean value (true or false). This ensures that the function behaves correctly under normal conditions.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 015@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Call GetVoiceGuidance with a valid enabled variable | enabled = false | result = Core::ERROR_NONE, enabled = true or false | Should Pass |
*/
TEST(UserSettingsTest, GetVoiceGuidance_ReturnsErrorNoneWithValidEnabledValue) {…}

/**
* @brief Test to verify GetVoiceGuidanceHints returns ERROR_NONE with a valid hints value
*
* This test checks the GetVoiceGuidanceHints function to ensure it returns ERROR_NONE when called with a valid hints value. The test also verifies that the hints value is either true or false.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 016@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Call GetVoiceGuidanceHints with a valid hints value | hints = false | result = Core::ERROR_NONE, hints = true or false | Should Pass |
*/
TEST(UserSettingsTest, GetVoiceGuidanceHints_ReturnsErrorNone_WithValidHintsValue) {…}

/**
* @brief Test to verify the retrieval of a valid voice guidance rate
*
* This test checks if the GetVoiceGuidanceRate function correctly retrieves the voice guidance rate and returns the expected result code. It ensures that the function behaves as expected when called with valid parameters.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 017@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Call GetVoiceGuidanceRate with a valid rate variable | rate = 0.0 | result = Core::ERROR_NONE, Rate should fall within the inclusive range of 0.1 to 10 | Should Pass |
*/
TEST(UserSettingsTest, ValidRateRetrieval) {…}

/**
* @brief Test the registration of a valid notification object
*
* This test verifies that the Register method successfully registers a valid notification object and returns the expected result.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 018@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Create a valid notification object | notification = new Exchange::IUserSettings::INotification() | Notification object created | Should be successful |
* | 02| Register the notification object | InterfacePointer->Register(notification) | result = Core::ERROR_NONE | Should Pass |
* | 03| Verify the result | EXPECT_EQ(result, Core::ERROR_NONE) | result = Core::ERROR_NONE | Should Pass |
*/
TEST(UserSettingsTest, RegisterWithValidNotificationObject) {…}

/**
* @brief Test the Register function with a null notification object
*
* This test verifies that the Register function correctly handles the case where a null notification object is passed. 
* It ensures that the function returns an error code indicating an invalid parameter.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 019@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Call Register with null notification object | notification = nullptr | result = Core::ERROR_INVALID_PARAMETER, EXPECT_EQ(result, Core::ERROR_INVALID_PARAMETER) | Should Fail |
*/
TEST(UserSettingsTest, RegisterWithNullNotificationObject) {…}
