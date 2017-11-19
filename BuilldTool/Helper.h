#pragma once
#include "xxhash.h"
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <sstream>
#include <unordered_map>
#include <windows.h>
using namespace std::chrono;
using std::ifstream;
using std::ofstream;
using std::string;
using std::tuple;
using tp = std::chrono::high_resolution_clock::time_point;
tp t1, t2;
namespace fs = std::experimental::filesystem;
struct Helper {
	static void replaceVars(string &str) {
		string helper = "";
		int foundstart = str.find("var ");
		while (foundstart > 0) {
			foundstart += 4;
			helper = str.substr(foundstart);
			int foundend = helper.find("\n");
			if (foundend > 0)
				helper = helper.substr(0, foundend);
			string name = helper.substr(0, helper.find(":"));
			string val = helper.substr(helper.find(":") + 1, helper.size());
			replaceAll(str, "var " + helper, "");
			replaceAll(str, "$" + name, val);
			foundstart = str.find("var ");
		}
	}
	static bool getBetween(std::string& str, std::string& replaced, const std::string& s, const std::string& e) {
		if (str.empty())
			return false;
		int foundStart = str.find(s);
		if (foundStart > -1) {
			int foundEnd = str.substr(foundStart).find(e);
			if (foundEnd > -1) {
				int start = foundStart + s.size();
				int end = foundEnd - s.size();
				replaced = str.substr(start, end);
				str.replace(foundStart, foundEnd, "");
				return true;
			}
		}
		return false;
	}
	static void replaceAll(std::string& str, const std::string& from, const std::string& to) {
		if (from.empty())
			return;
		int start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
		}
	}
	static std::vector<std::string> split(const std::string& s, const char& c) {
		std::string buff{ "" };
		std::vector<std::string> v;

		for (auto n : s) {
			if (n != c) buff += n; else
				if (n == c) {
					v.push_back(buff);
					buff = "";
				}
		}
		if (buff != "") v.push_back(buff);

		return v;
	};
	static std::string hashfile(const char *filename) {
		std::ifstream fp(filename);
		std::stringstream ss;

		// Unable to hash file, return an empty hash.
		if (!fp.is_open()) {
			return "";
		}

		// Hashing
		uint32_t magic = 5381;
		char c;
		while (fp.get(c)) {
			magic = ((magic << 5) + magic) + c; // magic * 33 + c
		}

		ss << std::hex << std::setw(8) << std::setfill('0') << magic;
		return ss.str();
	}
	static string getVar(string &str, const string &var) {
		int foundstart = str.find(var);
		string out = "";
		if (foundstart >= 0) {
			int foundstart2 = foundstart + var.size();
			int foundend = str.find("\n", foundstart2);
			out = str.substr(foundstart2, foundend - foundstart2 - 1);
			str.erase(foundstart, foundend - foundstart + 1);
		}
		return out;
	}
	static string getList(string &str, const string &var, const string &pp) {
		int foundstart = str.find(var);
		string out = "";
		if (foundstart >= 0) {
			int foundstart2 = foundstart + var.size();
			int foundend = str.find("}", foundstart2);
			out = str.substr(foundstart2, foundend - foundstart2);
			replaceAll(out, "\r", "");
			std::vector<string> helper = split(out, '\n');
			out = "";
			for (int i = 1; i < helper.size(); ++i) {
				string checkExtension = helper[i];
				int found = checkExtension.find(".");
				if (found >= 1 && found > checkExtension.size() - 3) {
					if (checkExtension.find("\\") > 0 || checkExtension.find("/") > 0)
						out += helper[i] + " ";
				}
				else
					out += pp + helper[i] + " ";
			}
			if (out.size() > 0)
				out = out.substr(0, out.size() - 1);
			str.erase(foundstart, foundend - foundstart + 1);
		}

		return out;
	}

	static string readfile(const char *filename) {
		string buffer;
		if (std::ifstream t{ filename, std::ifstream::binary }) {
			t.seekg(0, std::ios::end);
			buffer.resize(t.tellg());
			t.seekg(0);
			t.read(buffer.data(), buffer.size());
		}
		return buffer;
	}
};