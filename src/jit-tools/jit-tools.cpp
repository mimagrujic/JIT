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
    << "\tcheck the object type:\n"
     << "jit cat-file -t <hash>\n"
    << "\tcheck the object's content:\n"
    << "jit cat-file -p <hash>\n"
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
        if (commandArgs[1] == "commit") {
            string message = "";
            int numOfWords = commandArgs.size();
            for (int i = 2; i < numOfWords; i++) {
                message += commandArgs[i] + " ";
            }
            commit(message);
        }
        if (commandArgs[1] == "cat-file" && (commandArgs[2] == "-t" || commandArgs[2] == "-p"))
            cout << readObject(commandArgs[2], commandArgs[3]) << "\n";
        if (commandArgs[1] == "log")
            showLog();
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

vector<unsigned char> zlibCompress(const string &data) {
    uLong srcLen = data.size();
    uLong destLen = compressBound(srcLen);
    vector<unsigned char> compressed(destLen);

    if (compress(compressed.data(), &destLen, reinterpret_cast<const Bytef*>(data.c_str()), srcLen) != Z_OK) {
        throw runtime_error("Zlib compression failed");
    }

    compressed.resize(destLen);
    return compressed;
}

string decompressZlib(const vector<unsigned char> &compressed) {
    uLongf decompressedSize = 100000;
    vector<unsigned char> buffer(decompressedSize);

    if (uncompress(buffer.data(), &decompressedSize, compressed.data(), compressed.size()) != Z_OK)
        throw runtime_error("Zlib decompression failed.\n");

    return string(reinterpret_cast<char*>(buffer.data()), decompressedSize);
}
string createABlob(string currHead, string fileName) {
    hash<string> hash;
    stringstream fileDataBuffer;
    ifstream dataIN(fileName);
    fileDataBuffer << dataIN.rdbuf();
    dataIN.close();
    string blobString = "";
    string res;

    if (indexJSON[currHead].contains(fileName)) {
        blobString = "blob " + to_string(fileDataBuffer.str().length()) + '\0' + fileDataBuffer.str();
        string hashedString = to_string(hash(blobString));
        if (hashedString != indexJSON[currHead][fileName]) {
            vector<unsigned char> compressed = zlibCompress(blobString);
            string dir = hashedString.substr(0, 2);
            string blobName = hashedString.substr(2);
            if (!filesystem::exists(".jit/objects/" + dir))
                filesystem::create_directory(".jit/objects/" + dir);
            ofstream blobFile(".jit/objects/" + dir + "/" + blobName, ios::binary);
            blobFile.write(reinterpret_cast<char*>(compressed.data()), compressed.size());
            blobFile.close();

            indexJSON[currHead][fileName] = hashedString;
            cout << "ADDED:    " << fileName << "\n";
            res = "blob\t\t" + hashedString + "\t\t" + fileName + "\n";
        } else {
            res = "blob\t\t" + hashedString + "\t\t" + fileName + "\n";
            return res;
        }
    } else {
        blobString = "blob " + to_string(fileDataBuffer.str().length()) + '\0' + fileDataBuffer.str();
        string hashedString = to_string(hash(blobString));
        vector<unsigned char> compressed = zlibCompress(blobString);
        string dir = hashedString.substr(0, 2);
        string blobName = hashedString.substr(2);
        filesystem::create_directory(".jit/objects/" + dir);
        ofstream blobFile(".jit/objects/" + dir + "/" + blobName, ios::binary);
        blobFile.write(reinterpret_cast<char*>(compressed.data()), compressed.size());
        blobFile.close();

        indexJSON[currHead][fileName] = hashedString;
        cout << "ADDED:    " << fileName << "\n";
        res = "blob\t\t" + hashedString + "\t\t" + fileName + "\n";
    }

    return res;
}

void stageAllChanges() {
    ifstream indexIN(".jit/index.json");
    indexIN >> indexJSON;

    string currHead = getHead();

    for (auto i = filesystem::recursive_directory_iterator("."); i != filesystem::recursive_directory_iterator(); i++) {
        if (i->path().filename() == ".jit" || i->path().filename() == "a.exe") {
            i.disable_recursion_pending();
        } else {
            string fileName = i->path().filename().string();
            string path = i->path().string();
            if (!filesystem::is_directory(fileName)) {
                createABlob(currHead, path);
            }
        }

    }
    ofstream indexOUT(".jit/index.json");
    indexOUT << indexJSON.dump(4);

    indexIN.close();
    indexOUT.close();
}

