#ifndef JIT_TOOLS_H
#define JIT_TOOLS_H

#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <sys/stat.h>
#include <ctime>
#include <functional>
#include <utility>
#include "../json/json.hpp"

using namespace std;
using json = nlohmann::json;

void showHelpDocumentation();
void commandHandling(const vector<string> &commandArgs, string &pathToJitRepo);
string initializeRepository(const vector<string> &commandArgs);
string getLastModifiedTime(const string& fileName);
void stageAllChanges();
void jitStatus();
void createNewBranch(string &pathToJitRepo, const string &name);
void switchBranch(string &pathToJitRepo, const string &name);
void switchToNewBranch(string &pathToJitRepo, const string &name);
void showCurrentBranch();
void listAllBranches(string &pathToJitRepo);
void deleteBranch(string &pathToJitRepo, const string &name);
string getHead();

#endif //JIT_TOOLS_H
