#include "Login.hpp"

#include <array>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <span>
#include <sstream>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include "../fs/FileTree.hpp"
#include "../fs/Node.hpp"
#include "../session/SessionData.hpp"
#include "../ui/Messages.hpp"
#include "../ui/Syntax.hpp"
#include "../util/Misc.hpp"

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace {

    constexpr size_t passwordHashRounds = 100'000;

    uint32_t rotateRight(const uint32_t value, const uint32_t amount) {
        return (value >> amount) | (value << (32 - amount));
    }

    std::array<uint8_t, 32> sha256(const std::string_view input) {
        static constexpr std::array<uint32_t, 64> constants = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
        };

        std::vector<uint8_t> message(input.begin(), input.end());
        const uint64_t bitLength = message.size() * 8;
        message.push_back(0x80);
        while ((message.size() % 64) != 56) message.push_back(0);
        for (int shift = 56; shift >= 0; shift -= 8) message.push_back(static_cast<uint8_t>(bitLength >> shift));

        uint32_t hash[] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
        for (size_t block = 0; block < message.size(); block += 64) {
            uint32_t words[64]{};
            for (size_t i = 0; i < 16; ++i) {
                words[i] = (static_cast<uint32_t>(message[block + i * 4]) << 24) | (static_cast<uint32_t>(message[block + i * 4 + 1]) << 16) |
                           (static_cast<uint32_t>(message[block + i * 4 + 2]) << 8) | message[block + i * 4 + 3];
            }
            for (size_t i = 16; i < 64; ++i) {
                const uint32_t s0 = rotateRight(words[i - 15], 7) ^ rotateRight(words[i - 15], 18) ^ (words[i - 15] >> 3);
                const uint32_t s1 = rotateRight(words[i - 2], 17) ^ rotateRight(words[i - 2], 19) ^ (words[i - 2] >> 10);
                words[i] = words[i - 16] + s0 + words[i - 7] + s1;
            }
            uint32_t a = hash[0], b = hash[1], c = hash[2], d = hash[3], e = hash[4], f = hash[5], g = hash[6], h = hash[7];
            for (size_t i = 0; i < 64; ++i) {
                const uint32_t s1 = rotateRight(e, 6) ^ rotateRight(e, 11) ^ rotateRight(e, 25);
                const uint32_t choice = (e & f) ^ (~e & g);
                const uint32_t temp1 = h + s1 + choice + constants[i] + words[i];
                const uint32_t s0 = rotateRight(a, 2) ^ rotateRight(a, 13) ^ rotateRight(a, 22);
                const uint32_t majority = (a & b) ^ (a & c) ^ (b & c);
                const uint32_t temp2 = s0 + majority;
                h = g; g = f; f = e; e = d + temp1; d = c; c = b; b = a; a = temp1 + temp2;
            }
            hash[0] += a; hash[1] += b; hash[2] += c; hash[3] += d; hash[4] += e; hash[5] += f; hash[6] += g; hash[7] += h;
        }

        std::array<uint8_t, 32> digest{};
        for (size_t i = 0; i < digest.size(); ++i) digest[i] = static_cast<uint8_t>(hash[i / 4] >> (24 - (i % 4) * 8));
        return digest;
    }

    std::string hexEncode(const std::span<const uint8_t> bytes) {
        std::ostringstream stream;
        for (const uint8_t byte : bytes) stream << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(byte);
        return stream.str();
    }

    std::string hashPassword(const std::string_view password, const std::string_view salt) {
        std::string digest = std::string(salt) + ':' + std::string(password);
        for (size_t i = 0; i < passwordHashRounds; ++i) {
            const auto bytes = sha256(digest);
            digest.assign(reinterpret_cast<const char*>(bytes.data()), bytes.size());
        }
        return "sha256$" + std::string(salt) + '$' + hexEncode(std::span(reinterpret_cast<const uint8_t*>(digest.data()), digest.size()));
    }

    std::string createPasswordHash(const std::string_view password) {
        std::array<uint8_t, 16> salt{};
        std::random_device random;
        for (uint8_t& byte : salt) byte = static_cast<uint8_t>(random());
        return hashPassword(password, hexEncode(salt));
    }

    bool secureEquals(const std::string_view first, const std::string_view second) {
        if (first.size() != second.size()) return false;
        uint8_t difference = 0;
        for (size_t i = 0; i < first.size(); ++i) difference |= static_cast<uint8_t>(first[i] ^ second[i]);
        return difference == 0;
    }

    bool isHashedPassword(const std::string_view password) {
        constexpr std::string_view prefix = "sha256$";
        return password.size() == prefix.size() + 32 + 1 + 64 && password.starts_with(prefix) && password[prefix.size() + 32] == '$';
    }

    bool verifyPassword(const std::string_view password, const std::string_view storedPassword) {
        constexpr std::string_view prefix = "sha256$";
        if (!isHashedPassword(storedPassword)) return false;
        const size_t saltEnd = storedPassword.find('$', prefix.size());
        return secureEquals(hashPassword(password, storedPassword.substr(prefix.size(), saltEnd - prefix.size())), storedPassword);
    }

}

