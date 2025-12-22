// grid.h
#ifndef GRID_H
#define GRID_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <shader.h>

class Grid {
public:
    void init(Shader& shaderRef, float half = 50.0f) {
        shader = &shaderRef;
        halfSize = half;

        float plane[] = {
            -halfSize, 0.0f, -halfSize,
             halfSize, 0.0f, -halfSize,
             halfSize, 0.0f,  halfSize,
            -halfSize, 0.0f,  halfSize
        };
        unsigned int indices[] = {0, 1, 2, 0, 2, 3};

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(plane), plane, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    void draw(const glm::mat4& view,
              const glm::mat4& projection,
              const glm::vec3& camPos)
    {
        if (!shader) return;

        shader->use();
        glm::mat4 model = glm::translate(glm::mat4(1.0f),
                                         glm::vec3(camPos.x, 0.0f, camPos.z));
        shader->setMat4("model", model);
        shader->setMat4("view", view);
        shader->setMat4("projection", projection);

        shader->setVec3("colorThin", 0.3f, 0.3f, 0.3f);
        shader->setVec3("colorThick", 0.6f, 0.6f, 0.6f);
        shader->setFloat("spacing", 1.0f);
        shader->setFloat("thickEvery", 10.0f);
        shader->setFloat("fadeDist", 60.0f);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

private:
    unsigned int VAO = 0, VBO = 0, EBO = 0;
    Shader* shader = nullptr;
    float halfSize = 50.0f;
};

#endif
