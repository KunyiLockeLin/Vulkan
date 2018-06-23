#include "qeheader.h"


QeVKBuffer::~QeVKBuffer(){
	if (buffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(VK->device, buffer, nullptr);
		buffer = VK_NULL_HANDLE;
	}
	if (memory != VK_NULL_HANDLE) {
		vkFreeMemory(VK->device, memory, nullptr);
		memory = VK_NULL_HANDLE;
	}
	if (view != VK_NULL_HANDLE) {
		vkDestroyBufferView(VK->device, view, nullptr);
		view = VK_NULL_HANDLE;
	}
}

QeVKImage::~QeVKImage(){
	if (image != VK_NULL_HANDLE) {
		vkDestroyImage(VK->device, image, nullptr);
		image = VK_NULL_HANDLE;
	}
	if (memory != VK_NULL_HANDLE){
		vkFreeMemory(VK->device, memory, nullptr);
		memory = VK_NULL_HANDLE;
	}
	if (view != VK_NULL_HANDLE) {
		vkDestroyImageView(VK->device, view, nullptr);
		view = VK_NULL_HANDLE;
	}
	if (sampler != VK_NULL_HANDLE) {
		vkDestroySampler(VK->device, sampler, nullptr);
		sampler = VK_NULL_HANDLE;
	}
}

 QeVulkan::~QeVulkan() {

	vkDestroySurfaceKHR(VK->instance, surface, nullptr);
	surface = VK_NULL_HANDLE;
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	pipelineLayout = VK_NULL_HANDLE;
	
	size_t size = descriptorSetLayouts.size();
	for (size_t i = 0; i<size ;++i ) {
		vkDestroyDescriptorSetLayout(device, descriptorSetLayouts[i], nullptr);
	}
	descriptorSetLayouts.clear();

	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	descriptorPool = VK_NULL_HANDLE;
	vkDestroyCommandPool(device, commandPool, nullptr);
	commandPool = VK_NULL_HANDLE;

	vkDestroyDevice(device, nullptr);
	device = VK_NULL_HANDLE;
	DestroyDebugReportCallbackEXT(instance, callback, nullptr);
	callback = VK_NULL_HANDLE;
	vkDestroyInstance(instance, nullptr);
	instance = VK_NULL_HANDLE;
}

void QeVulkan::init() {

	if (bInit) return;
	bInit = true;

	createInstance();
	setupDebugCallback();
	surface = createSurface(WIN->window, WIN->windowInstance);
	pickPhysicalDevice();
	createLogicalDevice();
	createDescriptorPool();
	createDescriptorSetLayout();
	pipelineLayout = createPipelineLayout();
	createCommandPool();
}

void QeVulkan::updateRender(float time) {}
void QeVulkan::updateCompute(float time) {}

VkResult QeVulkan::CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	
	if (func != nullptr)	return func(instance, pCreateInfo, pAllocator, pCallback);
	else					return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void QeVulkan::DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	
	if (func != nullptr)	func(instance, callback, pAllocator);
}

void QeVulkan::createInstance() {
	if (DEBUG->isDebug() && !checkValidationLayerSupport())	LOG("validation layers requested, but not available!");
	
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = AST->getXMLValue(2, AST->CONFIG, "applicationName");

	std::vector<std::string> vs = ENCODE->split(AST->getXMLValue(2, AST->CONFIG, "applicationVersion"), ".");
	appInfo.applicationVersion = VK_MAKE_VERSION(atoi(vs[0].c_str()), atoi(vs[1].c_str()), atoi(vs[2].c_str()));
	
	appInfo.pEngineName = AST->getXMLValue(2, AST->CONFIG, "engineName");
	vs = ENCODE->split(AST->getXMLValue(2, AST->CONFIG, "engineVersion"), ".");
	appInfo.engineVersion = VK_MAKE_VERSION(atoi(vs[0].c_str()), atoi(vs[1].c_str()), atoi(vs[2].c_str()));

	vs = ENCODE->split(AST->getXMLValue(2, AST->CONFIG, "VulkanAPIVersion"), ".");
	appInfo.apiVersion = VK_MAKE_VERSION(atoi(vs[0].c_str()), atoi(vs[1].c_str()), atoi(vs[2].c_str()));

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledLayerNames = nullptr;
	//createInfo.enabledExtensionCount = 0;
	//createInfo.ppEnabledExtensionNames = 0;

	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	if (DEBUG->isDebug()) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else	createInfo.enabledLayerCount = 0;

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)	LOG("failed to create instance!");
}

