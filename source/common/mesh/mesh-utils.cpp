#include "mesh-utils.hpp"
#include <limits>

// We will use "Tiny OBJ Loader" to read and process '.obj" files
#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobj/tiny_obj_loader.h>

#include <iostream>

our::Mesh* our::mesh_utils::loadOBJ(const std::string& filename) {

    // The data that we will use to initialize our mesh
    std::vector<our::Vertex> vertices;
    std::vector<GLuint> elements;

    // Since the OBJ can have duplicated vertices, we make them unique using this map
    // The key is the vertex, the value is its index in the vector "vertices".
    // That index will be used to populate the "elements" vector.
    std::unordered_map<our::Vertex, GLuint> vertex_map;

    // The data loaded by Tiny OBJ Loader
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str())) {
        std::cerr << "Failed to load obj file \"" << filename << "\" due to error: " << err << std::endl;
        return nullptr;
    }
    if (!warn.empty()) {
        std::cout << "WARN while loading obj file \"" << filename << "\": " << warn << std::endl;
    }

    float minX = std::numeric_limits<float>::max(), maxX = std::numeric_limits<float>::min();
    float minZ = std::numeric_limits<float>::max(), maxZ = std::numeric_limits<float>::min();
    glm::vec3 farLeft, farRight, zNearest, zFurthest;

    // An obj file can have multiple shapes where each shape can have its own material
    // Ideally, we would load each shape into a separate mesh or store the start and end of it in the element buffer to be able to draw each shape separately
    // But we ignored this fact since we don't plan to use multiple materials in the examples
    for (const auto &shape : shapes) {
        for (const auto &index : shape.mesh.indices) {
            Vertex vertex = {};

            // Read the data for a vertex from the "attrib" object
            vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
            };

            vertex.tex_coord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1]
            };


            vertex.color = {
                    attrib.colors[3 * index.vertex_index + 0] * 255,
                    attrib.colors[3 * index.vertex_index + 1] * 255,
                    attrib.colors[3 * index.vertex_index + 2] * 255,
                    255
            };

            // See if we already stored a similar vertex
            auto it = vertex_map.find(vertex);
            if (it == vertex_map.end()) {
                // if no, add it to the vertices and record its index
                auto new_vertex_index = static_cast<GLuint>(vertices.size());
                vertex_map[vertex] = new_vertex_index;
                elements.push_back(new_vertex_index);
                vertices.push_back(vertex);
            } else {
                // if yes, just add its index in the elements vector
                elements.push_back(it->second);
            }

            // Testing whether this vertex's x-coordinate is closer to the far left.
            if (vertex.position.x < minX) {
                farLeft = glm::vec3(vertex.position);
                minX = vertex.position.x;
            }

            // Testing whether this vertex's x-coordinate is closer to the far right.
            if (vertex.position.x > maxX) {
                farRight = glm::vec3(vertex.position);
                maxX = vertex.position.x;
            }

            if (vertex.position.z < minZ) {
                zFurthest = glm::vec3(vertex.position);
                minZ = vertex.position.z;
            }

            if (vertex.position.z > maxZ) {
                zNearest = glm::vec3(vertex.position);
                maxZ = vertex.position.z;
            }
        }
    }

    our::Mesh *newMesh = new our::Mesh(vertices, elements);
    newMesh->farLeft = farLeft; newMesh->farRight = farRight;
    newMesh->zNearest = zNearest; newMesh->zFurthest = zFurthest;
    return newMesh;
}