static std::string getPassword() {
#ifdef _WIN32
    std::string pass;
    char ch;
    while ((ch = static_cast<char>(_getch())) != '\r') {
        if (ch == '\b') {
            if (!pass.empty()) pass.pop_back();
        } else {
            pass += ch;
        }
    }
    std::cout << '\n';
#else
    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::string pass;
    std::getline(std::cin, pass);

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << '\n';
#endif

    return pass;
}

void ChangePassword(const std::string& username, const std::string& password) {
    Node* etc = FS::root->GetChild("etc", "dir");
    if (!etc) {
        etc = FS::root->NewChild("etc", "dir");
    }

    Node* loginPass = etc->GetChild("login", "txt");
    if (!loginPass) {
        loginPass = etc->NewChild("login", "txt");
    }

    const std::vector<std::string> users = split(loginPass->_value, '\n');
    std::string output;
    const std::string passwordHash = createPasswordHash(password);
    output.reserve(loginPass->_value.size() + passwordHash.size() + 32);

    bool changed = false;
    for (const std::string& user : users) {
        size_t index = user.find('\0');
        if (index == std::string::npos) {
            index = user.size();
        }
        if (const std::string_view name(user.data(), index);
            name == username) {
            output += username;
            output += '\0';
            output += passwordHash;
            output += '\n';
            changed = true;
            continue;
        }

        output += user;
        output += '\n';
    }

    if (!changed) {
        output += username;
        output += '\0';
        output += passwordHash;
        output += '\n';
    }

    loginPass->_value = std::move(output);
}

PassReturn GetCorrectPass(const std::string& userName) {
    PassReturn passReturn;

    Node* etc = FS::root->GetChild("etc", "dir");
    if (!etc) {
        FS::root->NewChild("etc", "dir");
        return passReturn;
    }

    const Node* loginPass = etc->GetChild("login", "txt");
    if (!loginPass) {
        etc->NewChild("login", "txt");
        return passReturn;
    }

    for (const std::string& user : split(loginPass->_value, '\n')) {
        if (const size_t index = user.find('\0');
            index != std::string::npos && index + 1 < user.size() && user.compare(0, index, userName) == 0) {
            passReturn.success = true;
            passReturn.pass = user.substr(index + 1);
            break;
        }
    }

    return passReturn;
}

bool LoginRoot() {
    auto [success, pass] = GetCorrectPass("//root//");
    if (!success) {
        while (true) {
            alert("Please enter a new root password: ", stx::yellow);
            const std::string newPass = getPassword();
            alert("Please confirm the new root password: ", stx::yellow);
            if (newPass == getPassword() && !newPass.empty()) {
                ChangePassword("//root//", newPass);
                SData::user.root = true;
                break;
            }
            alert(msg::pass_set_fail, stx::yellow);
        }
        return true;
    }

    for (size_t i = 0; i < 3; ++i) {
        alert("Password for root: ", stx::yellow);

        const std::string enteredPassword = getPassword();
        if (verifyPassword(enteredPassword, pass)) {
            SData::user.root = true;
            return true;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return false;
}

int Login() {
    Node* home = FS::root->GetChild("home", "dir");
    if (!home) {
        home = FS::root->NewChild("home", "dir");
    }

    int dirAmount = 0;
    for (const std::unique_ptr<Node>& child : home->_children) {
        if (child->_type == "dir") {
            ++dirAmount;
        }
    }
    if (dirAmount == 0) {
        std::string name;

        while (true) {
            alert("No user found. Enter new username: ", stx::yellow);
            std::getline(std::cin, name);

            if (name.empty() || name.contains('.') || name.contains('/')) {
                alert(msg::invalid_username, stx::red);
            } else break;
        }

        home->NewChild(name, "dir");
        ++dirAmount;
    }

    std::string userName;
    if (dirAmount == 1) {
        for (const std::unique_ptr<Node>& child : home->_children) {
            if (child->_type == "dir") {
                userName = child->_name;
            }
        }
    } else {
        std::string temp;
        while (true) {
            stx::ClearConsole();
            std::cout << "Please enter user name (";
            int currentDir = 0;
            for (const std::unique_ptr<Node>& child : home->_children) {
                if (child->_type == "dir") {
                    std::cout << child->_name;
                    ++currentDir;
                    if (currentDir != dirAmount) {
                        std::cout << " / ";
                    }
                }
            }
            std::cout << "): ";
            std::getline(std::cin, temp);

            if (home->GetChild(temp, "dir")) {
                userName = temp;
                break;
            }
        }
    }

    auto [success, pass] = GetCorrectPass(userName);
    if (!success) {
        while (true) {
            alert("Please enter a new " + userName + " password: ", stx::yellow);
            const std::string newPass = getPassword();
            alert("Please confirm the new " + userName + " password: ", stx::yellow);
            if (newPass == getPassword() && !newPass.empty()) {
                ChangePassword(userName, newPass);
                SData::user.name = userName;
                FS::current = FS::root->GetAbsolute("/home/" + userName);
                return 0;
            }
            alert(msg::pass_set_fail, stx::yellow);
        }
    }

    for (size_t i = 0; i < 3; ++i) {
        std::cout << "Password for " << userName << ": ";
        const std::string enteredPassword = getPassword();
        if (verifyPassword(enteredPassword, pass)) {
            SData::user.name = userName;
            FS::current = FS::root->GetAbsolute("/home/" + userName);
            return 0;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return -1;
}
