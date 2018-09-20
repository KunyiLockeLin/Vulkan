#pragma once

#include "qeheader.h"

enum QeRenderType {
	eRender_KHR = 0,
	eRender_main = 1,
	eRender_color = 2,
	eRender_MAX = 3
};

struct QeDataEnvironment {
	QeVector4f ambientColor;
	QeDataCamera camera;
	QeVector4f param; // 0: light num, 1: gamma, 2: exposure
};

struct QeDataViewport {
	VkViewport viewport;
	VkRect2D scissor;
	QeCamera* camera = nullptr;
	//std::vector<QeLight*> lights;

	QeDataDescriptorSet commonDescriptorSet;
	QeDataEnvironment environmentData;
	QeVKBuffer environmentBuffer;

	QeDataViewport():environmentBuffer(eBuffer_uniform),
		commonDescriptorSet(eDescriptorSetLayout_Common) {}

	~QeDataViewport();
};

struct QeBufferSubpass {

	QeVector4f param1; // 0: blurHorizontal, 1: blurScale, 2: blurStrength
};

struct QeDataSubpass {
	QeBufferSubpass bufferData;
	QeVKBuffer buffer;
	QeAssetGraphicsShader graphicsShader;
	QeDataDescriptorSet descriptorSet;
	QeDataGraphicsPipeline graphicsPipeline;

	QeDataSubpass() : buffer(eBuffer_uniform), descriptorSet(eDescriptorSetLayout_Postprocessing) {}
	~QeDataSubpass() {}
};

struct QeDataRender {

	std::vector<QeDataViewport*> viewports;
	int cameraOID;
	VkViewport viewport;
	VkRect2D scissor;

	QeVKImage colorImage, colorImage2, depthStencilImage, multiSampleColorImage;// , multiSampleDepthStencilImage;
	std::vector<VkFramebuffer> frameBuffers;

	std::vector<VkCommandBuffer> commandBuffers;
	VkSemaphore semaphore = VK_NULL_HANDLE;
	
	VkRenderPass renderPass = VK_NULL_HANDLE;
	std::vector<QeDataSubpass*> subpass;

	QeDataRender() :colorImage(eImage_render), colorImage2(eImage_render), depthStencilImage(eImage_depthStencil),
		multiSampleColorImage(eImage_attach)/*, multiSampleDepthStencilImage(eImage_multiSampleDepthStencil)*/{}
	~QeDataRender();
};

struct QeDataSwapchain {
	VkSwapchainKHR khr = VK_NULL_HANDLE;
	VkExtent2D extent;
	VkFormat format;
	std::vector<QeVKImage> images;
};

struct QeDataDrawCommand {
	VkCommandBuffer commandBuffer;
	QeCamera* camera;
	QeDataDescriptorSet* commonDescriptorSet;
	VkRenderPass renderPass;
	QeRenderType type;
};

class QeGraphics
{
public:

	QeGraphics(QeGlobalKey& _key): lightsBuffer(eBuffer_storage) {}
	~QeGraphics();

	std::vector<QeDataRender*> renders;
	//std::vector<QeLight*> lights;
	std::vector<QeVector3f> clearColors;
	QeDataSwapchain swapchain;

	int currentTargetViewport = 0;

	VkSemaphore renderCompleteSemaphore = VK_NULL_HANDLE;
	std::vector<VkFence> fences;
	VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;

	std::vector<QeModel*> models;
	std::vector<QeModel*> alphaModels
		;
	std::vector<QeLight*> lights;
	QeVKBuffer lightsBuffer;
	bool bUpdateLight= false;

	void initialize();
	void addNewViewport(QeRenderType type);
	void popViewport(QeRenderType type);
	void updateViewport();
	void updateBuffer();
	void addLight(QeLight* light);
	void removeLight(QeLight* light);
	void update1();
	void update2();
	void setTargetCamera(int index);
	QeCamera* getTargetCamera();
	QeDataRender * getRender(QeRenderType type, int cameraOID, VkExtent2D renderSize);
	//bool bUpdateComputeCommandBuffers = false;
	bool bUpdateDrawCommandBuffers = false;
	bool bRecreateRender = false;

	//std::vector<VkSemaphore> computeSemaphores;
	//std::vector<VkFence> computeFences;

	//VkSemaphore textOverlayComplete;

	QeDataRender* createRender(QeRenderType type, int cameraOID, VkExtent2D renderSize);
	void refreshRender();
	void cleanupRender();
	void drawFrame();
	void updateDrawCommandBuffers();
	//void updateComputeCommandBuffers();

	void sortAlphaModels(QeCamera * camera);

	void cleanupPipeline();
	void recreatePipeline();

	void updateComputeCommandBuffer(VkCommandBuffer& commandBuffer, QeCamera* camera, QeDataDescriptorSet* commonDescriptorSet);
	void updateDrawCommandBuffer(QeDataDrawCommand* command);
};