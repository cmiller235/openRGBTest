/*-----------------------------------------*\
|  JginYueInternalUSBController.cpp         |
|                                           |
|  Driver for JginYue internal USB          |
|  lighting controller                      |
|                                           |
|                                           |
|                                           |
\*-----------------------------------------*/
#include <cstring>
#include <string>
#include <stdio.h>

#include "RGBController.h"
#include "ResourceManager.h"
#include "SettingsManager.h"
#include "JginYueInternalUSBController.h"
#include "dependencies/dmiinfo.h"

#define JGINYUE_USB_GENERAL_COMMAND_HEADER              0x01
#define JGINYUE_USB_LED_STRIPE_SET_COMMAND_HEADER       0x02
#define JGINYUE_USB_MODE_SET_COMMAND_HEADER             0x03
#define JGINYUE_USB_PER_LED_SET_COMMAND_HEADER          0x04

#define JGINYUE_USB_GET_FW_VERSION                      0xA0
#define JGINYUE_USB_GET_FW_REPLY                        0x5A
#define JGINYUE_RG_DEFAULT                              0x00
#define JGINYUE_RG_SWAP                                 0x01


JginYueInternalUSBController::JginYueInternalUSBController(hid_device* dev_handle, const char* path)
{
    DMIInfo         dmi;

    dev           = dev_handle;
    location      = path;
    device_name   = "JGINYUE " + dmi.getMainboard();

    Init_device(device_config);
}

JginYueInternalUSBController::~JginYueInternalUSBController()
{
    hid_close(dev);
}
unsigned int JginYueInternalUSBController::GetZoneCount()
{
    return(JGINYUE_MAX_ZONES);
}

std::string JginYueInternalUSBController::GetDeviceLocation()
{
    return("HID:" + location);
}

std::string JginYueInternalUSBController::GetDeviceName()
{
    return(device_name);
}

std::string JginYueInternalUSBController::GetSerialString()
{
    return("");
}

std::string JginYueInternalUSBController::GetDeviceFWVirson()
{
    unsigned char   usb_buf[16];

    memset(usb_buf,0x00,sizeof(usb_buf));

    usb_buf[0x00]  = JGINYUE_USB_GENERAL_COMMAND_HEADER;
    usb_buf[0x01]  = JGINYUE_USB_GET_FW_VERSION;

    hid_write(dev, usb_buf, 16);
    hid_read(dev, usb_buf, 16);

    if((usb_buf[0x00]!=JGINYUE_USB_GENERAL_COMMAND_HEADER)||(usb_buf[0x01]!=JGINYUE_USB_GET_FW_REPLY))
    {
        return "";
    }

    std::string     Major_virson =  std::to_string(usb_buf[0x02]);
    std::string     Minor_virson =  std::to_string(usb_buf[0x03]);
    return ("Major_virson " + Major_virson + "Minor_virson " + Minor_virson);
}


void JginYueInternalUSBController::Init_device(AreaConfiguration* ptr_device_cfg)
{
    for(int index_config=1; index_config <=JGINYUE_MAX_ZONES; index_config++)
    {
        ptr_device_cfg[index_config].Brightness = JGINYUE_USB_BRIGHTNESS_DEFAULT;
        ptr_device_cfg[index_config].Color_B = 0xFF;
        ptr_device_cfg[index_config].Color_G = 0xFF;
        ptr_device_cfg[index_config].Color_R = 0xFF;
        ptr_device_cfg[index_config].RG_Swap = JGINYUE_RG_DEFAULT;
        ptr_device_cfg[index_config].Speed = JGINYUE_USB_SPEED_DEFAULT;
        ptr_device_cfg[index_config].LED_numbers = JGINYUE_ADDRESSABLE_MAX_LEDS;
        ptr_device_cfg[index_config].Mode_active = JGINYUE_USB_MODE_STATIC;
    }
}

