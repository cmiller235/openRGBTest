/*-----------------------------------------*\
|  RGBController_CorsairCmdrPro.cpp         |
|                                           |
|  Generic RGB Interface for Corsair        |
|  Commander Pro                            |
|                                           |
|  Adam Honse (CalcProgrammer1) 1/16/2020   |
\*-----------------------------------------*/

#include "RGBController_CorsairCmdrPro.h"


RGBController_CorsairCmdrPro::RGBController_CorsairCmdrPro(CorsairCmdrProController* corsair_ptr)
{
    corsair = corsair_ptr;

    name = "Corsair Commander Pro";

    type = DEVICE_TYPE_LEDSTRIP;
    
    mode Direct;
    Direct.name  = "Direct";
    Direct.value = 0xFFFF;
    Direct.flags = MODE_FLAG_HAS_COLOR | MODE_FLAG_PER_LED_COLOR;
    modes.push_back(Direct);

    mode RainbowWave;
    RainbowWave.name      = "Rainbow Wave";
    RainbowWave.value     = CORSAIR_CMDR_PRO_MODE_RAINBOW_WAVE;
    RainbowWave.flags     = MODE_FLAG_HAS_SPEED;
    RainbowWave.speed_min = CORSAIR_CMDR_PRO_SPEED_SLOW;
    RainbowWave.speed_max = CORSAIR_CMDR_PRO_SPEED_FAST;
    RainbowWave.speed     = CORSAIR_CMDR_PRO_SPEED_MEDIUM;
    modes.push_back(RainbowWave);

    mode ColorShift;
    ColorShift.name       = "Color Shift";
    ColorShift.value      = CORSAIR_CMDR_PRO_MODE_COLOR_SHIFT;
    ColorShift.flags      = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_COLOR | MODE_FLAG_RANDOM_COLOR;
    ColorShift.speed_min  = CORSAIR_CMDR_PRO_SPEED_SLOW;
    ColorShift.speed_max  = CORSAIR_CMDR_PRO_SPEED_FAST;
    ColorShift.speed      = CORSAIR_CMDR_PRO_SPEED_MEDIUM;
    modes.push_back(ColorShift);

    mode ColorPulse;
    ColorPulse.name      = "Color Pulse";
    ColorPulse.value     = CORSAIR_CMDR_PRO_MODE_COLOR_PULSE;
    ColorPulse.flags     = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_COLOR | MODE_FLAG_RANDOM_COLOR;
    ColorPulse.speed_min = CORSAIR_CMDR_PRO_SPEED_SLOW;
    ColorPulse.speed_max = CORSAIR_CMDR_PRO_SPEED_FAST;
    ColorPulse.speed     = CORSAIR_CMDR_PRO_SPEED_MEDIUM;
    modes.push_back(ColorPulse);

    /*-------------------------------------------------*\
    | Set size of colors array                          |
    \*-------------------------------------------------*/
    unsigned int led_count = 0;
    for (unsigned int channel_idx = 0; channel_idx < CORSAIR_CMDR_PRO_NUM_CHANNELS; channel_idx++)
    {
        led_count += corsair->channel_leds[channel_idx];
    }
    colors.resize(led_count);

    /*-------------------------------------------------*\
    | Set zones and leds                                |
    \*-------------------------------------------------*/
    unsigned int led_idx = 0;
    for (unsigned int channel_idx = 0; channel_idx < CORSAIR_CMDR_PRO_NUM_CHANNELS; channel_idx++)
    {
        if(corsair->channel_leds[channel_idx] > 0)
        {
            zone* new_zone = new zone;

            char ch_idx_string[2];
            sprintf(ch_idx_string, "%d", channel_idx + 1);

            new_zone->name = "Corsair Channel ";
            new_zone->name.append(ch_idx_string);
            new_zone->type = ZONE_TYPE_LINEAR;

            std::vector<int> *new_zone_map = new std::vector<int>();

            for (unsigned int led_ch_idx = 0; led_ch_idx < corsair->channel_leds[channel_idx]; led_ch_idx++)
            {
                char led_idx_string[3];
                sprintf(led_idx_string, "%d", led_ch_idx + 1);

                led new_led;
                new_led.name = "Corsair Channel ";
                new_led.name.append(ch_idx_string);
                new_led.name.append(", LED ");
                new_led.name.append(led_idx_string);

                leds.push_back(new_led);
                leds_channel.push_back(channel_idx);

                new_zone_map->push_back(led_idx);
                led_idx++;
            }

            new_zone->map.push_back(*new_zone_map);
            zones.push_back(*new_zone);
            zones_channel.push_back(channel_idx);
        }
    }
}

int RGBController_CorsairCmdrPro::GetMode()
{
    return 0;
}

void RGBController_CorsairCmdrPro::SetMode(int mode)
{
    active_mode = mode;

    if(modes[active_mode].value == 0xFFFF)
    {
        UpdateLEDs();
    }
    else
    {
        for(int channel = 0; channel < CORSAIR_CMDR_PRO_NUM_CHANNELS; channel++)
        {
            corsair->SetChannelEffect(channel,
                                      modes[active_mode].value,
                                      modes[active_mode].speed,
                                      0,
                                      modes[active_mode].random,
                                      RGBGetRValue(colors[0]),
                                      RGBGetGValue(colors[0]),
                                      RGBGetBValue(colors[0]),
                                      RGBGetRValue(colors[1]),
                                      RGBGetGValue(colors[1]),
                                      RGBGetBValue(colors[1]));
        }
    }
}

void RGBController_CorsairCmdrPro::SetCustomMode()
{

}

void RGBController_CorsairCmdrPro::UpdateLEDs()
{
    for(std::size_t zone_idx = 0; zone_idx < zones.size(); zone_idx++)
    {
        unsigned int channel = zones_channel[zone_idx];

        std::vector<RGBColor> channel_colors;

        for(std::size_t color = 0; color < colors.size(); color++)
        {
            if(leds_channel[color] == channel)
            {
                channel_colors.push_back(colors[color]);
            }
        }

        if(channel_colors.size() > 0)
        {
            corsair->SetChannelLEDs(channel, channel_colors);
        }
    }
}

void RGBController_CorsairCmdrPro::UpdateZoneLEDs(int zone)
{
    unsigned int channel = zones_channel[zone];

    std::vector<RGBColor> channel_colors;

    for(std::size_t color = 0; color < colors.size(); color++)
    {
        if(leds_channel[color] == channel)
        {
            channel_colors.push_back(colors[color]);
        }
    }

    if(channel_colors.size() > 0)
    {
        corsair->SetChannelLEDs(channel, channel_colors);
    }
}

void RGBController_CorsairCmdrPro::UpdateSingleLED(int led)
{
    unsigned int channel = leds_channel[led];

    std::vector<RGBColor> channel_colors;

    for(std::size_t color = 0; color < colors.size(); color++)
    {
        if(leds_channel[color] == channel)
        {
            channel_colors.push_back(colors[color]);
        }
    }

    if(channel_colors.size() > 0)
    {
        corsair->SetChannelLEDs(channel, channel_colors);
    }
}
