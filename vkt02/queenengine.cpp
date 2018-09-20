#include "qeheader.h"


void QueenEngine::run() {
	
	bClosed = false;
	bRestart = false;

	QeAssetXML* node = AST->getXML(AST->CONFIG);
	node = AST->getXMLNode(node, 1, "setting");

	DEBUG->initialize();

	WIN->initialize();
	VK->initialize();
	GRAP->initialize();

	if(sceneEID ==0)	AST->getXMLiValue(&sceneEID, node, 1, "startScene");
	FPSTimer.setTimer(1000 / std::stoi(AST->getXMLValue(node, 2, "envir", "FPS")));

	node = AST->getXMLEditNode("scenes", sceneEID);
	SCENE->clear();
	SCENE->initialize(node);

	mainLoop();
}

void QueenEngine::mainLoop() {
	while (!bClosed && !bRestart) {

		int passMilliSecond;
		if (FPSTimer.checkTimer(passMilliSecond)) {

			currentFPS = 1000 / passMilliSecond;
			deltaTime = float(passMilliSecond) / 1000;

			VK->update1();
			GRAP->update1();
			WIN->update1();
			SCENE->update1();
			//OBJMGR->update1();

			WIN->update2();
			SCENE->update2();
			//OBJMGR->update2();
			GRAP->update2();
			VK->update2();
		}
	}
	vkDeviceWaitIdle(VK->device);
}