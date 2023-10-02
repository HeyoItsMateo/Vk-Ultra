#ifndef hModel
#define hModel

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "vk.buffers.h"
#include "vk.primitives.h"

namespace vk {
    struct Mesh {
        alignas (16) glm::mat4 matrix = glm::mat4(1.f);
        template <typename T>
        inline Mesh(std::vector<T> vertices, std::vector<uint16_t> indices) 
            : indexCount(static_cast<uint32_t>(indices.size())),
            VBO(vertices.size() * sizeof(T), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
            EBO(indices.size() * sizeof(uint16_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
            stageVBO(vertices.data(), VBO.size),
            stageEBO(indices.data(), EBO.size)
        {
            stageVBO.transferData(VBO.buffer);
            stageEBO.transferData(EBO.buffer);
        }
        ~Mesh() = default;
    public:
        Buffer VBO, EBO;
        template<typename T>
        inline void update(std::vector<T>& vertices, std::vector<uint16_t>& indices) {
            stageVBO.update(vertices, VBO.buffer);
            //stageEBO.update(indices, EBO.buffer);
        }
        void draw(uint32_t instanceCount = 1) {
            VkCommandBuffer& commandBuffer = EngineCPU::renderCommands[SwapChain::currentFrame];
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &VBO.buffer, offsets);
            vkCmdBindIndexBuffer(commandBuffer, EBO.buffer, 0, VK_INDEX_TYPE_UINT16);

            vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, 0, 0, 0);
        }
    protected:
        StageBuffer stageVBO, stageEBO;
        uint32_t indexCount;
    };

    struct Voxel : Mesh {
        glm::vec4 center;
        glm::vec4 dimensions{ 1.f };
        Voxel(glm::vec4 Center = { 0,0,0,1 }, glm::vec4 Dimensions = { 1,1,1,1 })
            : Mesh(createVertices(), createIndices())
        {
            center = Center;
            dimensions = Dimensions;
            genMatrix(Center, Dimensions);
        }
        Voxel operator()(glm::vec4 Center = { 0,0,0,1 }, glm::vec4 Dimensions = { 1,1,1,1 }) {
            return Voxel(Center, Dimensions);
        }
    public:
        /*
        bool containsVertex = false;
        template <typename T>
        inline void checkVoxel(std::vector<T>& vertices) {
            for (auto& vertex : vertices) {
                float dist = glm::distance(vertex.position, center);
                float dotp = glm::dot(center, vertex.position);

                if ((0.5f <= dotp <= 1.5f) and (dist <= glm::length(dimensions))) {
                    content.insert(&vertex);
                    containsVertex = true;
                    break;
                }
            }
        }
        
        void checkNode() {
            for (auto& vert : content) {
                Primitive v = *vert;
                float dist = glm::distance(v.position, center);
                float dotp = glm::dot(center, v.position);

                if ((dotp < 0.5f) or (dotp > 1.5f) or (dist > glm::length(dimensions))) {
                    content.erase(vert);
                    std::cout << "Erased a vertex!\n";
                }
            }
        }
        */
    private:
        std::vector<lineList> createVertices() {
            std::vector<lineList> vertices(8);
            std::vector<glm::vec4> metrics = { { 1,  1,  1,  1 },
                                               { 1,  1, -1,  1 },
                                               { 1, -1, -1,  1 },
                                               { 1, -1,  1,  1 } };
            for (int i = 0; i < 8; i++) {
                if (i < 4) {
                    vertices[i].position = metrics[i];
                    vertices[i].normal = metrics[i];
                    vertices[i].color = glm::vec4(1.f);
                    vertices[i].texCoord = glm::vec2(0.f);
                }
                else {
                    metrics[i - 4] *= glm::vec4(-1, 1, 1, 1);
                    vertices[i].position = metrics[i - 4];
                    vertices[i].normal = metrics[i - 4];
                    vertices[i].color = glm::vec4(1.f);
                    vertices[i].texCoord = glm::vec2(0.f);
                }
            }
            return vertices;
        }
        std::vector<uint16_t> createIndices() {
            std::vector<uint16_t> indices(24);
            indices[7] = 0;
            indices[15] = 4;
            for (int x = 0; x < 7; x++) {
                indices[x] = static_cast<uint16_t>(floor((x + 1) / 2));
                indices[x + 8] = static_cast<uint16_t>(floor((x + 1) / 2) + 4);
            }
            int temp0 = 4, temp1 = 0;
            for (int x = 16; x < 24; x += 2) {
                indices[x] = temp0;
                indices[x + 1] = temp1;

                temp0++;
                temp1++;
            }
            return indices;
        }
        void genMatrix(glm::vec3 Center, glm::vec3 Dimensions) {
            glm::mat4 _matrix(1.f);
            matrix = glm::scale(glm::translate(_matrix, Center), Dimensions);
        }
    };

    struct Plane : Mesh {
        Plane(glm::vec2 Dimensions, glm::vec2 Resolution);
    private:
        std::vector<triangleList> createVertices(glm::vec2 dimensions, glm::vec2 resolution);
        std::vector<uint16_t> createIndices(glm::vec2 dimensions);
    };
}

#endif