void jitStatus() {
    string currHead = getHead();
    ifstream indexIN(".jit/index.json");
    indexIN >> indexJSON;
    bool isStaged = false;
    unordered_map<string, string> stagedDataMap;
    auto &branchFiles = indexJSON[currHead];
    for (auto it = branchFiles.begin(); it != branchFiles.end();) {
        string filename = it.key();
        auto &hash = it.value();
        stagedDataMap[filename] = hash;
        if (!filesystem::exists(filename)) {
            cout << "DELETED: " << filename << "\n";
            it = branchFiles.erase(it);
            isStaged = true;
        } else
            it++;
    }
    for (auto i = filesystem::recursive_directory_iterator("."); i != filesystem::recursive_directory_iterator(); i++) {
        if (i->path().filename() == ".jit" || i->path().filename() == "a.exe")
            i.disable_recursion_pending();
        else {
            string filename = i->path().string();
            if (!filesystem::is_directory(filename)) {
                if (stagedDataMap.find(filename) != stagedDataMap.end()) {
                    stringstream fileDataBuffer;
                    ifstream dataIN(filename);
                    fileDataBuffer << dataIN.rdbuf();
                    dataIN.close();
                    hash<string> hash;

                    string hashedStr = to_string(hash("blob " + to_string(fileDataBuffer.str().length()) + '\0' + fileDataBuffer.str()));

                    if (hashedStr != stagedDataMap[filename]) {
                        cout << "MODIFIED:    " << i->path() << "\n";
                        isStaged = true;
                    }
                } else {
                    cout << "MODIFIED:    " << i->path() << "\n";
                    isStaged = true;
                }
            }
        }
    }
    if (!isStaged)
        cout << "Nothing to commit, working tree clean.\n";

    ofstream indexOUT(".jit/index.json");
    indexOUT << indexJSON.dump(4);
    indexIN.close();
    indexOUT.close();
}

string readObject(string option, string hash) {
    string dirName = hash.substr(0, 2);
    string objectFileName = hash.substr(2);
    string resString;
    ifstream objectStream(".jit/objects/" + dirName + "/" + objectFileName, ios::binary);
    vector<unsigned char> compressed((istreambuf_iterator<char>(objectStream)),istreambuf_iterator<char>());
    objectStream.close();
    string decompressed = decompressZlib(compressed);
    if (option == "-t") {
        int fstSpace = decompressed.find(" ");
        resString = decompressed.substr(0, fstSpace);
    } else {
        int nul = decompressed.find('\0');
        resString = decompressed.substr(nul + 1);
    }

    return resString;
}

string createTree(string directory) {
    string head = getHead();
    string treeContent = "";
    for (auto &i : filesystem::directory_iterator(directory)) {
        if (i.path().filename() == ".jit" || i.path().filename() == "a.exe")
            continue;
        string path = i.path().string();
        if (!filesystem::is_directory(path)) {
            treeContent += createABlob(head, path);
        } else {
            treeContent += createTree(path);
        }
    }
    hash<string> hash;
    string treeFileContent = "tree " + to_string(treeContent.size()) + '\0' + treeContent;
    string hashedString = to_string(hash(treeFileContent));
    vector<unsigned char> compressed = zlibCompress(treeFileContent);
    string dir = hashedString.substr(0, 2);
    string treeName = hashedString.substr(2);
    if (!filesystem::exists(".jit/objects/" + dir))
        filesystem::create_directory(".jit/objects/" + dir);
    ofstream treeFileStream(".jit/objects/" + dir + "/" + treeName, ios::binary);
    treeFileStream.write(reinterpret_cast<char *>(compressed.data()), compressed.size());
    treeFileStream.close();

    string res = "tree\t\t" + hashedString + "\t\t" + directory + "\n";
    return res;
}

string createCommit(string lastCommitHash, string rootTreeHash, string message) {
    string commitString = "";
    commitString += "\ntree\t\t" + rootTreeHash + "\n";
    if (lastCommitHash != "")
        commitString += "parent\t\t" + lastCommitHash + "\n";
    char username[UNLEN+1];
    DWORD username_len = UNLEN+1;
    GetUserName(username, &username_len);
    string user = username;
    commitString += "author\t\t" + user + "\n";
    auto t = time(nullptr);
    auto tm = *localtime(&t);
    stringstream ss;
    ss << put_time(&tm, "%d-%m-%Y, %H:%M:%S");
    string time = ss.str();
    commitString += "time\t\t" + time + "\n";
    commitString += "\n" + message + "\n";

    int len = commitString.size();
    string commit = "commit " + to_string(len) + '\0' + commitString;
    hash<string> hash;
    string hashedString = to_string(hash(commit));
    vector<unsigned char> compressed = zlibCompress(commit);
    string dir = hashedString.substr(0, 2);
    string commitFile = hashedString.substr(2);
    if (!filesystem::exists(".jit/objects/" + dir))
        filesystem::create_directory(".jit/objects/" + dir);
    ofstream commitStream(".jit/objects/" + dir + "/" + commitFile, ios::binary);
    commitStream.write(reinterpret_cast<const char *>(compressed.data()), compressed.size());
    commitStream.close();

    return hashedString;
}

