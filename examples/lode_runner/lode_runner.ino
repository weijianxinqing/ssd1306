/*
    Copyright (C) 2017 Alexey Dynda

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
/*
 *   Attiny85 PINS
 *             ____
 *   RESET   -|_|  |- 3V
 *   SCL (3) -|    |- (2)
 *   SDA (4) -|    |- (1) - BUZZER
 *   GND     -|____|- (0) - BUTTONS module
 *
 *   Atmega328 PINS: connect LCD to A4/A5, BUZZER on D3,
 *   Z-keypad ADC module on A0 pin.
 */

#include "game_field.h"
#include "ssd1306.h"
#include "ssd1306_i2c_conf.h"
#include "player_sprites.h"
#include "coin_sprite.h"
#include "hero_states.h"
#include "buttons.h"

/* Do not include wire.h for Attiny controllers */
#ifndef SSD1306_EMBEDDED_I2C
    #include <Wire.h>
#endif

#if defined(__AVR_ATtiny25__) | defined(__AVR_ATtiny45__) | defined(__AVR_ATtiny85__)
#define BUZZER      1
#define BUTTON_PIN  A0
#else // For Arduino Nano/Atmega328 we use different pins
#define BUZZER      3
#define BUTTON_PIN  A0
#endif

/**
 * Just produces some sound depending on params 
 */
void beep(int bCount,int bDelay);

/* Define player sprite */
SPRITE   playerSprite;

/* The variable is used for player animation      *
 * The graphics defined for the hero has 2 images *
 * for each direction. So, the variable is either *
 * 0 or 1. */
uint8_t  playerAnimation = 0;

/* Timestamp when playerAnimation was changed last time */
uint16_t playerAnimationTs = 0;

/* Number of coins collected */
uint8_t  goldCollection = 0;

void showGameInfo()
{
    for (uint8_t i=0; i<goldCollection; i++)
    {
        ssd1306_drawBitmap(0 + (i<<3),0,8,8,coinImage);
    }
}

void movePlayer(SPRITE &playerSprite, uint8_t direction)
{
    uint8_t x = playerSprite.x + 3;
    bool animated = false;
    /* If player doesn't stand on the ground, and doesn't hold the pipe,
     * make the player to fall down. */
    if ( !isSolid(gameField[blockIdx(x,playerSprite.y + 8)]) &&
         (!isPipe(gameField[blockIdx(x,playerSprite.y)]) ||
          !isPipe(gameField[blockIdx(x,playerSprite.y + 6)]) ) )
    {
        playerSprite.x = (((x) >> 3) << 3);
        playerSprite.y += 1;
        playerSprite.data = &playerFlyingImage[MAN_ANIM_FLYING][playerAnimation][0];
        animated = true;
    }
    else
    {
        switch (direction)
        {
            case BUTTON_RIGHT:
                if (isWalkable(gameField[blockIdx(playerSprite.x + 8,playerSprite.y)]))
                {
                    playerSprite.x += 1;
                    if (isPipe(gameField[blockIdx(playerSprite.x + 3,playerSprite.y)]))
                    {
                        playerSprite.data = &playerFlyingImage[MAN_ANIM_RIGHT_PIPE][playerAnimation][0];
                    }
                    else
                    {
                        playerSprite.data = &playerFlyingImage[MAN_ANIM_RIGHT][playerAnimation][0];
                    }
                    animated = true;
                }
                break;
            case BUTTON_LEFT:
                if (isWalkable(gameField[blockIdx(playerSprite.x - 1,playerSprite.y)]))
                {
                    playerSprite.x -= 1;
                    if (isPipe(gameField[blockIdx(playerSprite.x + 3,playerSprite.y)]))
                    {
                        playerSprite.data = &playerFlyingImage[MAN_ANIM_LEFT_PIPE][playerAnimation][0];
                    }
                    else
                    {
                        playerSprite.data = &playerFlyingImage[MAN_ANIM_LEFT][playerAnimation][0];
                    }
                    animated = true;
                }
                break;
            case BUTTON_UP:
                if (isStair(gameField[blockIdx(x,playerSprite.y+7)]))
                {
                    playerSprite.x = (((x) >> 3) << 3);
                    playerSprite.y -= 1;
                    playerSprite.data = &playerFlyingImage[MAN_ANIM_UP][playerAnimation][0];
                    animated = true;
                }
                break;
            case BUTTON_DOWN:
                if ( isStair(gameField[blockIdx(x,playerSprite.y+8)]) ||
                   (!isSolid(gameField[blockIdx(x,playerSprite.y+8)]) &&
                     isPipe(gameField[blockIdx(x,playerSprite.y)])) )
                {
                    playerSprite.x = (((x) >> 3) << 3);
                    playerSprite.y += 1;
                    playerSprite.data = &playerFlyingImage[MAN_ANIM_DOWN][playerAnimation][0];
                    animated = true;
                }
                break;
            default:
                break;
        }
    }
    if (animated && ((uint16_t)millis() - playerAnimationTs > 150))
    {
        playerAnimationTs = millis();
        playerAnimation = playerAnimation ? 0 : 1;
        beep(10,20);
        if (isGold(gameField[blockIdx(x,playerSprite.y + 3)]))
        {
            gameField[blockIdx(x,playerSprite.y + 3)] = 0;
            goldCollection++;
            showGameInfo();
            /* Produce sound every time the player moves */
            beep(20,40);
            beep(20,80);
            beep(20,120);
            beep(20,80);
            beep(20,40);
        }
    }
}


void setup()
{
    /* Do not init Wire library for Attiny controllers */
#ifndef SSD1306_EMBEDDED_I2C
    Wire.begin();
    Wire.setClock( 400000  );
#endif
    ssd1306_128x64_i2c_init();
    ssd1306_fillScreen(0x00);
    /* Set range of the gameField on the screen in blocks. */
    field.setRect( (SSD1306_RECT) { 0, 1, 15, 7 } );
    playerSprite = ssd1306_createSprite( 8, 8, 8, playerFlyingImage[MAN_ANIM_FLYING][playerAnimation] );
    field.add( playerSprite );
    field.refreshScreen();
    showGameInfo();
    pinMode(BUZZER, OUTPUT);
}


void loop()
{
    static unsigned long lastTs = millis();
    /* Move sprite every 40 milliseconds */
    if ((unsigned long)(millis() - lastTs) >= 40)
    {
        lastTs += 40;
        uint8_t direction = getPressedButton(BUTTON_PIN);
        movePlayer(playerSprite, direction);
        field.drawSprites();
    }
}


void beep(int bCount,int bDelay)
{
    for (int i = 0; i<=bCount*2; i++)
    {
        digitalWrite(BUZZER,i&1);
        for(int i2=0; i2<bDelay; i2++)
        {
            __asm__("nop\n\t");
#if F_CPU > 8000000
            __asm__("nop\n\t");
            __asm__("nop\n\t");
            __asm__("nop\n\t");
#endif
        }
    }
    digitalWrite(BUZZER,LOW);
}


