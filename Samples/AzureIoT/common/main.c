/* Copyright (c) Microsoft Corporation. All rights reserved.
   Licensed under the MIT License. */

// This sample C application demonstrates how to use Azure Sphere devices with Azure IoT
// services, using the Azure IoT C SDK.
//
// It implements a simulated thermometer device, with the following features:
// - Telemetry upload (simulated temperature, device moved events) using Azure IoT Hub events.
// - Reporting device state (serial number) using device twin/read-only properties.
// - Mutable device state (telemetry upload enabled) using device twin/writeable properties.
// - Alert messages invoked from the cloud using device methods.
//
// It can be configured using the top-level CMakeLists.txt to connect either directly to an
// Azure IoT Hub, to an Azure IoT Edge device, or to use the Azure Device Provisioning service to
// connect to either an Azure IoT Hub, or an Azure IoT Central application. All connection types
// make use of the device certificate issued by the Azure Sphere security service to authenticate,
// and supply an Azure IoT PnP model ID on connection.
//
// It uses the following Azure Sphere libraries:
// - eventloop (system invokes handlers for timer events)
// - gpio (digital input for button, digital output for LED)
// - log (displays messages in the Device Output window during debugging)
// - networking (network interface connection status)
//
// You will need to provide information in the application manifest to use this application. Please
// see README.md and the other linked documentation for full details.

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// applibs_versions.h defines the API struct versions to use for applibs APIs.
#include "applibs_versions.h"
#include <applibs/eventloop.h>
#include <applibs/networking.h>
#include <applibs/log.h>

#include "eventloop_timer_utilities.h"
#include "user_interface.h"
#include "exitcodes.h"
#include "cloud.h"
#include "options.h"
#include "connection.h"

static volatile sig_atomic_t exitCode = ExitCode_Success;

// Initialization/Cleanup
static ExitCode InitPeripheralsAndHandlers(void);
static void ClosePeripheralsAndHandlers(void);

// Interface callbacks
static void ExitCodeCallbackHandler(ExitCode ec);
static void ButtonPressedCallbackHandler(UserInterface_Button button);

// Cloud
static const char *CloudResultToString(Cloud_Result result);
static void ConnectionChangedCallbackHandler(bool connected);
static void DisplayAlertCallbackHandler(const char *alertMessage);

// Timer / polling
static EventLoop *eventLoop = NULL;
static EventLoopTimer *telemetryTimer = NULL;

// Cloud connection check
static bool isConnected = false;

// Cloud Property/Telemetry
static const char *serialNumber = "ksiatuser01";
static Cloud_Telemetry telemetry = {.temperature = 30.f};

// KSIA Academy
// another telemetry
/* user code */

/// <summary>
///     Signal handler for termination requests. This handler must be async-signal-safe.
/// </summary>
static void TerminationHandler(int signalNumber)
{
    // Don't use Log_Debug here, as it is not guaranteed to be async-signal-safe.
    exitCode = ExitCode_TermHandler_SigTerm;
}

/// <summary>
///     Main entry point for this sample.
/// </summary>
int main(int argc, char *argv[])
{
    Log_Debug("Azure IoT Application starting.\n");

    exitCode = Options_ParseArgs(argc, argv);

    if (exitCode != ExitCode_Success) {
        return exitCode;
    }

    exitCode = InitPeripheralsAndHandlers();

    bool isNetworkingReady = false;
    if ((Networking_IsNetworkingReady(&isNetworkingReady) == -1) || !isNetworkingReady) {
        Log_Debug("WARNING: Network is not ready. Device cannot connect until network is ready.\n");

        // KSIA Academy
        // control WiFi LED 
        /* user code */   
        UserInterface_SetStatus_WifiLED(isNetworkingReady);
    }

    // Main loop
    while (exitCode == ExitCode_Success) {
        EventLoop_Run_Result result = EventLoop_Run(eventLoop, -1, true);
        // Continue if interrupted by signal, e.g. due to breakpoint being set.
        if (result == EventLoop_Run_Failed && errno != EINTR) {
            exitCode = ExitCode_Main_EventLoopFail;
        }
    }

    ClosePeripheralsAndHandlers();

    Log_Debug("Application exiting.\n");

    return exitCode;
}

static void ExitCodeCallbackHandler(ExitCode ec)
{
    exitCode = ec;
}

static const char *CloudResultToString(Cloud_Result result)
{
    switch (result) {
    case Cloud_Result_OK:
        return "OK";
    case Cloud_Result_NoNetwork:
        return "No network connection available";
    case Cloud_Result_OtherFailure:
        return "Other failure";
    }

    return "Unknown Cloud_Result";
}

