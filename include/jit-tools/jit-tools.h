#ifndef JIT_TOOLS_H
#define JIT_TOOLS_H

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


#endif //JIT_TOOLS_H
