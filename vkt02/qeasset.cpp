#include "qeheader.h"


VkVertexInputBindingDescription QeVertex::getBindingDescription() {
	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(QeVertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 4> QeVertex::getAttributeDescriptions() {
	std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = {};

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(QeVertex, pos);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(QeVertex, color);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(QeVertex, texCoord);

	attributeDescriptions[3].binding = 0;
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[3].offset = offsetof(QeVertex, normal);

	return attributeDescriptions;
}

bool QeVertex::operator==(const QeVertex& other) const {
	return pos == other.pos && normal == other.normal && texCoord == other.texCoord && color == other.color;
}

std::string QeAsset::trim(std::string s) {

	if (s.empty())	return s;
	s.erase(0, s.find_first_not_of(" "));
	s.erase(0, s.find_first_not_of("\n"));
	s.erase(0, s.find_first_not_of(" "));
	s.erase(s.find_last_not_of(" ") + 1);
	s.erase(s.find_last_not_of("\n") + 1);
	s.erase(s.find_last_not_of(" ") + 1);
	return s;
}

QeAssetJSON* QeAsset::getJSON(const char* _filePath) {
	std::map<std::string, QeAssetJSON*>::iterator it = astJSONs.find(_filePath);

	if (it != astJSONs.end())	return it->second;

	std::ifstream file(_filePath, std::ios::ate | std::ios::binary);
	if (!file.is_open()) return nullptr;
	
	file.seekg(0, file.end);
	int length = int(file.tellg());
	file.seekg(0);

	char* buffer = new char[length];
	file.read(buffer, length);
	int index = 0;
	QeAssetJSON* head = decodeJSON(buffer,index);
	astJSONs[_filePath] = head;
	delete buffer;
	return head;
}

QeAssetJSON* QeAsset::decodeJSON(const char* buffer, int &index ) {	
	/* key	
	0: {
	1: }
	2: "
	3: :
	4: [
	5: ]
	6: ,
	*/
	const char keys[] = "{}\":[],";
	QeAssetJSON* node = new QeAssetJSON();
	int lastIndex = index, currentIndex = index, lastKey = 0, currentKey = 0;
	char* newChar = nullptr;
	std::vector<std::string> *vsBuffer = nullptr;
	bool bValue = false;
	int count = 0;
	std::string key;
	while (1) {
		currentIndex = int(strcspn(buffer+lastIndex, keys)+ lastIndex);
		currentKey = int(strchr(keys, buffer[currentIndex]) - keys);
		
		if (currentKey == 0 && lastKey != 3 && lastKey != 4)	count++;

		if (lastKey == currentKey && currentKey == 2 ) {
			std::string s(buffer+lastIndex, currentIndex - lastIndex);
			s = trim(s);
			if (bValue){
				bValue = false;
				
				node->eKeysforValues.push_back(key);
				node->eValues.push_back(s);
			}
			else	key = s;
		}
		else if (lastKey == 3) {
			if (currentKey == 2) bValue = true;

			else if (currentKey == 0) {
				node->eKeysforNodes.push_back(key);
				node->eNodes.push_back(decodeJSON(buffer, currentIndex));
			}
			else if (currentKey != 4) {
				std::string s(buffer + lastIndex, currentIndex - lastIndex);
				s = trim(s);
				node->eKeysforValues.push_back(key);
				node->eValues.push_back(s);
			}
		}
		else if (lastKey == 4) {
			if (currentKey == 0) {
				std::vector<QeAssetJSON*> vjson;
				while (1) {
					vjson.push_back(decodeJSON(buffer, currentIndex));
					lastIndex = currentIndex + 1;
					currentIndex = int(strcspn(buffer + lastIndex, keys)+ lastIndex);
					currentKey = int(strchr(keys, buffer[currentIndex]) - keys);

					if (currentKey != 6) break;
				}
				node->eKeysforArrayNodes.push_back(key);
				node->eArrayNodes.push_back(vjson);
			}else {
				currentIndex = int(strchr(buffer + lastIndex, ']') - buffer);
				currentKey = 5;
				std::vector<std::string> vs;
				
				std::string s(buffer + lastIndex, currentIndex - lastIndex);
				char s2[512];
				strncpy_s(s2, s.c_str(), 512);
				char *context = NULL;
				const char* key1 = ",\"\n";
				char* pch = strtok_s(s2, key1, &context);
				
				while (pch != NULL){
					std::string s(pch); 
					s = trim(s);
					if (s.length() > 0) vs.push_back(s);
					pch = strtok_s(NULL, key1, &context);
				}
				node->eKeysforArrayValues.push_back(key);
				node->eArrayValues.push_back(vs);
			}
		}

		lastKey = currentKey;
		lastIndex = currentIndex + 1;

		if (currentKey == 1) {
			count--;
			if (count == 0) break;
		}
	}

	index = currentIndex;
	return node;
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
	delete keys1;
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
	delete keys1;
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
	delete keys1;
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
	delete keys1;
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
	delete keys1;
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
	delete keys1;
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
	delete keys1;
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
	delete keys1;
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


QeAssetXML* QeAsset::getXML(const char* _filePath) {
	std::map<std::string, QeAssetXML*>::iterator it = astXMLs.find(_filePath);

	if (it != astXMLs.end())	return it->second;

	std::ifstream file(_filePath, std::ios::ate | std::ios::binary);
	if (!file.is_open()) return nullptr;

	file.seekg(0, file.end);
	int length = int(file.tellg());
	file.seekg(0);

	char * buffer = new char[length];
	file.read(buffer, length);
	int index = 0;
	QeAssetXML* head = decodeXML(buffer, index);
	astXMLs[_filePath] = head;
	return head;

}
QeAssetXML* QeAsset::decodeXML(const char* buffer, int &index) {
	/* key
	0: <
	1: >
	2: /
	3: =
	4: "
	5: ?
	*/
	const char keys[] = "<>/=\"?";

	QeAssetXML* node = new QeAssetXML();
	int lastIndex = index, currentIndex = index, lastKey = 0, currentKey = 0;
	char* newChar = nullptr;
	std::vector<std::string> *vsBuffer = nullptr;
	bool bRoot = true;

	while (1) {
		currentIndex = int(strcspn(buffer + lastIndex, keys) + lastIndex);
		currentKey = int(strchr(keys, buffer[currentIndex]) - keys);
		
		if (currentKey == 5) {
			lastIndex = currentIndex + 1;
			currentIndex = int(strchr(buffer + lastIndex, '?')- buffer);
			lastIndex = currentIndex + 1;
			bRoot = true;
		}
		else if ( currentKey == 0 ) {
			
			if (bRoot) bRoot = false;

			else if (buffer[currentIndex + 1] != '/')	node->nexts.push_back(decodeXML(buffer, currentIndex));

			else if (lastKey == 1) {
				std::string s(buffer + lastIndex, currentIndex - lastIndex);
				s = trim(s);
				node->value = s;
			}
		}
		else if (lastKey == 0) {

			if (currentKey == 1) {
				std::string s(buffer + lastIndex, currentIndex - lastIndex);
				s = trim(s);
				node->key = s;
			}
			else if (currentKey == 3) {
				int index = int(strchr(buffer + lastIndex, ' ')- buffer);
				std::string s(buffer + lastIndex, index - lastIndex);
				s = trim(s);
				node->key = s;
				std::string s1(buffer + index, currentIndex - index);
				s1 = trim(s1);
				node->eKeys.push_back(s1);
			}
		}
		else if (lastKey == 2 && currentKey == 1)	break;
	
		else if (lastKey == 4 ) {

			if (currentKey == 3) {
				std::string s(buffer + lastIndex, currentIndex - lastIndex);
				s = trim(s);
				node->eKeys.push_back(s);
			}
			else if (currentKey == lastKey) {
				std::string s(buffer + lastIndex, currentIndex - lastIndex);
				s = trim(s);
				node->eVaules.push_back(s);
			}
		}
		lastKey = currentKey;
		lastIndex = currentIndex + 1;
	}
	index = currentIndex;
	return node;
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
	delete keys1;
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
	delete keys1;
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
	delete keys1;
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
	delete keys1;
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
		if (index1 == size) break;
	}
	return source;
}

QeAssetModel* QeAsset::getModel(const char* _filename) {

	std::string _filePath = combinePath(_filename, eAssetModel);
	std::map<std::string, QeAssetModel*>::iterator it = astModels.find(_filePath);

	if (it != astModels.end())	return it->second;

	//return getModelGLTF(_filename);
	return getModelOBJ(_filename);
}

QeAssetModel* QeAsset::getModelGLTF(const char* _filename) {

	std::string _filePath = combinePath(_filename, eAssetModel);
	std::map<std::string, QeAssetModel*>::iterator it = astModels.find(_filePath);

	if (it != astModels.end())	return it->second;
	QeAssetJSON* head = getJSON(_filePath.c_str());
	/*const char* ret1 = getJSONValue(head, 2, "accessors", "componentType");
	QeAssetJSON* ret2 = getJSONNode(head, 2, "materials", "pbrMetallicRoughness");
	std::vector<std::string>* ret3 = getJSONArrayValues(head, 3, "materials", "pbrMetallicRoughness", "baseColorFactor");
	std::vector<QeAssetJSON*>* ret4 = getJSONArrayNodes(head, 1, "bufferViews");*/
	return nullptr;
}

QeAssetModel* QeAsset::getModelOBJ(const char* _filename) {

	std::string _filePath = combinePath(_filename, eAssetModel);
	std::map<std::string, QeAssetModel*>::iterator it = astModels.find(_filePath);

	if (it != astModels.end())	return it->second;

	std::ifstream file(_filePath.c_str(), std::ios::ate | std::ios::binary);
	if (!file.is_open()) return nullptr;
	file.seekg(0);

	char line[500];

	std::vector<QeVector3f> normalV;
	std::vector<QeVector2f> texCoordV;

	QeVector3f tempV3;
	QeVector2f tempV2;
	QeVector3i tempV3p, tempV3t, tempV3n;
	char mtlPath[200];
	
	QeAssetModel* model = new QeAssetModel();

	while (file.getline(line, sizeof(line))) {
		
		if (line[0] == 'm' && line[1] == 't' && line[2] == 'l' && line[3] == 'l' && line[4] == 'i' && line[5] == 'b') {
			sscanf_s(line, "mtllib  %s", mtlPath,  (unsigned int)sizeof(mtlPath) );
		}
		else if (line[0] == 'v' && line[1] == ' ') {
			sscanf_s(line, "v %f %f %f", &(tempV3.x), &(tempV3.y), &(tempV3.z));
			QeVertex vet;
			vet.pos = tempV3;
			vet.color ={ 1.0f, 1.0f, 1.0f };
			model->vertices.push_back(vet);
		} 
		else if (line[0] == 'v' && line[1] == 't') {
			sscanf_s(line, "vt %f %f", &(tempV2.x), &(tempV2.y));
			texCoordV.push_back(tempV2);
		}
		else if (line[0] == 'v' && line[1] == 'n') {
			sscanf_s(line, "vn %f %f %f", &(tempV3.x), &(tempV3.y), &(tempV3.z));
			normalV.push_back(tempV3);
		}
		else if (line[0] == 'f' && line[1] == ' ') {
			
			if (strstr(line, "//")) {
				sscanf_s(line, "f %d//%d %d//%d %d//%d", &(tempV3p.x), &(tempV3n.x),
					&(tempV3p.y), &(tempV3n.y), &(tempV3p.z), &(tempV3n.z));
			}
			else if (texCoordV.empty()) {
				sscanf_s(line, "f %d %d %d", &(tempV3p.x), &(tempV3p.y), &(tempV3p.z));
			}
			else if (normalV.empty()) {
				sscanf_s(line, "f %d/%d %d/%d %d/%d", &(tempV3p.x), &(tempV3t.x),
					&(tempV3p.y), &(tempV3t.y), &(tempV3p.z), &(tempV3t.z));
			}
			else{
				sscanf_s(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &(tempV3p.x), &(tempV3t.x),
					&(tempV3n.x), &(tempV3p.y), &(tempV3t.y), &(tempV3n.y), &(tempV3p.z), &(tempV3t.z), &(tempV3n.z));
			}
			
			tempV3p -= 1;

			if (!texCoordV.empty()) {
				tempV3t -= 1;
				model->vertices[tempV3p.x].texCoord = texCoordV[tempV3t.x];
				model->vertices[tempV3p.y].texCoord = texCoordV[tempV3t.y];
				model->vertices[tempV3p.z].texCoord = texCoordV[tempV3t.z];
			}
			if (!normalV.empty()) {
				tempV3n -= 1;
				model->vertices[tempV3p.x].normal = normalV[tempV3n.x];
				model->vertices[tempV3p.y].normal = normalV[tempV3n.y];
				model->vertices[tempV3p.z].normal = normalV[tempV3n.z];
			}
			model->indices.push_back(tempV3p.x);
			model->indices.push_back(tempV3p.y);
			model->indices.push_back(tempV3p.z);
		}
	}
	file.close();

	VLK->createBufferData((void*)model->vertices.data(), sizeof(model->vertices[0]) * model->vertices.size(), model->vertexBuffer, model->vertexBufferMemory);
	VLK->createBufferData((void*)model->indices.data(), sizeof(model->indices[0]) * model->indices.size(), model->indexBuffer, model->indexBufferMemory);

	model->indexSize = model->indices.size();

	astModels[_filePath] = model;

	if (strlen(mtlPath) != 0)
		model->pMaterial = getMateialMTL(mtlPath);

	return model;
}

QeAssetMaterial* QeAsset::getMateialMTL(const char* _filename) {

	std::string _filePath = combinePath(_filename, eAssetMaterial);
	std::map<std::string, QeAssetMaterial*>::iterator it = astMaterials.find(_filePath.c_str());

	if (it != astMaterials.end())	return it->second;

	std::ifstream file(_filePath.c_str(), std::ios::ate | std::ios::binary);
	if (!file.is_open()) return nullptr;
	file.seekg(0);

	QeAssetMaterial* mtl = new QeAssetMaterial();
	char line[500];
	char diffuseMapPath[500];
	QeDataMaterial mtl1;
	mtl1.ambient.w = 1;
	mtl1.diffuse.w = 1;
	mtl1.specular.w = 1;
	mtl1.emissive.w = 1;
	char sv[500]="";
	char sg[500]="";
	char sf[500]="";

	while (file.getline(line, sizeof(line))) {

		if (line[0] == 'm' && line[1] == 'a' && line[2] == 'p' && line[3] == '_' && line[4] == 'K' && line[5] == 'd') 
			sscanf_s(line, "map_Kd %s", diffuseMapPath, (unsigned int) sizeof(diffuseMapPath));
	
		else if (line[0] == 'N' && line[1] == 's') 
			sscanf_s(line, "Ns %f", &mtl1.param.x);
		
		else if (line[0] == 'K' && line[1] == 'a') 
			sscanf_s(line, "Ka %f %f %f", &(mtl1.ambient.x), &(mtl1.ambient.y), &(mtl1.ambient.z));

		else if (line[0] == 'K' && line[1] == 'd') 
			sscanf_s(line, "Kd %f %f %f", &(mtl1.diffuse.x), &(mtl1.diffuse.y), &(mtl1.diffuse.z));

		else if (line[0] == 'K' && line[1] == 's') 
			sscanf_s(line, "Ks %f %f %f", &(mtl1.specular.x), &(mtl1.specular.y), &(mtl1.specular.z));
	
		else if (line[0] == 'K' && line[1] == 'e') 
			sscanf_s(line, "Ke %f %f %f", &(mtl1.emissive.x), &(mtl1.emissive.y), &(mtl1.emissive.z));

		else if (line[0] == 'N' && line[1] == 'i') 
			sscanf_s(line, "Ni %f", &(mtl1.param.y));
	
		else if (line[0] == 'd' && line[1] == ' ') 
			sscanf_s(line, "d %f", &(mtl1.param.z));

		else if (line[0] == 's' && line[1] == 'v')
			sscanf_s(line, "sv %s", sv, (unsigned int) sizeof(sv));
		else if (line[0] == 's' && line[1] == 'g')
			sscanf_s(line, "sg %s", sg, (unsigned int) sizeof(sg));
		else if (line[0] == 's' && line[1] == 'f')
			sscanf_s(line, "sf %s", sf, (unsigned int) sizeof(sf));
		 
	}

	file.close();
	mtl->value = mtl1;
	VLK->createUniformBuffer(sizeof(QeDataMaterial), mtl->materialBuffer, mtl->materialBufferMemory);
	VLK->setMemory(mtl->materialBufferMemory, (void*)&mtl1, sizeof(mtl1));

	astMaterials[_filePath] = mtl;

	if (strlen(diffuseMapPath) != 0)
		mtl->pDiffuseMap = getImage(diffuseMapPath);

	if(strlen(sv) == 0)
		mtl->pShaderVert = getShader(getXMLValue(3, CONFIG, "defaultShader", "vert"));
	else
		mtl->pShaderVert = getShader(sv);

	if (strlen(sg) == 0)
		mtl->pShaderGeom = getShader(getXMLValue(3, CONFIG, "defaultShader", "geom"));
	else
		mtl->pShaderGeom = getShader(sg);

	if (strlen(sf) == 0)
		mtl->pShaderFrag = getShader(getXMLValue(3, CONFIG, "defaultShader", "frag"));
	else
		mtl->pShaderFrag = getShader(sf);

	return mtl;
}

QeAssetImage* QeAsset::getImage(const char* _filename) {
	std::string _filePath = combinePath(_filename, eAssetTexture);
	std::map<std::string, QeAssetImage*>::iterator it = astTextures.find(_filePath.c_str());

	if (it != astTextures.end())	return it->second;

	return getImageBMP(_filename);
	//return getImagePNG(_filename);
}

QeAssetImage* QeAsset::getImagePNG(const char* _filename) {

	std::string _filePath = combinePath(_filename, eAssetTexture);
	std::map<std::string, QeAssetImage*>::iterator it = astTextures.find(_filePath.c_str());
	if (it != astTextures.end())	return it->second;

	std::ifstream _file(_filePath.c_str(), std::ios::ate | std::ios::binary);
	if (!_file.is_open()) return nullptr;
	_file.seekg(0);

	char header[122];
	_file.read(header, sizeof(header));

	if (header[0] != 0x89 || header[1] != 0x50 || header[2] != 0x4E || header[3] != 0x47
		|| header[4] != 0x0D || header[5] != 0x0A || header[6] != 0x1A || header[7] != 0x0A) {
		_file.close();
		return false;
	}


	return nullptr;
}

QeAssetImage* QeAsset::getImageBMP(const char* _filename) {

	std::string _filePath = combinePath(_filename, eAssetTexture);
	std::map<std::string, QeAssetImage*>::iterator it = astTextures.find(_filePath.c_str());
	if (it != astTextures.end())	return it->second;

	std::ifstream _file(_filePath.c_str(), std::ios::ate | std::ios::binary);
	if (!_file.is_open()) return nullptr;
	_file.seekg(0);

	char header[122];
	_file.read(header, sizeof(header));

	if (header[0] != 'B' || header[1] != 'M') {
		_file.close();
		return false;
	}
	
	int dataPos = *(int*)&(header[0x0A]);
	if (dataPos == 0) {
		_file.close();
		return false;
	}

	int width = *(int*)&(header[0x12]);
	int height = *(int*)&(header[0x16]);
	short int bits = *(int*)&(header[0x1C]);
	int bytes = bits / 8;
	//int imageSize = *(int*)&(header[0x22]);
	int	imageSize = width *height * 4;
	
	std::vector<char> data;
	data.resize(imageSize);
	
	if (bytes == 4) {
		_file.seekg(dataPos);
		_file.read(data.data(), imageSize);
	}
	else {
		int fileIndex = 0;
		int dataIndex = 0;
		unsigned char c = 0xff;
		data.assign(imageSize, c);

		while (dataIndex<imageSize) {
			_file.seekg(dataPos+ fileIndex);
			_file.read(data.data()+ dataIndex, bytes);
			fileIndex += bytes;
			dataIndex += 4;
		}
	}
	_file.close();

	QeAssetImage* image = new QeAssetImage();
	VLK->createImageData((void*)data.data(), VK_FORMAT_B8G8R8A8_UNORM, imageSize, width, height, image->textureImage, image->textureImageMemory);
	image->textureImageView = VLK->createImageView(image->textureImage, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

	image->textureSampler = VLK->createTextureSampler();
	astTextures[_filePath] = image;
	return image;
}

QeAssetShader* QeAsset::getShader(const char* _filename) {

	std::string _filePath = combinePath(_filename, eAssetShader);
	std::map<std::string, QeAssetShader*>::iterator it = astShaders.find(_filePath.c_str());
	if (it != astShaders.end())	return it->second;

	std::ifstream file(_filePath, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	QeAssetShader* shader = new QeAssetShader();
	shader->shader = VLK->createShaderModel((void*)buffer.data(), int(buffer.size()));
	astShaders[_filename] = shader;
	return shader;
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
	case eAssetShader:
		rtn = getXMLValue(3, CONFIG, "path", "sharder");
		break;
	case eAssetTexture:
		rtn = getXMLValue(3, CONFIG, "path", "texture");
		break;
	}
	return rtn.append(_filename).c_str();
}