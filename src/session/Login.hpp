#pragma once

#include <string>

struct PassReturn {
    bool success = false;
    std::string pass;
};

void ChangePassword(const std::string& username, const std::string& password);
PassReturn GetCorrectPass(const std::string& userName);
bool LoginRoot();
int Login();
