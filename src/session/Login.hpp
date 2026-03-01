#pragma once
#include "../ui/Messages.hpp"

struct PassReturn {
    bool success = false;
    std::string pass;
};

inline bool changePassword(const std::string& username, const std::string& password) {
    const Node* etc = getChild(FS::root, "etc", "dir");
    if (!etc)
        return false;
    Node* loginPass = getChild(etc, "login", "txt");
    if (!loginPass)
        return false;

    const std::vector<std::string> users = split(loginPass->value, '\n');
    std::string output;
    output.reserve(loginPass->value.size() + password.size() + 32);

    bool changed = false;
    for (const std::string& user : users) {
        size_t index = user.find(' ');
        if (index == std::string::npos)
            index = user.size();
        if (const std::string_view name(user.data(), index);
            name == username) {
            output += username;
            output += " ";
            output += password;
            output += "\n";
            changed = true;
            continue;
        }

        output += user;
        output += "\n";
    }

    if (!changed) {
        output += username;
        output += " ";
        output += password;
        output += "\n";
    }

    loginPass->value = std::move(output);
    return true;
}

inline PassReturn getCorrectPass(const std::string& userName) {
    PassReturn passReturn;

    const Node* etc = getChild(FS::root, "etc", "dir");
    if (!etc)
        return passReturn;
    const Node* loginPass = getChild(etc, "login", "txt");
    if (!loginPass)
        return passReturn;
    for (const std::string& user : split(loginPass->value, '\n')) {
        if (const size_t index = user.find(' ');
            index != std::string::npos && index + 1 < user.size() && user.compare(0, index, userName) == 0) {
            passReturn.success = true;
            passReturn.pass = user.substr(index + 1);
            break;
        }
    }

    return passReturn;
}

inline bool LoginRoot() {
    std::string enteredPass;
    auto [success, pass] = getCorrectPass("..root..");
    if (!success) {
        std::string newPass;
        std::string confirmPass;
        while (true) {
            std::cout << "Please enter a new root password: ";
            std::getline(std::cin, newPass);
            std::cout << "Please confirm the new root password: ";
            std::getline(std::cin, confirmPass);
            if (newPass == confirmPass && !newPass.empty()) {
                changePassword("..root..", newPass);
                SData::root = true;
                break;
            }
            alert(msg::pass_set_fail, stx::yellow);
        }
        return true;
    }

    for (auto i{0uz}; i < 3; ++i) {
        std::cout << "Password for root: ";
        std::getline(std::cin, enteredPass);

        if (enteredPass == pass) {
            SData::root = true;
            return true;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return false;
}

inline int Login() {
    const Node* home = getChild(FS::root, "home", "dir");
    if (!home)
        return 1;

    int dirAmount = 0;
    for (const std::unique_ptr<Node>& child : home->children)
        if (child->type == "dir")
            ++dirAmount;
    if (dirAmount == 0)
        return 1;

    std::string userName;
    if (dirAmount == 1)
        for (const std::unique_ptr<Node>& child : home->children) {
            if (child->type == "dir")
                userName = child->name;
        }
    else {
        std::string temp;
        while (true) {
            std::cout << "Please enter user name (";
            int currentDir = 0;
            for (const std::unique_ptr<Node>& child : home->children) {
                if (child->type == "dir") {
                    std::cout << child->name;
                    ++currentDir;
                    if (currentDir != dirAmount)
                        std::cout << " / ";
                }
            }
            std::cout << "): ";
            std::getline(std::cin, temp);

            if (getChild(home, temp, "dir")) {
                userName = temp;
                break;
            }
        }
    }

    std::string enteredPass;
    auto [success, pass] = getCorrectPass(userName);
    if (!success) {
        std::string newPass;
        std::string confirmPass;
        while (true) {
            std::cout << "Please enter a new " << userName << " password: ";
            std::getline(std::cin, newPass);
            std::cout << "Please confirm the new " << userName << " password: ";
            std::getline(std::cin, confirmPass);
            if (newPass == confirmPass && !newPass.empty()) {
                changePassword(userName, newPass);
                SData::username = userName;
                FS::current = getAbsolute("/home/" + userName);
                return 0;
            }
            alert(msg::pass_set_fail, stx::yellow);
        }
    }

    for (auto i{0uz}; i < 3; ++i) {
        std::cout << "Password for " << userName << ": ";
        std::getline(std::cin, enteredPass);

        if (enteredPass == pass) {
            SData::username = userName;
            FS::current = getAbsolute("/home/" + userName);
            return 0;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return -1;
}