void QeVulkan::setupDebugCallback() {
	if (!DEBUG->isDebug()) return;

	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = debugCallback;

	if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS)	LOG("failed to set up debug callback!");
}

void QeVulkan::pickPhysicalDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0)	LOG("failed to find GPUs with Vulkan support!");

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {
			physicalDevice = device;
			break;
	}}

	if (physicalDevice == VK_NULL_HANDLE)	LOG("failed to find a suitable GPU!");

	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
	//vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
	//vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);
}

void QeVulkan::createLogicalDevice() {
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

	std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily, indices.computeFamily };
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	float queuePriority = 1.0f;
	for (int queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.pNext = nullptr;
		queueCreateInfo.flags = 0;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.fillModeNonSolid = VK_TRUE;
	deviceFeatures.multiViewport = VK_TRUE;
	deviceFeatures.geometryShader = VK_TRUE;
	deviceFeatures.tessellationShader = VK_TRUE;

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledLayerNames = nullptr;
	//createInfo.enabledExtensionCount = 0;
	//createInfo.ppEnabledExtensionNames = 0;
	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (DEBUG->isDebug()) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else	createInfo.enabledLayerCount = 0;

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)	LOG("failed to create logical device!");

	vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily, 0, &presentQueue);
	vkGetDeviceQueue(device, indices.computeFamily, 0, &computeQueue);
}

void QeVulkan::createSwapChain(VkSurfaceKHR& surface, VkSwapchainKHR& swapChain, VkExtent2D& swapChainExtent, VkFormat& swapChainImageFormat, std::vector<QeVKImage>& swapChainImages) {
	
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		imageCount = swapChainSupport.capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT| VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
	uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily, (uint32_t)indices.computeFamily };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	//createInfo.oldSwapchain = oldSwapchain;

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)	LOG("failed to create swap chain!");

	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount, eImage_swapchain);
	std::vector<VkImage> swapchainImages2;
	swapchainImages2.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapchainImages2.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

	for (uint32_t i = 0; i < imageCount; i++) {
		swapChainImages[i].image = swapchainImages2[i];
		createImage(swapChainImages[i], 0, 0, 0, swapChainImageFormat, nullptr);
		//swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

VkRenderPass QeVulkan::createRenderPass(VkFormat format, int subpassNum, bool bMainRender) {

	std::vector<VkAttachmentDescription> attachments;
	attachments.resize(subpassNum+1);

	attachments[0].flags = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
	attachments[0].format = format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	
	if (subpassNum == 1 && bMainRender) {
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}
	else {
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	attachments[1].format = findDepthFormat();
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	if (subpassNum == 2 && bMainRender) {
		attachments[2].flags = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
		attachments[2].format = format;
		attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[2].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}

	std::vector<VkSubpassDescription> subpasses;
	subpasses.resize(subpassNum);

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	subpasses[0].flags = 0;
	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].colorAttachmentCount = 1;
	subpasses[0].pColorAttachments = &colorAttachmentRef;
	subpasses[0].pDepthStencilAttachment = &depthAttachmentRef;

	if (subpassNum == 2) {

		VkAttachmentReference inputAttachmentRef = {};
		inputAttachmentRef.attachment = 0;
		inputAttachmentRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference colorAttachmentRef1 = {};
		colorAttachmentRef1.attachment = 2;
		colorAttachmentRef1.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		subpasses[1].flags = 0;
		subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses[1].inputAttachmentCount = 1;
		subpasses[1].pInputAttachments = &inputAttachmentRef;
		subpasses[1].colorAttachmentCount = 1;
		subpasses[1].pColorAttachments = &colorAttachmentRef1;
	}

	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size());;
	renderPassInfo.pSubpasses = subpasses.data();
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());;
	renderPassInfo.pDependencies = dependencies.data();

	VkRenderPass renderPass;
	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)	LOG("failed to create render pass!");
	return renderPass;
}

