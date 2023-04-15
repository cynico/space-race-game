#pragma once

#include <glad/gl.h>
#include "GLFW/glfw3.h"
#include "vertex.hpp"
#include <iostream>

namespace our {

    #define ATTRIB_LOC_POSITION 0
    #define ATTRIB_LOC_COLOR    1
    #define ATTRIB_LOC_TEXCOORD 2
    #define ATTRIB_LOC_NORMAL   3

    class Mesh {
        // Here, we store the object names of the 3 main components of a mesh:
        // A vertex array object, A vertex buffer and an element buffer
        unsigned int VBO, EBO;
        unsigned int VAO;
        // We need to remember the number of elements that will be draw by glDrawElements 
        GLsizei elementCount;
    public:

        void getAll() {
            std::cout << VBO << std::endl;
            std::cout << VAO << std::endl;
            std::cout << EBO << std::endl;

            std::cout << elementCount << std::endl;
        }

        // The constructor takes two vectors:
        // - vertices which contain the vertex data.
        // - elements which contain the indices of the vertices out of which each rectangle will be constructed.
        // The mesh class does not keep a these data on the RAM. Instead, it should create
        // a vertex buffer to store the vertex data on the VRAM,
        // an element buffer to store the element data on the VRAM,
        // a vertex array object to define how to read the vertex & element buffer during rendering 
        Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& elements)
        {
            // DONE: (Req 2) Write this function
            // remember to store the number of elements in "elementCount" since you will need it for drawing
            // For the attribute locations, use the constants defined above: ATTRIB_LOC_POSITION, ATTRIB_LOC_COLOR, etc

            // Creating Vertex Buffer
            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);            

            // Creating element buffer.
            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(unsigned int), &elements[0], GL_STATIC_DRAW);

            elementCount = elements.size();
            
            // Creating a vertex array.
            glGenVertexArrays(1, &VAO);

            glBindVertexArray(VAO);

                glBindBuffer(GL_ARRAY_BUFFER, VBO);

                glEnableVertexAttribArray(ATTRIB_LOC_POSITION);
                glVertexAttribPointer(ATTRIB_LOC_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

                glEnableVertexAttribArray(ATTRIB_LOC_COLOR);
                glVertexAttribPointer(ATTRIB_LOC_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void *)offsetof(Vertex, color));

                glEnableVertexAttribArray(ATTRIB_LOC_TEXCOORD);
                glVertexAttribPointer(ATTRIB_LOC_TEXCOORD, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void *)offsetof(Vertex, tex_coord));

                glEnableVertexAttribArray(ATTRIB_LOC_NORMAL);
                glVertexAttribPointer(ATTRIB_LOC_NORMAL, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void *)offsetof(Vertex, normal));

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

            glBindVertexArray(0);
        }

        // this function should render the mesh
        void draw() 
        {
            // DONE: (Req 2) Write this function
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, elementCount, GL_UNSIGNED_INT, (void *)0);
        }

        // this function should delete the vertex & element buffers and the vertex array object
        ~Mesh(){

            // DONE: (Req 2) Write this function
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
            glDeleteBuffers(1, &EBO);

        }

        Mesh(Mesh const &) = delete;
        Mesh &operator=(Mesh const &) = delete;
    };

}