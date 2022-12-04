#ifndef __hemlock_ui_input_keys_hpp
#define __hemlock_ui_input_keys_hpp

namespace hemlock {
    namespace ui {
        enum class PhysicalKey {
#define PUT(X)        H_##X = SDL_SCANCODE_##X,
#define PUTASYM(X, Y) H_##X = SDL_SCANCODE_##Y,
            PUT(0)
            PUT(1) PUT(2) PUT(3) PUT(4) PUT(5) PUT(6) PUT(7) PUT(8) PUT(9) PUT(A) PUT(B
            ) PUT(C) PUT(D) PUT(E) PUT(F) PUT(G) PUT(H) PUT(I) PUT(J) PUT(K) PUT(L
            ) PUT(M) PUT(N) PUT(O) PUT(P) PUT(Q) PUT(R) PUT(S) PUT(T) PUT(U) PUT(V
            ) PUT(W) PUT(X) PUT(Y) PUT(Z) PUT(F1) PUT(F2) PUT(F3) PUT(F4) PUT(F5) PUT(F6
            ) PUT(F7) PUT(F8) PUT(F9) PUT(F10) PUT(F11) PUT(F12) PUT(F13) PUT(F14
            ) PUT(F15) PUT(F16) PUT(F17) PUT(F18
            ) PUT(F19) PUT(F20) PUT(F21) PUT(F22
            ) PUT(F23) PUT(F24) PUT(AC_BACK) PUT(AC_BOOKMARKS) PUT(AC_FORWARD
            ) PUT(AC_HOME) PUT(AC_REFRESH) PUT(AC_SEARCH
            ) PUT(AC_STOP) PUT(AGAIN
            ) PUT(ALTERASE) PUT(APOSTROPHE) PUT(APPLICATION) PUT(AUDIOMUTE
            ) PUT(AUDIONEXT) PUT(AUDIOPLAY) PUT(AUDIOPREV) PUT(AUDIOSTOP
            ) PUT(BACKSLASH) PUT(BACKSPACE) PUT(BRIGHTNESSDOWN
            ) PUT(BRIGHTNESSUP) PUT(CALCULATOR) PUT(CANCEL) PUT(CAPSLOCK
            ) PUT(CLEAR) PUT(CLEARAGAIN) PUT(COMMA) PUT(COMPUTER
            ) PUT(COPY) PUT(CRSEL) PUT(CURRENCYSUBUNIT) PUT(CURRENCYUNIT) PUT(CUT
            ) PUT(DECIMALSEPARATOR) PUT(DELETE) PUT(DISPLAYSWITCH) PUT(DOWN) PUT(EJECT
            ) PUT(END) PUT(EQUALS) PUT(ESCAPE) PUT(EXECUTE) PUT(EXSEL) PUT(FIND
            ) PUT(GRAVE) PUT(HELP) PUT(HOME) PUT(INSERT
            ) PUT(KBDILLUMDOWN) PUT(KBDILLUMTOGGLE) PUT(KBDILLUMUP) PUT(KP_0
            ) PUT(KP_00) PUT(KP_000) PUT(KP_1) PUT(KP_2) PUT(KP_3) PUT(KP_4) PUT(KP_5
            ) PUT(KP_6) PUT(KP_7
            ) PUT(KP_8) PUT(KP_9
            ) PUT(KP_A) PUT(KP_AMPERSAND
            ) PUT(KP_AT) PUT(KP_B) PUT(KP_BACKSPACE) PUT(KP_BINARY) PUT(KP_C
            ) PUT(KP_CLEAR) PUT(KP_CLEARENTRY) PUT(KP_COLON) PUT(KP_COMMA
            ) PUT(KP_D) PUT(KP_DBLAMPERSAND) PUT(KP_DBLVERTICALBAR) PUT(KP_DECIMAL
            ) PUT(KP_DIVIDE) PUT(KP_E) PUT(KP_ENTER) PUT(KP_EQUALS) PUT(KP_EQUALSAS400
            ) PUT(KP_EXCLAM) PUT(KP_F) PUT(KP_GREATER) PUT(KP_HASH) PUT(KP_HEXADECIMAL
            ) PUT(KP_LEFTBRACE) PUT(KP_LEFTPAREN
            ) PUT(KP_LESS) PUT(KP_MEMADD
            ) PUT(KP_MEMDIVIDE) PUT(KP_MEMMULTIPLY) PUT(KP_MEMRECALL
            ) PUT(KP_MEMSTORE) PUT(KP_MEMSUBTRACT) PUT(KP_MINUS) PUT(KP_MULTIPLY
            ) PUT(KP_OCTAL) PUT(KP_PERCENT) PUT(KP_PERIOD) PUT(KP_PLUS) PUT(KP_PLUSMINUS
            ) PUT(KP_POWER) PUT(KP_RIGHTBRACE) PUT(KP_RIGHTPAREN
            ) PUT(KP_SPACE) PUT(KP_TAB) PUT(KP_VERTICALBAR) PUT(KP_XOR) PUT(LALT
            ) PUT(LCTRL) PUT(LEFT) PUT(LEFTBRACKET) PUTASYM(LCMD, LGUI) PUT(LSHIFT
            ) PUT(MAIL) PUT(MEDIASELECT) PUT(MENU
            ) PUT(MINUS) PUT(MODE) PUT(MUTE) PUT(NUMLOCKCLEAR) PUT(OPER) PUT(OUT
            ) PUT(PAGEDOWN) PUT(PAGEUP) PUT(PASTE
            ) PUT(PAUSE) PUT(PERIOD) PUT(POWER) PUT(PRINTSCREEN) PUT(PRIOR)
                PUT(RALT) PUT(RCTRL) PUT(RETURN) PUT(RETURN2) PUTASYM(RCMD, RGUI)
                    PUT(RIGHT) PUT(RIGHTBRACKET) PUT(RSHIFT) PUT(SCROLLLOCK) PUT(SELECT)
                        PUT(SEMICOLON) PUT(SEPARATOR) PUT(SLASH) PUT(SLEEP)
                            PUT(SPACE) PUT(STOP) PUT(SYSREQ)
                                PUT(TAB) PUT(THOUSANDSSEPARATOR) PUT(UNDO)
                                    PUT(UNKNOWN) PUT(UP) PUT(VOLUMEDOWN) PUT(VOLUMEUP)
                                        PUT(WWW) PUT(INTERNATIONAL1) PUT(INTERNATIONAL2)
                                            PUT(INTERNATIONAL3) PUT(INTERNATIONAL4)
                                                PUT(INTERNATIONAL5) PUT(INTERNATIONAL6
                                                ) PUT(INTERNATIONAL7) PUT(INTERNATIONAL8
                                                ) PUT(INTERNATIONAL9) PUT(LANG1)
                                                    PUT(LANG2) PUT(LANG3) PUT(LANG4)
                                                        PUT(LANG5) PUT(LANG6) PUT(LANG7)
                                                            PUT(LANG8) PUT(LANG9)
                                                                PUT(NONUSBACKSLASH)
                                                                    PUT(NONUSHASH)
#undef PUTASYM
#undef PUT
        };

