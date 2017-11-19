#pragma once
#include <string>
#include <vector>

enum class NodeType {
	c, cpp, cppm
};
struct Node {
	std::string name;
	std::string outname;
	std::string src;
	NodeType type = NodeType::cppm;
	bool needsCompilation = true;
	bool compiled = false;
	std::string hash;
	std::vector<Node*> children;
	std::vector<Node*> parents;
	bool forceRecompile = false;

};