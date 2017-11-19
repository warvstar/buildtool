#pragma once
#include <string>
#include <vector>
#include <unordered_map>
struct Node;
struct ProjFile {
	std::string fn;
	std::string hash;
};
class BuildTool {
	std::unordered_map<std::string, std::string> files;
	std::unordered_map<std::string, Node*> nodeMap;
	std::vector<std::string> cFiles;
	std::vector<std::string> cppFiles;
	std::vector<std::string> cppmFiles;
	std::vector<std::string> objects;
	std::vector<std::string> package;
	std::vector<std::string> moduleDirs;
	std::string compileArgs = "";
	std::string compileModArgs = "";
	std::string linkArgs = "";
	std::string compiler = "";
	std::string linker = "";
	std::string outDir = "";
	std::string debugDir = "";
	std::string linkOut = "";
	std::string defines = "";
	std::string includeDirs = "";
	std::string libDirs = "";
	std::string sourceDirs = "";
	std::string libs = "";
	std::string excludelibs = "";
	std::string gcctoolchain = "";
	std::string sysroot = "";
	std::string sysrootLinker = "";
	bool successfulCompile = true;
	bool successfulLink = true;
	bool successfulPackage = true;
	void addModuleDir(const std::string &dir) { moduleDirs.emplace_back(dir); }
	std::string linkerSettingsHash = "";
	std::unordered_map<std::string, std::string> getProjFiles();
	void setProjFiles();
	std::vector<std::string> getImports(std::string src);
	void extractSharedFiles(const std::string & str1, const std::string & str2);
	void extractModules(Node * node);
	void extractFiles(const std::string & str1);
	void getFilesVS();
	void getFiles();
	void CleanExit();
	void SetNeedsCompilation(Node & node);
	void CompileNodes();
	bool CompileCPP(Node & node, bool force);
	bool CompileC(Node & node, bool force);
	bool CompileCPPM(Node & node, bool force);
	bool Compile(Node & node);
	void link();
	void doPackage();
	void getPackage(std::string & str);
	void getArgs(std::string pn);
	void Clean();
public:
	void Run(bool useVS, bool clean, bool rebuild);
};