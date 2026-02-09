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
        Release,
        EnumMax
    };

    enum class EKeyTemporalState : uint8_t
    {
        Up,
        Down,
        Press,
        Release,
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

    inline uint8_t t_state_bit(EKeyTemporalState state)
    {
        uint8_t s = 1;
        return (s << uint32_t(state));
    }

    struct KeyState
    {
        uint8_t state = 0;

        bool is_set(EKeyState s)
        {
            return (state_bit(s) & state);
        }

        void set(EKeyState s)
        {
            state = state | state_bit(s);
        }
    };

    struct KeyTemporalState
    {
        uint8_t state = 0;

        bool is_set(EKeyTemporalState s)
        {
            return (t_state_bit(s) & state);
        }

        void set(EKeyTemporalState s)
        {
            state = state | t_state_bit(s);
        }

        void unset(EKeyTemporalState s)
        {
            state = state & ~t_state_bit(s);
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

    struct KeyTemporalStates
    {
        std::array<KeyTemporalState, key_size()> tstates;

        void clear()
        {
            tstates = std::array<KeyTemporalState, key_size()>();
        }

        void deduct(const KeyStates &states)
        {
            for (auto i = 0; i < key_size(); i++)
            {
                auto k = states.states[i];
                auto t = tstates[i];

                tstates[i].unset(EKeyTemporalState::Press);
                tstates[i].unset(EKeyTemporalState::Release);

                if (k.is_set(EKeyState::Press))
                {
                    tstates[i].set(EKeyTemporalState::Press);
                    tstates[i].set(EKeyTemporalState::Down);
                    tstates[i].unset(EKeyTemporalState::Up);
                }
                else if (k.is_set(EKeyState::Release))
                {
                    tstates[i].set(EKeyTemporalState::Release);
                    tstates[i].set(EKeyTemporalState::Up);
                    tstates[i].unset(EKeyTemporalState::Down);
                }
            }
        }

        void set(const EKey &k, const KeyTemporalState &s)
        {
            auto idx = key_uint(k);
            tstates[idx] = s;
        }

        KeyTemporalState get(const EKey &k) const
        {
            auto idx = key_uint(k);
            return tstates[idx];
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

        KeyTemporalStates temporal_deduct_key_states;

        void PreFrameUpdate()
        {
            temporal_deduct_key_states.deduct(current_key_states);
        }

        void PostFrameUpdate()
        {
            current_key_states.swap(previous_key_states);
            current_key_states.clear();
            previous_mouse_states = current_mouse_states;
            current_mouse_states.frame();
        }
    };
}