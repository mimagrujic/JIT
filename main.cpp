#include "include/jit-tools/jit-tools.h"

int main(){

    string commandLine;
    string pathToJitRepo;
    cout << "\nWelcome to Jit!\nSimilar to Git, yet a bit different!\n\nFor help, type in \"help\"\n\n";
    cout << "jit > ";
    while (getline(cin, commandLine)) {
        if (commandLine == "exit" || commandLine == "quit") {
            return 0;
        }
        if (commandLine.empty()) {
            cout << "jit > ";
            continue;
        }
        stringstream ss(commandLine);
        vector<string> commandArgs;
        string token;
        while (getline(ss, token, ' ')) {
            commandArgs.push_back(token);
        }

        commandHandling(commandArgs, pathToJitRepo);

        cout << "jit > ";
    }

    return 0;
}