void QeVulkan::createDescriptorSetLayout() {
	
	size_t size = descriptorSetLayoutDatas.size();
	descriptorSetLayouts.resize(size);

	for ( int i = 0; i< size;++i ) {
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		size_t size1 = descriptorSetLayoutDatas[i].size();

		for (size_t j = 0; j< size1; ++j) {

			VkDescriptorSetLayoutBinding binding;
			binding.descriptorType = descriptorSetLayoutDatas[i][j].type;
			binding.descriptorCount = 1;
			binding.pImmutableSamplers = nullptr;

			switch (descriptorSetLayoutDatas[i][j].type) {
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
				binding.stageFlags = VK_SHADER_STAGE_ALL;
				break;
			case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
				binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
				break;
			case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
				binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
				break;
			}

			for (int k = 0; k< descriptorSetLayoutDatas[i][j].count;++k ) {
				binding.binding = k + descriptorSetLayoutDatas[i][j].startID;
				bindings.push_back(binding);
			}
		}
		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayouts[i]) != VK_SUCCESS)	LOG("failed to create descriptor set layout!");
	}
}

VkPipelineLayout QeVulkan::createPipelineLayout() {

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = uint32_t(descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

	if (PUSH_CONSTANTS_SIZE > 0) {
		VkPushConstantRange push_constant_range = {};
		push_constant_range.stageFlags = VK_SHADER_STAGE_ALL;
		push_constant_range.offset = 0;
		push_constant_range.offset = PUSH_CONSTANTS_SIZE * sizeof(float);
		pushConstants.resize(PUSH_CONSTANTS_SIZE);
		pipelineLayoutInfo.pPushConstantRanges = &push_constant_range;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
	}
	VkPipelineLayout pipelineLayout;
	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)	LOG("failed to create pipeline layout!");

	return pipelineLayout;
}

void QeVulkan::updatePushConstnats(VkCommandBuffer command_buffer) {

	size_t size = pushConstants.size();
	if(size>0)
		vkCmdPushConstants(command_buffer, pipelineLayout, VK_SHADER_STAGE_ALL, 0, uint32_t(size * sizeof(float)), pushConstants.data());
}

void QeVulkan::createFramebuffers(std::vector<VkFramebuffer>& framebuffers, QeVKImage& presentImage, QeVKImage& depthImage,
								  std::vector<QeVKImage>& swapChainImages, VkExtent2D& swapChainExtent, VkRenderPass& renderPass, int subpassNum, bool bMainRender) {
	framebuffers.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++) {

		std::vector<VkImageView> attachments;
		attachments.resize(subpassNum + 1);
		if (!bMainRender || subpassNum > 1) attachments[0] = presentImage.view;
		else								attachments[0] = swapChainImages[i].view;
		
		attachments[1] = depthImage.view;

		if (bMainRender && subpassNum>1) attachments[2] = swapChainImages[i].view;

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)	LOG("failed to create framebuffer!");
	}
}

void QeVulkan::createCommandPool() {
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, surface);

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.pNext = nullptr;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	//poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
	poolInfo.queueFamilyIndex = queueFamilyIndices.computeFamily;

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)	LOG("failed to create graphics command pool!");
}

void QeVulkan::createPresentDepthImage(QeVKImage& presentImage, QeVKImage& depthImage, VkExtent2D& swapChainExtent) {
	VkFormat depthFormat = findDepthFormat();

	int imageSize = swapChainExtent.width*swapChainExtent.height * 4;

	createImage(depthImage, imageSize, swapChainExtent.width, swapChainExtent.height, depthFormat, nullptr);
	transitionImageLayout(depthImage.image, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	createImage(presentImage, imageSize, swapChainExtent.width, swapChainExtent.height, VK_FORMAT_B8G8R8A8_UNORM, nullptr);

}

VkFormat QeVulkan::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	LOG("failed to find supported format!");
	return VK_FORMAT_UNDEFINED;
}

