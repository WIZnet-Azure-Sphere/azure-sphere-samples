/* Copyright (c) Microsoft Corporation. All rights reserved.
   Licensed under the MIT License. */

#include <errno.h>
#include <string.h>

#include <applibs/gpio.h>
#include <applibs/log.h>

#include "user_interface.h"

// The following #include imports a "sample appliance" hardware definition. This provides a set of
// named constants such as SAMPLE_BUTTON_1 which are used when opening the peripherals, rather
// that using the underlying pin names. This enables the same code to target different hardware.
//
// By default, this app targets hardware that follows the MT3620 Reference Development Board (RDB)
// specification, such as the MT3620 Dev Kit from Seeed Studio. To target different hardware, you'll
// need to update the TARGET_HARDWARE variable in CMakeLists.txt - see instructions in that file.
//
// You can also use hardware definitions related to all other peripherals on your dev board because
// the sample_appliance header file recursively includes underlying hardware definition headers.
// See https://aka.ms/azsphere-samples-hardwaredefinitions for further details on this feature.
#include <hw/wiznet_asg210_v1.2.h>

#include "eventloop_timer_utilities.h"

static void ButtonPollTimerEventHandler(EventLoopTimer *timer);
static bool IsButtonPressed(int fd, GPIO_Value_Type *oldState);
static void CloseFdAndPrintError(int fd, const char *fdName);

// File descriptors - initialized to invalid value
static int buttonAGpioFd = -1;
static int statusAzureLedGpioFd = -1;
static int statusWifiLedGpioFd = -1;

static EventLoopTimer *buttonPollTimer = NULL;

static ExitCode_CallbackType failureCallbackFunction = NULL;
static UserInterface_ButtonPressedCallbackType buttonPressedCallbackFunction = NULL;

// State variables
static GPIO_Value_Type buttonAState = GPIO_Value_High;

/// <summary>
///     Check whether a given button has just been pressed.
/// </summary>
/// <param name="fd">The button file descriptor</param>
/// <param name="oldState">Old state of the button (pressed or released)</param>
/// <returns>true if pressed, false otherwise</returns>
static bool IsButtonPressed(int fd, GPIO_Value_Type *oldState)
{
    bool isButtonPressed = false;
    GPIO_Value_Type newState;
    int result = GPIO_GetValue(fd, &newState);
    if (result != 0) {
        Log_Debug("ERROR: Could not read button GPIO: %s (%d).\n", strerror(errno), errno);
        failureCallbackFunction(ExitCode_IsButtonPressed_GetValue);
    } else {
        // Button is pressed if it is low and different than last known state.
        isButtonPressed = (newState != *oldState) && (newState == GPIO_Value_Low);
        *oldState = newState;
    }

    return isButtonPressed;
}

/// <summary>
///     Button timer event:  Check the status of the button
/// </summary>
static void ButtonPollTimerEventHandler(EventLoopTimer *timer)
{
    if (ConsumeEventLoopTimerEvent(timer) != 0) {
        failureCallbackFunction(ExitCode_ButtonTimer_Consume);
        return;
    }

    if (IsButtonPressed(buttonAGpioFd, &buttonAState) && NULL != buttonPressedCallbackFunction) {
        buttonPressedCallbackFunction(UserInterface_Button_A);
    }

}

/// <summary>
///     Closes a file descriptor and prints an error on failure.
/// </summary>
/// <param name="fd">File descriptor to close</param>
/// <param name="fdName">File descriptor name to use in error message</param>
static void CloseFdAndPrintError(int fd, const char *fdName)
{
    if (fd >= 0) {
        int result = close(fd);
        if (result != 0) {
            Log_Debug("ERROR: Could not close fd %s: %s (%d).\n", fdName, strerror(errno), errno);
        }
    }
}

ExitCode UserInterface_Initialise(EventLoop *el,
                                  UserInterface_ButtonPressedCallbackType buttonPressedCallback,
                                  ExitCode_CallbackType failureCallback)
{
    failureCallbackFunction = failureCallback;
    buttonPressedCallbackFunction = buttonPressedCallback;

    // Open USER_BUTTON GPIO as input
    Log_Debug("Opening SAMPLE_BUTTON_1 as input.\n");
    buttonAGpioFd = GPIO_OpenAsInput(WIZNET_ASG210_USER_BUTTON_SW2);
    if (buttonAGpioFd == -1) {
        Log_Debug("ERROR: Could not open USER_BUTTON: %s (%d).\n", strerror(errno), errno);
        return ExitCode_Init_Button;
    }    

    // WIFI_CONNECTION_LED is used to show state
    Log_Debug("Opening WIFI_CONNECTION_LED as output.\n");
    statusWifiLedGpioFd = GPIO_OpenAsOutput(WIZNET_ASG210_STATUS_LED2_WIFI, GPIO_OutputMode_PushPull,
                                        GPIO_Value_High);
    if (statusWifiLedGpioFd == -1) {
        Log_Debug("ERROR: Could not open SAMPLE_LED: %s (%d).\n", strerror(errno), errno);
        return ExitCode_Init_Led;
    }

    // KSIA Academy
    // define GPIO for Azure Connection LED
    /* user code */

    // Set up a timer to poll for button events.
    static const struct timespec buttonPressCheckPeriod = {.tv_sec = 0, .tv_nsec = 1000 * 1000};
    buttonPollTimer =
        CreateEventLoopPeriodicTimer(el, &ButtonPollTimerEventHandler, &buttonPressCheckPeriod);
    if (buttonPollTimer == NULL) {
        return ExitCode_Init_ButtonPollTimer;
    }

    return ExitCode_Success;
}

void UserInterface_Cleanup(void)
{
    DisposeEventLoopTimer(buttonPollTimer);

    // Leave the LEDs off
    if (statusWifiLedGpioFd >= 0) {
        GPIO_SetValue(statusWifiLedGpioFd, GPIO_Value_High);
    }

    CloseFdAndPrintError(buttonAGpioFd, "UserButton");
    CloseFdAndPrintError(statusWifiLedGpioFd, "WifiStatusLed");
}

void UserInterface_SetStatus_WifiLED(bool status)
{
    GPIO_SetValue(statusWifiLedGpioFd, status ? GPIO_Value_Low : GPIO_Value_High);
}