# Azure Sphere Samples
This repository contains samples for the [Azure Sphere platform](https://www.microsoft.com/azure-sphere/) that are created and maintained by Microsoft.

You may also be interested in:
- https://github.com/Azure/azure-sphere-hardware-designs/ - maintained hardware designs for Azure Sphere
- https://github.com/Azure/azure-sphere-gallery/ - gallery of further unmaintained content from Microsoft

Please also see the Codethink, MediaTek, and Azure RTOS repositories for more sample applications for the MT3620 real-time cores:
- https://github.com/CodethinkLabs/mt3620-m4-samples
- https://github.com/MediaTek-Labs/mt3620_m4_software
- https://github.com/Azure-Samples/Azure-RTOS-on-Azure-Sphere-Mediatek-MT3620

## Using the samples
See the [Azure Sphere Getting Started](https://www.microsoft.com/azure-sphere/get-started/) page for details on getting an [Azure Sphere development kit](https://aka.ms/AzureSphereHardware) and setting up your PC for development. You should complete the Azure Sphere [Installation Quickstarts](https://learn.microsoft.com/azure-sphere/install/overview) and [Tutorials](https://learn.microsoft.com/azure-sphere/install/qs-overview) to validate that your environment is configured properly before using the samples here.

Clone this entire repository locally. The repository includes the [hardware definition files](./HardwareDefinitions/) that are required to run the samples on a range of Azure Sphere hardware.

Each folder within the samples subdirectory contains a README.md file that describes the samples therein. Follow the instructions for each individual sample to build and deploy it to your Azure Sphere hardware to learn about the features that the sample demonstrates.

## Contributing
This project welcomes contributions and suggestions. Most contributions require you to agree to a Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us the rights to use your contribution. For details, visit https://cla.microsoft.com.

When you submit a pull request, a CLA-bot will automatically determine whether you need to provide a CLA and decorate the PR appropriately (for example, label, comment). Simply follow the instructions provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

## Licenses

For information about the licenses that apply to a particular sample, see the License and README.md files in each subdirectory.

## Samples by category

| Categories                        | Samples                       |
|-----------------------------------|-------------------------------|
| Application lifecycle             | [Deferred update](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/DeferredUpdate/DeferredUpdate_HighLevelApp) <br/> [Device to Cloud](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/DeviceToCloud/ExternalMcuLowPower) <br/> [Power down](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/Powerdown/Powerdown_HighLevelApp) |
| External MCUs                     | [Device to Cloud](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/DeviceToCloud/ExternalMcuLowPower) <br/> [External MCU update](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/ExternalMcuUpdate) <br/> [Wi-Fi setup via BLE](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/WifiSetupAndDeviceControlViaBle) |
| Microsoft Azure                   | [Azure IoT](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/AzureIoT) |
| Multi-core samples                |  [Hello World](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/HelloWorld)  <br/>[Inter-core communication](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/IntercoreComms) |
| Networking & time                 | [Certificates](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/Certificates/Cert_HighLevelApp) <br/> [Custom NTP](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/CustomNTP/CustomNTP_HighLevelApp) <br/> [DHCP](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/DHCP) <br/> [DNS service discovery](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/DNSServiceDiscovery) <br/> [HTTPS cURL Easy](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/HTTPS/HTTPS_Curl_Easy) <br/> [HTTPS cURL Multi](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/HTTPS/HTTPS_Curl_Multi) <br/> [Private network services](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/PrivateNetworkServices) <br/> [System time](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/SystemTime) <br/> [Wi-Fi](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/WiFi/WiFi_HighLevelApp) <br/> [Wi-Fi setup via BLE](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/WifiSetupAndDeviceControlViaBle) <br/> [WolfSSL API](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/WolfSSL/WolfSSL_HighLevelApp) |
| Peripherals, sensors, &   devices | [Hello World](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/HelloWorld) <br/> [ADC](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/ADC/ADC_HighLevelApp) <br/> [GPIO](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/GPIO/GPIO_HighLevelApp) <br/> [I2C](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/I2C/I2C_LSM6DS3_HighLevelApp) <br/> [PWM](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/PWM/PWM_HighLevelApp) <br/> [SPI](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/SPI/SPI_LSM6DS3_HighLevelApp) <br/> [UART](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/UART/UART_HighLevelApp) |
| Power & memory                    |  [External MCU low power](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/DeviceToCloud/ExternalMcuLowPower) <br/> [Power down](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/Powerdown/Powerdown_HighLevelApp) |
| Storage                           | [Mutable storage](https://github.com/Azure/azure-sphere-samples/tree/main/Samples/MutableStorage) |
