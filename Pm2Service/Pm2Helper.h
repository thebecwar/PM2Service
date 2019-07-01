#pragma once

#ifndef PM2HELPER_H
#define PM2HELPER_H

#include <string>

void FindPm2Path(std::wstring& path);
void BuildPm2Command(std::wstring& args, std::wstring& target);


#endif
