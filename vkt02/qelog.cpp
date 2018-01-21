#include "qelog.h"



void QeLog::init()
{
	mode = QeDebugMode(atoi(AST->getXMLValue(2, AST->CONFIG, "debug")));

	if (mode == eModeOutput) {
		time_t rawtime;
		struct tm timeinfo;
		char buffer[128];

		time(&rawtime);
		localtime_s(&timeinfo, &rawtime);

		strftime(buffer, sizeof(buffer), "%y%m%d%I%M%S", &timeinfo);
		outputPath = AST->getXMLValue(3, AST->CONFIG, "path", "log");
		outputPath += "log";
		outputPath += buffer;
		outputPath += ".txt";
	}
}

bool QeLog::isDebug()	{	return mode == eModeNoDebug ? false : true;	}
bool QeLog::isConsole() {	return mode == eModeConsole ? true	: false; }


void QeLog::print(std::string& msg) {

	if (mode == eModeNoDebug) return;

	time_t rawtime;
	struct tm timeinfo;
	char buffer[128];

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);

	strftime(buffer, sizeof(buffer), "%y%m%d%I%M%S: ", &timeinfo);
	std::string s = buffer;
	s += msg;

	if (mode == eModeConsole)		std::cout << s.c_str() << std::endl;
	else if (mode == eModeOutput) {

		std::fstream ffile(outputPath);
		if (ffile.is_open()) {
			ffile.seekg(0, ffile.end);
			ffile << s << std::endl;
			ffile.close();
		}
		else {
			std::ofstream ofile(outputPath);
			if (ofile.is_open()) {
				ofile << s << std::endl;
				ofile.close();
			}
		}
	}
}
