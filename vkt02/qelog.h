#pragma once

#include "qeheader.h"

enum QeDebugMode {
	eModeNoDebug,
	eModeConsole,
	eModeOutput,
	eModeConsoleOutput,
};

class QeLog
{
public:
	QeLog(QeGlobalKey& _key) {}
	~QeLog();

	QeDebugMode mode = eModeNoDebug;
	std::ofstream ofile;

	void initialize();
	std::string stack(int from, int to);
	void print(std::string& msg, bool bShowStack=false, int stackLevel=4);
	bool isDebug();
	bool isConsole();
	bool isOutput();
};