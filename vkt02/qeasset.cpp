#include "qeheader.h"


QeAssetModel::~QeAssetModel() {	pMaterial = nullptr;	}

QeAssetXML::~QeAssetXML() {

	std::vector<QeAssetXML*>::iterator it = nexts.begin();
	while (it != nexts.end()) {
		if ((*it) != nullptr) delete (*it);
		++it;
	}
	nexts.clear();
}

QeAssetJSON::~QeAssetJSON() {

	std::vector<QeAssetJSON*>::iterator it = eNodes.begin();
	while (it != eNodes.end()) {
		if ((*it) != nullptr) delete (*it);
		++it;
	}
	eNodes.clear();

	std::vector<std::vector<QeAssetJSON*>>::iterator it1 = eArrayNodes.begin();
	while (it1 != eArrayNodes.end()) {
		it = it1->begin();
		while (it != it1->end()) {
			if ((*it) != nullptr) delete (*it);
			++it;
		}
		++it1;
	}
	eArrayNodes.clear();
}

QeAsset::~QeAsset() {

	std::map<std::string, QeAssetXML*>::iterator it = astXMLs.begin();
	while (it != astXMLs.end()) {
		if ((it->second) != nullptr) delete (it->second);
		++it;
	}
	astXMLs.clear();

	std::map<std::string, QeAssetJSON*>::iterator it1 = astJSONs.begin();
	while (it1 != astJSONs.end()) {
		if ((it1->second) != nullptr) delete (it1->second);
		++it1;
	}
	astJSONs.clear();

	std::map<std::string, QeAssetModel*>::iterator it2 = astModels.begin();
	while (it2 != astModels.end()) {
		if ((it2->second) != nullptr) delete (it2->second);
		++it2;
	}
	astModels.clear();

	std::map<std::string, QeAssetMaterial*>::iterator it3 = astMaterials.begin();
	while (it3 != astMaterials.end()) {
		if ((it3->second) != nullptr) delete (it3->second);
		++it3;
	}
	astMaterials.clear();

	std::map<std::string, VkShaderModule>::iterator it4 = astShaders.begin();
	while (it4 != astShaders.end()) {
		vkDestroyShaderModule(VK->device, it4->second, nullptr);
		++it4;
	}
	astShaders.clear();

	std::map<std::string, QeVKImage*>::iterator it5 = astTextures.begin();
	while (it5 != astTextures.end()) {
		if ((it5->second) != nullptr) delete (it5->second);
		++it5;
	}
	astTextures.clear();
}

