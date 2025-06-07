#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

void showHelpDocumentation();
void commandHandling(const vector<string> &commandArgs, string &pathToJitRepo);
string initializeRepository(const vector<string> &commandArgs);
void stageAllChanges(string &pathToJitRepo);
void createNewBranch(string &pathToJitRepo, const string &name);
void switchBranch(string &pathToJitRepo, const string &name);
void switchToNewBranch(string &pathToJitRepo, const string &name);
void showCurrentBranch(string &pathToJitRepo);
void listAllBranches(string &pathToJitRepo);
void deleteBranch(string &pathToJitRepo, const string &name);


int main(){

    string commandLine;
    string pathToJitRepo;
    cout << "\nWelcome to Jit!\nSimilar to Git, yet a bit different!\n\nFor help, type in \"help\"\n\n";
    cout << "jit > ";
    while (getline(cin, commandLine)) {
        if (commandLine == "exit" || commandLine == "quit") {
            return 0;
        }
        if (commandLine == "help") {
            showHelpDocumentation();
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
    << "current branch:\n"
    << "\tjit current\n"
    << "all branches:\n"
    << "\tjit branches\n"
    << "delete a branch:\n"
    << "\tjit delete -b <name>\n"
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

void commandHandling(const vector<string> &commandArgs, string &pathToJitRepo) {
    if (commandArgs[0] != "jit") {
        cout << "Unknown command. Did you mean 'jit' ?\n";
    }
    else {
        if (commandArgs[1] == "init") {
            pathToJitRepo = initializeRepository(commandArgs);
        }
        if (commandArgs[1] == "add") {
            stageAllChanges(pathToJitRepo);
        }
        if (commandArgs[1] == "branch") {
            createNewBranch(pathToJitRepo, commandArgs[2]);
        }
        if (commandArgs[1] == "goto" && commandArgs[2] != "new") {
            switchBranch(pathToJitRepo, commandArgs[2]);
        }
        if (commandArgs[1] == "goto" && commandArgs[2] == "new") {
            switchToNewBranch(pathToJitRepo, commandArgs[3]);
        }
        if (commandArgs[1] == "current") {
            showCurrentBranch(pathToJitRepo);
        }
        if (commandArgs[1] == "branches") {
            listAllBranches(pathToJitRepo);
        }
        if (commandArgs[1] == "delete" && commandArgs[2] == "-b") {
            deleteBranch(pathToJitRepo, commandArgs[3]);
        }
    }

}

string initializeRepository(const vector<string> &commandArgs) {
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

    return (commandArgs[2] + "/");
}

void stageAllChanges(string &pathToJitRepo) {

}


void createNewBranch(string &pathToJitRepo, const string &name) {
    fstream branch;
    branch.open(pathToJitRepo + ".jit/refs/branches/" + name, fstream::in | fstream::out | fstream::trunc);
    fstream head;
    head.open(pathToJitRepo + ".jit/head", fstream::in);

    stringstream headRefBuffer;
    headRefBuffer << head.rdbuf();
    string refToHead = headRefBuffer.str();

    int lastSlash = refToHead.rfind('/');
    string currHead = refToHead.substr(lastSlash + 1);
    currHead.erase(currHead.find_last_not_of(" \n\r\t") + 1);

    fstream currHeadBranch;
    currHeadBranch.open(pathToJitRepo + ".jit/refs/branches/" + currHead, fstream::in);

    stringstream lastCommit;
    lastCommit << currHeadBranch.rdbuf();
    string lastCommitHash = lastCommit.str();
    branch << lastCommitHash;

    currHeadBranch.close();
    head.close();
    branch.close();
}

void switchBranch(string &pathToJitRepo, const string &name) {
    fstream head;
    head.open(pathToJitRepo + ".jit/head", fstream::out | fstream::trunc);
    head << "ref: refs/branches/" + name;
    head.close();
}

void switchToNewBranch(string &pathToJitRepo, const string &name) {
    createNewBranch(pathToJitRepo, name);
    switchBranch(pathToJitRepo, name);
}

void showCurrentBranch(string &pathToJitRepo) {
    fstream head;
    head.open(pathToJitRepo + ".jit/head", fstream::in);

    stringstream headRefBuffer;
    headRefBuffer << head.rdbuf();
    string refToHead = headRefBuffer.str();

    int lastSlash = refToHead.rfind('/');
    string currHead = refToHead.substr(lastSlash + 1);
    currHead.erase(currHead.find_last_not_of(" \n\r\t") + 1);

    cout << "* " << currHead << "\n";

    head.close();
}

void listAllBranches(string &pathToJitRepo) {
    fstream head;
    head.open(pathToJitRepo + ".jit/head", fstream::in);

    stringstream headRefBuffer;
    headRefBuffer << head.rdbuf();
    string refToHead = headRefBuffer.str();

    int lastSlash = refToHead.rfind('/');
    string currHead = refToHead.substr(lastSlash + 1);
    currHead.erase(currHead.find_last_not_of(" \n\r\t") + 1);

    head.close();

    cout << "* " + currHead << "\n";

    string path = pathToJitRepo + ".jit/refs/branches/";
    for (auto &entry : filesystem::directory_iterator(path)) {
        string branchPath = entry.path().string();
        int lastSLash = branchPath.rfind('/');
        string branchName = branchPath.substr(lastSLash + 1);
        branchName.erase(branchName.find_last_not_of(" \n\r\t") + 1);
        if (branchName != currHead)
            cout << branchName << "\n";
    }
}

void deleteBranch(string &pathToJitRepo, const string &name) {
    fstream head;
    head.open(pathToJitRepo + ".jit/head", fstream::in);

    stringstream headRefBuffer;
    headRefBuffer << head.rdbuf();
    string refToHead = headRefBuffer.str();

    int lastSlash = refToHead.rfind('/');
    string currHead = refToHead.substr(lastSlash + 1);
    currHead.erase(currHead.find_last_not_of(" \n\r\t") + 1);

    head.close();

    if (currHead == name) {
        cout << "error: Cannot delete a branch HEAD is pointing to.\n";
    } else {
        string path = pathToJitRepo + ".jit/refs/branches/" + name;
        if (remove(path.c_str())) {
            cout << "error: file deletion unsuccessful.\n";
        }
    }
}