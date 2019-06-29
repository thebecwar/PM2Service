#pragma once

#ifndef COMMANDS_H
#define COMMANDS_H

typedef int (*Command)(int, const wchar_t**);

typedef struct
{
    const wchar_t* command;
    int minArgs;
    const wchar_t* shortHelp;
    const wchar_t* longHelp;
    Command cmd;
} CommandInfo;

extern CommandInfo AllCommands[];

int RunCommand(int argc, const wchar_t** argv);

#endif