void commit(string message) {
    string treeFile = createTree(".");
    int fstTab = treeFile.find("\t\t");
    int sndTab = treeFile.rfind("\t\t");
    string hash = treeFile.substr(fstTab + 2, sndTab - (fstTab + 2));

    string head = getHead();
    fstream branch(".jit/refs/branches/" + head, ios::in | ios::out);
    string lastCommitHash;
    branch >> lastCommitHash;
    branch.close();

    string commitHash = createCommit(lastCommitHash, hash, message);
    ofstream commitFile(".jit/refs/branches/" + head, ios::trunc);
    commitFile << commitHash;
    commitFile.close();
}

void showLog() {
    string head = getHead();
    string recentCommit;
    ifstream branch(".jit/refs/branches/" + head, ios::in);
    branch >> recentCommit;
    branch.close();
    if (recentCommit == "") {
        cout << "fatal: your current branch does not have any commits yet.\n";
        return;
    }
    while (true) {
        cout << "------------------------------------";
        string commitContent = readObject("-p", recentCommit);
        cout << commitContent << "\n";
        if (commitContent.find("parent") == string::npos)
            break;
        string parent = commitContent.substr(commitContent.find("parent") + 6);
        parent = parent.substr(parent.find("\t\t") + 2);
        parent = parent.substr(0, parent.find("\n"));
        recentCommit = parent;
    }
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

void initializeMap(unordered_map<string, string> &map, string treeHash, string filePath) {
    vector<string> objects;
    stringstream ss(readObject("-p", treeHash));
    string token;
    while (getline(ss, token, '\n'))
        objects.push_back(token);
    for (string s : objects) {
        int fst = s.find("\t\t");
        int snd = s.rfind("\t\t");
        string type = s.substr(0, fst);
        string name = s.substr(snd + 2);
        string hash = s.substr(fst + 2, snd - fst - 2);
        map[name] = hash;
        if (type == "tree")
            initializeMap(map, hash, name + "/");
    }
}

void restoreFileContent(string previousBranch, string currBranch) {
    fstream previous(".jit/refs/branches/" + previousBranch, ios::in);
    string previousHash;
    previous >> previousHash;
    previous.close();
    fstream current(".jit/refs/branches/" + currBranch, ios::in);
    string currentHash;
    current >> currentHash;
    current.close();
    if (previousBranch == currBranch) {
        cout << "Already on branch " << currBranch << "\n";
        return;
    }
    if (previousHash == currentHash)
        return;

    string previousTreeHash = readObject("-p", previousHash);
    size_t start = previousTreeHash.find("tree\t\t");
    size_t end = previousTreeHash.find("\n", start);
    previousTreeHash = previousTreeHash.substr(start + 6, end - start - 6);
    string currentTreeHash = readObject("-p", currentHash);
    start = currentTreeHash.find("tree\t\t");
    end = currentTreeHash.find("\n", start);
    currentTreeHash = currentTreeHash.substr(start + 6, end - start - 6);

    unordered_map<string, string> previousTreeMap;
    initializeMap(previousTreeMap, previousTreeHash, "");
    unordered_map<string, string> currentTreeMap;
    initializeMap(currentTreeMap, currentTreeHash, "");


    for (auto it = currentTreeMap.begin(); it != currentTreeMap.end();) {
        if (previousTreeMap.find(it->first) != previousTreeMap.end()) {
            if (previousTreeMap[it->first] == it->second || filesystem::is_directory(it->first)) {
                previousTreeMap.erase(it->first);
                it = currentTreeMap.erase(it);
                continue;
            }
            string fileHash = it->second;
            string name = it->first;
            string content = readObject("-p", fileHash);
            ofstream inStream(name, ios::out | ios::trunc);
            inStream << content;
            inStream.close();
            previousTreeMap.erase(it->first);
            it = currentTreeMap.erase(it);
        } else {
            if (!filesystem::is_directory(it->first)) {
                auto parentPath = filesystem::path(it->first).parent_path();
                filesystem::create_directories(parentPath);

                string fileHash = it->second;
                string name = it->first;
                string content = readObject("-p", fileHash);
                ofstream inStream(name, ios::out);
                inStream << content;
                inStream.close();
                it = currentTreeMap.erase(it);
            } else {
                filesystem::create_directories(it->first);
                it = currentTreeMap.erase(it);
            }
        }
    }

    for (auto it = previousTreeMap.begin(); it != previousTreeMap.end(); it++) {
        if (filesystem::is_directory(it->first))
            filesystem::remove_all(it->first);
        else
            filesystem::remove(it->first);
    }
}

void switchBranch(string &pathToJitRepo, const string &name) {
    string previousBranch = getHead();
    fstream head;
    head.open(pathToJitRepo + ".jit/head", fstream::out | fstream::trunc);
    head << "ref: refs/branches/" + name;
    head.close();
    restoreFileContent(previousBranch, name);
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