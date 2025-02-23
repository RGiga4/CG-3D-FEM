#include <mesh.hpp>

#include <functional>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <config.hpp>
#include <buffer.hpp>

/*void
geometry::bind() {
    glBindVertexArray(vao);
}

void
geometry::release() {
    glBindVertexArray(0);
}

void
geometry::destroy() {
    release();
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
}*/



/*
std::vector<geometry>
loadScene(const char* filename, bool smooth, const glm::vec4& color) {
    Assimp::Importer importer;
    int process = aiProcess_JoinIdenticalVertices;
    if (smooth) {
        process |= aiProcess_GenSmoothNormals;
    } else {
        process |= aiProcess_GenNormals;
    }
    const aiScene* scene = importer.ReadFile(SHADER_ROOT + "../models/" + filename, process);
    if (scene == nullptr) return {};

    std::vector<geometry> objects;
    std::function<void(aiNode*, glm::mat4)> traverse;
    traverse = [&](aiNode* node, glm::mat4 t) {
        aiMatrix4x4 aim = node->mTransformation;
        glm::mat4 new_t(
            aim.a1, aim.b1, aim.c1, aim.d1,
            aim.a2, aim.b2, aim.c2, aim.d2,
            aim.a3, aim.b3, aim.c3, aim.d3,
            aim.a4, aim.b4, aim.c4, aim.d4
        );
        t = new_t * t;

        aiMatrix4x4 local_t;
        if (node->mNumMeshes > 0) {
            for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
                aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

                geometry m{};
                m.positions.resize(mesh->mNumVertices);
                m.normals.resize(mesh->mNumVertices);
                m.colors.resize(mesh->mNumVertices, glm::vec4(0.9f, 0.9f, 0.9f, 1.f));
                m.faces.resize(mesh->mNumFaces);

                float* vbo_data = new float[mesh->mNumVertices * 10];
                unsigned int* ibo_data = new unsigned int[mesh->mNumFaces * 3];
                for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
                    glm::vec3 pos(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
                    glm::vec3 nrm(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

                    m.positions[i] = pos;
                    m.normals[i] = nrm;
                    m.colors[i] = color;

                    vbo_data[10 * i + 0] = pos[0];
                    vbo_data[10 * i + 1] = pos[1];
                    vbo_data[10 * i + 2] = pos[2];
                    vbo_data[10 * i + 3] = nrm[0];
                    vbo_data[10 * i + 4] = nrm[1];
                    vbo_data[10 * i + 5] = nrm[2];
                    vbo_data[10 * i + 6] = color[0];
                    vbo_data[10 * i + 7] = color[1];
                    vbo_data[10 * i + 8] = color[2];
                    vbo_data[10 * i + 9] = color[3];
                }

                for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
                    glm::uvec3 face(mesh->mFaces[i].mIndices[0],
                                    mesh->mFaces[i].mIndices[1],
                                    mesh->mFaces[i].mIndices[2]);
                    m.faces[i] = face;
                    ibo_data[i * 3 + 0] = face[0];
                    ibo_data[i * 3 + 1] = face[1];
                    ibo_data[i * 3 + 2] = face[2];
                }

                glGenVertexArrays(1, &m.vao);
                glBindVertexArray(m.vao);
                m.vbo = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, mesh->mNumVertices * 10 * sizeof(float), vbo_data);
                m.ibo = makeBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, mesh->mNumFaces * 3 * sizeof(unsigned int), ibo_data);
                glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)0);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(3*sizeof(float)));
                glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(6*sizeof(float)));
                glEnableVertexAttribArray(0);
                glEnableVertexAttribArray(1);
                glEnableVertexAttribArray(2);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ibo);
                m.transform = t;
                m.vertex_count = 3*mesh->mNumFaces;
                objects.push_back(m);

                delete [] vbo_data;
                delete [] ibo_data;
            }
        }

        for (uint32_t i = 0; i < node->mNumChildren; ++i) {
            traverse(node->mChildren[i], t);
        }
    };

    traverse(scene->mRootNode, glm::identity<glm::mat4>());
    return objects;
}

geometry
loadMesh(const char* filename, bool smooth) {
    return loadScene(filename, smooth)[0];
}

geometry
loadMesh(const char* filename, bool smooth, const glm::vec4& color) {
    return loadScene(filename, smooth, color)[0];
}*/
