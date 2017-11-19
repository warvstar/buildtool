#include "BuildTool.h"
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char *argv[]) {
	bool useVS = false;
	bool clean = false;
	bool rebuild = false;
	std::string current_exec_name = argv[0]; // Name of the current exec program
	std::string first_arg;
	std::vector<std::string> all_args;
	if (argc > 1) {
		first_arg = argv[1];
		all_args.assign(argv + 1, argv + argc);
	}
	for (auto &arg : all_args) {
		if (arg == "vs")
			useVS = true;
		if (arg == "clean")
			clean = true;
	}
	BuildTool buildtool;
	buildtool.Run(useVS, clean, rebuild);
	return 0;
}