// This function is similar to the above one, provided originally with the project's
// code, but instead of returning our::Mesh, returns our::MultipleMeshes. That is,
// this function loads and returns the objects in the .obj file into separate meshes
// encapsulated by the our::MultipleMeshes (which has as a member the list of meshes it contains)
// while the above one merges all together into one mesh.
our::MultipleMeshes* our::mesh_utils::loadMultipleOBJ(const std::string& filename) {

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str())) {
        std::cerr << "Failed to load obj file \"" << filename << "\" due to error: " << err << std::endl;
        return nullptr;
    }
    if (!warn.empty()) {
        std::cout << "WARN while loading obj file \"" << filename << "\": " << warn << std::endl;
    }

    float minX = std::numeric_limits<float>::max(), maxX = std::numeric_limits<float>::min();
    float minZ = std::numeric_limits<float>::max(), maxZ = std::numeric_limits<float>::min();
    // These will hold the far left and the far right vertices in the mesh.
    glm::vec3 farLeft, farRight, zNearest, zFurthest;

    // Creating the list of meshes to return later in an our::MultipleMeshes object.
    std::list<our::Mesh*>* listOfMeshes = new std::list<our::Mesh*>();

    for (const auto &shape : shapes) {
        
        // The main difference here to the above function: we define "vertices"
        // and "elements" PER shape, and we create a new mesh of a new pair of them
        // for EVERY shape.

        std::vector<our::Vertex> vertices;
        std::vector<GLuint> elements;

        std::unordered_map<our::Vertex, GLuint> vertex_map;

        for (const auto &index : shape.mesh.indices) {
            Vertex vertex = {};

            vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
            };

            vertex.tex_coord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1]
            };


            vertex.color = {
                    attrib.colors[3 * index.vertex_index + 0] * 255,
                    attrib.colors[3 * index.vertex_index + 1] * 255,
                    attrib.colors[3 * index.vertex_index + 2] * 255,
                    255
            };

            auto it = vertex_map.find(vertex);
            if (it == vertex_map.end()) {
                auto new_vertex_index = static_cast<GLuint>(vertices.size());
                vertex_map[vertex] = new_vertex_index;
                elements.push_back(new_vertex_index);
                vertices.push_back(vertex);
            } else {
                elements.push_back(it->second);
            }

            // Testing whether this vertex's x-coordinate is closer to the far left.
            if (vertex.position.x < minX) {
                farLeft = glm::vec3(vertex.position);
                minX = vertex.position.x;
            }

            // Testing whether this vertex's x-coordinate is closer to the far right.
            if (vertex.position.x > maxX) {
                farRight = glm::vec3(vertex.position);
                maxX = vertex.position.x;
            }

            if (vertex.position.z < minZ) {
                zFurthest = glm::vec3(vertex.position);
                minZ = vertex.position.z;
            }

            if (vertex.position.z > maxZ) {
                zNearest = glm::vec3(vertex.position);
                maxZ = vertex.position.z;
            }
        }

        our::Mesh *newMesh = new our::Mesh(vertices, elements);
        newMesh->zNearest = zNearest; newMesh->zFurthest = zFurthest;
        newMesh->farLeft = farLeft; newMesh->farRight = farRight;

        // Creating the mesh and pushing it to the listOfMeshes.
        listOfMeshes->push_back(newMesh);
    }

    // Returning the new our::MultipleMeshes object.
    return new our::MultipleMeshes(listOfMeshes);

}

// Create a sphere (the vertex order in the triangles are CCW from the outside)
// Segments define the number of divisions on the both the latitude and the longitude
our::Mesh* our::mesh_utils::sphere(const glm::ivec2& segments){
    std::vector<our::Vertex> vertices;
    std::vector<GLuint> elements;

    // We populate the sphere vertices by looping over its longitude and latitude
    for(int lat = 0; lat <= segments.y; lat++){
        float v = (float)lat / segments.y;
        float pitch = v * glm::pi<float>() - glm::half_pi<float>();
        float cos = glm::cos(pitch), sin = glm::sin(pitch);
        for(int lng = 0; lng <= segments.x; lng++){
            float u = (float)lng/segments.x;
            float yaw = u * glm::two_pi<float>();
            glm::vec3 normal = {cos * glm::cos(yaw), sin, cos * glm::sin(yaw)};
            glm::vec3 position = normal;
            glm::vec2 tex_coords = glm::vec2(u, v);
            our::Color color = our::Color(255, 255, 255, 255);
            vertices.push_back({position, color, tex_coords, normal});
        }
    }

    for(int lat = 1; lat <= segments.y; lat++){
        int start = lat*(segments.x+1);
        for(int lng = 1; lng <= segments.x; lng++){
            int prev_lng = lng-1;
            elements.push_back(lng + start);
            elements.push_back(lng + start - segments.x - 1);
            elements.push_back(prev_lng + start - segments.x - 1);
            elements.push_back(prev_lng + start - segments.x - 1);
            elements.push_back(prev_lng + start);
            elements.push_back(lng + start);
        }
    }

    return new our::Mesh(vertices, elements);
}