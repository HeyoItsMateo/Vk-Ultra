#pragma once

#include "vk.ubo.h"
#include "vk.textures.h"
#include "vk.graphics.h"

#include "Mesh.h"
#include "Geometry.h"
#include "Planet.h"

pgl::Planet icosphere(10, 0.5f, 4);

vk::UBO icoMat(icosphere.matrix, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT);

std::vector<VkDescriptorSet> icoSet {
    ubo.Sets[vk::SwapChain::currentFrame],
    icoMat.Sets[vk::SwapChain::currentFrame],
};
std::vector<VkDescriptorSetLayout> icoLayout {
    ubo.SetLayout,
    icoMat.SetLayout,
};

vk::Shader icoShaders[] = {
    {"ico.vert", VK_SHADER_STAGE_VERTEX_BIT},
    {"ico.frag", VK_SHADER_STAGE_FRAGMENT_BIT}
};

vk::GraphicsPPL<triangleList, VK_POLYGON_MODE_LINE> icoPPL(icoShaders, icoSet, icoLayout);