void JginYueInternalUSBController::WriteZoneMode
    (
    unsigned char   zone,
    unsigned char   mode,
    RGBColor        rgb,
    unsigned char   speed,
    unsigned char   brightness,
    unsigned char   direction
    )
{
    int Active_zone;
    unsigned char usb_buf[65];
    switch (zone)
    {
    case 0x01:
        Active_zone=1;
        break;
    case 0x02:
        Active_zone=2;
        break;
    default:
        break;
    }
    device_config[Active_zone].Mode_active = mode;
    device_config[Active_zone].Direct_Mode_control = 0x00;
    device_config[Active_zone].Speed = speed;
    device_config[Active_zone].Brightness = brightness;
    device_config[Active_zone].Direction = direction;
    device_config[Active_zone].Color_B = RGBGetBValue(colors[0]);
    device_config[Active_zone].Color_G = RGBGetGValue(colors[0]);
    device_config[Active_zone].Color_R = RGBGetRValue(colors[0]);

    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0x00]   = JGINYUE_USB_LED_STRIPE_SET_COMMAND_HEADER;
    usb_buf[0x01]   = zone;
    usb_buf[0x02]   = device_config[Active_zone].LED_numbers;
    usb_buf[0x03]   = device_config[Active_zone].RG_Swap;
    usb_buf[0x04]   = device_config[Active_zone].Direction;
    usb_buf[0x05]   = device_config[Active_zone].Direct_Mode_control;
    hid_write(dev, usb_buf ,65);

    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0x00]   = JGINYUE_USB_MODE_SET_COMMAND_HEADER;
    usb_buf[0x01]   = zone;
    usb_buf[0x02]   = device_config[Active_zone].Mode_active;
    usb_buf[0x03]   = device_config[Active_zone].Color_R;
    usb_buf[0x04]   = device_config[Active_zone].Color_G;
    usb_buf[0x05]   = device_config[Active_zone].Color_B;
    usb_buf[0x06]   = device_config[Active_zone].Brightness;
    usb_buf[0x07]   = device_config[Active_zone].Speed;
    hid_write(dev, usb_buf ,65);
}

void JginYueInternalUSBController::DirectLEDControl
    (
    RGBColor*              colors,
    unsigned char          zone
    )
{
    int Active_zone;
    unsigned char usb_buf[302];
    switch (zone)
    {
    case 0x01:
        Active_zone=1;
        break;
    case 0x02:
        Active_zone=2;
        break;
    default:
        break;
    }
    device_config[Active_zone].Mode_active=JGINYUE_USB_MODE_DIRECT;
    device_config[Active_zone].Direct_Mode_control=0x01;

    memset(usb_buf, 0x00, sizeof(usb_buf));


    usb_buf[0x00]   = JGINYUE_USB_LED_STRIPE_SET_COMMAND_HEADER;
    usb_buf[0x01]   = zone;
    usb_buf[0x02]   = device_config[Active_zone].LED_numbers;
    usb_buf[0x03]   = device_config[Active_zone].RG_Swap;
    usb_buf[0x04]   = device_config[Active_zone].Direction;
    usb_buf[0x05]   = device_config[Active_zone].Direct_Mode_control;

    hid_write(dev, usb_buf ,302);
    memset(usb_buf, 0x00, sizeof(usb_buf));
    usb_buf[0x00]   = JGINYUE_USB_PER_LED_SET_COMMAND_HEADER;
    usb_buf[0x01]   = zone;

    for(unsigned int color_idx = 0; color_idx < device_config[Active_zone].LED_numbers; color_idx++)
    {
        usb_buf[color_idx * 3 + 3]  = RGBGetRValue(colors[color_idx]);
        usb_buf[color_idx * 3 + 4]  = RGBGetGValue(colors[color_idx]);
        usb_buf[color_idx * 3 + 5]  = RGBGetBValue(colors[color_idx]);
    }
    hid_write(dev,usb_buf,302);
}


void JginYueInternalUSBController::Area_resize(unsigned char led_numbers,unsigned char zone)
{
    unsigned char usb_buf[65];
    int Active_zone;
    switch (zone)
    {
    case 0x01:
        Active_zone=1;
        break;
    case 0x02:
        Active_zone=2;
        break;
    default:
        break;
    }
    device_config[Active_zone].LED_numbers=led_numbers;

    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0x00]   = JGINYUE_USB_LED_STRIPE_SET_COMMAND_HEADER;
    usb_buf[0x01]   = zone;
    usb_buf[0x02]   = device_config[Active_zone].LED_numbers;
    usb_buf[0x03]   = device_config[Active_zone].RG_Swap;
    usb_buf[0x04]   = device_config[Active_zone].Direction;
    usb_buf[0x05]   = device_config[Active_zone].Direct_Mode_control;
    hid_write(dev, usb_buf ,65);
}

void JginYueInternalUSBController::SetRGSwap(unsigned char RGSwap)
{
    if((RGSwap!=0x00)&&(RGSwap!=0x01))
    {
        return;
    }

    for(int index_config=1; index_config <=JGINYUE_MAX_ZONES; index_config++)
    {
        device_config[index_config].RG_Swap = RGSwap;
    }
}
