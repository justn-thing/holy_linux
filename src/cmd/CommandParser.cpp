#include "CommandParser.hpp"

#include "../fs/FileTree.hpp"
#include "../session/Login.hpp"
#include "../session/SessionData.hpp"

std::vector<std::string> TokenizeCommandLine(const std::string& input) {
    std::vector<std::string> tokens;

    std::string temp;
    bool openQuotes = false;
    char quoteChar {};

    for (const char& c : input) {
        if (openQuotes && c == quoteChar) {
            openQuotes = false;
        } else if (!openQuotes && (c == '"' || c == '\'')) {
            openQuotes = true;
            quoteChar = c;
        } else if (!openQuotes && c == ' ') {
            if (!temp.empty()) {
                tokens.emplace_back(temp);
                temp.clear();
            }
        } else {
            temp += c;
        }
    }

    if (!temp.empty()) tokens.emplace_back(temp);
    return tokens;
}

void ParseToken(CommandParams& result, std::string& token, const bool first, bool& literalMode, bool& redirected) {
    if (first && token == "sudo") {
        if (SData::user.root || LoginRoot()) {
            result.sudo = true;
        }
    } else if (result.cmd.empty()) {
        result.cmd = token;
    } else if (!literalMode && token == "--") {
        literalMode = true;
    } else if (!literalMode && token.starts_with("--")) {
        result.longFlags.emplace_back(token.substr(2));
    } else if (!literalMode && token[0] == '-') {
        result.shortFlags += token.substr(1);
    } else if (!redirected && token == "&&") {
        redirected = true;
        result.redirectMode = RedirectMode::And;
    } else if (!redirected && token == "||") {
        redirected = true;
        result.redirectMode = RedirectMode::Or;
    } else if (!redirected && token == ">") {
        redirected = true;
        result.redirectMode = RedirectMode::Overwrite;
    } else if (!redirected && token == ">>") {
        redirected = true;
        result.redirectMode = RedirectMode::Append;
    } else {
        result.args.emplace_back(std::move(token));
    }
}

CommandParams ParseCommandTokens(std::vector<std::string>& tokens) {
    CommandParams result;

    bool first = true;
    bool literalMode = false;
    bool redirected = false;
    std::vector<std::string> redirectCmdTokens;

    for (std::string& token : tokens) {
        if (redirected) {
            if (result.redirectMode == RedirectMode::Overwrite || result.redirectMode == RedirectMode::Append) {
                result.redirectTarget = GetAbsolute(token);
                break;
            }

            redirectCmdTokens.emplace_back(token);

            continue;
        }

        ParseToken(result, token, first, literalMode, redirected);

        first = false;
    }

    if (!redirectCmdTokens.empty()) {
        result.redirectCmd = std::make_unique<CommandParams>(ParseCommandTokens(redirectCmdTokens));
    }

    return result;
}

CommandParams ParseCommandLine(const std::string& input) {
    std::vector<std::string> tokens = TokenizeCommandLine(input);

    CommandParams result = std::move(ParseCommandTokens(tokens));

    return result;
}
