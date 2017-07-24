#pragma once

#include "qeheader.h"


struct UniformBufferObject {
	QeMatrix4x4f model;
	QeMatrix4x4f view;
	QeMatrix4x4f proj;
};

class QeModel
{
private:
public:
	QeVector3f pos;
	float face;
	float up;
	QeVector3f size;

	QeAssetModel* modelData;

	VkPipelineLayout pipelineLayout;
	VkDescriptorSet descriptorSet;
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;
	std::vector<VkCommandBuffer> commandBuffers;

	VkPipeline graphicsPipeline;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;

	void recreate();
	void createCommandBuffers();
	void createDescriptorSet();
	void createUniformBuffer();
	void updateDescriptorSet();
	void updateUniformBuffer();
	void createGraphicsPipeline();
	void update(float time);
	void createDescriptorSetLayout();
	void createDescriptorPool();

public: 
	QeModel() {}
	~QeModel() {}
	void init(const char* _filename);
	void cleanup();
	void setPosFaceUpSize(QeVector3f _pos, float _face, float _up, QeVector3f _size);
	void move(QeVector3f _pos);
	void setPosition(QeVector3f _pos);
	void rotateFace( float _face);
	void setFace(float _face);
	void rotateUp(float _up);
	void setUp(float _up);
	void enlarge(QeVector3f _size);
	void setSize(QeVector3f _size);
	QeMatrix4x4f getMatModel();
};