VkFormat QeVulkan::findDepthFormat() {
	return findSupportedFormat(
	{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

bool QeVulkan::hasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void QeVulkan::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {
	
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (hasStencilComponent(format))	barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	else	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;//1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_HOST_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
		barrier.srcAccessMask = 0;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED ) {
		barrier.srcAccessMask = 0;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	}
	else	LOG("unsupported layout transition!");

	if ( newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if ( newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if ( newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if ( newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else	LOG("unsupported layout transition!");

	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier );

	endSingleTimeCommands(commandBuffer);
}

void QeVulkan::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, int layer) {
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = layer;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	endSingleTimeCommands(commandBuffer);
}

VkCommandBuffer QeVulkan::beginSingleTimeCommands() {
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void QeVulkan::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void QeVulkan::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(commandBuffer);
}

uint32_t QeVulkan::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	LOG("failed to find suitable memory type!");
	return VK_NULL_HANDLE;
}

void QeVulkan::createSyncObjects(std::vector<VkSemaphore>& imageAvailableSemaphores, std::vector<VkSemaphore>& renderFinishedSemaphores, std::vector<VkFence>& inFlightFences) {

	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) LOG("failed to create synchronization objects for a frame!");
	}
}

VkSurfaceFormatKHR QeVulkan::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
		return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}
	return availableFormats[0];
}

VkPresentModeKHR QeVulkan::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)		return availablePresentMode;
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) bestMode = availablePresentMode;
	}
	return bestMode;
}

VkExtent2D QeVulkan::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		int width = 0, height = 0;
		WIN->getWindowSize(width, height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

SwapChainSupportDetails QeVulkan::querySwapChainSupport(VkPhysicalDevice& device, VkSurfaceKHR& surface) {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

bool QeVulkan::isDeviceSuitable(VkPhysicalDevice device) {
	QueueFamilyIndices indices = findQueueFamilies(device, surface);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	//bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
		//swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return indices.isComplete() && extensionsSupported && supportedFeatures.samplerAnisotropy;
}

bool QeVulkan::checkDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

QueueFamilyIndices QeVulkan::findQueueFamilies(VkPhysicalDevice& device, VkSurfaceKHR& surface) {
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
			indices.computeFamily = i;
		}

		VkBool32 presentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (queueFamily.queueCount > 0 && presentSupport)	indices.presentFamily = i;
		if (indices.isComplete())	break;

		i++;
	}

	return indices;
}

std::vector<const char*> QeVulkan::getRequiredExtensions() {

	std::vector<const char*> extensions = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };

	if (DEBUG->isDebug()) {
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	return extensions;
}

bool QeVulkan::checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound)	return false;
	}

	return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL QeVulkan::debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData) {
	LOG("validation layer: " + msg );
	return VK_FALSE;
}

void QeVulkan::createCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers, size_t size) {
	commandBuffers.resize(size);

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) LOG("failed to allocate command buffers!");
}

VkSurfaceKHR QeVulkan::createSurface(HWND& window, HINSTANCE& windowInstance) {
	VkResult err = VK_SUCCESS;
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.pNext = nullptr;
	surfaceCreateInfo.flags = 0;
	surfaceCreateInfo.hinstance = windowInstance;
	surfaceCreateInfo.hwnd = window;

	VkSurfaceKHR surface;
	err = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);

	if (err != VK_SUCCESS) LOG("Could not create surface!");
	return surface;
}

void QeVulkan::createDescriptorSet(QeDataDescriptorSet& descriptorSet) {

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayouts[descriptorSet.type];

	if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet.set) != VK_SUCCESS) LOG("failed to allocate descriptor set!");
}

/*void QeVulkan::setMemory(VkDeviceMemory& memory, void* data, VkDeviceSize size, void** mapped) {

	if(!(*mapped)) vkMapMemory(device, memory, 0, size, 0, mapped);
	memcpy(*mapped, data, size);
}

void QeVulkan::setMemory(VkDeviceMemory& memory, void* data, VkDeviceSize size) {

	void* buf;
	vkMapMemory(device, memory, 0, size, 0, &buf);
	memcpy(buf, data, size);
	vkUnmapMemory(device, memory);
}*/

