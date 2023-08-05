#ifndef hDBO
#define hDBO

struct Uniforms {
    float dt = 1.f;
    alignas(16) glm::mat4 model = glm::mat4(1.f);
    camMatrix camera;
    void update(double lastTime, float FOVdeg = 45.f, float nearPlane = 0.01f, float farPlane = 1000.f) {
        std::jthread t1([this]
            { modelUpdate(); }
        );
        std::jthread t2([this, FOVdeg, nearPlane, farPlane]
            { camera.update(FOVdeg, nearPlane, farPlane); }
        );
        std::jthread t3([this, lastTime]
            { deltaTime(lastTime); }
        );
    }
private:
    void modelUpdate() {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    void deltaTime(double lastTime) {
        double currentTime = glfwGetTime();
        dt = (currentTime - lastTime);
    }
};

//template<typename T>
//struct UBO : VkCommand, VkDescriptor {
//    VkDeviceSize bufferSize;
//    std::vector<VkBuffer> Buffer;
//    std::vector<VkDeviceMemory> Memory;
//
//    constexpr UBO(T& ubo, VkShaderStageFlags flag, VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
//        uniforms = ubo;
//        createDataBuffer();
//
//        createDescriptorSetLayout(type, flag);
//        createDescriptorPool(type);
//        createDescriptorSets(type);
//    }
//    ~UBO() {
//        vkUnmapMemory(VkGPU::device, stagingBufferMemory);
//        vkDestroyBuffer(VkGPU::device, stagingBuffer, nullptr);
//        vkFreeMemory(VkGPU::device, stagingBufferMemory, nullptr);
//
//        std::for_each(std::execution::par, Buffer.begin(), Buffer.end(), 
//            [&](VkBuffer buffer) { vkDestroyBuffer(VkGPU::device, buffer, nullptr); });
//
//        std::for_each(std::execution::par, Memory.begin(), Memory.end(),
//            [&](VkDeviceMemory memory) { vkFreeMemory(VkGPU::device, memory, nullptr); });
//
//        vkDestroyDescriptorPool(VkGPU::device, Pool, nullptr);
//        vkDestroyDescriptorSetLayout(VkGPU::device, SetLayout, nullptr);
//    }
//
//    void update() {
//        uniforms.update(VkSwapChain::lastTime);
//
//        memcpy(data, &uniforms, (size_t)bufferSize);
//
//        copyBuffer(stagingBuffer, Buffer[VkSwapChain::currentFrame], bufferSize);
//    }
//private:
//    T uniforms;
//    void* data;
//    VkBuffer stagingBuffer;
//    VkDeviceMemory stagingBufferMemory;
//    void createDataBuffer() {
//        Buffer.resize(MAX_FRAMES_IN_FLIGHT);
//        Memory.resize(MAX_FRAMES_IN_FLIGHT);
//
//        bufferSize = sizeof(T);
//
//        createBuffer(stagingBuffer, stagingBufferMemory, bufferSize,
//            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
//            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
//
//        vkMapMemory(VkGPU::device, stagingBufferMemory, 0, bufferSize, 0, &data);
//        memcpy(data, &uniforms, (size_t)bufferSize);
//
//        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
//            createBuffer(Buffer[i], Memory[i], bufferSize,
//                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
//                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
//
//            copyBuffer(stagingBuffer, Buffer[i], bufferSize);
//        }
//    }
//    void createBuffer(VkBuffer& buffer, VkDeviceMemory& memory, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
//        VkBufferCreateInfo bufferInfo
//        { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
//        bufferInfo.size = size;
//        bufferInfo.usage = usage;
//        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//
//        if (vkCreateBuffer(VkGPU::device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
//            throw std::runtime_error("failed to create buffer!");
//        }
//
//        VkMemoryRequirements memRequirements;
//        vkGetBufferMemoryRequirements(VkGPU::device, buffer, &memRequirements);
//
//        VkMemoryAllocateInfo allocInfo
//        { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
//        allocInfo.allocationSize = memRequirements.size;
//        allocInfo.memoryTypeIndex = VkGPU::findMemoryType(memRequirements.memoryTypeBits, properties);
//
//        if (vkAllocateMemory(VkGPU::device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
//            throw std::runtime_error("failed to allocate buffer memory!");
//        }
//
//        vkBindBufferMemory(VkGPU::device, buffer, memory, 0);
//    }
//    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
//        beginCommand();
//
//        VkBufferCopy copyRegion{};
//        copyRegion.size = size;
//        vkCmdCopyBuffer(VkCommand::buffer, srcBuffer, dstBuffer, 1, &copyRegion);
//
//        endCommand();
//    }
//
//    void createDescriptorSets(VkDescriptorType type) {
//        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, SetLayout);
//        VkDescriptorSetAllocateInfo allocInfo{};
//        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//        allocInfo.descriptorPool = Pool;
//        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
//        allocInfo.pSetLayouts = layouts.data();
//
//        Sets.resize(MAX_FRAMES_IN_FLIGHT);
//        if (vkAllocateDescriptorSets(VkGPU::device, &allocInfo, Sets.data()) != VK_SUCCESS) {
//            throw std::runtime_error("failed to allocate descriptor sets!");
//        }
//
//        std::vector<VkWriteDescriptorSet> descriptorWrites(MAX_FRAMES_IN_FLIGHT);
//        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
//
//            VkDescriptorBufferInfo bufferInfo{};
//            bufferInfo.buffer = Buffer[i];
//            bufferInfo.offset = 0;
//            bufferInfo.range = bufferSize;
//
//            VkWriteDescriptorSet writeUBO = VkUtils::writeDescriptor(0, type, Sets[i]);
//            writeUBO.pBufferInfo = &bufferInfo;
//            descriptorWrites[i] = writeUBO;
//        }
//        vkUpdateDescriptorSets(VkGPU::device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
//    }
//};



#endif