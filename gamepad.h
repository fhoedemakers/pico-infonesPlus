/*
 * author : Shuichi TAKANO
 * since  : Fri Jul 30 2021 04:42:27
 */
#ifndef _510036F3_0134_6411_4376_A918ACA8AC4C
#define _510036F3_0134_6411_4376_A918ACA8AC4C

#include <stdint.h>

namespace io
{
    struct GamePadState
    {
        bool connected{false};
        struct Button
        {
            inline static constexpr int A = 1 << 0;
            inline static constexpr int B = 1 << 1;
            inline static constexpr int X = 1 << 2;
            inline static constexpr int Y = 1 << 3;
            inline static constexpr int SELECT = 1 << 6;
            inline static constexpr int START = 1 << 7;

            inline static constexpr int LEFT = 1 << 31;
            inline static constexpr int RIGHT = 1 << 30;
            inline static constexpr int UP = 1 << 29;
            inline static constexpr int DOWN = 1 << 28;
        };

        enum class Hat
        {
            N,
            NE,
            E,
            SE,
            S,
            SW,
            W,
            NW,
            RELEASED,
        };

        uint8_t axis[3]{0x80, 0x80, 0x80};
        Hat hat{Hat::RELEASED};
        uint32_t buttons{0};

    public:
        void convertButtonsFromAxis(int axisX, int axisY);
        void convertButtonsFromHat();
        void flagConnected(bool connected) { this->connected = connected; }
        bool isConnected() const { return connected; }
    };

    GamePadState &getCurrentGamePadState(int i);
}

#endif /* _510036F3_0134_6411_4376_A918ACA8AC4C */