void QeVulkan::createDescriptorPool() {
	std::array<VkDescriptorPoolSize, 5> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = MAX_DESCRIPTOR_UNIFORM_NUM;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = MAX_DESCRIPTOR_SAMPLER_NUM;
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	poolSizes[2].descriptorCount = MAX_DESCRIPTOR_INPUTATTACH_NUM;
	poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
	poolSizes[3].descriptorCount = MAX_DESCRIPTOR_STORAGETEXEL_NUM;
	poolSizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes[4].descriptorCount = MAX_DESCRIPTOR_STORAG_NUM;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = MAX_DESCRIPTOR_NUM;

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) LOG("failed to create descriptor pool!");
}

QeDataGraphicsPipeline* QeVulkan::createGraphicsPipeline(QeAssetGraphicsShader* shader, QeGraphicsPipelineType type, bool bAlpha, uint8_t subpassIndex) {

	std::vector<QeDataGraphicsPipeline*>::iterator it = graphicsPipelines.begin();
	while ( it != graphicsPipelines.end()) {
		if ((*it)->shader->vert == shader->vert && (*it)->shader->tesc == shader->tesc && (*it)->shader->tese == shader->tese &&
			(*it)->shader->geom == shader->geom && (*it)->shader->frag == shader->frag && (*it)->bAlpha == bAlpha && (*it)->subpassIndex == subpassIndex) {
			return *it;
		}
		++it;
	}

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	if (shader->vert != VK_NULL_HANDLE) {
		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = shader->vert;
		vertShaderStageInfo.pName = "main";
		shaderStages.push_back(vertShaderStageInfo);
	}
	if (shader->tesc != VK_NULL_HANDLE) {
		VkPipelineShaderStageCreateInfo tescShaderStageInfo = {};
		tescShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		tescShaderStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		tescShaderStageInfo.module = shader->tesc;
		tescShaderStageInfo.pName = "main";
		shaderStages.push_back(tescShaderStageInfo);
	}	
	if (shader->tese != VK_NULL_HANDLE) {
		VkPipelineShaderStageCreateInfo teseShaderStageInfo = {};
		teseShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		teseShaderStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		teseShaderStageInfo.module = shader->tese;
		teseShaderStageInfo.pName = "main";
		shaderStages.push_back(teseShaderStageInfo);
	}
	if (shader->geom != VK_NULL_HANDLE) {
		VkPipelineShaderStageCreateInfo geomShaderStageInfo = {};
		geomShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		geomShaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
		geomShaderStageInfo.module = shader->geom;
		geomShaderStageInfo.pName = "main";
		shaderStages.push_back(geomShaderStageInfo);
	}
	if (shader->frag != VK_NULL_HANDLE) {
		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = shader->frag;
		fragShaderStageInfo.pName = "main";

		/*
		// Shader bindings based on specialization constants are marked by the new "constant_id" layout qualifier:
		//	layout (constant_id = 0) const int LIGHTING_MODEL = 0;
		//	layout (constant_id = 1) const float PARAM_TOON_DESATURATION = 0.0f;
		struct SpecializationData {
			uint32_t lightingModel;
			float toonDesaturationFactor = 0.5f;
		} specializationData;

		std::array<VkSpecializationMapEntry, 2> specializationMapEntries;
		specializationMapEntries[0].constantID = 0;
		specializationMapEntries[0].size = sizeof(specializationData.lightingModel);
		specializationMapEntries[0].offset = 0;

		specializationMapEntries[1].constantID = 1;
		specializationMapEntries[1].size = sizeof(specializationData.toonDesaturationFactor);
		specializationMapEntries[1].offset = offsetof(SpecializationData, toonDesaturationFactor);
		
		VkSpecializationInfo specializationInfo{};
		specializationInfo.dataSize = sizeof(specializationData);
		specializationInfo.mapEntryCount = static_cast<uint32_t>(specializationMapEntries.size());
		specializationInfo.pMapEntries = specializationMapEntries.data();
		specializationInfo.pData = &specializationData;

		fragShaderStageInfo.pSpecializationInfo = &specializationInfo;
		*/
		shaderStages.push_back(fragShaderStageInfo);
	}

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	if (type == eGraphicsPipeLine_Postprogessing) { // bVetex
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;
	}
	else {
		std::string s = "";  // Magic line!!!! In release mode, if it's removed, attributeDescriptions would become bad value and shutdown.
		VkVertexInputBindingDescription bindingDescription = QeVertex::getBindingDescription();
		std::array<VkVertexInputAttributeDescription, 7> attributeDescriptions = QeVertex::getAttributeDescriptions();
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	}

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	if (type == eGraphicsPipeLine_Point || type == eGraphicsPipeLine_Postprogessing)	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	else if(type == eGraphicsPipeLine_Line)								inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	else if (shader->tesc && shader->tese )								inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
	else																inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = (bShowMesh && type != eGraphicsPipeLine_Postprogessing) ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 1.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;
	rasterizer.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.minSampleShading = 0.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;// VK_TRUE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkStencilOpState stencil_test_parameters = {};
	stencil_test_parameters.failOp = VK_STENCIL_OP_KEEP;
	stencil_test_parameters.passOp = VK_STENCIL_OP_KEEP;
	stencil_test_parameters.depthFailOp = VK_STENCIL_OP_KEEP;
	stencil_test_parameters.compareOp = VK_COMPARE_OP_ALWAYS;
	stencil_test_parameters.compareMask = 0;
	stencil_test_parameters.writeMask = 0;
	stencil_test_parameters.reference = 0;

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

	if (type == eGraphicsPipeLine_Postprogessing) { // DepthTest
		depthStencil.depthTestEnable = VK_FALSE;
		depthStencil.depthWriteEnable = VK_FALSE;
	}
	else {
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
	}
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;// VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0;
	depthStencil.maxDepthBounds = 1;
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = stencil_test_parameters;
	depthStencil.back = stencil_test_parameters;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	
	if (!bAlpha) { // Alpha
		colorBlendAttachment.blendEnable = VK_FALSE;
	}
	else {
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;// VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;// VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;// VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;// VK_BLEND_FACTOR_ZERO;// VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	}
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 1.0f;
	colorBlending.blendConstants[1] = 1.0f;
	colorBlending.blendConstants[2] = 1.0f;
	colorBlending.blendConstants[3] = 1.0f;

	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 3;
	dynamicState.pDynamicStates = dynamicStates;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = (uint32_t)shaderStages.size();
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = VP->renderPass;
	pipelineInfo.subpass = subpassIndex;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (shader->tesc && shader->tese) {
		VkPipelineTessellationStateCreateInfo tessellationInfo = {};
		tessellationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		tessellationInfo.pNext = nullptr;
		tessellationInfo.flags = 0;
		tessellationInfo.patchControlPoints = 3;
		pipelineInfo.pTessellationState = &tessellationInfo;
	}

	VkPipeline graphicsPipeline;
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) LOG("failed to create graphics pipeline!");
	
	QeDataGraphicsPipeline* s = new QeDataGraphicsPipeline();
	s->shader = shader;
	s->bAlpha = bAlpha;
	s->subpassIndex = subpassIndex;
	s->type = type;
	s->graphicsPipeline = graphicsPipeline;
	graphicsPipelines.push_back(s);

	size_t size = graphicsPipelines.size();

	return graphicsPipelines[size-1];
}

