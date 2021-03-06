/* Build tools used for building clang projects for different systems
Command line: buildtool.exe platformtype

TODO:
Thread the compiler across all available cores
Compile and/or link remotely and patch diff

*/
#include "Helper.h" //Includes and various defines
#include "Node.h"
#include "BuildTool.h"
#include "Timer.h"

std::unordered_map<string, string> BuildTool::getProjFiles() {
	fs::path p = "./Files.txt";
	std::ifstream t(p);
	string line;
	std::unordered_map<string, string> files2;
	if (getline(t, line))
		linkerSettingsHash = line;

	while (getline(t, line)) {
		auto s = Helper::split(line, '|');
		if (s.size() > 1)
			files2[s[0]] = s[1];
		else
			files2[s[0]] = "";
	}
	return files2;
}
void BuildTool::setProjFiles() {
	string out = linkerSettingsHash + "\n";
	std::cout << linkerSettingsHash.c_str() << std::endl;

	for (auto &node : nodeMap) {
		out += node.second->name + "|" + node.second->hash + "\n";
	}
	ofstream myfile;
	myfile.open("./Files.txt");
	myfile << out.c_str();
	myfile.close();
}
std::vector<string> BuildTool::getImports(string src) {
	string imp = "";
	std::vector<string> imps;
	while (Helper::getBetween(src, imp, string("import "), string(";"))) {
		// Find if this module resides in our prebuilt mo
		imps.push_back(imp + ".cppm");
	}
	return imps;
}
void BuildTool::extractSharedFiles(const string &str1, const string &str2) {
	std::ifstream t(str1);
	string str((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());
	// std::cout << str << endl;
	int foundstart = str.find("Include=\"");
	Helper::replaceAll(str, "$(MSBuildThisFileDirectory)", str2);
	while (foundstart > 0) {
		foundstart += 9;
		int foundend = str.find("\"", foundstart);
		string found = str.substr(foundstart, foundend - foundstart);
		// std::cout << found << std::endl;
		str = str.substr(foundend);
		int isC = found.find(".c");
		int isCPP = found.find(".cpp");
		int isCPPM = found.find(".cppm");
		if (isCPPM > 0) {
			fs::path path = found;
			string path2 = path.parent_path().string();
			addModuleDir(path2 + "\\");
			cppmFiles.push_back(found);
		}
		else if (isCPP > 0) {
			fs::path path = found;
			string path2 = path.parent_path().string();
			addModuleDir(path2 + "\\");
			cppFiles.push_back(found);
		}
		else if (isC > 0) {
			fs::path path = found;
			string path2 = path.parent_path().string();
			addModuleDir(path2 + "\\");
			cFiles.push_back(found);
		}
		foundstart = str.find("Include=\"");
	}
	for (auto &cpp : cppFiles) {
	}
}

void BuildTool::extractModules(Node *node) {
	string helper = "import ";
	string src = node->src;
	int foundstart = src.find(helper);

	while (foundstart >= 0) {
		if (foundstart != 0) {
			helper = "\nimport ";
			foundstart = src.find(helper);
			if (foundstart < 0) {
				helper = " import ";
				foundstart = src.find(helper);
				if (foundstart < 0)
					break;
			}
		}
		int foundend = src.substr(foundstart + helper.size()).find(";");
		if (foundend > 0) {
			string found = src.substr(foundstart + helper.size(), foundend);
			src = src.substr(foundstart + helper.size() + foundend);
			bool foundModule = false;
			Node *gotNode = nullptr;
			for (auto &dir : moduleDirs) {
				string path = dir + found + ".cppm";
				auto got = nodeMap.find(path);
				if (got != nodeMap.end()) {
					foundModule = true;
					gotNode = got->second;
					break;
				}
				// else
				// printf("Could not find module(%s)\n", path.c_str());
			}
			if (foundModule) {
				// printf("%s has import:%s\n", node.name.c_str(),
				// gotNode->name.c_str());
				gotNode->parents.push_back(node);
				node->children.push_back(gotNode);
			}
			else
				printf("Could not find module(%s) in any module directory.\n", found.c_str());
		}
		helper = "import ";
		foundstart = src.find(helper);
	}
	for (auto &child : node->children) {
		extractModules(child);
	}
}
void BuildTool::extractFiles(const string &str1) {
	string str = Helper::readfile(str1.c_str());
	// std::cout << str << endl;
	int foundstart = str.find("Include=\"");
	while (foundstart > 0) {
		foundstart += 9;
		int foundend = str.find("\"", foundstart);
		string found = str.substr(foundstart, foundend - foundstart);
		// std::cout << found << std::endl;
		str = str.substr(foundend);
		int isC = found.find(".c");
		int isCPP = found.find(".cpp");
		int isCPPM = found.find(".cppm");
		if (isCPPM > 0) {
			fs::path path = found;
			string path2 = path.parent_path().string();
			addModuleDir(path2 + "\\");
			cppmFiles.push_back(found);
		}
		else if (isCPP > 0) {
			fs::path path = found;
			string path2 = path.parent_path().string();
			addModuleDir(path2 + "\\");
			cppFiles.push_back(found);
		}
		else if (isC > 0) {
			fs::path path = found;
			string path2 = path.parent_path().string();
			addModuleDir(path2 + "\\");
			cFiles.push_back(found);
		}

		foundstart = str.find("Include=\"");
	}
}
void BuildTool::getFiles() {
		std::vector<string> dirs = Helper::split(sourceDirs, '\n');
		dirs.push_back("./");
		for (auto &dir : dirs)
		for (auto &p : fs::directory_iterator(dir)) {
			auto f = p.path().string();
			bool isC = p.path().extension().string() == ".c";
			bool isCPP = p.path().extension().string() == ".cpp";
			bool isCPPM = p.path().extension().string() == ".cppm";
			if (isCPPM) {
				addModuleDir(dir + "\\");
				cppmFiles.push_back(f);
			}
			else if (isCPP) {
				cppFiles.push_back(f);
			}
			else if (isC) {
				cFiles.push_back(f);
			}
		}

}
void BuildTool::getFilesVS() {
	for (auto &p : fs::directory_iterator("./"))
		if (p.path().extension() == ".vcxproj") {
			extractFiles(p.path().string());
			std::ifstream t(p.path());
			string str((std::istreambuf_iterator<char>(t)),
				std::istreambuf_iterator<char>());
			string helper = "Import Project=\"";
			int foundstart = str.find(helper);
			while (foundstart > 0) {
				foundstart += helper.size();
				int foundend = str.find("\"", foundstart);
				string found = str.substr(foundstart, foundend - foundstart);
				// std::cout << found << std::endl;
				str = str.substr(foundend);
				int isVCXITEMS = found.find(".vcxitems");
				if (isVCXITEMS > 0) {
					int last = found.find_last_of("\\");
					string path = found.substr(0, last + 1);
					extractSharedFiles(found, path);
				}
				foundstart = str.find("Import Project=\"");
			}
		}
	std::vector<string>::iterator it;
	it = std::unique(moduleDirs.begin(), moduleDirs.end());
	moduleDirs.resize(std::distance(moduleDirs.begin(), it));
	// for (auto& dir : moduleDirs)
	// printf("Dir:%s\n", dir.c_str());
	for (auto &cppm : cppmFiles) {
		int last = cppm.find_last_of("\\");
		string fnsafe = cppm;
		if (last > 0)
			fnsafe = cppm.substr(last + 1, cppm.size());
		auto nameWithoutExt = fnsafe.substr(0, fnsafe.size() - 5);
		string out = debugDir + nameWithoutExt + ".pcm.o";
		Node *node = new Node();
		node->name = cppm;
		node->outname = out;
		node->type = NodeType::cppm;
		nodeMap[cppm] = node;
	}
	for (auto &cpp : cppFiles) {
		int last = cpp.find_last_of("\\");
		string fnsafe = cpp;
		if (last > 0)
			fnsafe = cpp.substr(last, cpp.size());
		string out = debugDir + fnsafe + ".o";
		Node *node = new Node();
		node->name = cpp;
		node->outname = out;
		node->type = NodeType::cpp;
		nodeMap[cpp] = node;
	}
	for (auto &c : cFiles) {
		int last = c.find_last_of("\\");
		string fnsafe = c;
		if (last > 0)
			fnsafe = c.substr(last, c.size());
		string out = debugDir + fnsafe + ".o";
		Node *node = new Node();
		node->name = c;
		node->outname = out;
		node->type = NodeType::c;
		nodeMap[c] = node;
	}
	for (auto &node : nodeMap) {
		if (ifstream f{ node.second->name, std::ifstream::binary }) {
			string buffer;
			f.seekg(0, std::ios::end);
			buffer.resize(f.tellg());
			f.seekg(0);
			f.read(buffer.data(), buffer.size());
			Node *node2 = node.second;
			node2->src = buffer;
		}
		else { // Project file not found, do not try to compile, remove from
			   // nodeMap
		}
	}
	for (auto &node : nodeMap) {
		extractModules(node.second);
	}
}

/*For ~7000 files (under 2000 lines), we can check for changes in under 500ms
Now lets compile while checking files, compiling will most likely take longer than 500ms.
*/
void BuildTool::CleanExit() {
	setProjFiles();
	exit(0);
}
/*void BuildTool::setNeedsCompilation(Node &node) {
for (auto &child : node.children) {
if (child->children.size() > 0)
setNeedsCompilation(*child);
child->Compile();
}
}*/
static int filesCompiled = 0;
void BuildTool::SetNeedsCompilation(Node&node) {
	for (auto &parent : node.parents) {
		parent->forceRecompile = true;
		// printf("Setting needsCompilation of parent (%s) of (%s) to true.\n",
		// node->parent->name.c_str(), node->name.c_str());
		parent->needsCompilation = true;
		SetNeedsCompilation(*parent);
	}
}
// Modules need to include their dirs before we have access to them, same as
// headers
void BuildTool::CompileNodes() {
	for (auto node : nodeMap) {
		Compile(*node.second);
	}
}
/*void called_from_async(string i, int sI) {
std::cout << "Starting compile:" << i << std::endl;
std::this_thread::sleep_for(std::chrono::seconds(sI));
std::cout << "Finished compile:" << i << std::endl;
}*/
bool BuildTool::CompileCPP(Node& node, bool force) {
	auto abc2 = XXH64(node.src.c_str(), node.src.size(), 0);
	string hash2 = std::to_string(abc2);
	if (!force) {
		auto got = files.find(node.name);
		if (got != files.end()) {
			if (hash2 != got->second) {
				node.needsCompilation = true;
				// printf("Old hash:%s \nNew hash:%s\n", got->second.c_str(),

			}
			else {
				node.needsCompilation = false;
				// printf("Node:%s does not need compiling.\n", name.c_str());
				// printf("Old hash:%s \nNew hash:%s\n", got->second.c_str(),
				node.hash = got->second;
				return false;
			}
		}
	}
	node.compiled = true;
	int last = node.name.find_last_of("\\");
	string fnsafe = node.name;
	if (last > 0)
		fnsafe = node.name.substr(last, node.name.size());
	std::cout << "Starting compile:" << node.name << std::endl;
	string args = compileArgs;
	string out = debugDir + fnsafe + ".o";
	Helper::replaceAll(args, "$infile", node.name);
	Helper::replaceAll(args, "$outfile", out);
	remove(out.c_str());
	std::system(args.c_str());
	std::ifstream f(out.c_str());
	if (f.good()) {
		node.needsCompilation = false;
		node.hash = hash2;
		filesCompiled++;
		objects.push_back(out);
		f.close();
	}
	else {
		successfulCompile = false;
		CleanExit();
	}
	return true;
}
bool BuildTool::CompileC(Node& node, bool force) {
	auto abc2 = XXH64(node.src.c_str(), node.src.size(), 0);
	string hash2 = std::to_string(abc2);
	if (!force) {
		auto got = files.find(node.name);
		if (got != files.end()) {
			if (hash2 != got->second) {
				node.needsCompilation = true;
				// printf("Old hash:%s \nNew hash:%s\n", got->second.c_str(),

			}
			else {
				node.needsCompilation = false;
				// printf("Node:%s does not need compiling.\n", name.c_str());
				// printf("Old hash:%s \nNew hash:%s\n", got->second.c_str(),
				node.hash = got->second;
				return false;
			}
		}
	}
	node.compiled = true;
	int last = node.name.find_last_of("\\");
	string fnsafe = node.name;
	if (last > 0)
		fnsafe = node.name.substr(last, node.name.size());
	std::cout << "Starting compile:" << node.name << std::endl;
	string args = compileArgs;
	string out = debugDir + fnsafe + ".o";
	Helper::replaceAll(args, "$infile", node.name);
	Helper::replaceAll(args, "$outfile", out);
	remove(out.c_str());
	std::system(args.c_str());
	ifstream f(out.c_str());
	if (f.good()) {
		node.needsCompilation = false;
		node.hash = hash2;
		filesCompiled++;
		objects.push_back(out);
		f.close();
	}
	else {
		printf("Can't find:%s", out.c_str());
		successfulCompile = false;
		CleanExit();
	}
	return true;
}
bool BuildTool::CompileCPPM(Node& node, bool force) {
	auto abc2 = XXH64(node.src.c_str(), node.src.size(), 0);
	string hash2 = std::to_string(abc2);
	if (!force) {
		auto got = files.find(node.name);
		if (got != files.end()) {
			if (hash2 != got->second) {
				node.needsCompilation = true;
				// printf("Old hash:%s \nNew hash:%s\n", got->second.c_str(),

			}
			else {
				node.hash = got->second;
				node.needsCompilation = false;
				// printf("Node:%s does not need compiling.\n", node2->name.c_str());
				return false;
			}
		}
	}
	node.compiled = true;
	std::cout << "Starting precompile of module:" << node.name << std::endl;
	string args = compileModArgs;
	int last = node.name.find_last_of("\\");
	string fnsafe = node.name;
	if (last > 0)
		fnsafe = node.name.substr(last + 1, node.name.size());
	auto nameWithoutExt = fnsafe.substr(0, fnsafe.size() - 5);
	string out = debugDir + nameWithoutExt + ".pcm";
	Helper::replaceAll(args, "$infile", node.name);
	Helper::replaceAll(args, "$outfile", out);
	remove(out.c_str());
	args += " --precompile";
	remove(out.c_str());
	std::system(args.c_str());
	ifstream f(out.c_str());
	string pcm = out;
	if (f.good()) {
		node.needsCompilation = false;
		node.hash = hash2;
		filesCompiled++;
		// printf("Compiled to:%s\n", out.c_str());
		f.close();
		std::cout << "Starting compile of pcm:" << pcm << std::endl;
		string args = compileModArgs;
		string out = pcm + ".o";
		Helper::replaceAll(args, "$infile", pcm);
		Helper::replaceAll(args, "$outfile", out);
		std::system(args.c_str());
		ifstream f(out.c_str());
		if (f.good()) {
			objects.push_back(out);
			f.close();
		}
		else {
			printf("Problem compiling pcm:%s\n", out.c_str());
			printf("%s\n", args.c_str());
			successfulCompile = false;
			CleanExit();
		}
	}
	else {
		printf("Can't precompile module:%s\n", out.c_str());
		printf("%s \n", args.c_str());
		successfulCompile = false;
		CleanExit();
	}
	return true;
}
bool BuildTool::Compile(Node& node) {
	for (auto &child : node.children) {
		Compile(*child);
	}
	// printf("Children have been recompiled?:%d", childrenHaveBeenRecompiled);
	if (node.needsCompilation) {
		switch (node.type) {
			case NodeType::c:
			{
				node.needsCompilation = false;
				auto ret = CompileC(node, node.forceRecompile);
				if (ret) {
					SetNeedsCompilation(node);
					// printf("", name.c_str());
				}
				// printf("%s has been recompiled?:%d\n", name.c_str(), ret);
				return ret;
				break;
			}
			case NodeType::cpp:
			{
				node.needsCompilation = false;
				auto ret = CompileCPP(node, node.forceRecompile);
				if (ret) {
					SetNeedsCompilation(node);
					// printf("", name.c_str());
				}
				// printf("%s has been recompiled?:%d\n", name.c_str(), ret);
				return ret;
				break;
			}
			case NodeType::cppm:
			{
				node.needsCompilation = false;
				auto ret = CompileCPPM(node, node.forceRecompile);
				if (ret)
					SetNeedsCompilation(node);
				// printf("%s has been recompiled?:%d\n", name.c_str(), ret);
				return ret;
				break;
			}
			default:
				break;
		}
	}
	return false;
}
void BuildTool::link() {
	// objects.clear();

	string hash = std::to_string(XXH64(linkArgs.c_str(), linkArgs.size(), 0));
	if (linkerSettingsHash == hash && objects.size() < 1) {
		printf("Nothing has changed. Skipping link.\n");
		return;
	}
	std::cout << "Starting link:" << std::endl;
	if (!successfulCompile)
		return;
	string objs = "";
	for (auto &node : nodeMap) {
		objs += node.second->outname + " ";
	}
	string args = linkArgs;
	string out = outDir + linkOut;
	Helper::replaceAll(args, "$infile", objs);
	Helper::replaceAll(args, "$outfile", out);
	remove(string(out).c_str());
	//printf(args.c_str());
	std::system(args.c_str());
	ifstream f(string(out).c_str());
	if (f.good()) {
		// std::cout << "Link complete, took seconds." << std::endl;
		linkerSettingsHash = hash;
		f.close();
	}
	else {
		// std::cout << "Link failed." << std::endl;
		successfulLink = false;
		CleanExit();
	}
}

void BuildTool::doPackage() {
	if (package.size() <= 0)
		return;
	std::cout << "Starting package:" << std::endl;
	if (!successfulLink)
		return;
	string out = linkOut;
	for (auto cmd : package) {
		Helper::replaceAll(cmd, "$outfile", out);
		std::system(cmd.c_str());
	}
	// replaceAll(args, "$infile", objs);
	// std::system(args.c_str());
	successfulPackage = false;
}
void BuildTool::getPackage(string &str) {
	string var = "package:";
	int foundstart = str.find(var);
	string helper = "";
	if (foundstart >= 0) {
		int foundstart2 = foundstart + var.size();
		int foundend = str.find("}", foundstart2);
		helper = str.substr(foundstart2, foundend - foundstart2);
		std::vector<string> out = Helper::split(helper, '\n');
		if (out.size()) {         // check if there any elements in the vector array
			out.erase(out.begin()); // erase the firs element
		}
		package = out;
	}
}
void BuildTool::getArgs(string pn) {
	//fs::path p = "./args.txt";
	//std::ifstream t(p);
	string str = Helper::readfile("./args.txt");
	Helper::replaceVars(str);
	int foundstart = str.find("platform:");
	while (foundstart > 0) {
		foundstart += 9;
		int foundend = str.find("\n", foundstart);
		string found = str.substr(foundstart, foundend - foundstart);
		string platformName = found;
		str = str.substr(foundend + 1);
		/*if (platformName != pn) {
			foundstart = str.find("platform:");
			continue;
		}*/
		//foundend = str.find("platform:");
		//if (foundend >= 1)
			//str = str.substr(0, foundend - 1);
		compiler = Helper::getVar(str, "compiler:");
		linker = Helper::getVar(str, "linker:");
		outDir = Helper::getVar(str, "outdir:");
		debugDir = Helper::getVar(str, "debugdir:");
		linkOut = Helper::getVar(str, "outfile:");
		gcctoolchain = Helper::getVar(str, "gcc-toolchain:");
		sysroot = Helper::getVar(str, "sysroot:");
		sysrootLinker = Helper::getVar(str, "sysrootLinker:");
		defines = Helper::getList(str, "defines:", "-D");
		includeDirs = Helper::getList(str, "includes:", "-I");
		sourceDirs = Helper::getList(str, "sourceDirs:", "\n");
		if (linker == "lld-link")
			libDirs = Helper::getList(str, "libDirs:", "-libpath:");
		else
			libDirs = Helper::getList(str, "libDirs:", "-L");
		if (linker == "lld-link")
			libs = Helper::getList(str, "libs:", "-lib:");
		else
			libs = Helper::getList(str, "libs:", "-l");
		excludelibs = Helper::getList(str, "excludelibs:", "--exlude-libs");
		compileArgs = compiler + " " + Helper::getVar(str, "compileArgs:");
		compileModArgs = compiler + " " + Helper::getVar(str, "compileModArgs:");
		linkArgs = linker + " " + Helper::getVar(str, "linkArgs:");
		break;
	}
	compileArgs += " -fprebuilt-module-path=" + debugDir;
	compileModArgs += " -fprebuilt-module-path=" + debugDir;
	// printf(debugDir.c_str());
	Helper::replaceAll(compileArgs, "$includes", includeDirs);
	Helper::replaceAll(compileArgs, "$gcc-toolchain", gcctoolchain);
	Helper::replaceAll(compileArgs, "$sysroot", sysroot);
	Helper::replaceAll(compileArgs, "$defines", defines);
	Helper::replaceAll(compileArgs, "$defines", defines);
	Helper::replaceAll(compileModArgs, "$includes", includeDirs);
	Helper::replaceAll(compileModArgs, "$gcc-toolchain", gcctoolchain);
	Helper::replaceAll(compileModArgs, "$sysroot", sysroot);
	Helper::replaceAll(compileModArgs, "$defines", defines);
	Helper::replaceAll(compileModArgs, "$defines", defines);
	Helper::replaceAll(linkArgs, "$sysrootLinker", sysrootLinker);
	Helper::replaceAll(linkArgs, "$excludelibs", excludelibs);
	Helper::replaceAll(linkArgs, "$libs", libs);
	Helper::replaceAll(linkArgs, "$libDirs", libDirs);
	fs::path debugDirPath = debugDir;
	fs::path outDirPath = outDir;
	getPackage(str);
	if (!fs::exists(debugDirPath)) {
		printf("Debug directory does not exist:%s\n", debugDir.c_str());
		CleanExit();
	}
	if (!fs::exists(debugDirPath)) {
		printf("Out directory does not exist:%s\n", debugDir.c_str());
		CleanExit();
	}

}
#ifdef _DEBUG
#include <direct.h>  
#endif
void BuildTool::Clean() {
	for (auto &file : files) {
		fs::path p(file.first);
		std::string ext = p.extension().string();
		std::string f = p.filename().string();
		if (ext == ".cppm") {
			string out = debugDir + f.substr(0, f.size() - 4) + "pcm.o";
			remove(out.c_str());
		}
		else {
			string out = debugDir + f + ".o";
			remove(out.c_str());
		}
	}
	ofstream myfile;
	myfile.open("./Files.txt");
	myfile << "";
	myfile.close();
}
#define PRINT true
void BuildTool::Run(bool useVS, bool clean, bool rebuild) {
	printf("starting.\n");
#ifdef _DEBUG
	_chdir("../AndroidExampleProj/");
#endif
	files = getProjFiles();
	string platformString = "android:debug";
	Timer timers;
	timers.StartTimer(string("getArgs"));
	getArgs(platformString);
	timers.EndTimer(PRINT);

	if (clean) {
		timers.StartTimer(string("Clean"));
		Clean();
		if (!rebuild) {
			timers.EndTimer(PRINT);
			return;
		}
		timers.EndTimer(PRINT);
	}
	// Read cpp and cppm files from projfile
	timers.StartTimer(string("getFiles"));
	if (useVS)
		getFilesVS();
	else
		getFiles();
	timers.EndTimer(PRINT);
	timers.StartTimer(string("CompileNodes"));
	CompileNodes();
	timers.EndTimer(PRINT);
	printf("Compiled %d files.\n", filesCompiled);

	// Reaching IO limits, 4000 files faster than 500ms is not very likely without
	// using ramdisk then maybe expect 20-40% increase.  Have a chache server that
	// detects changes to files (and optionally compile them on change)

	timers.StartTimer(string("Link"));
	link();
	timers.EndTimer(PRINT);
	setProjFiles();
	doPackage();
}
