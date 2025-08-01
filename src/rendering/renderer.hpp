#include "buffer_writter.hpp"
#include "command_buffer.hpp"
#include "glm/glm.hpp"
#include "mapped_buffer.hpp"
#include "types.hpp"
#include "vk/deletion.hpp"
#include "vk/descriptors.hpp"
#include "vk/textures.hpp"
#include "world_renderer.hpp"
#include <functional>
#include <memory>
#include <vector>
namespace vblck
{
namespace render
{

struct Plane
{
	glm::vec3 normal;
	float d;
	inline void norm()
	{
		auto len = glm::length(normal);
		normal /= len;
		d /= len;
	}
};

struct Frustum
{
	Plane top, bottom, left, right, front, back;
};

struct Camera
{
	glm::vec3 position{};
	glm::vec3 forward{};
	float aspect = 1;
	float znear = 0.01f;
	float zfar = 10000.f;
	float fov = 45.f;

	glm::mat4 getProjection();
	glm::mat4 getView();

	Frustum getFrustum();
};

struct RenderSate
{
	Camera camera{};
	Camera cullCamera{};
};

constexpr unsigned int FRAME_OVERLAP = 2;

struct FrameData
{
	VkSemaphore swapchainSemaphore;
	VkFence renderFence;

	VkCommandPool commandPool;
	std::unique_ptr<CommandBuffer> mainCommandBuffer;

	vk::DeletionQueue deletionQueue;
};

struct GlobalRenderData
{
	vk::DescriptorAllocator allocator{};
	VkDescriptorSetLayout globalDescriptorLayout{};
	VkDescriptorSet globalDescriptors[FRAME_OVERLAP]{};
	MappedBuffer globalBuffer{};

	void create();
	void destroy();
	VkDescriptorSet getGlobalDescriptor();

	void writeDescriptors(VkCommandBuffer cmd);
};

struct Renderer
{
	int frameNumber;
	VkInstance instance{};
	VkPhysicalDevice chosenGPU{};
	VkDevice device{};
	VkSurfaceKHR surface{};
	VkQueue graphicsQueue;
	VmaAllocator vma{};
	uint32_t graphicsQueueFamily;
	VkPhysicalDeviceProperties props;

	VkSwapchainKHR swapchain{};
	std::vector<VkImage> swapchainImages{};
	std::vector<VkImageView> swapchainImageViews{};
	std::vector<VkSemaphore> renderSemaphores{};
	VkFormat swapchainImageFormat{};

	VkExtent2D screenExtent{};
	GlobalRenderData renderData{};

	vk::DeletionQueue mainDeletionQueue;
	vk::DeletionQueue frameDeletionQueue;

	FrameData frames[FRAME_OVERLAP];

	BufferWritter bufferWritter;

	vk::Texture2D backbuffer;
	vk::DepthTexture depthBuffer;

	WorldRenderer worldRenderer;

	vk::Texture2DArray textureAtlas{};

	struct
	{
		Frustum camFrustum;
	} computedUtils;

	void initVMA();
	void initCommands();
	void initSyncStructures();
	void initIMGUI();

	void destroySwapchain();
	void cleanup();

	void renderLogic(CommandBuffer* cmd);
	void renderImGUI(VkCommandBuffer cmd, VkImageView targetImageView);

	void initRenderers();

	inline void deletePendingObjects()
	{
		for(size_t i = 0; i < FRAME_OVERLAP; i++)
		{
			frames[i].deletionQueue.deleteQueue(device, vma);
		}
		frameDeletionQueue.deleteQueue(device, vma);
	}

	static Renderer* renderInstance;

	Renderer(VkInstance instance,
			 VkPhysicalDevice chosenGPU,
			 VkDevice device,
			 VkSurfaceKHR surface,
			 VkQueue graphicsQueue,
			 uint32_t graphicsQueueFamily,
			 int w,
			 int h)
		: instance(instance)
		, chosenGPU(chosenGPU)
		, device(device)
		, surface(surface)
		, graphicsQueue(graphicsQueue)
		, graphicsQueueFamily(graphicsQueueFamily)
	{
		renderInstance = this;
		frameNumber = 0;
		vkGetPhysicalDeviceProperties(chosenGPU, &props);
		initVMA();
		textureAtlas = loadTexture2DArray("res/textures/atlas.png", 16, 16);
		recreateSwapchain(w, h);
		initCommands();
		initSyncStructures();

		renderData.create();

		initRenderers();
	}

	~Renderer()
	{
		cleanup();
	}

	static inline Renderer* get()
	{
		return renderInstance;
	}

	inline uint32_t getFrameIndex()
	{
		return frameNumber % FRAME_OVERLAP;
	}

	FrameData& getCurrentFrame()
	{
		return frames[frameNumber % FRAME_OVERLAP];
	};

	vk::Texture2D loadTexture2D(const char* path);
	vk::Texture2DArray loadTexture2DArray(const char* path, int ncols, int nrows);

	vk::Texture2D* getBackbuffer()
	{
		return &backbuffer;
	}

	void recreateSwapchain(int w, int h);

	void renderFrame(RenderSate& state);
};
} // namespace render
} // namespace vblck