VkPipeline QeVulkan::createComputePipeline(VkShaderModule shader) {

	VkPipelineShaderStageCreateInfo shaderStageInfo = {};
	shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	shaderStageInfo.module = shader;
	shaderStageInfo.pName = "main";

	VkComputePipelineCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.stage = shaderStageInfo;
	createInfo.layout = pipelineLayout;		

	VkPipeline pipeline;
	if (VK_SUCCESS != vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline)) LOG("Could not create compute pipeline.");
	
	return pipeline;
}

void QeVulkan::updateDescriptorSet(void* data, QeDataDescriptorSet& descriptorSet) {

	uint8_t* pos = (uint8_t*)data;
	std::vector<VkWriteDescriptorSet> descriptorWrites;
	std::list<VkDescriptorBufferInfo> bufInfos;
	std::list<VkDescriptorImageInfo> imgInfos;

	VkWriteDescriptorSet descriptorWrite;
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.pNext = nullptr;
	descriptorWrite.dstSet = descriptorSet.set;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorCount = 1;

	size_t size = descriptorSetLayoutDatas[descriptorSet.type].size();
	for (size_t i = 0; i < size; ++i) {

		descriptorWrite.descriptorType = descriptorSetLayoutDatas[descriptorSet.type][i].type;

		for (int j = 0; j < descriptorSetLayoutDatas[descriptorSet.type][i].count; ++j) {

			descriptorWrite.dstBinding = j + descriptorSetLayoutDatas[descriptorSet.type][i].startID;
			descriptorWrite.pImageInfo = nullptr;
			descriptorWrite.pBufferInfo = nullptr;
			descriptorWrite.pTexelBufferView = nullptr;
			switch (descriptorSetLayoutDatas[descriptorSet.type][i].type) {

			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:

				if ((*(VkBuffer*)(pos)) != VK_NULL_HANDLE) {

					VkDescriptorBufferInfo bufInfo;
					bufInfo.buffer = *(VkBuffer*)(pos);
					bufInfo.offset = 0;
					bufInfo.range = VK_WHOLE_SIZE;
					bufInfos.push_back(bufInfo);

					descriptorWrite.pBufferInfo = &bufInfos.back();
					descriptorWrites.push_back(descriptorWrite);
				}
				pos += sizeof(VkBuffer);

				break;
			case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:

				if ((*(VkImageView*)(pos)) != VK_NULL_HANDLE) {

					VkDescriptorImageInfo imgInfo;
					imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imgInfo.imageView = *(VkImageView*)(pos);
					imgInfo.sampler = *(VkSampler*)(pos + sizeof(VkSampler));
					imgInfos.push_back(imgInfo);

					descriptorWrite.pImageInfo = &imgInfos.back();
					descriptorWrites.push_back(descriptorWrite);
				}
				pos += sizeof(VkImageView) + sizeof(VkSampler);

				break;
			case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:

				if ((*(VkImageView*)(pos)) != VK_NULL_HANDLE) {

					VkDescriptorImageInfo imgInfo;
					imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imgInfo.imageView = *(VkImageView*)(pos);
					imgInfo.sampler = VK_NULL_HANDLE;
					imgInfos.push_back(imgInfo);

					descriptorWrite.pImageInfo = &imgInfos.back();
					descriptorWrites.push_back(descriptorWrite);
				}
				pos += sizeof(VkImageView);

				break;
			case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:

				if ((*(VkBufferView*)(pos)) != VK_NULL_HANDLE) {

					descriptorWrite.pTexelBufferView = (VkBufferView*)(pos);
					descriptorWrites.push_back(descriptorWrite);
				}
				pos += sizeof(VkBufferView);

				break;
			}
		}
	}
	vkUpdateDescriptorSets(device, uint32_t(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

VkShaderModule QeVulkan::createShaderModel(void* data, VkDeviceSize size) {
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = size;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(data);
	
	VkShaderModule shader;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shader) != VK_SUCCESS) LOG("failed to create shader module!");

	return shader;
}

void QeVulkan::generateMipmaps(VkImage image, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {

	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	for (uint32_t i = 1; i < mipLevels; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		VkImageBlit blit = {};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth / 2, mipHeight / 2, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(commandBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	endSingleTimeCommands(commandBuffer);
}

void QeVulkan::createBuffer(QeVKBuffer& buffer, VkDeviceSize size, void* data) {

	VkBufferUsageFlags usage;
	VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	bool bBuffer = true;
	bool bMemory = true;
	bool bView = false;

	switch (buffer.type) {
	case eBuffer:
		usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		break;

	case eBuffer_vertex:
		usage =  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; // VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
		//properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		break;

	case eBuffer_index:
		usage =  VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		break;

	case eBuffer_storage:
	case eBuffer_storage_compute_shader_return:
		usage =  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		break;

	case eBuffer_vertex_texel:
		usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
		bView = true;
		break;

	case eBuffer_uniform:
		usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		break;
	}
	// buffer
	if (bBuffer) {
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer.buffer) != VK_SUCCESS) LOG("failed to create buffer!");
	}

	// memory
	if (bMemory) {
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer.buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &buffer.memory) != VK_SUCCESS) LOG("failed to allocate buffer memory!");

		vkBindBufferMemory(device, buffer.buffer, buffer.memory, 0);
	}

	// view
	if ( bView ) {
		VkBufferViewCreateInfo buffer_view_create_info = {};
		buffer_view_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
		buffer_view_create_info.pNext = nullptr;
		buffer_view_create_info.flags = 0;
		buffer_view_create_info.buffer = buffer.buffer;
		buffer_view_create_info.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		buffer_view_create_info.offset = 0;
		buffer_view_create_info.range = VK_WHOLE_SIZE;

		VkResult result = vkCreateBufferView(device, &buffer_view_create_info, nullptr, &buffer.view);
		if (VK_SUCCESS != result) LOG("Could not creat buffer view.");
	}

	if (data)	setMemoryBuffer(buffer, size, data);
}
void QeVulkan::setMemoryBuffer(QeVKBuffer& buffer, VkDeviceSize size, void* data) {
	
	if (buffer.mapped)	memcpy(buffer.mapped, data, size);
	else {
		vkMapMemory(device, buffer.memory, 0, size, 0, &buffer.mapped);
		if (buffer.mapped) {
			memcpy(buffer.mapped, data, size);
		}
		else {
			QeVKBuffer staging(eBuffer);
			createBuffer(staging, size, data);
			copyBuffer(staging.buffer, buffer.buffer, size);
		}
	}
}

