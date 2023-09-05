#include "vk.engine.h"

#include <algorithm>
#include <functional>

bool hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

vk::Window window("Vulkan");
vk::Engine app;

pop population(1000);
vk::SSBO ssbo(population.particles, VK_SHADER_STAGE_COMPUTE_BIT);

std::vector<Vertex> vertices = {
    {{-0.5f,  0.5f,  0.5f, 1.f}, {1.0f, 0.0f, 0.0f, 1.f}, {0.0f, 0.0f}},
    {{ 0.5f,  0.5f,  0.5f, 1.f}, {0.0f, 1.0f, 0.0f, 1.f}, {1.0f, 0.0f}},
    {{ 0.5f,  0.5f, -0.5f, 1.f}, {0.0f, 0.0f, 1.0f, 1.f}, {1.0f, 1.0f}},
    {{-0.5f,  0.5f, -0.5f, 1.f}, {1.0f, 1.0f, 1.0f, 1.f}, {0.0f, 1.0f}},

    {{-0.5f, -0.5f,  0.5f, 1.f}, {1.0f, 0.0f, 0.0f, 1.f}, {0.0f, 0.0f}},
    {{ 0.5f, -0.5f,  0.5f, 1.f}, {0.0f, 1.0f, 0.0f, 1.f}, {1.0f, 0.0f}},
    {{ 0.5f, -0.5f, -0.5f, 1.f}, {0.0f, 0.0f, 1.0f, 1.f}, {1.0f, 1.0f}},
    {{-0.5f, -0.5f, -0.5f, 1.f}, {1.0f, 1.0f, 1.0f, 1.f}, {0.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4
};

vk::Model model(vertices, indices);

vk::Plane plane({ 30, 20 }, { 0.25, 0.25 });

Octree tree(vertices, 0.01f);
vk::SSBO modelSSBO(tree.matrices, VK_SHADER_STAGE_VERTEX_BIT);

vk::Uniforms uniforms;
vk::UBO ubo(uniforms, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT);

vk::Texture planks("textures/planks.png");
//std::vector<VkTexture> textures{planks};
vk::TextureSet textureSet(planks);

std::vector<VkDescriptorSet> descSet1 { 
    ubo.Sets[vk::SwapChain::currentFrame], 
    textureSet.Sets[vk::SwapChain::currentFrame], 
    ssbo.Sets[vk::SwapChain::currentFrame]
};
std::vector<VkDescriptorSetLayout> SetLayouts { 
    ubo.SetLayout, 
    textureSet.SetLayout, 
    ssbo.SetLayout
};

vk::Shader vertShader("vertex.vert", VK_SHADER_STAGE_VERTEX_BIT);
vk::Shader fragShader("vertex.frag", VK_SHADER_STAGE_FRAGMENT_BIT);
//TODO: FIgure out how to compile multiple shaders at once
//vk::ShaderSet ShaderSet_1({"vertex.vert", "vertex.frag"});
std::vector<vk::Shader*> shaders = {
    &vertShader, 
    &fragShader
};
//TODO: Using multiple shader compiling, compile multiple pipelines at once.
vk::GraphicsPPL<Vertex> pipeline(shaders, descSet1, SetLayouts); //descSet1


std::vector<VkDescriptorSet> descSet2{
    ubo.Sets[vk::SwapChain::currentFrame],
    textureSet.Sets[vk::SwapChain::currentFrame],
    modelSSBO.Sets[vk::SwapChain::currentFrame]
};
std::vector<VkDescriptorSetLayout> SetLayouts_2{
    ubo.SetLayout,
    textureSet.SetLayout,
    modelSSBO.SetLayout
};
vk::Shader octreeVert("octree.vert", VK_SHADER_STAGE_VERTEX_BIT);
vk::Shader octreeFrag("octree.frag", VK_SHADER_STAGE_FRAGMENT_BIT);
//std::vector<vk::Shader*> octreeShaders = { &octreeVert, &octreeFrag };
std::vector<vk::Shader*> octreeShaders = {
    &octreeVert, 
    &octreeFrag
};
vk::derivativePPL octreePPL(pipeline.pipeline, octreeShaders, descSet2, SetLayouts_2);//descSet2

//vk::Shader octreeVert("octree.vert", VK_SHADER_STAGE_VERTEX_BIT);
//vk::Shader octreeFrag("octree.frag", VK_SHADER_STAGE_FRAGMENT_BIT);
//const std::vector<vk::Shader> testShaders {
//    {"octree.vert", VK_SHADER_STAGE_VERTEX_BIT}, 
//    {"octree.frag", VK_SHADER_STAGE_FRAGMENT_BIT}
//};
//vk::GraphicsPipeline2<Voxel> testPipe(descSet2, testShaders, SetLayouts_2);

vk::Shader compShader("shader.comp", VK_SHADER_STAGE_COMPUTE_BIT);
vk::ComputePipeline computePPL(descSet1, compShader.stageInfo, SetLayouts);

vk::Shader pointVert("point.vert", VK_SHADER_STAGE_VERTEX_BIT);
vk::Shader pointFrag("point.frag", VK_SHADER_STAGE_FRAGMENT_BIT);
std::vector<vk::Shader*> compShaders = { &pointVert, &pointFrag };
vk::ParticlePipeline particlePPL(pipeline.pipeline, compShaders, descSet1, SetLayouts);

//TODO: Learn discrete differential geometry
//TODO: Implement waves~
//TODO: Optimize the swapchain and rendering process
//TODO: Optimize a fuckload of stuff with shader caching, pipeline caching, parallelization, etc.

VkPhysicalDeviceProperties properties;

int main() {
    vkGetPhysicalDeviceProperties(vk::GPU::physicalDevice, &properties);

    std::printf("maxWorkgroupSize is: %i \n", *properties.limits.maxComputeWorkGroupSize);
    std::printf("maxWorkGroupInvocations is: %i \n", properties.limits.maxComputeWorkGroupInvocations);
    std::printf("maxWorkGroupCount is: %i \n", *properties.limits.maxComputeWorkGroupCount);

    try {
        //runProcess("shader.comp");
        while (!glfwWindowShouldClose(vk::Window::handle)) {
            glfwPollEvents();
            std::jthread t1(glfwSetKeyCallback, vk::Window::handle, vk::userInput);
            
            tree.updateTree(vertices);
            ubo.update(uniforms);
            app.run(pipeline, octreePPL, plane, tree, particlePPL, computePPL, ssbo, ubo, uniforms);
            
        }
        vkDeviceWaitIdle(vk::GPU::device);
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
