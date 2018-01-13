#include "qeheader.h"

void QeLight::init() {
	data.pos	= QeVector4f(0, 0, 0, 1);
	data.dir	= QeVector4f(1, 1, 1, 1);
	data.color	= QeVector4f(1, 1, 1, 1);
	data.param	= QeVector4f(0, 1, 1, 1);
	rotateCenter= QeVector3f(0, 0, 0);
}

void QeLight::setProperty(QeAssetXML* _property) {

	const char* c;

	c = AST->getXMLValue(_property, 1, "r");
	if (c != nullptr)	data.color.x = float(atof(c));
	c = AST->getXMLValue(_property, 1, "g");
	if (c != nullptr)	data.color.y = float(atof(c));
	c = AST->getXMLValue(_property, 1, "b");
	if (c != nullptr)	data.color.z = float(atof(c));

	c = AST->getXMLValue(_property, 1, "show");
	if (c != nullptr && atoi(c) == 1) {
		billboard = OBJMGR->getBillboard(0);
		billboard->modelData->pMaterial->value.diffuse = data.color;
	}

	c = AST->getXMLValue(_property, 1, "posX");
	if (c != nullptr)	data.pos.x = float(atof(c));
	c = AST->getXMLValue(_property, 1, "posY");
	if (c != nullptr)	data.pos.y = float(atof(c));
	c = AST->getXMLValue(_property, 1, "posZ");
	if (c != nullptr)	data.pos.z = float(atof(c));

	c = AST->getXMLValue(_property, 1, "rotateCenterX");
	if (c != nullptr)	rotateCenter.x = float(atof(c));
	c = AST->getXMLValue(_property, 1, "rotateCenterY");
	if (c != nullptr)	rotateCenter.y = float(atof(c));
	c = AST->getXMLValue(_property, 1, "rotateCenterZ");
	if (c != nullptr)	rotateCenter.z = float(atof(c));

	c = AST->getXMLValue(_property, 1, "dirX");
	if (c != nullptr)	data.dir.x = float(atof(c));
	c = AST->getXMLValue(_property, 1, "dirY");
	if (c != nullptr)	data.dir.y = float(atof(c));
	c = AST->getXMLValue(_property, 1, "dirZ");
	if (c != nullptr)	data.dir.z = float(atof(c));

	c = AST->getXMLValue(_property, 1, "speed");
	if (c != nullptr)	speed = atoi(c);

	c = AST->getXMLValue(_property, 1, "type");
	if (c != nullptr)	data.param.x = float(atoi(c));
	c = AST->getXMLValue(_property, 1, "intensity");
	if (c != nullptr)	data.param.y = float(atof(c));
	c = AST->getXMLValue(_property, 1, "coneAngle");
	if (c != nullptr)	data.param.z = float(atof(c));
}

void QeLight::update(float time) {

	if (speed != 0) {
		float angle = -time * speed;
		QeMatrix4x4f mat;
		mat *= MATH->rotateZ(angle);
		QeVector4f pos = data.pos - rotateCenter;
		pos = mat*pos;
		data.pos = pos + rotateCenter;
	}
	if (billboard != nullptr) billboard->pos = data.pos;
}