void QeVulkan::createImage(QeVKImage& image, VkDeviceSize size, uint16_t width, uint16_t height, VkFormat format, void* data) {

	uint32_t mipLevels = 1; //static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

	VkImageTiling tiling= VK_IMAGE_TILING_OPTIMAL;
	VkImageUsageFlags usage;
	VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	VkImageCreateFlags createFlags = 0;
	VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
	int arrayLayer = 1;
	bool bImage = true;
	bool bMemory = true;
	bool bView = true;
	bool bSampler = false;
	
	switch ( image.type ) {
	case eImage_depth:
		usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		break;

	case eImage_inputAttach:
		usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT; // VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
		data = new unsigned char[size];
		memset(data, 0, size);
		break;

	case eImage_swapchain:
		bImage = false;
		bMemory = false;
		break;

	case eImage_2D:
		usage = VK_IMAGE_USAGE_SAMPLED_BIT; // VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
		bSampler = true;
		break;

	case eImage_cube:
		createFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		arrayLayer = 6;
		viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		usage =  VK_IMAGE_USAGE_SAMPLED_BIT;
		bSampler = true;
		break;

	//case eImage_inputAttach:
	//	usage = ;
	//	break;
	}

	// image
	if (bImage) {
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.flags = createFlags;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = arrayLayer;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;// VK_IMAGE_LAYOUT_PREINITIALIZED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(device, &imageInfo, nullptr, &image.image) != VK_SUCCESS) LOG("failed to create image!");
	}

	// memory
	if (bMemory) {
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image.image, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &image.memory) != VK_SUCCESS) LOG("failed to allocate image memory!");

		vkBindImageMemory(device, image.image, image.memory, 0);
	}

	// view
	if (bView) {
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image.image;
		viewInfo.viewType = viewType;
		viewInfo.format = format;
		viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;// VK_REMAINING_MIP_LEVELS;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;// VK_REMAINING_ARRAY_LAYERS;

		if (vkCreateImageView(device, &viewInfo, nullptr, &image.view) != VK_SUCCESS) LOG("failed to create texture image view!");
	}

	// sampler
	if (bSampler) {
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		//samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;// VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;// VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;// VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.maxAnisotropy = 1;// 16;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;// VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		samplerInfo.mipLodBias = 0;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.minLod = 0;
		samplerInfo.maxLod = 1;

		if (vkCreateSampler(VK->device, &samplerInfo, nullptr, &image.sampler) != VK_SUCCESS) LOG("failed to create texture sampler!");
	}

	// data
	if (data)	setMemoryImage(image, size, width, height, format, data, 0);
}


void QeVulkan::setMemoryImage(QeVKImage& image, VkDeviceSize size, uint16_t width, uint16_t height, VkFormat format, void* data, uint8_t layer) {

	uint32_t mipLevels = 1; //static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
	QeVKBuffer staging(eBuffer);
	createBuffer(staging, size, data);

	transitionImageLayout(image.image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
	//transitionImageLayout(image, format, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	copyBufferToImage(staging.buffer, image.image, static_cast<uint32_t>(width), static_cast<uint32_t>(height), layer);
	//generateMipmaps(image, width, height, mipLevels);
}