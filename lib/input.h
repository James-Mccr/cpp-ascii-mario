#pragma once

enum class UserInput { None, Left, Right, Up, Down, Quit };

class Input
{
public:
    UserInput Read();
};