#include "../../include/jit-tools/jit-tools.h"

json indexJSON;

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
     << "unstaged changes:\n"
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
        if (commandArgs[1] == "init")
            pathToJitRepo = initializeRepository(commandArgs);
        if (commandArgs[1] == "add")
            stageAllChanges();
        if (commandArgs[1] == "branch")
            createNewBranch(pathToJitRepo, commandArgs[2]);
        if (commandArgs[1] == "goto" && commandArgs[2] != "new")
            switchBranch(pathToJitRepo, commandArgs[2]);
        if (commandArgs[1] == "goto" && commandArgs[2] == "new")
            switchToNewBranch(pathToJitRepo, commandArgs[3]);
        if (commandArgs[1] == "current")
            showCurrentBranch();
        if (commandArgs[1] == "branches")
            listAllBranches(pathToJitRepo);
        if (commandArgs[1] == "delete" && commandArgs[2] == "-b")
            deleteBranch(pathToJitRepo, commandArgs[3]);
        if (commandArgs[1] == "status")
            jitStatus();
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

            ofstream index(commandArgs[2] + "/" + ".jit/index.json");
            indexJSON["master"] = json::object();
            index << indexJSON.dump(4);

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

string getLastModifiedTime(const string& fileName) {
    struct stat fileStat;

    if (stat(fileName.c_str(), &fileStat) == 0) {
        time_t mtime = fileStat.st_mtime;

        char buffer[20];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&mtime));

        return string(buffer);
    } else {
        return "Error: File not found!";
    }
}

void stageAllChanges() {
    for (auto i = filesystem::recursive_directory_iterator("."); i != filesystem::recursive_directory_iterator(); i++) {
        if (i->path().filename() == ".jit" || i->path().filename() == "a.exe")
            i.disable_recursion_pending();
        else {
            string fileName = i->path().filename().string();
            fstream head;
            head.open( ".jit/head", fstream::in);
            stringstream headRefBuffer;
            headRefBuffer << head.rdbuf();
            string refToHead = headRefBuffer.str();
            int lastSlash = refToHead.rfind('/');
            string currHead = refToHead.substr(lastSlash + 1);
            currHead.erase(currHead.find_last_not_of(" \n\r\t") + 1);

            ifstream indexIN(".jit/index.json");
            indexIN >> indexJSON;

            stringstream fileDataBuffer;
            ifstream dataIN(fileName);
            fileDataBuffer << dataIN.rdbuf();

            hash<string> hash;

            if (indexJSON[currHead].contains(fileName)) {
                string mtime = getLastModifiedTime(fileName);
                string hashedStr = to_string(hash(fileDataBuffer.str()));
                if (indexJSON[currHead][fileName] != mtime
                    && indexJSON[currHead][fileName]["hash"] != hashedStr) {
                    indexJSON[currHead][fileName]["mtime"] = mtime;
                    indexJSON[currHead][fileName]["hash"] = hashedStr;
                }
            } else {
                indexJSON[currHead][fileName] = json::object();
                indexJSON[currHead][fileName]["mtime"] = getLastModifiedTime(fileName);
                indexJSON[currHead][fileName]["hash"] = to_string(hash(fileDataBuffer.str()));
            }
            ofstream indexOUT(".jit/index.json");
            indexOUT << indexJSON.dump(4);

            head.close();
            indexIN.close();
            indexOUT.close();
            dataIN.close();
        }
    }
}

void jitStatus() {
    string currHead = getHead();
    ifstream indexIN(".jit/index.json");
    indexIN >> indexJSON;
    bool isStaged = false;
    unordered_map<string, pair<string, string>> stagedFiles;
    for (auto &[filename, data] : indexJSON[currHead].items()) {
        stagedFiles[filename].first = data["hash"];
        stagedFiles[filename].second = data["mtime"];
    }
    for (auto i = filesystem::recursive_directory_iterator("."); i != filesystem::recursive_directory_iterator(); i++) {
        if (i->path().filename() == ".jit" || i->path().filename() == "a.exe")
            i.disable_recursion_pending();
        else {
            string filename = i->path().filename().string();
            if (stagedFiles.find(filename) != stagedFiles.end()) {
                stringstream fileDataBuffer;
                ifstream dataIN(filename);
                fileDataBuffer << dataIN.rdbuf();
                dataIN.close();
                hash<string> hash;

                string hashedStr = to_string(hash(fileDataBuffer.str()));
                string mtime = getLastModifiedTime(filename);

                if (hashedStr != stagedFiles[filename].first) {
                    cout << "UNSTAGED: " << filename << "\n";
                    isStaged = true;
                }

            } else {
                cout << "UNSTAGED: " << filename << "\n";
                isStaged = true;
            }
        }
    }
    indexIN.close();
    if (!isStaged)
        cout << "Nothing to commit, working tree clean.\n";
}

string getHead() {
    fstream head;
    head.open(".jit/head", fstream::in);

    stringstream headRefBuffer;
    headRefBuffer << head.rdbuf();
    string refToHead = headRefBuffer.str();

    int lastSlash = refToHead.rfind('/');
    string currHead = refToHead.substr(lastSlash + 1);
    currHead.erase(currHead.find_last_not_of(" \n\r\t") + 1);

    head.close();
    return currHead;
}


void createNewBranch(string &pathToJitRepo, const string &name) {
    fstream branch;
    branch.open(pathToJitRepo + ".jit/refs/branches/" + name, fstream::in | fstream::out | fstream::trunc);
    string currHead = getHead();
    fstream currHeadBranch;
    currHeadBranch.open(pathToJitRepo + ".jit/refs/branches/" + currHead, fstream::in);

    stringstream lastCommit;
    lastCommit << currHeadBranch.rdbuf();
    string lastCommitHash = lastCommit.str();
    branch << lastCommitHash;

    ifstream indexIN(pathToJitRepo + ".jit/index.json");
    indexIN >> indexJSON;
    indexJSON[name] = json::object();
    ofstream indexOUT(pathToJitRepo + ".jit/index.json");
    indexOUT << indexJSON.dump(4);

    indexIN.close();
    indexOUT.close();
    currHeadBranch.close();
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

void showCurrentBranch() {
    string head = getHead();
    cout << "* " << head << "\n";
}

void listAllBranches(string &pathToJitRepo) {
    string currHead = getHead();
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
    string currHead = getHead();
    if (currHead == name) {
        cout << "error: Cannot delete a branch HEAD is pointing to.\n";
        return;
    } else {
        string path = pathToJitRepo + ".jit/refs/branches/" + name;
        if (remove(path.c_str())) {
            cout << "error: file deletion unsuccessful.\n";
        }
    }
    ifstream indexIN(pathToJitRepo + ".jit/index.json");
    indexIN >> indexJSON;
    indexJSON.erase(name);
    indexIN.close();
    ofstream indexOUT(pathToJitRepo + ".jit/index.json");
    indexOUT << indexJSON.dump(4);
    indexOUT.close();
}