/*
    Copyright (C) 2016-2017 Alexey Dynda

    This file is part of SSD1306 library.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "font6x8.h"
#include "ssd1306.h"
#include "intf/ssd1306_interface.h"

#include "i2c/ssd1306_i2c.h"
#include "lcd/ssd1306_128x64.h"
#include "lcd/ssd1306_128x32.h"
#include "intf/ssd1306_commands.h"

static uint8_t s_displayHeight;
static uint8_t s_invertByte = 0x00000000;
const uint8_t *s_font6x8 = ssd1306xled_font6x8;

uint8_t      ssd1306_displayHeight()
{
    return s_displayHeight;
}


void         ssd1306_i2cInit()
{
    ssd1306_startTransmission = ssd1306_i2cStart;
    ssd1306_endTransmission = ssd1306_i2cStop;
    ssd1306_sendByte = ssd1306_i2cSendByte;
}

void    ssd1306_128x64_i2c_init()
{
    ssd1306_i2cInit();
    ssd1306_endTransmission();
    s_displayHeight = 64;
    for( uint8_t i=0; i<s_oled128x64_dataLength; i++)
    {
        ssd1306_sendCommand(pgm_read_byte(&s_oled128x64_initData[i]));
    }
}

void    ssd1306_128x32_i2c_init()
{
    ssd1306_i2cInit();
    ssd1306_endTransmission();
    s_displayHeight = 32;
    for( uint8_t i=0; i<s_oled128x32_dataLength; i++)
    {
        ssd1306_sendCommand(pgm_read_byte(&s_oled128x32_initData[i]));
    }
}

void ssd1306_setRamBlock(uint8_t x, uint8_t y, uint8_t w)
{
    ssd1306_startTransmission();
    ssd1306_sendByte(0x00);	//write command
    ssd1306_sendByte(SSD1306_COLUMNADDR);
    ssd1306_sendByte(x);
    ssd1306_sendByte(x + w - 1);
    ssd1306_sendByte(0xb0+y);
    ssd1306_sendByte((x>>4) | 0x10);
    ssd1306_sendByte(x&0x0f);
    ssd1306_endTransmission();
}

void ssd1306_setPos(uint8_t x, uint8_t y)
{
    ssd1306_startTransmission();
    ssd1306_sendByte(0x00);	//write command
    ssd1306_sendByte(0xb0+y);
    ssd1306_sendByte((x>>4) | 0x10);
    ssd1306_sendByte(x&0x0f);
    ssd1306_endTransmission();
}

void ssd1306_fillScreen(uint8_t fill_Data)
{
    fill_Data ^= s_invertByte;
    ssd1306_setRamBlock(0, 0, 128);
    ssd1306_dataStart();
    for(uint8_t m=0; m<(s_displayHeight >> 3); m++)
    {
        for(uint8_t n=0; n<128; n++)
        {
            ssd1306_sendByte(fill_Data);
        }
    }
    ssd1306_endTransmission();
}

void ssd1306_clearScreen()
{
    ssd1306_setRamBlock(0, 0, 128);
    ssd1306_dataStart();
    for(uint8_t m=0;m<(s_displayHeight >> 3);m++)
    {
        for(uint8_t n=0;n<128;n++)
        {
            ssd1306_sendByte(s_invertByte);
        }
    }
    ssd1306_endTransmission();
}


void ssd1306_displayOff()
{
    ssd1306_sendCommand(SSD1306_DISPLAYOFF);
}


void ssd1306_displayOn()
{
    ssd1306_sendCommand(SSD1306_DISPLAYON);
}

void ssd1306_charF6x8(uint8_t x, uint8_t y, const char ch[], EFontStyle style)
{
    uint8_t c,i,j=0;
    ssd1306_setRamBlock(0, 0, 128);
    ssd1306_setPos(x,y);
    ssd1306_dataStart();
    while(ch[j] != '\0')
    {
        c = ch[j] - 32;
        if(x>126)
        {
            x=0;
            y++;
        }
        uint8_t ldata = 0;
        for(i=0;i<6;i++)
        {
            uint8_t data;
            if ( style == STYLE_NORMAL )
            {
                data = pgm_read_byte(&s_font6x8[c*6+i]);
            }
            else if ( style == STYLE_BOLD )
            {
                uint8_t temp = pgm_read_byte(&s_font6x8[c*6+i]);
                data = temp | ldata;
                ldata = temp;
            }
            else
            {
                uint8_t temp = pgm_read_byte(&s_font6x8[c*6+i + 1]);
                data = (temp & 0xF0) | ldata;
                ldata = (temp & 0x0F);
            }
            ssd1306_sendByte(data^s_invertByte);
        }
        x += 6;
        j++;
    }
    ssd1306_endTransmission();
}

void         ssd1306_setFont6x8(const uint8_t * progmemFont)
{
    s_font6x8 = progmemFont;
}

void         ssd1306_putPixel(uint8_t x, uint8_t y)
{
    ssd1306_setRamBlock(0, 0, 128);
    ssd1306_setPos(x,y >> 3);
    ssd1306_dataStart();
    ssd1306_sendByte((1 << (y & 0x07))^s_invertByte);
    ssd1306_endTransmission();
}

void         ssd1306_putPixels(uint8_t x, uint8_t y, uint8_t pixels)
{
    ssd1306_setRamBlock(0, 0, 128);
    ssd1306_setPos(x,y);
    ssd1306_dataStart();
    ssd1306_sendByte(pixels^s_invertByte);
    ssd1306_endTransmission();
}


void ssd1306_drawCanvas(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *buf)
{
    uint8_t i, j;
    ssd1306_setRamBlock(x, y, w);
    ssd1306_dataStart();
    for(j=0; j<(h >> 3); j++)
    {
        for(i=0;i<w;i++)
        {
            ssd1306_sendByte(s_invertByte^*buf++);
        }
    }
    ssd1306_endTransmission();
}

void ssd1306_drawBitmap(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *buf)
{
    uint8_t i, j;
    ssd1306_setRamBlock(x, y, w);
    ssd1306_dataStart();
    for(j=0; j<(h >> 3); j++)
    {
        for(i=w;i>0;i--)
        {
            ssd1306_sendByte(s_invertByte^pgm_read_byte(buf++));
        }
    }
    ssd1306_endTransmission();
}


void ssd1306_clearBlock(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
    uint8_t i, j;
    ssd1306_setRamBlock(x, y, w);
    ssd1306_dataStart();
    for(j=0; j<(h >> 3); j++)
    {
        for(i=w;i>0;i--)
        {
            ssd1306_sendByte(s_invertByte);
        }
    }
    ssd1306_endTransmission();
}


void ssd1306_drawSpriteData(uint8_t x, uint8_t y, const uint8_t *sprite)
{
   uint8_t i;
   ssd1306_setPos(x,y);
   ssd1306_dataStart();
   for(i=0;i<8;i++)
   {
       ssd1306_sendByte(s_invertByte^pgm_read_byte(&sprite[i]));
   }
   ssd1306_endTransmission();
}


void ssd1306_drawSprite(SPRITE *sprite)
{
    uint8_t posy = sprite->y >> 3;
    uint8_t offsety = sprite->y & 0x7;
    if (posy < (s_displayHeight >> 3))
    {
        ssd1306_setPos(sprite->x, posy);
        ssd1306_dataStart();
        for (uint8_t i=0; i < sprite->w; i++)
        {
            ssd1306_sendByte( s_invertByte^(pgm_read_byte( &sprite->data[i] ) << offsety) );
        }
        ssd1306_endTransmission();
    }
    if (offsety && (posy + 1 < (s_displayHeight >> 3)))
    {
        ssd1306_setPos(sprite->x, posy + 1);
        ssd1306_dataStart();
        for (uint8_t i=0; i < sprite->w; i++)
        {
            ssd1306_sendByte( s_invertByte^(pgm_read_byte( &sprite->data[i] ) >> (8 - offsety)) );
        }
    }
    ssd1306_endTransmission();
    sprite->lx = sprite->x;
    sprite->ly = sprite->y;
}


void ssd1306_eraseSprite(SPRITE *sprite)
{
    uint8_t posy = sprite->y >> 3;
    uint8_t offsety = sprite->y & 0x7;
    ssd1306_setPos(sprite->x, posy);
    ssd1306_dataStart();
    for (uint8_t i=0; i < sprite->w; i++)
    {
       ssd1306_sendByte( s_invertByte );
    }
    ssd1306_endTransmission();
    if (offsety)
    {
        ssd1306_setPos(sprite->x, posy + 1);
        ssd1306_dataStart();
        for (uint8_t i=0; i < sprite->w; i++)
        {
           ssd1306_sendByte( s_invertByte );
        }
    }
    ssd1306_endTransmission();
}


void ssd1306_eraseTrace(SPRITE *sprite)
{
    uint8_t y1 = sprite->ly >> 3;
    uint8_t y2 = (sprite->ly + 7) >> 3;
    if (sprite->ly < sprite->y)
        y2 = min(y2, (uint8_t)((sprite->y >> 3) - 1));
    else
        y1 = max(y1, (sprite->ly + 7) >> 3);
    for(uint8_t y = y1; y <= y2; y++)
    {
        ssd1306_setPos(sprite->lx, y);
        ssd1306_dataStart();
        for(uint8_t x = sprite->lx; x < sprite->lx + sprite->w; x++)
        {
            ssd1306_sendByte( s_invertByte );
        }
        ssd1306_endTransmission();
    }
    if (sprite->lx != sprite->x)
    {
        uint8_t x1 = sprite->lx;
        uint8_t x2 = sprite->lx + sprite->w - 1;
        if (sprite->x < sprite->lx)
            x1 = max(x1, sprite->x + sprite->w);
        else
            x2 = min((uint8_t)(sprite->x - 1), x2);
        for(uint8_t y = sprite->ly >> 3; y <= (sprite->ly + 7) >> 3; y++)
        {
            ssd1306_setPos(x1, y);
            ssd1306_dataStart();
            for(uint8_t x = x1; x <= x2; x++)
            {
                ssd1306_sendByte( s_invertByte );
            }
            ssd1306_endTransmission();
        }
    }
}


SSD1306_RECT ssd1306_rectFromSprite(uint8_t x, uint8_t y, uint8_t w)
{
    return { x, (uint8_t)(y & 0xF8), (uint8_t)(x + w - 1), (uint8_t)((y + 7) & 0xF8) };
}

SSD1306_RECT ssd1306_rect(SPRITE * sprite)
{
    return { 0, 0, 0, 0 };
}


SPRITE       ssd1306_createSprite(uint8_t x, uint8_t y, uint8_t w, const uint8_t *data)
{
    return {x,y,w,x,y,data,nullptr};
}

void         ssd1306_replaceSprite(SPRITE *sprite, const uint8_t *data)
{
    sprite->data = data;
}

void         ssd1306_invertMode()
{
    ssd1306_sendCommand(SSD1306_INVERTDISPLAY);
}

void         ssd1306_normalMode()
{
    ssd1306_sendCommand(SSD1306_NORMALDISPLAY);
}

void         ssd1306_negativeMode()
{
    s_invertByte = 0xFF;
}

void         ssd1306_positiveMode()
{
    s_invertByte = 0x00;
}
