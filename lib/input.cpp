#include "input.h"
#include <ncurses.h>

UserInput Input::Read()
{
    int i = getch();
    switch (i)
    {
        case 'q': return UserInput::Quit;
        case KEY_LEFT: return UserInput::Left;
        case KEY_RIGHT: return UserInput::Right;
        case KEY_UP: return UserInput::Up;
        case KEY_DOWN: return UserInput::Down;
        default: return UserInput::None;
    }
}