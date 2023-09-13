#include "vk.engine.h"

#include <algorithm>
#include <functional>

bool hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

vk::Window window("Vulkan");
vk::Engine app;

vk::Uniforms uniforms;
vk::UBO ubo(uniforms, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT);

pop population(1000);
vk::SSBO ssbo(population.particles, VK_SHADER_STAGE_COMPUTE_BIT);

vk::Texture planks("textures/planks.png");

vk::Texture_ testPlanks("textures/planks.png");
vk::Sampler sampler(testPlanks.mipLevels);



std::vector<triangleList> vertices = {
    {{-0.5f,  0.5f,  0.5f, 1.f}, {-0.5f,  0.5f,  0.5f, 1.f}, {1.0f, 0.0f, 0.0f, 1.f}, {0.0f, 0.0f}},
    {{ 0.5f,  0.5f,  0.5f, 1.f}, { 0.5f,  0.5f,  0.5f, 1.f}, {0.0f, 1.0f, 0.0f, 1.f}, {1.0f, 0.0f}},
    {{ 0.5f,  0.5f, -0.5f, 1.f}, { 0.5f,  0.5f, -0.5f, 1.f}, {0.0f, 0.0f, 1.0f, 1.f}, {1.0f, 1.0f}},
    {{-0.5f,  0.5f, -0.5f, 1.f}, {-0.5f,  0.5f, -0.5f, 1.f}, {1.0f, 1.0f, 1.0f, 1.f}, {0.0f, 1.0f}},

    {{-0.5f, -0.5f,  0.5f, 1.f}, {-0.5f, -0.5f,  0.5f, 1.f}, {1.0f, 0.0f, 0.0f, 1.f}, {0.0f, 0.0f}},
    {{ 0.5f, -0.5f,  0.5f, 1.f}, { 0.5f, -0.5f,  0.5f, 1.f}, {0.0f, 1.0f, 0.0f, 1.f}, {1.0f, 0.0f}},
    {{ 0.5f, -0.5f, -0.5f, 1.f}, { 0.5f, -0.5f, -0.5f, 1.f}, {0.0f, 0.0f, 1.0f, 1.f}, {1.0f, 1.0f}},
    {{-0.5f, -0.5f, -0.5f, 1.f}, {-0.5f, -0.5f, -0.5f, 1.f}, {1.0f, 1.0f, 1.0f, 1.f}, {0.0f, 1.0f}}
};
const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4
};
vk::Mesh model(vertices, indices);

std::vector<VkDescriptorSet> testSet{
    ubo.Sets[vk::SwapChain::currentFrame],
    sampler.Sets[vk::SwapChain::currentFrame],
    testPlanks.Sets[vk::SwapChain::currentFrame]
};
std::vector<VkDescriptorSetLayout> testLayouts{
    ubo.SetLayout,
    sampler.SetLayout,
    testPlanks.SetLayout
};
std::vector<vk::Shader> test{
    {"vertex.vert", VK_SHADER_STAGE_VERTEX_BIT},
    {"vertex.frag", VK_SHADER_STAGE_FRAGMENT_BIT}
};

vk::GraphicsPPL<triangleList> testPPL(test, testSet, testLayouts);

//std::vector<vk::Texture> textures{planks};


vk::Plane plane({ 30, 20 }, { 0.25, 0.25 });

std::vector<VkDescriptorSet> descSet1{
    ubo.Sets[vk::SwapChain::currentFrame],
    ssbo.Sets[vk::SwapChain::currentFrame]
};
std::vector<VkDescriptorSetLayout> SetLayouts{
    ubo.SetLayout,
    ssbo.SetLayout
};
std::vector<vk::Shader> planeShaders {
    {"plane.vert", VK_SHADER_STAGE_VERTEX_BIT},
    {"plane.frag", VK_SHADER_STAGE_FRAGMENT_BIT}
};

vk::GraphicsPPL<triangleList, VK_POLYGON_MODE_LINE> pipeline(planeShaders, descSet1, SetLayouts);

vk::Octree tree(vertices, 0.01f);
vk::SSBO modelSSBO(tree.matrices, VK_SHADER_STAGE_VERTEX_BIT);

std::vector<VkDescriptorSet> descSet2{
    ubo.Sets[vk::SwapChain::currentFrame],
    modelSSBO.Sets[vk::SwapChain::currentFrame]
};
std::vector<VkDescriptorSetLayout> SetLayouts_2{
    ubo.SetLayout,
    modelSSBO.SetLayout
};
std::vector<vk::Shader> octreeShaders = {
    {"instanced.vert", VK_SHADER_STAGE_VERTEX_BIT},
    {"instanced.frag", VK_SHADER_STAGE_FRAGMENT_BIT}
};

vk::GraphicsPPL<lineList> octreePPL(octreeShaders, descSet2, SetLayouts_2);//descSet2

vk::Shader compShader("point.comp", VK_SHADER_STAGE_COMPUTE_BIT);
vk::ComputePPL computePPL(compShader, descSet1, SetLayouts);

std::vector<vk::Shader> pointShaders {
    {"point.vert", VK_SHADER_STAGE_VERTEX_BIT}, 
    {"point.frag", VK_SHADER_STAGE_FRAGMENT_BIT} 
};

vk::GraphicsPPL<Particle, VK_POLYGON_MODE_POINT> particlePPL(pointShaders, descSet1, SetLayouts);

//TODO: Create compute shader that generates a texture/image
//TODO: Make copy and move operators for pretty much everything

//TODO: Learn discrete differential geometry
//TODO: Implement waves~
//TODO: Optimize the swapchain and rendering process
//TODO: Optimize a fuckload of stuff with shader caching, pipeline caching, parallelization, etc.

VkPhysicalDeviceProperties properties;

int main() {
    //TODO: Make physical device properties file
    vkGetPhysicalDeviceProperties(vk::GPU::physicalDevice, &properties);

    try {
        while (!glfwWindowShouldClose(vk::Window::handle)) {
            glfwPollEvents();
            std::jthread t1(glfwSetKeyCallback, vk::Window::handle, vk::userInput);
            
            //tree.updateTree(vertices);
            ubo.update(uniforms);
            app.run(testPPL, octreePPL, model, tree.rootNode, particlePPL, computePPL, ssbo);
            
        }
        vkDeviceWaitIdle(vk::GPU::device);
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
