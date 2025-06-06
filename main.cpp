#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

void showHelpDocumentation();
void commandHandling(const vector<string> &vector);
void initializeRepository(const vector<string> &commandArgs);
void stageAllChanges(const vector<string> &vector);

int main(){

    string commandLine;
    cout << "\nWelcome to Jit!\nSimilar to Git, yet a bit different!\n\nFor help, type in \"help\"\n\n";
    cout << "jit > ";
    while (getline(cin, commandLine)) {
        if (commandLine == "exit" || commandLine == "quit") {
            return 0;
        }
        if (commandLine == "help") {
            showHelpDocumentation();
        }
        stringstream ss(commandLine);
        vector<string> commandArgs;
        string token;
        while (getline(ss, token, ' ')) {
            commandArgs.push_back(token);
        }

        commandHandling(commandArgs);

        cout << "jit > ";
    }

    return 0;
}




void showHelpDocumentation() {
    cout << "\n\n--------------------------------------------------\n\n"
     << "JIT\n\n"
     << "init a repo:\n"
     << "\tjit init <path | .>\n"
     << "staging changes:\n"
     << "\tjit add <list | .>\n"
     << "commiting:\n"
     << "\tjit commit <message>\n"
     << "reset optons:\n"
     << "\tjit soft <head~1 | hash of parent>\n"
     << "\tjit medium <head~1 | hash of parent>\n"
     << "\tjit hard <head~1 | hash of parent>\n"
     << "showing log:\n"
     << "\tjit log\n"
     << "creating a branch:\n"
     << "\tjit branch <name>\n"
     << "branch switch:\n"
     << "\tjit goto <branch name | hash of branch>\n"
     << "create + switch to branch:\n"
     << "\tjit goto new <name>\n"
     << "status show:\n"
     << "\tjit status\n"
     << "merge:\n"
     << "\tjit merge <branch name>\n"
     << "cloning:\n"
     << "\tjit clone <path to repo>\n\n"
     << "--------------------------------------------------\n\n";
}

void commandHandling(const vector<string> &commandArgs) {
    if (commandArgs[0] != "jit") {
        cout << "Unknown command. Did you mean 'jit' ?\n";
    }
    else {
        if (commandArgs[1] == "init") {
            initializeRepository(commandArgs);
        }
        if (commandArgs[1] == "add") {
            stageAllChanges(commandArgs);
        }
    }

}

void initializeRepository(const vector<string> &commandArgs) {
    if (filesystem::exists(commandArgs[2] + "/" + ".jit")) {
        cout << "Working repository already initialized.\n";
    } else {
        try {
            filesystem::create_directories(commandArgs[2] + "/" + ".jit/objects");
            filesystem::create_directories(commandArgs[2] + "/" + ".jit/refs/branches");

            fstream master;
            master.open(commandArgs[2] + "/" + ".jit/refs/branches/master", fstream::in | fstream::out | fstream::trunc);
            fstream index;
            index.open(commandArgs[2] + "/" + ".jit/index", fstream::in | fstream::out | fstream::app);
            fstream head;
            head.open(commandArgs[2] + "/" + ".jit/head", fstream::in | fstream::out | fstream::trunc);
            head << "ref: refs/branches/master\n";

            head.close();
            index.close();
            master.close();
        } catch (const exception& e) {
            cout << e.what() << "\n";
        }
    }
}

void stageAllChanges(const vector<string> &commandArgs) {

}