        enum class VirtualKey {
#define PUT(X)        H_##X = SDLK_##X,
#define PUTASYM(X, Y) H_##X = SDLK_##Y,
            PUT(0)
            PUT(1) PUT(2) PUT(3) PUT(4) PUT(5) PUT(6) PUT(7) PUT(8) PUT(9
            ) PUTASYM(A, a) PUTASYM(B, b) PUTASYM(C, c) PUTASYM(D, d) PUTASYM(E, e)
                PUTASYM(F, f) PUTASYM(G, g) PUTASYM(H, h) PUTASYM(I, i) PUTASYM(
                    J, j
                ) PUTASYM(K, k) PUTASYM(L, l) PUTASYM(M, m) PUTASYM(N, n) PUTASYM(O, o)
                    PUTASYM(P, p) PUTASYM(Q, q) PUTASYM(R, r) PUTASYM(S, s) PUTASYM(
                        T, t
                    ) PUTASYM(U, u) PUTASYM(V, v) PUTASYM(W, w) PUTASYM(X, x)
                        PUTASYM(Y, y) PUTASYM(Z, z) PUT(F1) PUT(F2) PUT(F3) PUT(F4
                        ) PUT(F5) PUT(F6) PUT(F7) PUT(F8) PUT(F9) PUT(F10) PUT(F11
                        ) PUT(F12) PUT(F13) PUT(F14) PUT(F15) PUT(F16) PUT(F17) PUT(F18
                        ) PUT(F19) PUT(F20) PUT(F21) PUT(F22) PUT(F23) PUT(F24
                        ) PUT(AC_BACK) PUT(AC_BOOKMARKS) PUT(AC_FORWARD) PUT(AC_HOME
                        ) PUT(AC_REFRESH) PUT(AC_SEARCH) PUT(AC_STOP) PUT(AGAIN
                        ) PUT(ALTERASE) PUTASYM(APOSTROPHE, QUOTE) PUT(APPLICATION
                        ) PUT(AUDIOMUTE) PUT(AUDIONEXT) PUT(AUDIOPLAY) PUT(AUDIOPREV
                        ) PUT(AUDIOSTOP) PUT(BACKSLASH) PUT(BACKSPACE
                        ) PUT(BRIGHTNESSDOWN
                        ) PUT(BRIGHTNESSUP) PUT(CALCULATOR) PUT(CANCEL) PUT(CAPSLOCK
                        ) PUT(CLEAR) PUT(CLEARAGAIN) PUT(COMMA) PUT(COMPUTER
                        ) PUT(COPY) PUT(CRSEL) PUT(CURRENCYSUBUNIT) PUT(CURRENCYUNIT
                        ) PUT(CUT) PUT(DECIMALSEPARATOR) PUT(DELETE) PUT(DISPLAYSWITCH
                        ) PUT(DOWN) PUT(EJECT) PUT(END) PUT(EQUALS) PUT(ESCAPE
                        ) PUT(EXECUTE
                        ) PUT(EXSEL) PUT(FIND) PUTASYM(GRAVE, BACKQUOTE) PUT(HELP
                        ) PUT(HOME) PUT(INSERT
                        ) PUT(KBDILLUMDOWN) PUT(KBDILLUMTOGGLE) PUT(KBDILLUMUP) PUT(KP_0
                        ) PUT(KP_00) PUT(KP_000) PUT(KP_1) PUT(KP_2) PUT(KP_3) PUT(KP_4
                        ) PUT(KP_5) PUT(KP_6
                        ) PUT(KP_7) PUT(KP_8
                        ) PUT(KP_9) PUT(KP_A
                        ) PUT(KP_AMPERSAND) PUT(KP_AT) PUT(KP_B) PUT(KP_BACKSPACE
                        ) PUT(KP_BINARY) PUT(KP_C) PUT(KP_CLEAR
                        ) PUT(KP_CLEARENTRY) PUT(KP_COLON) PUT(KP_COMMA) PUT(KP_D
                        ) PUT(KP_DBLAMPERSAND) PUT(KP_DBLVERTICALBAR) PUT(KP_DECIMAL
                        ) PUT(KP_DIVIDE) PUT(KP_E) PUT(KP_ENTER
                        ) PUT(KP_EQUALS) PUT(KP_EQUALSAS400) PUT(KP_EXCLAM
                        ) PUT(KP_F) PUT(KP_GREATER) PUT(KP_HASH) PUT(KP_HEXADECIMAL
                        ) PUT(KP_LEFTBRACE) PUT(KP_LEFTPAREN
                        ) PUT(KP_LESS) PUT(KP_MEMADD
                        ) PUT(KP_MEMDIVIDE) PUT(KP_MEMMULTIPLY) PUT(KP_MEMRECALL
                        ) PUT(KP_MEMSTORE) PUT(KP_MEMSUBTRACT) PUT(KP_MINUS
                        ) PUT(KP_MULTIPLY) PUT(KP_OCTAL) PUT(KP_PERCENT) PUT(KP_PERIOD
                        ) PUT(KP_PLUS) PUT(KP_PLUSMINUS) PUT(KP_POWER) PUT(KP_RIGHTBRACE
                        ) PUT(KP_RIGHTPAREN
                        ) PUT(KP_SPACE) PUT(KP_TAB) PUT(KP_VERTICALBAR) PUT(KP_XOR
                        ) PUT(LALT) PUT(LCTRL) PUT(LEFT
                        ) PUT(LEFTBRACKET) PUTASYM(LCMD, LGUI) PUT(LSHIFT) PUT(MAIL
                        ) PUT(MEDIASELECT) PUT(MENU
                        ) PUT(MINUS) PUT(MODE) PUT(MUTE) PUT(NUMLOCKCLEAR) PUT(OPER
                        ) PUT(OUT) PUT(PAGEDOWN) PUT(PAGEUP) PUT(PASTE
                        ) PUT(PAUSE) PUT(PERIOD) PUT(POWER) PUT(PRINTSCREEN) PUT(PRIOR
                        ) PUT(RALT) PUT(RCTRL) PUT(RETURN
                        ) PUT(RETURN2) PUTASYM(RCMD, RGUI) PUT(RIGHT
                        ) PUT(RIGHTBRACKET) PUT(RSHIFT) PUT(SCROLLLOCK) PUT(SELECT
                        ) PUT(SEMICOLON) PUT(SEPARATOR) PUT(SLASH) PUT(SLEEP
                        ) PUT(SPACE) PUT(STOP) PUT(SYSREQ
                        ) PUT(TAB) PUT(THOUSANDSSEPARATOR) PUT(UNDO
                        ) PUT(UNKNOWN) PUT(UP) PUT(VOLUMEDOWN) PUT(VOLUMEUP) PUT(WWW)
                            PUTASYM(INTERNATIONAL1, UNKNOWN) PUTASYM(
                                INTERNATIONAL2,
                                UNKNOWN
                            ) PUTASYM(INTERNATIONAL3, UNKNOWN)
                                PUTASYM(INTERNATIONAL4, UNKNOWN) PUTASYM(
                                    INTERNATIONAL5,
                                    UNKNOWN
                                ) PUTASYM(INTERNATIONAL6, UNKNOWN)
                                    PUTASYM(INTERNATIONAL7, UNKNOWN)
                                        PUTASYM(INTERNATIONAL8, UNKNOWN)
                                            PUTASYM(INTERNATIONAL9, UNKNOWN) PUTASYM(
                                                LANG1, UNKNOWN
                                            ) PUTASYM(LANG2, UNKNOWN)
                                                PUTASYM(LANG3, UNKNOWN)
                                                    PUTASYM(LANG4, UNKNOWN)
                                                        PUTASYM(LANG5, UNKNOWN) PUTASYM(
                                                            LANG6, UNKNOWN
                                                        ) PUTASYM(LANG7, UNKNOWN)
                                                            PUTASYM(LANG8, UNKNOWN)
                                                                PUTASYM(LANG9, UNKNOWN)
                                                                    PUTASYM(
                                                                        NONUSBACKSLASH,
                                                                        UNKNOWN
                                                                    )
                                                                        PUTASYM(
                                                                            NONUSHASH,
                                                                            UNKNOWN
                                                                        )
#undef PUTASYM
#undef PUT
        };
    }  // namespace ui
}  // namespace hemlock
namespace hui = hemlock::ui;

#endif  // __hemlock_ui_input_keys_hpp
