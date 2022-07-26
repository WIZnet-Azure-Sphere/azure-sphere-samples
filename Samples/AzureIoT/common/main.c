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

// 20220726 taylor
#if 1
#include <applibs/uart.h>
#include <applibs/gpio.h>
#include <hw/wiznet_asg210_v1.2.h>
#include "queue.h"
#endif

//
#include "azure_iot.h"

static volatile sig_atomic_t exitCode = ExitCode_Success;

// Initialization/Cleanup
static ExitCode InitPeripheralsAndHandlers(void);
static void ClosePeripheralsAndHandlers(void);

// Interface callbacks
static void ExitCodeCallbackHandler(ExitCode ec);
static void ButtonPressedCallbackHandler(UserInterface_Button button);

// Cloud
//
// AzureIoT_SendTelemetry(const char *jsonMessage, void *context);
//
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

// 20220726 taylor
#if 1
static int uartFd = -1;
#if 0
static int gpioButtonFd = -1;
#endif
static int gpioNRS232Fd = -1;
static int gpioNSCL = -1;

str_qUART isu3;
unsigned char *isu3_buf_p;

str_qElement element;
str_Element *element_buf_p;

EventRegistration *uartEventReg = NULL;

void SendUartMessage(int uartFd, const char *dataToSend);
#endif

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

    SendUartMessage(uartFd, "==================================\r\n");
    SendUartMessage(uartFd, "     AZURE SPHERE IoT by KSIA     \r\n");
    SendUartMessage(uartFd, "==================================\r\n");

    isu3_buf_p = (uint8_t *)malloc(1024);
    qUART_init(&isu3, isu3_buf_p, 1024);

    element_buf_p = (str_Element *)malloc(sizeof(str_Element) * 100);
    Log_Debug("element_buf_p = 0x%x\r\n", element_buf_p);
    qElement_init(&element, element_buf_p, 100);

    bool isNetworkingReady = false;
    if ((Networking_IsNetworkingReady(&isNetworkingReady) == -1) || !isNetworkingReady) {
        Log_Debug("WARNING: Network is not ready. Device cannot connect until network is ready.\n");

        UserInterface_SetStatus_WifiLED(isNetworkingReady);
    }

    // Main loop
    while (exitCode == ExitCode_Success) {
        EventLoop_Run_Result result = EventLoop_Run(eventLoop, -1, true);
        // Continue if interrupted by signal, e.g. due to breakpoint being set.
        if (result == EventLoop_Run_Failed && errno != EINTR) {
            exitCode = ExitCode_Main_EventLoopFail;
        }

        if (qUART_getlen(&isu3) >= 80) {

            unsigned char isu3_temp[100];
            unsigned int qUART_len;

            qUART_len = qUART_getlen(&isu3);
            if (qUART_len > 100) {
                qUART_cp(&isu3, isu3_temp, 100);
            } else {
                qUART_cp(&isu3, isu3_temp, qUART_len);
            }

            unsigned int i;
            unsigned int stx = 0;
            unsigned int etx = 0;
            for (i = 0; i < qUART_len; i++) {
                if (isu3_temp[i] == '{') {
                    stx = i;
                }
                if (isu3_temp[i] == '}') {
                    etx = i;
                }
            }

            if (etx == 0) {
                // 
            } else {
                if (stx != 0) {
                    // flush 0 to stx
                    qUART_dequeue(&isu3, isu3_temp, stx);
                } else {
                    memset(isu3_temp, 0, 100);
                    if (etx > 99) {
                        qUART_dequeue(&isu3, isu3_temp, 100);
                    } else {
                        qUART_dequeue(&isu3, isu3_temp, etx + 1);
                    }

                    qElement_enqueue(&element, (str_Element *)isu3_temp, 1);
                    Log_Debug("qElement_getlen %d\r\n", qElement_getlen(&element));
                    //qElement_disp(&element);
                }
            }
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

            telemetry.temperature += 20;
#if 0
            // Generate a simulated humidity.
            telemetry.humidity += 20;
#endif

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
    
#if 0
    telemetry.humidity = 50.0f;
#endif

    if (ConsumeEventLoopTimerEvent(timer) != 0) {
        exitCode = ExitCode_TelemetryTimer_Consume;
        return;
    }

    if (isConnected) {
        // Generate a simulated temperature.
        float delta = ((float)(rand() % 20)) / 20.0f - 1.0f; // between -1.0 and +1.0
        telemetry.temperature += delta;

#if 0
        float alpha = ((float)(rand() % 20)) / 20.0f - 1.0f; // between -1.0 and +1.0
        telemetry.humidity += alpha;
#endif

#if 0

        Cloud_Result result = Cloud_SendTelemetry(&telemetry);

        if (result != Cloud_Result_OK) {
                Log_Debug("WARNING: Could not send thermometer telemetry to cloud: %s\n",
                          CloudResultToString(result));
                SendUartMessage(uartFd,
                                "WARNING: Could not send thermometer telemetry to cloud: %s\n");
            } else {
                Log_Debug("INFO: Telemetry upload disabled; not sending telemetry.\n");
                SendUartMessage(uartFd,
                                "INFO: Telemetry upload disabled; not sending telemetry.\n");
            }
#else
        str_Element element_temp;
        memset((char *)&element_temp, 0, sizeof(str_Element));

        if (qElement_getlen(&element) > 0) {
            qElement_dequeue(&element, &element_temp, 1);

            SendUartMessage(uartFd, (char *)&element_temp);
            SendUartMessage(uartFd, "\r\n");

            AzureIoT_Result result = AzureIoT_SendTelemetry((char *)&element_temp, NULL);

            if (result != AzureIoT_Result_OK) {
                Log_Debug("WARNING: Could not send thermometer telemetry to cloud: %s\n",
                          CloudResultToString(result));
                SendUartMessage(uartFd,
                                "WARNING: Could not send thermometer telemetry to cloud: %s\n");
            } else {
                Log_Debug("INFO: Telemetry upload disabled; not sending telemetry.\n");
                SendUartMessage(uartFd,
                                "INFO: Telemetry upload disabled; not sending telemetry.\n");
            }

        } else {
            SendUartMessage(uartFd, "WARNING: NO DATA\n");
        }
#endif

    }
}

// 20220726 taylor
#if 1
/// <summary>
///     Helper function to send a fixed message via the given UART.
/// </summary>
/// <param name="uartFd">The open file descriptor of the UART to write to</param>
/// <param name="dataToSend">The data to send over the UART</param>
void SendUartMessage(int uartFd, const char *dataToSend)
{
    size_t totalBytesSent = 0;
    size_t totalBytesToSend = strlen(dataToSend);
    int sendIterations = 0;
    while (totalBytesSent < totalBytesToSend) {
        sendIterations++;

        // Send as much of the remaining data as possible
        size_t bytesLeftToSend = totalBytesToSend - totalBytesSent;
        const char *remainingMessageToSend = dataToSend + totalBytesSent;
        ssize_t bytesSent = write(uartFd, remainingMessageToSend, bytesLeftToSend);
        if (bytesSent == -1) {
            Log_Debug("ERROR: Could not write to UART: %s (%d).\n", strerror(errno), errno);
            exitCode = ExitCode_SendMessage_Write;
            return;
        }

        totalBytesSent += (size_t)bytesSent;
    }

    Log_Debug("Sent %zu bytes over UART in %d calls.\n", totalBytesSent, sendIterations);
}

/// <summary>
///     Handle UART event: if there is incoming data, print it.
///     This satisfies the EventLoopIoCallback signature.
/// </summary>
static void UartEventHandler(EventLoop *el, int fd, EventLoop_IoEvents events, void *context)
{
    const size_t receiveBufferSize = 256;
    uint8_t receiveBuffer[receiveBufferSize + 1]; // allow extra byte for string termination
    ssize_t bytesRead;

    // Read incoming UART data. It is expected behavior that messages may be received in multiple
    // partial chunks.
    bytesRead = read(uartFd, receiveBuffer, receiveBufferSize);
    if (bytesRead == -1) {
        Log_Debug("ERROR: Could not read UART: %s (%d).\n", strerror(errno), errno);
        exitCode = ExitCode_UartEvent_Read;
        return;
    }

    if (bytesRead > 0) {
        // Null terminate the buffer to make it a valid string, and print it
        receiveBuffer[bytesRead] = 0;
        Log_Debug("UART received %d bytes: '%s'.\n", bytesRead, (char *)receiveBuffer);

        qUART_enqueue(&isu3, (unsigned char *)receiveBuffer, (unsigned int)bytesRead);
        // SendUartMessage(uartFd, (char *)receiveBuffer);
    }
}
#endif

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

// 20220726 taylor
#if 1
    // Open WIZNET_ASG210_ISU3_N232_485_SEL GPIO, set as output with value GPIO_Value_Low for using
    // RS232 instead of RS485
    Log_Debug("Opening WIZNET_ASG210_ISU3_N232_485_SEL as output.\n");
    gpioNRS232Fd = GPIO_OpenAsOutput(WIZNET_ASG210_ISU3_N232_485_SEL, GPIO_OutputMode_PushPull,
                                     GPIO_Value_Low);
    if (gpioNRS232Fd == -1) {
        Log_Debug("ERROR: Could not open button GPIO: %s (%d).\n", strerror(errno), errno);
        return ExitCode_Init_OpenButton;
    }

    // Open WIZNET_ASG210_ISU3_NSDA_RXD_SEL GPIO, set as output with value GPIO_Value_High for using
    // RXD3 instead of I2C_SDA
    Log_Debug("Opening WIZNET_ASG210_ISU3_NSDA_RXD_SEL as output.\n");
    gpioNSCL = GPIO_OpenAsOutput(WIZNET_ASG210_ISU3_NSDA_RXD_SEL, GPIO_OutputMode_PushPull,
                                 GPIO_Value_High);
    if (gpioNSCL == -1) {
        Log_Debug("ERROR: Could not open button GPIO: %s (%d).\n", strerror(errno), errno);
        return ExitCode_Init_OpenButton;
    }

    // Create a UART_Config object, open the UART and set up UART event handler
    UART_Config uartConfig;
    UART_InitConfig(&uartConfig);
    uartConfig.baudRate = 115200;
    uartConfig.flowControl = UART_FlowControl_None;
    uartFd = UART_Open(WIZNET_ASG210_ISU3_UART, &uartConfig);
    if (uartFd == -1) {
        Log_Debug("ERROR: Could not open UART: %s (%d).\n", strerror(errno), errno);
        return ExitCode_Init_UartOpen;
    }
    uartEventReg = EventLoop_RegisterIo(eventLoop, uartFd, EventLoop_Input, UartEventHandler, NULL);
    if (uartEventReg == NULL) {
        return ExitCode_Init_RegisterIo;
    }

#if 0
    // Open WIZNET_ASG210_USER_BUTTON_SW2 GPIO as input, and set up a timer to poll it
    Log_Debug("Opening WIZNET_ASG210_USER_BUTTON_SW2 as input.\n");
    gpioButtonFd = GPIO_OpenAsInput(WIZNET_ASG210_USER_BUTTON_SW2);
    if (gpioButtonFd == -1) {
        Log_Debug("ERROR: Could not open button GPIO: %s (%d).\n", strerror(errno), errno);
        return ExitCode_Init_OpenButton;
    }
#endif
#endif

    struct timespec telemetryPeriod = {.tv_sec = 1, .tv_nsec = 0};
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
                            DisplayAlertCallbackHandler, ConnectionChangedCallbackHandler);
}

/// <summary>
///     Close peripherals and handlers.
/// </summary>
static void ClosePeripheralsAndHandlers(void)
{
    free(isu3_buf_p);
    free(element_buf_p);

    DisposeEventLoopTimer(telemetryTimer);
    Cloud_Cleanup();
    UserInterface_Cleanup();
    Connection_Cleanup();
    EventLoop_Close(eventLoop);

    Log_Debug("Closing file descriptors\n");
}
