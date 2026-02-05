#pragma once
#include <vector>
#include <array>
#include <cstdint>

namespace lmcore
{
    enum class EKey : uint32_t
    {
        ESC,

        W,
        S,
        D,
        A,
        E,
        Q,

        MouseRight,

        EnumMax,
    };

    enum class EKeyState : uint8_t
    {
        Press,
        Repeat,
        EnumMax
    };

    inline constexpr uint32_t key_size()
    {
        return static_cast<uint32_t>(EKey::EnumMax);
    }

    inline uint32_t key_uint(EKey key)
    {
        return static_cast<uint32_t>(key);
    }

    inline EKey uint_key(uint32_t num)
    {
        return static_cast<EKey>(num);
    }

    inline uint8_t state_bit(EKeyState state)
    {
        uint8_t s = 1;
        return (s << uint32_t(state));
    }

    struct KeyState
    {
        uint8_t state = 0;

        bool is_set(EKeyState s)
        {
            return (state_bit(s));
        }

        void set(EKeyState s)
        {
            state = state | state_bit(s);
        }
    };

    struct KeyStates
    {
        std::array<KeyState, key_size()> states;
        void clear()
        {
            states = std::array<KeyState, key_size()>();
        }

        void swap(KeyStates &other)
        {
            other.states.swap(states);
        }

        void set(const EKey &k, const KeyState &s)
        {
            auto idx = key_uint(k);
            states[idx] = s;
        }

        KeyState get(const EKey &k) const
        {
            auto idx = key_uint(k);
            return states[idx];
        }
    };

    struct MouseStates
    {
        double lastMouseX = 0.0;
        double lastMouseY = 0.0;

        void clear()
        {
            lastMouseX = 0.0;
            lastMouseY = 0.0;
        }

        void frame()
        {
            
        }
    };

    struct Control
    {
        const float moveSpeed = 5.0f;
        const float mouseSensitivity = 0.005f;

        MouseStates current_mouse_states;
        MouseStates previous_mouse_states;

        KeyStates current_key_states;
        KeyStates previous_key_states;
    };
}