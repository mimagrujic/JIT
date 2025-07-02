#ifndef JIT_TOOLS_H
#define JIT_TOOLS_H

#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <iomanip>
#include <ctime>
#include <functional>
#include <utility>
#include <zlib.h>
#include <windows.h>
#include <Lmcons.h>
#include "../json/json.hpp"

using namespace std;
using json = nlohmann::json;

void showHelpDocumentation();
void commandHandling(const vector<string> &commandArgs, string &pathToJitRepo);
string initializeRepository(const vector<string> &commandArgs);
void stageAllChanges();
void jitStatus();
void createNewBranch(string &pathToJitRepo, const string &name);
void switchBranch(string &pathToJitRepo, const string &name);
void switchToNewBranch(string &pathToJitRepo, const string &name);
void showCurrentBranch();
void listAllBranches(string &pathToJitRepo);
void deleteBranch(string &pathToJitRepo, const string &name);
string getHead();
void commit(string message);
vector<unsigned char> zlibCompress(const string &data);
string decompressZlib(const vector<unsigned char> &compressed);
string readObject(string option, string hash);
string createABlob(string currHead, string fileName);
string createTree(string directory);
string createCommit(string lastCommitHash, string rootTreeHash, string message);
void showLog();
void restoreFileContent(string previousBranch, string currBranch);
void initializeMap(unordered_map<string, string> &map, string treeHash, string filePath);

#endif //JIT_TOOLS_H