static void ButtonPressedCallbackHandler(UserInterface_Button button)
{
    if (button == UserInterface_Button_A) {
        if (isConnected) {

            // KSIA Academy
            // another telemetry

            // Generate a simulated humidity.
            telemetry.humidity += 20;

            /* user code */
            // init telemetry values

            // Generate a simulated temperature.            
            telemetry.temperature += 20;
            telemetry.voltage += 20;

            Cloud_Result result = Cloud_SendTelemetry(&telemetry);
            if (result != Cloud_Result_OK) {
                Log_Debug("WARNING: Could not send thermometer telemetry to cloud: %s\n",
                            CloudResultToString(result));
            } else {
                Log_Debug("INFO: Telemetry upload disabled; not sending telemetry.\n");
            }
        }
    }
}

static void DisplayAlertCallbackHandler(const char *alertMessage)
{
    Log_Debug("ALERT: %s\n", alertMessage);
}

static void ConnectionChangedCallbackHandler(bool connected)
{
    isConnected = connected;

    if (isConnected) {
        
        // KSIA Academy
        // control Azure Connection LED
        /* user code */
        UserInterface_SetStatus_AzureLED(isConnected);

        Cloud_Result result = Cloud_SendDeviceDetails(serialNumber);
        if (result != Cloud_Result_OK) {
            Log_Debug("WARNING: Could not send device details to cloud: %s\n",
                      CloudResultToString(result));
        }
    } 
}

static void TelemetryTimerCallbackHandler(EventLoopTimer *timer)
{
    // init telemetry values
    telemetry.temperature = 30.f;

    // KSIA Academy
    // another telemetry
    /* user code */
    telemetry.humidity = 50.0f;
    telemetry.voltage = 0.0f;
    
    if (ConsumeEventLoopTimerEvent(timer) != 0) {
        exitCode = ExitCode_TelemetryTimer_Consume;
        return;
    }

    if (isConnected) {
        // Generate a simulated temperature.
        float delta = ((float)(rand() % 20)) / 20.0f - 1.0f; // between -1.0 and +1.0
        telemetry.temperature += delta;
            
        // KSIA Academy
        // another telemetry
        /* user code */
        float alpha = ((float)(rand() % 20)) / 20.0f - 1.0f; // between -1.0 and +1.0
        telemetry.humidity += alpha;

        float beta = (float)(rand() % 20);
        telemetry.voltage += beta;

        Cloud_Result result = Cloud_SendTelemetry(&telemetry);
        if (result != Cloud_Result_OK) {
            Log_Debug("WARNING: Could not send thermometer telemetry to cloud: %s\n",
                        CloudResultToString(result));
        } else {
            Log_Debug("INFO: Telemetry upload disabled; not sending telemetry.\n");
        }
    }
}

/// <summary>
///     Set up SIGTERM termination handler, initialize peripherals, and set up event handlers.
/// </summary>
/// <returns>
///     ExitCode_Success if all resources were allocated successfully; otherwise another
///     ExitCode value which indicates the specific failure.
/// </returns>
static ExitCode InitPeripheralsAndHandlers(void)
{
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = TerminationHandler;
    sigaction(SIGTERM, &action, NULL);

    eventLoop = EventLoop_Create();
    if (eventLoop == NULL) {
        Log_Debug("Could not create event loop.\n");
        return ExitCode_Init_EventLoop;
    }

    struct timespec telemetryPeriod = {.tv_sec = 5, .tv_nsec = 0};
    telemetryTimer =
        CreateEventLoopPeriodicTimer(eventLoop, &TelemetryTimerCallbackHandler, &telemetryPeriod);
    if (telemetryTimer == NULL) {
        return ExitCode_Init_TelemetryTimer;
    }

    ExitCode interfaceExitCode =
        UserInterface_Initialise(eventLoop, ButtonPressedCallbackHandler, ExitCodeCallbackHandler);

    if (interfaceExitCode != ExitCode_Success) {
        return interfaceExitCode;
    }

    void *connectionContext = Options_GetConnectionContext();

    return Cloud_Initialize(eventLoop, connectionContext, ExitCodeCallbackHandler,
                            DisplayAlertCallbackHandler,
                            ConnectionChangedCallbackHandler);
}

/// <summary>
///     Close peripherals and handlers.
/// </summary>
static void ClosePeripheralsAndHandlers(void)
{
    DisposeEventLoopTimer(telemetryTimer);
    Cloud_Cleanup();
    UserInterface_Cleanup();
    Connection_Cleanup();
    EventLoop_Close(eventLoop);

    Log_Debug("Closing file descriptors\n");
}
