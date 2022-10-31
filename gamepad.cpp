/*
 * author : Shuichi TAKANO
 * since  : Sun Jan 02 2022 18:19:28
 */

#include "gamepad.h"

#include <stdio.h>

namespace io
{
    namespace
    {
        GamePadState currentGamePad_[2];
    }

    GamePadState &getCurrentGamePadState(int i)
    {
        return currentGamePad_[i];
    }

    void
    GamePadState::convertButtonsFromAxis(int axisX, int axisY)
    {
        int x = axis[axisX];
        int y = axis[axisY];

        if (x < 64)
        {
            buttons |= Button::LEFT;
        }
        else if (x > 192)
        {
            buttons |= Button::RIGHT;
        }

        if (y < 64)
        {
            buttons |= Button::UP;
        }
        else if (y > 192)
        {
            buttons |= Button::DOWN;
        }
    }

    void
    GamePadState::convertButtonsFromHat()
    {
        static constexpr int table[] = {
            Button::UP,
            Button::UP | Button::RIGHT,
            Button::RIGHT,
            Button::DOWN | Button::RIGHT,
            Button::DOWN,
            Button::LEFT | Button::DOWN,
            Button::LEFT,
            Button::LEFT | Button::UP,
        };
        auto i = static_cast<int>(hat);
        if (i < 8)
        {
            buttons |= table[i];
        }
    }
}