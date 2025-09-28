#include "gl_test_renderer.hpp"
#include "engine/core/assets_manager.hpp"
#include "engine/renderer/open_gl/gl_buffer.hpp"
#include "engine/renderer/open_gl/gl_shader_program.hpp"
#include "engine/renderer/open_gl/gl_texture.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
#include <vector>

using namespace engine::renderer;
using namespace engine::core;

struct Vertex
{
  float pos[3];
  float normal[3];
  float uv[2];
};

static std::vector<Vertex> vertices = {
  // Передняя грань (Z = 0.5) - нормаль (0, 0, 1)
  { { -0.5f, -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
  { { 0.5f, -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
  { { 0.5f, 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
  { { -0.5f, 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },

  // Задняя грань (Z = -0.5) - нормаль (0, 0, -1)
  { { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f } },
  { { 0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } },
  { { 0.5f, 0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f } },
  { { -0.5f, 0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f } },

  // Верхняя грань (Y = 0.5) - нормаль (0, 1, 0)
  { { -0.5f, 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
  { { 0.5f, 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
  { { 0.5f, 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
  { { -0.5f, 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },

  // Нижняя грань (Y = -0.5) - нормаль (0, -1, 0)
  { { -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
  { { 0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } },
  { { 0.5f, -0.5f, 0.5f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } },
  { { -0.5f, -0.5f, 0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } },

  // Правая грань (X = 0.5) - нормаль (1, 0, 0)
  { { 0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
  { { 0.5f, -0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
  { { 0.5f, 0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } },
  { { 0.5f, 0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },

  // Левая грань (X = -0.5) - нормаль (-1, 0, 0)
  { { -0.5f, -0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
  { { -0.5f, -0.5f, 0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
  { { -0.5f, 0.5f, 0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },
  { { -0.5f, 0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } }
};

static std::vector<unsigned int> indices = { // Передняя грань
    0, 1, 2, 0, 2, 3,
    // Задняя грань
    4, 5, 6, 4, 6, 7,
    // Верхняя грань
    8, 9, 10, 8, 10, 11,
    // Нижняя грань
    12, 13, 14, 12, 14, 15,
    // Правая грань
    16, 17, 18, 16, 18, 19,
    // Левая грань
    20, 21, 22, 20, 22, 23};

GlTestRenderer::GlTestRenderer(engine::renderer::GlRenderer *renderer) : m_renderer{ renderer }
{
  auto tex = AssetsManager::loadTexture("wall.jpg");
  GLTextureDesc desc = {};
  desc.type = GLTextureType::Texture2D;
  desc.width = static_cast<uint32_t>(tex.width);
  desc.height = static_cast<uint32_t>(tex.height);
  desc.internalFormat = GLTextureInternalFormat::RGB8;
  desc.format = tex.channels == 4 ? GLTextureFormat::RGBA : GLTextureFormat::RGB;
  desc.dataType = GLTextureDataType::UByte;
  m_tex = std::make_unique<GLTexture>(desc);
  m_tex->setData(tex.data.get());

  m_materials = { // 1. Матовый пластик (красный)
    {
      glm::vec3(0.1f, 0.0f, 0.0f),// ambient
      10.0f,// shininess (низкая)
      glm::vec3(0.8f, 0.1f, 0.1f),// diffuse
      1.0f,// opacity (непрозрачный)
      glm::vec3(0.3f)// specular (слабые блики)
    },

    // 2. Полированный металл (золото)
    {
      glm::vec3(0.24725f, 0.1995f, 0.0745f),// ambient
      128.0f,// shininess (высокая)
      glm::vec3(0.75164f, 0.60648f, 0.22648f),// diffuse
      1.0f,// opacity
      glm::vec3(0.628281f, 0.555802f, 0.366065f)// specular (яркие блики)
    },

    // 3. Изумруд (полупрозрачный)
    {
      glm::vec3(0.0215f, 0.1745f, 0.0215f),// ambient
      76.8f,// shininess
      glm::vec3(0.07568f, 0.61424f, 0.07568f),// diffuse
      0.7f,// полупрозрачность
      glm::vec3(0.633f, 0.727811f, 0.633f)// specular
    },

    // 4. Резина (синяя)
    {
      glm::vec3(0.02f, 0.02f, 0.1f),// ambient
      15.0f,// shininess
      glm::vec3(0.1f, 0.1f, 0.6f),// diffuse
      1.0f,// opacity
      glm::vec3(0.1f)// specular (очень слабые блики)
    },

    // 5. Хромированная поверхность
    {
      glm::vec3(0.25f, 0.25f, 0.25f),// ambient
      256.0f,// shininess (очень высокая)
      glm::vec3(0.4f, 0.4f, 0.4f),// diffuse
      1.0f,// opacity
      glm::vec3(0.774597f)// specular (очень яркие блики)
    },

    // 6. Матовое стекло (бирюзовое)
    {
      glm::vec3(0.0f, 0.05f, 0.05f),// ambient
      50.0f,// shininess
      glm::vec3(0.4f, 0.8f, 0.8f),// diffuse
      0.5f,// полупрозрачность
      glm::vec3(0.3f)// specular
    }
  };

  m_instances.resize(70);
  for (size_t i = 0; i < m_instances.size(); ++i) {
    float x = (i % 30) * 1.5f - 15.0f;
    float z = (static_cast<float>(i) / 30) * 1.5f - 15.0f;
    m_instances[i].model = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, z));
    m_instances[i].materialId = static_cast<uint32_t>(i % m_materials.size());
  }

  m_vbo = GLBuffer::createVBO(vertices);
  m_ibo = GLBuffer::createIBO(indices);
  m_vao = std::make_unique<GLVertexArray>();
  m_lightVAO = std::make_unique<GLVertexArray>();
  m_ubo = GLBuffer::createUBO(m_uboData);
  m_materialsSSBO = GLBuffer::createSSBO(m_materials);
  m_ssbo = GLBuffer::createSSBO(m_instances);
  m_uboData.projection =
    glm::perspective(glm::radians(45.0f), static_cast<float>(m_width) / static_cast<float>(m_height), 0.1f, 1000.0f);
  m_uboData.view = glm::mat4(1.0f);
  m_uboData.view = glm::translate(m_uboData.view, glm::vec3(0.0f, 0.0f, -30.0f));
  m_ubo->update(m_uboData);

  m_vao->attachVertexBuffer(m_vbo.get(), 0, sizeof(Vertex), 0);
  m_vao->attachIndexBuffer(m_ibo.get());
  m_ubo->bindBase(0);
  m_ssbo->bindBase(2);
  m_materialsSSBO->bindBase(3);
  m_tex->bind(1);

  m_vao->setAttributeFormat(0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
  m_vao->bindAttribute(0, 0);
  m_vao->enableAttribute(0);

  m_vao->setAttributeFormat(1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
  m_vao->bindAttribute(1, 0);
  m_vao->enableAttribute(1);

  m_vao->setAttributeFormat(2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
  m_vao->bindAttribute(2, 0);
  m_vao->enableAttribute(2);

  m_lightVAO->attachVertexBuffer(m_vbo.get(), 0, sizeof(Vertex), 0);
  m_lightVAO->attachIndexBuffer(m_ibo.get());
  m_lightVAO->setAttributeFormat(0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
  m_lightVAO->bindAttribute(0, 0);
  m_lightVAO->enableAttribute(0);

  m_lightVAO->setAttributeFormat(1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
  m_lightVAO->bindAttribute(1, 0);
  m_lightVAO->enableAttribute(1);

  m_lightVAO->setAttributeFormat(2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
  m_lightVAO->bindAttribute(2, 0);
  m_lightVAO->enableAttribute(2);

#ifndef NDEBUG
  [[maybe_unused]] auto cbId =
    AssetsManager::subscribe([this]([[maybe_unused]] std::string filename) { m_shouldReloadShaders = true; });
#endif

  m_gltfModel = AssetsManager::loadModel("city/scene.gltf");

  reloadShaders();
}

void GlTestRenderer::reloadShaders()
{
  auto vertexShaderCode = AssetsManager::loadShader("test.vert");
  auto fragmentShaderCode = AssetsManager::loadShader("test.frag");
  ShaderProgramCreateDesc desc = { fragmentShaderCode, vertexShaderCode };
  m_shader = std::make_unique<GLShaderProgram>(desc);

  auto lightVertexShaderCode = AssetsManager::loadShader("light.vert");
  auto lightFragmentShaderCode = AssetsManager::loadShader("light.frag");
  ShaderProgramCreateDesc lightDesc = { lightFragmentShaderCode, lightVertexShaderCode };
  m_lightShader = std::make_unique<GLShaderProgram>(lightDesc);
}

void GlTestRenderer::render()
{
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  if (m_shouldReloadShaders) {
    reloadShaders();
    m_shouldReloadShaders = false;
  }
  m_shader->use();
  m_vao->bind();
  glDrawElementsInstanced(GL_TRIANGLES,
    static_cast<GLsizei>(indices.size()),
    GL_UNSIGNED_INT,
    nullptr,
    static_cast<GLsizei>(m_instances.size()));

  m_lightShader->use();
  m_lightVAO->bind();
  glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);

  // --- Garbage code to render glTF model ---
  m_shader->use(); // Or a different shader if you have one for models

  const auto& scene = m_gltfModel.scenes[m_gltfModel.defaultScene];
  for (int node_index : scene.nodes) {
    renderNode(node_index);
  }
  // --- End of garbage code ---

  ImGui::Begin("Test");
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
    static_cast<double>(1000.0f / ImGui::GetIO().Framerate),
    static_cast<double>(ImGui::GetIO().Framerate));
  ImGui::ColorEdit3("Light Color", glm::value_ptr(m_uboData.lightColor));
  ImGui::SliderFloat3("Light Position", glm::value_ptr(m_uboData.lightPos), -10.0f, 10.0f);
  ImGui::SliderFloat("Constant", &m_uboData.constant, 0.0f, 1.0f);
  ImGui::SliderFloat("Linear", &m_uboData.linear, 0.0f, 1.0f);
  ImGui::SliderFloat("Quadratic", &m_uboData.quadratic, 0.0f, 1.0f);
  ImGui::End();

  ImGui::Begin("Material");
  ImGui::ColorEdit3("Ambient", glm::value_ptr(m_materials[0].ambient));
  ImGui::ColorEdit3("Diffuse", glm::value_ptr(m_materials[0].diffuse));
  ImGui::ColorEdit3("Specular", glm::value_ptr(m_materials[0].specular));
  ImGui::SliderFloat("Shininess", &m_materials[0].shininess, 0.0f, 128.0f);
  ImGui::SliderFloat("Opacity", &m_materials[0].opacity, 0.0f, 1.0f);
  ImGui::End();

  ImGui::Begin("Directional Light");
  ImGui::ColorEdit3("Color", glm::value_ptr(m_uboData.globalLightColor));
  ImGui::SliderFloat3("Direction", glm::value_ptr(m_uboData.globalLightDir), -1.0f, 1.0f);
  ImGui::End();
}

void GlTestRenderer::resize(int width, int height)
{
  m_width = width;
  m_height = height;
  m_uboData.projection =
    glm::perspective(glm::radians(60.0f), static_cast<float>(m_width) / static_cast<float>(m_height), 0.1f, 100.0f);
}

void GlTestRenderer::update(float dt)
{
  m_uboData.elapsedTime += dt;
  m_ubo->update(m_uboData);

  for (size_t i = 0; i < m_instances.size(); ++i) {
    m_instances[i].model = glm::rotate(
      m_instances[i].model, glm::radians(dt * (static_cast<float>(i) * 0.01f + 1.0f)), glm::vec3(1.0f, 1.0f, 1.0f));
  }
  m_uboData.lightPos = glm::vec3(-8.0f + std::sin(m_uboData.elapsedTime) * 6.0f, 2.0f, -15.0f);
  m_ssbo->update(m_instances);
  m_materialsSSBO->update(m_materials);
}

void GlTestRenderer::renderNode(int node_index) {
    const auto& node = m_gltfModel.nodes[node_index];
    if (node.mesh > -1) {
        const auto& mesh = m_gltfModel.meshes[node.mesh];
        for (const auto& primitive : mesh.primitives) {
            const auto& indexAccessor = m_gltfModel.accessors[primitive.indices];
            const auto& indexBufferView = m_gltfModel.bufferViews[indexAccessor.bufferView];
            const auto& indexBuffer = m_gltfModel.buffers[indexBufferView.buffer];

            const float* positionBuffer = nullptr;
            const float* normalBuffer = nullptr;
            const float* texcoordBuffer = nullptr;
            size_t numVertices = 0;

            if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
                const auto& accessor = m_gltfModel.accessors.at(primitive.attributes.at("POSITION"));
                const auto& bufferView = m_gltfModel.bufferViews.at(accessor.bufferView);
                const auto& buffer = m_gltfModel.buffers.at(bufferView.buffer);
                numVertices = accessor.count;
                positionBuffer = reinterpret_cast<const float*>(&(buffer.data[bufferView.byteOffset + accessor.byteOffset]));
            }

            if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
                const auto& accessor = m_gltfModel.accessors.at(primitive.attributes.at("NORMAL"));
                const auto& bufferView = m_gltfModel.bufferViews.at(accessor.bufferView);
                const auto& buffer = m_gltfModel.buffers.at(bufferView.buffer);
                normalBuffer = reinterpret_cast<const float*>(&(buffer.data[bufferView.byteOffset + accessor.byteOffset]));
            }

            if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
                const auto& accessor = m_gltfModel.accessors.at(primitive.attributes.at("TEXCOORD_0"));
                const auto& bufferView = m_gltfModel.bufferViews.at(accessor.bufferView);
                const auto& buffer = m_gltfModel.buffers.at(bufferView.buffer);
                texcoordBuffer = reinterpret_cast<const float*>(&(buffer.data[bufferView.byteOffset + accessor.byteOffset]));
            }


            if (positionBuffer) {
                std::vector<Vertex> modelVertices;
                modelVertices.resize(numVertices);
                for(size_t i=0; i<numVertices; ++i) {
                    modelVertices[i].pos[0] = positionBuffer[i*3 + 0];
                    modelVertices[i].pos[1] = positionBuffer[i*3 + 1];
                    modelVertices[i].pos[2] = positionBuffer[i*3 + 2];

                    if(normalBuffer) {
                        modelVertices[i].normal[0] = normalBuffer[i*3 + 0];
                        modelVertices[i].normal[1] = normalBuffer[i*3 + 1];
                        modelVertices[i].normal[2] = normalBuffer[i*3 + 2];
                    }

                    if(texcoordBuffer) {
                        modelVertices[i].uv[0] = texcoordBuffer[i*2 + 0];
                        modelVertices[i].uv[1] = texcoordBuffer[i*2 + 1];
                    } else {
                        modelVertices[i].uv[0] = 0.0f;
                        modelVertices[i].uv[1] = 0.0f;
                    }
                }

                auto vbo = GLBuffer::createVBO(modelVertices);
                std::unique_ptr<GLBuffer> ibo;

                const void* indexData = &(indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset]);

                switch (indexAccessor.componentType) {
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
                        std::vector<unsigned int> indices(indexAccessor.count);
                        memcpy(indices.data(), indexData, indexAccessor.count * sizeof(unsigned int));
                        ibo = GLBuffer::createIBO(indices);
                        break;
                    }
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
                        std::vector<unsigned short> indices(indexAccessor.count);
                        memcpy(indices.data(), indexData, indexAccessor.count * sizeof(unsigned short));
                        ibo = GLBuffer::createIBO(indices);
                        break;
                    }
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
                        std::vector<unsigned char> indices(indexAccessor.count);
                        memcpy(indices.data(), indexData, indexAccessor.count * sizeof(unsigned char));
                        ibo = GLBuffer::createIBO(indices);
                        break;
                    }
                    default:
                        // Should not happen for valid glTF
                        continue;
                }

                if (!ibo) {
                    continue;
                }

                auto vao = std::make_unique<GLVertexArray>();
                vao->attachVertexBuffer(vbo.get(), 0, sizeof(Vertex), 0);
                vao->attachIndexBuffer(ibo.get());

                vao->setAttributeFormat(0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
                vao->bindAttribute(0, 0);
                vao->enableAttribute(0);

                vao->setAttributeFormat(1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
                vao->bindAttribute(1, 0);
                vao->enableAttribute(1);

                vao->setAttributeFormat(2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
                vao->bindAttribute(2, 0);
                vao->enableAttribute(2);

                vao->bind();
                glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexAccessor.count), indexAccessor.componentType, (void*)0);
            }
        }
    }

    for (int child_index : node.children) {
        renderNode(child_index);
    }
}