VkVertexInputBindingDescription QeVertex::getBindingDescription() {
	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(QeVertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 7> QeVertex::getAttributeDescriptions() {
	std::array<VkVertexInputAttributeDescription, 7> attributeDescriptions = {};

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(QeVertex, pos);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(QeVertex, color);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(QeVertex, uv);

	attributeDescriptions[3].binding = 0;
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[3].offset = offsetof(QeVertex, normal);

	attributeDescriptions[4].binding = 0;
	attributeDescriptions[4].location = 4;
	attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[4].offset = offsetof(QeVertex, tangent);

	attributeDescriptions[5].binding = 0;
	attributeDescriptions[5].location = 5;
	attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[5].offset = offsetof(QeVertex, joint);

	attributeDescriptions[6].binding = 0;
	attributeDescriptions[6].location = 6;
	attributeDescriptions[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[6].offset = offsetof(QeVertex, weight);

	return attributeDescriptions;
}

bool QeVertex::operator==(const QeVertex& other) const {
	return pos == other.pos && normal == other.normal && uv == other.uv && color == other.color;
}

std::vector<char> QeAsset::loadFile(const char* _filePath) {

	std::vector<char> ret;
	std::ifstream file(_filePath, std::ios::ate | std::ios::binary);
	if (!file.is_open()) return ret;

	file.seekg(0, file.end);
	int length = int(file.tellg());
	file.seekg(0);

	ret.resize(length);
	file.read(ret.data(), length);
	file.close();
	return ret;
}

QeAssetJSON* QeAsset::getJSON(const char* _filePath) {
	std::map<std::string, QeAssetJSON*>::iterator it = astJSONs.find(_filePath);

	if (it != astJSONs.end())	return it->second;

	std::vector<char> buffer = loadFile(_filePath);

	int index = 0;
	QeAssetJSON* head = ENCODE->decodeJSON(buffer.data(), index);
	astJSONs[_filePath] = head;
	return head;
}

const char*	 QeAsset::getJSONValue(int length, ...) {
	va_list keys;
	va_start(keys, length);

	const char* key = va_arg(keys, const char*);
	QeAssetJSON* source = getJSON(key);
	if (source == nullptr) {
		va_end(keys);
		return nullptr;
	}
	const char** keys1 = new const char*[length - 1];
	for (int i = 0; i<(length - 1); ++i)	keys1[i] = va_arg(keys, const char*);

	const char* ret = getJSONValue(source, keys1, length);
	va_end(keys);
	delete[] keys1;
	return ret;
}

const char*	 QeAsset::getJSONValue(QeAssetJSON* source, int length, ...) {
	if (source == nullptr)	return nullptr;

	va_list keys;
	va_start(keys, length);

	const char** keys1 = new const char*[length];
	for (int i = 0; i< length; ++i)	keys1[i] = va_arg(keys, const char*);

	const char* ret = getJSONValue(source, keys1, length);
	va_end(keys);
	delete[] keys1;
	return ret;
}

const char*	 QeAsset::getJSONValue(QeAssetJSON* source, const char* keys[], int length) {
	if (source == nullptr) return nullptr;

	for (int index = 0; index < length; ++index) {

		if (index == (length - 1)) {
			int size = int(source->eKeysforValues.size());
			for (int index1 = 0; index1 < size; ++index1){
				if (strcmp(keys[index], source->eKeysforValues[index1].c_str()) == 0)
					return source->eValues[index1].c_str();
			}
			break;
		}
	
		int size = int(source->eKeysforNodes.size());
		int index1 = 0;
		for (; index1<size; ++index1) {
			if (strcmp(keys[index], source->eKeysforNodes[index1].c_str()) == 0) {
				source = source->eNodes[index1];
				break;
			}
		}
		if (index1 != size) continue;
		
		size = int(source->eKeysforArrayNodes.size());
		index1 = 0;
		for (; index1<size; ++index1) {
			if (strcmp(keys[index], source->eKeysforArrayNodes[index1].c_str()) == 0) {
				int size2 = int(source->eArrayNodes[index1].size());
				int index2 = 0;
				for (; index2<size2;++index2 ) {
					const char* ret = getJSONValue(source->eArrayNodes[index1][index2], &keys[index+1], length - index-1);
					if (ret != nullptr) return ret;
				}
				break;
			}
		}
		break;
	}
	return nullptr;
}

QeAssetJSON* QeAsset::getJSONNode(int length, ...) {
	va_list keys;
	va_start(keys, length);

	const char* key = va_arg(keys, const char*);
	QeAssetJSON* source = getJSON(key);
	if (source == nullptr) {
		va_end(keys);
		return nullptr;
	}
	const char** keys1 = new const char*[length - 1];
	for (int i = 0; i<(length - 1); ++i)	keys1[i] = va_arg(keys, const char*);

	source = getJSONNode(source, keys1, length);
	va_end(keys);
	delete[] keys1;
	return source;
}

QeAssetJSON* QeAsset::getJSONNode(QeAssetJSON* source, int length, ...) {

	if (source == nullptr)	return nullptr;

	va_list keys;
	va_start(keys, length);

	const char** keys1 = new const char*[length];
	for (int i = 0; i< length; ++i)	keys1[i] = va_arg(keys, const char*);

	source = getJSONNode(source, keys1, length);
	va_end(keys);
	delete[] keys1;
	return source;
}

QeAssetJSON* QeAsset::getJSONNode(QeAssetJSON* source, const char* keys[], int length) {
	if (source == nullptr) return nullptr;

	int size = int(source->eKeysforNodes.size());
	for (int index = 0; index < size; ++index) {
		if (strcmp(keys[0], source->eKeysforNodes[index].c_str()) == 0){
			
			if(length == 1)	return source->eNodes[index];
			
			return getJSONNode(source->eNodes[index], &keys[1], length - 1);
		}
	}

	size = int(source->eKeysforArrayNodes.size());
	for (int index = 0; index<size; ++index) {
		if (strcmp(keys[0], source->eKeysforArrayNodes[index].c_str()) == 0) {

			if ( length == 1 ) return source->eArrayNodes[index][0];

			int size1 = int(source->eArrayNodes[index].size());
		
			for (int index1 = 0; index1<size1; ++index1) {
				QeAssetJSON* ret = getJSONNode(source->eArrayNodes[index][index1], &keys[1], length- 1);
				if (ret != nullptr) return ret;
			}
			break;
		}
	}
	return nullptr;
}

std::vector<std::string>*	QeAsset::getJSONArrayValues(int length, ...) {
	va_list keys;
	va_start(keys, length);

	const char* key = va_arg(keys, const char*);
	QeAssetJSON* source = getJSON(key);
	if (source == nullptr) {
		va_end(keys);
		return nullptr;
	}
	const char** keys1 = new const char*[length - 1];
	for (int i = 0; i<(length - 1); ++i)	keys1[i] = va_arg(keys, const char*);

	std::vector<std::string>* ret = getJSONArrayValues(source, keys1, length);
	va_end(keys);
	delete[] keys1;
	return ret;
}

std::vector<std::string>*	QeAsset::getJSONArrayValues(QeAssetJSON* source, int length, ...) {
	if (source == nullptr)	return nullptr;

	va_list keys;
	va_start(keys, length);

	const char** keys1 = new const char*[length];
	for (int i = 0; i< length; ++i)	keys1[i] = va_arg(keys, const char*);

	std::vector<std::string>* ret = getJSONArrayValues(source, keys1, length);
	va_end(keys);
	delete[] keys1;
	return ret;
}

std::vector<std::string>*	QeAsset::getJSONArrayValues(QeAssetJSON* source, const char* keys[], int length) {
	if (source == nullptr) return nullptr;

	for (int index = 0; index < length; ++index) {

		if (index == (length - 1)) {
			int size = int(source->eKeysforArrayValues.size());
			for (int index1 = 0; index1 < size; ++index1) {
				if (strcmp(keys[index], source->eKeysforArrayValues[index1].c_str()) == 0)
					return &source->eArrayValues[index1];
			}
			break;
		}

		int size = int(source->eKeysforNodes.size());
		int index1 = 0;
		for (; index1<size; ++index1) {
			if (strcmp(keys[index], source->eKeysforNodes[index1].c_str()) == 0) {
				source = source->eNodes[index1];
				break;
			}
		}
		if (index1 != size) continue;

		size = int(source->eKeysforArrayNodes.size());
		index1 = 0;
		for (; index1<size; ++index1) {
			if (strcmp(keys[index], source->eKeysforArrayNodes[index1].c_str()) == 0) {
				int size2 = int(source->eArrayNodes[index1].size());
				int index2 = 0;
				for (; index2<size2; ++index2) {
					std::vector<std::string>* ret = getJSONArrayValues(source->eArrayNodes[index1][index2], &keys[index + 1], length - index - 1);
					if (ret != nullptr) return ret;
				}
				break;
			}
		}
		break;
	}
	return nullptr;
}

std::vector<QeAssetJSON*>*	QeAsset::getJSONArrayNodes(int length, ...) {
	va_list keys;
	va_start(keys, length);

	const char* key = va_arg(keys, const char*);
	QeAssetJSON* source = getJSON(key);
	if (source == nullptr) {
		va_end(keys);
		return nullptr;
	}
	const char** keys1 = new const char*[length - 1];
	for (int i = 0; i<(length - 1); ++i)	keys1[i] = va_arg(keys, const char*);

	std::vector<QeAssetJSON*>* ret = getJSONArrayNodes(source, keys1, length);
	va_end(keys);
	delete[] keys1;
	return ret;
}

std::vector<QeAssetJSON*>*	QeAsset::getJSONArrayNodes(QeAssetJSON* source, int length, ...) {
	if (source == nullptr)	return nullptr;

	va_list keys;
	va_start(keys, length);

	const char** keys1 = new const char*[length];
	for (int i = 0; i< length; ++i)	keys1[i] = va_arg(keys, const char*);

	std::vector<QeAssetJSON*>* ret = getJSONArrayNodes(source, keys1, length);
	va_end(keys);
	delete[] keys1;
	return ret;
}

std::vector<QeAssetJSON*>*	QeAsset::getJSONArrayNodes(QeAssetJSON* source, const char* keys[], int length) {
	if (source == nullptr) return nullptr;

	int size = int(source->eKeysforArrayNodes.size());
	for (int index = 0; index < size; ++index) {
		if (strcmp(keys[0], source->eKeysforArrayNodes[index].c_str()) == 0) {

			if (length == 1) return &source->eArrayNodes[index];

			int size1 = int(source->eArrayNodes[index].size());

			for (int index1 = 0; index1 < size1; ++index1) {
				std::vector<QeAssetJSON*>* ret = getJSONArrayNodes(source->eArrayNodes[index][index1], &keys[1], length - 1);
				if (ret != nullptr) return ret;
			}
			break;
		}
	}

	if (length>1) {
		size = int(source->eKeysforNodes.size());
		for (int index = 0; index < size; ++index) {
			if (strcmp(keys[0], source->eKeysforNodes[index].c_str()) == 0) {

				return getJSONArrayNodes(source->eNodes[index], &keys[1], length - 1);
			}
		}
	}
	return nullptr;
}

bool QeAsset::getJSONbValue(bool* output, QeAssetJSON* source, int length, ...) {
	va_list keys;
	va_start(keys, length);

	const char** keys1 = new const char*[length];
	for (int i = 0; i< length; ++i)	keys1[i] = va_arg(keys, const char*);

	const char* ret = getJSONValue(source, keys1, length + 1);
	va_end(keys);
	delete[] keys1;

	if (ret) *output = atoi(ret);

	return ret;
}

bool QeAsset::getJSONiValue(int* output, QeAssetJSON* source, int length, ...) {
	va_list keys;
	va_start(keys, length);

	const char** keys1 = new const char*[length];
	for (int i = 0; i< length; ++i)	keys1[i] = va_arg(keys, const char*);

	const char* ret = getJSONValue(source, keys1, length + 1);
	va_end(keys);
	delete[] keys1;

	if (ret) *output = atoi(ret);

	return ret;
}

bool QeAsset::getJSONfValue(float* output, QeAssetJSON* source, int length, ...) {
	va_list keys;
	va_start(keys, length);

	const char** keys1 = new const char*[length];
	for (int i = 0; i< length; ++i)	keys1[i] = va_arg(keys, const char*);

	const char* ret = getJSONValue(source, keys1, length + 1);
	va_end(keys);
	delete[] keys1;

	if (ret) *output = float(atof(ret));

	return ret;
}

QeAssetXML* QeAsset::getXML(const char* _filePath) {
	std::map<std::string, QeAssetXML*>::iterator it = astXMLs.find(_filePath);

	if (it != astXMLs.end())	return it->second;

	//std::ifstream file(_filePath, std::ios::ate | std::ios::binary);
	//if (!file.is_open()) return nullptr;

	std::vector<char> buffer = loadFile(_filePath);

	int index = 0;
	QeAssetXML* head = ENCODE->decodeXML(buffer.data(), index);
	astXMLs[_filePath] = head;

	return head;
}

const char* QeAsset::getXMLValue(int length, ...) {

	va_list keys;
	va_start(keys, length);

	const char* key = va_arg(keys, const char*);
	QeAssetXML* source = getXML(key);
	if (source == nullptr) {
		va_end(keys);
		return nullptr;
	}
	const char** keys1 = new const char*[length-1];
	for (int i = 0; i<(length-1); ++i )	keys1[i] = va_arg(keys, const char*);

	const char* ret = getXMLValue(source, keys1, length);
	va_end(keys);
	delete[] keys1;
	return ret;
}

const char* QeAsset::getXMLValue(QeAssetXML* source, int length, ...) {
	
	if (source == nullptr)	return nullptr;
	
	va_list keys;
	va_start(keys, length);

	const char** keys1 = new const char*[length];
	for (int i = 0; i< length; ++i)	keys1[i] = va_arg(keys, const char*);

	const char* ret = getXMLValue(source, keys1, length+1);
	va_end(keys);
	delete[] keys1;
	return ret;
}

const char* QeAsset::getXMLValue(QeAssetXML* source, const char* keys[], int length) {

	if (source == nullptr) return nullptr;

	for (int index = 0; index < length; ++index) {

		if (index == (length - 1)) {
			if (strcmp(keys[index-1], source->key.c_str()) == 0) return source->value.c_str();
			break;
		}
		else if (index == (length - 2)) {
			int size = int(source->eKeys.size());
			for (int index1 = 0; index1 < size; ++index1)
				if (strcmp(keys[index], source->eKeys[index1].c_str()) == 0) {
					return source->eVaules[index1].c_str();
				}
		}

		int size = int(source->nexts.size());
		int index1 = 0;
		for (; index1<size; ++index1) {
			if (strcmp(keys[index], source->nexts[index1]->key.c_str()) == 0) {
				source = source->nexts[index1];
				break;
			}
		}
		if (index1 == size) break;
	}
	return nullptr;
}

QeAssetXML* QeAsset::getXMLNode(int length, ...) {

	va_list keys;
	va_start(keys, length);

	const char* key = va_arg(keys, const char*);
	QeAssetXML* source = getXML(key);
	if (source == nullptr) {
		va_end(keys);
		return nullptr;
	}
	length--;
	const char** keys1 = new const char*[length];
	for (int i = 0; i<length; ++i )	keys1[i] = va_arg(keys, const char*);

	source = getXMLNode(source, keys1, length);
	va_end(keys);
	delete[] keys1;
	return source;
}

QeAssetXML* QeAsset::getXMLNode(QeAssetXML* source, int length, ...) {

	if (source == nullptr)	return nullptr;
	
	va_list keys;
	va_start(keys, length);

	const char** keys1 = new const char*[length];
	for (int i = 0; i<length; ++i)	keys1[i] = va_arg(keys, const char*);

	source = getXMLNode(source, keys1, length);
	va_end(keys);
	delete[] keys1;
	return source;
}

QeAssetXML* QeAsset::getXMLNode(QeAssetXML* source, const char* keys[], int length) {

	if (source == nullptr)	return nullptr;

	for (int index = 0; index < length; ++index) {

		int size = int(source->nexts.size());
		int index1 = 0;
		for (; index1<size; ++index1) {
			if (strcmp(keys[index], source->nexts[index1]->key.c_str()) == 0) {
				source = source->nexts[index1];
				break;
			}
		}
		if (index1 == size) return nullptr;
	}
	return source;
}

const char* QeAsset::getEditType(QeObjectType type) {
	
	const char* out=nullptr;
	switch (type) {
	case eObject_Point:
		out = "points";
		break;
	case eObject_Camera:
		out = "cameras";
		break;
	case eObject_Light:
		out = "lights";
		break;
	case eObject_Line:
		out = "lines";
		break;
	case eObject_Billboard:
		out = "billboards";
		break;
	case eObject_Cubemap:
		out = "cubemaps";
		break;
	case eObject_Particle:
		out = "particles";
		break;
	case eObject_Model:
		out = "models";
		break;
	case eObject_Scene:
		out = "scenes";
		break;
	case eObject_Render:
		out = "models";
	}
	return out;
}


QeAssetXML* QeAsset::getXMLEditNode(QeObjectType type, int eid) {
	
	const char* cType = getEditType(type);

	if (!cType || strlen(cType) == 0) return nullptr;
	QeAssetXML* node = getXMLNode( 2, CONFIG, cType);

	if (node != nullptr && node->nexts.size() > 0) {
		for (int index = 0; index < node->nexts.size(); ++index) {
			int _eid=0;
			AST->getXMLiValue(&_eid, node->nexts[index], 1, "eid");
			if (_eid == eid) return node->nexts[index];
		}
	}
	return nullptr;
}


bool QeAsset::getXMLbValue(bool* output, QeAssetXML* source, int length, ...) {

	va_list keys;
	va_start(keys, length);

	const char** keys1 = new const char*[length];
	for (int i = 0; i< length; ++i)	keys1[i] = va_arg(keys, const char*);

	const char* ret = getXMLValue(source, keys1, length + 1);
	va_end(keys);
	delete[] keys1;

	if (ret) *output = atoi(ret);

	return ret;
}

bool QeAsset::getXMLiValue(int* output, QeAssetXML* source, int length, ...) {

	va_list keys;
	va_start(keys, length);

	const char** keys1 = new const char*[length];
	for (int i = 0; i< length; ++i)	keys1[i] = va_arg(keys, const char*);

	const char* ret = getXMLValue(source, keys1, length + 1);
	va_end(keys);
	delete[] keys1;

	if (ret) *output = atoi(ret);

	return ret;
}

bool QeAsset::getXMLfValue(float* output, QeAssetXML* source, int length, ...) {
	va_list keys;
	va_start(keys, length);

	const char** keys1 = new const char*[length];
	for (int i = 0; i< length; ++i)	keys1[i] = va_arg(keys, const char*);

	const char* ret = getXMLValue(source, keys1, length + 1);
	va_end(keys);
	delete[] keys1;

	if (ret) *output = float(atof(ret));

	return ret;
}

QeAssetModel* QeAsset::getModel(const char* _filename, bool bCubeMap) {

	std::string _filePath = combinePath(_filename, eAssetModel);
	std::map<std::string, QeAssetModel*>::iterator it = astModels.find(_filePath);

	if (it != astModels.end())	return it->second;

	char type = 0;

	if (strcmp("cube", _filename)==0)	type = eModelData_cube;
	else if (strcmp("axis", _filename) == 0)	type = eModelData_axis;
	else if (strcmp("grids", _filename) == 0)	type = eModelData_grids;
	else if (strcmp("line", _filename) == 0 )	type = eModelData_line;
	else if (strcmp("plane", _filename) == 0)	type = eModelData_plane;

	else {

		char *ret = strrchr((char*)_filename, '.');

		//if (strcmp(ret + 1, "obj") == 0)		type = 0;
		//else 
			if (strcmp(ret + 1, "gltf") == 0)	type = eModelData_gltf;
		//else if (strcmp(ret + 1, "glb") == 0)	type = 2;
	}

	QeAssetModel* model = nullptr;
	QeAssetJSON *json = nullptr;
	std::vector<char> buffer;
	QeVertex vertex;
	int index = 0;

	switch (type) {
	//case 0:
		//buffer = loadFile(_filePath.c_str());
		//model = ENCODE->decodeOBJ(buffer.data()); 
	//	break;
	case eModelData_gltf:
		json = getJSON(_filePath.c_str());
		model = ENCODE->decodeGLTF(json, bCubeMap);
		break;
	//case 2:
		//	model = ENCODE->decodeGLB(0);		
	//	break;

	case eModelData_cube:
		/*model = new QeAssetModel();
		model->scale = { 1,1,1 };
		model->indices = { 1,3,0,7,5,4,4,1,0,5,2,1,2,7,3,0,7,4,1,2,3,7,6,5,4,5,1,5,6,2,2,6,7,0,3,7 };

		model->indexSize = int(model->indices.size());
		vertex.uv = { 0,0,0,0 };
		vertex.color = { 1,1,1,1 };

		vertex.pos = { 1, -1, -1,1 };
		vertex.normal = { 0, 0, -1,1 };
		model->vertices.push_back(vertex);

		vertex.pos = { 1, -1, 1,1 };
		vertex.normal = { 1, 0, 0,1 };
		model->vertices.push_back(vertex);

		vertex.pos = { -1, -1, 1,1 };
		vertex.normal = { -1, 0, 0,1 };
		model->vertices.push_back(vertex);

		vertex.pos = { -1, -1, -1,1 };
		vertex.normal = { 0, 0, -1,1 };
		model->vertices.push_back(vertex);

		vertex.pos = { 1, 1, -1,1 };
		vertex.normal = { 1, 0, 0,1 };
		model->vertices.push_back(vertex);

		vertex.pos = { 1, 1, 1,1 };
		vertex.normal = { 0, 0, 1,1 };
		model->vertices.push_back(vertex);

		vertex.pos = { -1, 1, 1,1 };
		vertex.normal = { -1, 0, 0,1 };
		model->vertices.push_back(vertex);

		vertex.pos = { -1, 1, -1,1 };
		vertex.normal = { 0, 0, -1,1 };
		model->vertices.push_back(vertex);
		break;
		*/
		model = new QeAssetModel();
		model->scale = { 1,1,1 };
		model->indices = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35 };

		model->indexSize = int(model->indices.size());
		vertex.color = {1,1,1,1};
		
		//
		vertex.normal = { 0, 0, -1,1 };
		vertex.pos = { 1, -1, -1,1 };
		vertex.uv = { 0,0,0,0 };
		model->vertices.push_back(vertex);
		vertex.pos = { -1, 1, -1,1 };
		vertex.uv = { 1,1,0,0 };
		model->vertices.push_back(vertex);
		vertex.pos = { 1, 1, -1,1 };
		vertex.uv = { 1,0,0,0 };
		model->vertices.push_back(vertex);

		vertex.pos = { 1, -1, -1,1 };
		vertex.uv = { 0,0,0,0 };
		model->vertices.push_back(vertex);
		vertex.pos = { -1, -1, -1,1 };
		vertex.uv = { 0,1,0,0 };
		model->vertices.push_back(vertex);
		vertex.pos = { -1, 1, -1,1 };
		vertex.uv = { 1,1,0,0 };
		model->vertices.push_back(vertex);
		//
		vertex.normal = { 0, 0, 1,1 };
		vertex.pos = { -1, 1, 1,1 };
		vertex.uv = { 0,0,0,0 };
		model->vertices.push_back(vertex);
		vertex.pos = { 1, -1, 1,1 };
		vertex.uv = { 1,1,0,0 };
		model->vertices.push_back(vertex);
		vertex.pos = { 1, 1, 1,1 };
		vertex.uv = { 1,0,0,0 };
		model->vertices.push_back(vertex);

		vertex.pos = { -1, 1, 1,1 };
		vertex.uv = { 0,0,0,0 };
		model->vertices.push_back(vertex);
		vertex.pos = { -1, -1, 1,1 };
		vertex.uv = { 0,1,0,0 };
		model->vertices.push_back(vertex);
		vertex.pos = { 1, -1, 1,1 };
		vertex.uv = { 1,1,0,0 };
		model->vertices.push_back(vertex);
		//
		vertex.normal = { 1, 0, 0,1 };
		vertex.pos = { 1, 1, 1,1 };
		vertex.uv = { 0,0,0,0 };
		model->vertices.push_back(vertex);
		vertex.pos = { 1, -1, -1,1 };
		vertex.uv = { 1,1,0,0 };
		model->vertices.push_back(vertex);
		vertex.pos = { 1, 1, -1,1 };
		vertex.uv = { 1,0,0,0 };
		model->vertices.push_back(vertex);

		vertex.pos = { 1, 1, 1,1 };
		vertex.uv = { 0,0,0,0 };
		model->vertices.push_back(vertex);
		vertex.pos = { 1, -1, 1,1 };
		vertex.uv = { 0,1,0,0 };
		model->vertices.push_back(vertex);
		vertex.pos = { 1, -1, -1,1 };
		vertex.uv = { 1,1,0,0 };
		model->vertices.push_back(vertex);
		//
		vertex.normal = { 0, -1, 0,1 };
		vertex.pos = { 1, -1, 1,1 };
		vertex.uv = { 0,0,0,0 };
		model->vertices.push_back(vertex);
		vertex.pos = { -1, -1, -1,1 };
		vertex.uv = { 1,1,0,0 };
		model->vertices.push_back(vertex);
		vertex.pos = { 1, -1, -1,1 };
		vertex.uv = { 1,0,0,0 };
		model->vertices.push_back(vertex);

		vertex.pos = { 1, -1, 1,1 };
		vertex.uv = { 0,0,0,0 };
		model->vertices.push_back(vertex);
		vertex.pos = { -1, -1, 1,1 };
		vertex.uv = { 0,1,0,0 };
		model->vertices.push_back(vertex);
		vertex.pos = { -1, -1, -1,1 };
		vertex.uv = { 1,1,0,0 };
		model->vertices.push_back(vertex);
		//
		vertex.normal = { -1, 0, 0,1 };
		vertex.pos = { -1, -1, -1,1 };
		vertex.uv = { 0,0,0,0 };
		model->vertices.push_back(vertex);
		vertex.pos = { -1, 1, 1,1 };
		vertex.uv = { 1,1,0,0 };
		model->vertices.push_back(vertex);
		vertex.pos = { -1, 1, -1,1 };
		vertex.uv = { 1,0,0,0 };
		model->vertices.push_back(vertex);

		vertex.pos = { -1, -1, -1,1 };
		vertex.uv = { 0,0,0,0 };
		model->vertices.push_back(vertex);
		vertex.pos = { -1, -1, 1,1 };
		vertex.uv = { 0,1,0,0 };
		model->vertices.push_back(vertex);
		vertex.pos = { -1, 1, 1,1 };
		vertex.uv = { 1,1,0,0 };
		model->vertices.push_back(vertex);
		//
		vertex.normal = { 0, 1, 0,1 };
		vertex.pos = { 1, 1, -1,1 };
		vertex.uv = { 0,0,0,0 };
		model->vertices.push_back(vertex);
		vertex.pos = { -1, 1, 1,1 };
		vertex.uv = { 1,1,0,0 };
		model->vertices.push_back(vertex);
		vertex.pos = { 1, 1, 1,1 };
		vertex.uv = { 1,0,0,0 };
		model->vertices.push_back(vertex);

		vertex.pos = { 1, 1, -1,1 };
		vertex.uv = { 0,0,0,0 };
		model->vertices.push_back(vertex);
		vertex.pos = { -1, 1, -1,1 };
		vertex.uv = { 0,1,0,0 };
		model->vertices.push_back(vertex);
		vertex.pos = { -1, 1, 1,1 };
		vertex.uv = { 1,1,0,0 };
		model->vertices.push_back(vertex);
		break;

	case eModelData_axis:

		model = new QeAssetModel();
		model->scale = { 1, 1, 1 };
		vertex.pos = { 0, 0, 0,1 };

		vertex.normal = { 1, 0, 0,1 };
		vertex.color = { 1, 0, 0,1 };
		model->vertices.push_back(vertex);

		vertex.normal = { 0, 1, 0,1 };
		vertex.color = { 0, 1, 0,1 };
		model->vertices.push_back(vertex);

		vertex.normal = { 0, 0, 1, 1 };
		vertex.color = { 0, 0, 1,1 };
		model->vertices.push_back(vertex);
		break;

	case eModelData_grids:

		model = new QeAssetModel();
		model->scale = { 1 , 1 , 1 };
		vertex.color = { 0.5f, 0.5f, 0.5f,1.0f };

		for (index = -ACT->gridsNum; index <= ACT->gridsNum; ++index) {
			vertex.pos = { index, -ACT->gridsNum, 0,1 };
			vertex.normal = { 0, ACT->gridsNum *2, 0, 1 };
			model->vertices.push_back(vertex);

			vertex.pos = { -ACT->gridsNum, index, 0,1 };
			vertex.normal = { ACT->gridsNum *2, 0, 0, 1 };
			model->vertices.push_back(vertex);
		}
		break;

	case eModelData_line:

		model = new QeAssetModel();
		model->scale = { 1, 1, 1 };
		vertex.normal = { 1, 0, 0,1 };
		vertex.uv = { 0, 0,1,1 };
		vertex.color = { 1, 1, 1,1 };
		vertex.pos = { 0, 0, 0,1 };
		model->vertices.push_back(vertex);
		break;

	case eModelData_plane:

		model = new QeAssetModel();
		model->scale = { 1,1,1 };
		model->indices = { 0,2,1, 1,2,3 };

		model->indexSize = int(model->indices.size());
		vertex.uv = { 0,0,0,0 };
		vertex.color = { 1, 1, 1,1 };

		vertex.pos = { -0.5f, -0.5f, 0.0f,1.0f };
		vertex.normal = { 0, 0, 1,1 };
		model->vertices.push_back(vertex);

		vertex.uv = { 1,0,0,0 };
		vertex.pos = { -0.5f, 0.5f, 0.0f,1.0f};
		vertex.normal = { 0, 0, 1,1 };
		model->vertices.push_back(vertex);

		vertex.uv = { 0,1,0,0 };
		vertex.pos = { 0.5f, -0.5f, 0.0f,1.0f };
		vertex.normal = { 0, 0, 1,1 };
		model->vertices.push_back(vertex);

		vertex.uv = { 1,1,0,0 };
		vertex.pos = { 0.5f, 0.5f, 0.0f,1.0f };
		vertex.normal = { 0, 0, 1,1 };
		model->vertices.push_back(vertex);
		break;
	}

	//VK->createBufferData((void*)model->vertices.data(), sizeof(model->vertices[0]) * model->vertices.size(), model->vertex.buffer, model->vertex.memory, &model->vertex.mapped);
	VK->createBuffer(model->vertex, sizeof(model->vertices[0]) * model->vertices.size(), (void*)model->vertices.data());

	if (model->indexSize > 0) {
		//VK->createBufferData((void*)model->indices.data(), sizeof(model->indices[0]) * model->indices.size(), model->index.buffer, model->index.memory, &model->vertex.mapped);
		VK->createBuffer(model->index, sizeof(model->indices[0]) * model->indices.size(), (void*)model->indices.data());
	}
	//if (model->pMaterial && model->pMaterial->type == eMaterialPBR ) {
		//VK->createUniformBuffer(sizeof(model->pMaterial->value), model->pMaterial->uboBuffer.buffer, model->pMaterial->uboBuffer.memory);
		//VK->setMemory(model->pMaterial->uboBuffer.memory, (void*)&model->pMaterial->value, sizeof(model->pMaterial->value), &model->pMaterial->uboBuffer.mapped);
		//VK->createBuffer(model->pMaterial->uboBuffer, sizeof(model->pMaterial->value), (void*)&model->pMaterial->value);

		astMaterials[_filePath] = model->pMaterial;
	//}
	
	astModels[_filePath] = model;

	return model;
}

/*QeAssetMaterial* QeAsset::getMaterial(const char* _filename) {

	std::string _filePath = combinePath(_filename, eAssetMaterial);
	std::map<std::string, QeAssetMaterial*>::iterator it = astMaterials.find(_filePath.c_str());
	if (it != astMaterials.end())	return it->second;

	std::vector<char> buffer = loadFile(_filePath.c_str());
	QeAssetMaterial* mtl = ENCODE->decodeMTL(buffer.data());

	//VK->createBuffer(mtl->uboBuffer, sizeof(mtl->value), (void*)&mtl->value);
	//VK->createUniformBuffer(sizeof(mtl->value), mtl->uboBuffer.buffer, mtl->uboBuffer.memory);
	//VK->setMemory(mtl->uboBuffer.memory, (void*)&mtl->value, sizeof(mtl->value), &mtl->uboBuffer.mapped );

	astMaterials[_filePath] = mtl;

	return mtl;
}*/

QeAssetMaterial* QeAsset::getMaterialImage(const char* _filename, bool bCubeMap) {
	std::string _filePath = combinePath(_filename, eAssetTexture);
	std::map<std::string, QeAssetMaterial*>::iterator it = astMaterials.find(_filePath.c_str());
	if (it != astMaterials.end())	return it->second;

	QeAssetMaterial* mtl = new QeAssetMaterial();
	mtl->value.baseColor = {0,0,0,1};
	//mtl->value.phong.ambient = { 1,1,1,1 };
	//mtl->value.phong.diffuse = { 1,1,1,1 };
	//mtl->value.phong.specular = { 1,1,1,1 };
	//mtl->value.phong.emissive = { 1,1,1,1 };
	//mtl->value.phong.param = { 1,1,1,1 };
	if (_filePath.length() == 0) {}
	else if (bCubeMap)	mtl->image.pCubeMap = AST->getImage(_filename, bCubeMap);
	else				mtl->image.pBaseColorMap = AST->getImage(_filename, bCubeMap);

	//VK->createBuffer(mtl->uboBuffer, sizeof(mtl->value), (void*)&mtl->value);
	//VK->createUniformBuffer(sizeof(mtl->value), mtl->uboBuffer.buffer, mtl->uboBuffer.memory);
	//VK->setMemory(mtl->uboBuffer.memory, (void*)&mtl->value, sizeof(mtl->value), &mtl->uboBuffer.mapped);

	astMaterials[_filePath] = mtl;

	return mtl;
}

QeVKImage* QeAsset::getImage(const char* _filename, bool bCubeMap) {

	std::string _filePath = combinePath(_filename, eAssetTexture);
	std::map<std::string, QeVKImage*>::iterator it = astTextures.find(_filePath.c_str());

	if (it != astTextures.end())	return it->second;

	char type = 0; // 0:BMP, 1:PNG, 2:JPEG

	char *ret = strrchr((char*)_filePath.c_str(), '.');

	VkFormat format;

	if (strcmp(ret + 1, "bmp") == 0) {
		type = 0;
		format = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else if (strcmp(ret + 1, "png") == 0) {
		type = 1;
		format = VK_FORMAT_R8G8B8A8_UNORM;
	}
	else if (strcmp(ret + 1, "jpg") == 0 || strcmp(ret + 1, "jpeg") == 0) {
		type = 2;
		format = VK_FORMAT_R8G8B8A8_UNORM;
	}
	else return nullptr;

	QeVKImage* image;
	
	if(bCubeMap) image = new QeVKImage(eImage_cube);
	else		 image = new QeVKImage(eImage_2D);
	//image->sampler = VK->createTextureSampler();
	std::vector<std::string> imageList;

	if (bCubeMap) {
		/*
		POSITIVE_X	Right
		NEGATIVE_X	Left
		POSITIVE_Y	Top
		NEGATIVE_Y	Bottom 
		POSITIVE_Z	Back
		NEGATIVE_Z	Front
		*/
		imageList = { "\\posz", "\\negz", "\\negx", "\\posx", "\\posy", "\\negy" };
	} else
		imageList = { "" };

	int size = int(imageList.size());
	int cIndex = int(ret - _filePath.c_str());

	for (int i = 0;i<size; ++i) {
		std::string path(_filePath);
		path.insert(cIndex, imageList[i]);
		std::vector<char> buffer = loadFile(path.c_str());

		std::vector<unsigned char> data;
		int width, height, bytes;

		switch (type) {
		case 0:
			data = ENCODE->decodeBMP((unsigned char*)buffer.data(), &width, &height, &bytes);
			break;
		case 1:
			data = ENCODE->decodePNG((unsigned char*)buffer.data(), &width, &height, &bytes);
			break;
		case 2:
			data = ENCODE->decodeJPEG((unsigned char*)buffer.data(), buffer.size(), &width, &height, &bytes);
			break;
		}
		if (bytes != 4)	imageFillto32bits(&data, bytes);

		uint32_t mipLevels = 1;// static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

		//VK->createImageData((void*)data.data(), format, data.size(), width, height, image->image, image->memory, i, bCubeMap);
		VkExtent2D imageSize = { uint32_t(width), uint32_t(height) };
		if(i == 0)	VK->createImage(*image, data.size(), imageSize, format, (void*)data.data());
		else		VK->setMemoryImage(*image, data.size(), imageSize, format, (void*)data.data(), i);
	}
	//image->view = VK->createImageView(image->image, format, VK_IMAGE_ASPECT_COLOR_BIT, bCubeMap, mipLevels);

	astTextures[_filePath] = image;

	return image;
}

void QeAsset::imageFillto32bits(std::vector<unsigned char>* data, int bytes) {

	unsigned char c = 0xff;

	size_t size = data->size();
	size_t index = bytes;
	short int addBytes = 4 - bytes;

	while (index<=size) {
		
		for( int i =0 ; i<addBytes; ++i)	data->insert( data->begin()+index+i, c );

		index += 4;
		size = data->size();
	}
}

VkShaderModule QeAsset::getShader(const char* _filename) {

	std::string _filePath = combinePath(_filename, eAssetShader);
	std::map<std::string, VkShaderModule>::iterator it = astShaders.find(_filePath);
	if (it != astShaders.end())	return it->second;

	std::vector<char> buffer = loadFile(_filePath.c_str());

	VkShaderModule shader = VK->createShaderModel((void*)buffer.data(), int(buffer.size()));
	astShaders[_filePath] = shader;
	return shader;
}

QeAssetParticleRule* QeAsset::getParticle(int _eid) {

	std::map<int, QeAssetParticleRule*>::iterator it = astParticles.find(_eid);
	if (it != astParticles.end())	return it->second;

	QeAssetXML* node = getXMLEditNode(eObject_Particle, _eid);
	QeAssetParticleRule* particle = ENCODE->decodeParticle(node);
	astParticles[_eid] = particle;
	return particle;
}

std::string QeAsset::combinePath(const char* _filename, QeAssetType dataType) {

	if (strlen(_filename) > 3 && _filename[1] == ':' && _filename[2] == '\\')	return _filename;
	
	std::string rtn;
	switch (dataType) {
	
	case eAssetModel:
		rtn = getXMLValue(3, CONFIG, "path", "model");
		break;
	case eAssetMaterial:
		rtn = getXMLValue(3, CONFIG, "path", "material");
		break;
	case eAssetBin:
		rtn = getXMLValue(3, CONFIG, "path", "bin");
		break;
	case eAssetShader:
		rtn = getXMLValue(3, CONFIG, "path", "sharder");
		break;
	case eAssetTexture:
		rtn = getXMLValue(3, CONFIG, "path", "texture");
		break;
	}
	return rtn.append(_filename);
}

void QeAsset::setGraphicsShader(QeAssetGraphicsShader& shader, QeAssetXML* shaderData, const char* defaultShaderType) {

	const char* c = nullptr;

	if (shaderData) {
		c = getXMLValue(shaderData, 1, "vert");
		if (c != nullptr) shader.vert = getShader(c);
		c = getXMLValue(shaderData, 1, "tesc");
		if (c != nullptr) shader.tesc = getShader(c);
		c = getXMLValue(shaderData, 1, "tese");
		if (c != nullptr) shader.tese = getShader(c);
		c = getXMLValue(shaderData, 1, "geom");
		if (c != nullptr) shader.geom = getShader(c);
		c = getXMLValue(shaderData, 1, "frag");
		if (c != nullptr) shader.frag = getShader(c);
	}

	if (defaultShaderType && strlen(defaultShaderType)) {

		QeAssetXML * node = getXMLNode(4, CONFIG, "default", "graphicsShader", defaultShaderType);
		if (shader.vert == nullptr) {
			c = getXMLValue(node, 1, "vert");
			if (c != nullptr) shader.vert = getShader(c);
		}
		if (shader.tesc == nullptr) {
			c = getXMLValue(node, 1, "tesc");
			if (c != nullptr) shader.tesc = getShader(c);
		}
		if (shader.tese == nullptr) {
			c = getXMLValue(node, 1, "tese");
			if (c != nullptr) shader.tese = getShader(c);
		}
		if (shader.geom == nullptr) {
			c = getXMLValue(node, 1, "geom");
			if (c != nullptr) shader.geom = getShader(c);
		}
		if (shader.frag == nullptr) {
			c = getXMLValue(node, 1, "frag");
			if (c != nullptr) shader.frag = getShader(c);
		}
	}
}