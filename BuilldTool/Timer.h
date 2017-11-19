#pragma once
struct timerInfo {
	tp start;
	tp end;
	int time;
	bool running = true;
};
std::unordered_map<string, timerInfo> timers;
string timerStr = "";
void StartTimer(string &str) {
	timerStr = str;
	timers[str].start = high_resolution_clock::now();
}
auto EndTimer() {
	auto t = high_resolution_clock::now();
	auto time_span =
		duration_cast<std::chrono::milliseconds>(t - timers[timerStr].start);
	timers[timerStr].running = false;
	return std::to_string(timers[timerStr].time = time_span.count());
}
void PrintTimer() {
	auto got = timers.find(timerStr);
	if (got == timers.end())
		return;
	if (got->second.running)
		return;
	printf("%s took %d milliseconds.\n", timerStr.c_str(), got->second.time);
}
void PrintTimer(const string &str) {
	auto got = timers.find(str);
	if (got == timers.end())
		return;
	if (got->second.running)
		return;
	printf("%s took %d milliseconds.\n", str.c_str(), got->second.time);
}
void PrintAllTimers() {
	for (auto &timer : timers) {
		PrintTimer(timer.first);
	}
}
