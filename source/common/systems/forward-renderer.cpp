#include "forward-renderer.hpp"
#include "../mesh/mesh-utils.hpp"
#include "../texture/texture-utils.hpp"
#include "components/light.hpp"
#include "glad/gl.h"
#include "glm/geometric.hpp"
#include "shader/shader.hpp"
#include <glm/ext/matrix_transform.hpp>
#include "../our-util.hpp"

namespace our {

    void ForwardRenderer::initialize(glm::ivec2 windowSize, const nlohmann::json& config){
        // First, we store the window size for later use
        this->windowSize = windowSize;

        // Then we check if there is a sky texture in the configuration
        if(config.contains("sky")){
            // First, we create a sphere which will be used to draw the sky
            this->skySphere = mesh_utils::sphere(glm::ivec2(16, 16));
            
            // We can draw the sky using the same shader used to draw textured objects
            ShaderProgram* skyShader = new ShaderProgram();
            skyShader->attach("assets/shaders/textured.vert", GL_VERTEX_SHADER);
            skyShader->attach("assets/shaders/textured.frag", GL_FRAGMENT_SHADER);
            skyShader->link();
            
            //DONE: (Req 10) Pick the correct pipeline state to draw the sky
            // Hints: the sky will be drawn after the opaque objects so we would need depth testing but which depth funtion should we pick?
            // We will draw the sphere from the inside, so what options should we pick for the face culling.
            
            // Comment: 
            // Here, we create the sky pipeline state. Depth testing must be enabled,
            // and the function must be GL_EQUAL for the following reason.
            // The sky will be drawn AFTER the opaque objects, and before the transparent ones.
            // If the depth buffer has default values of 1.0 where no object was drawn,
            // and if I always transform the sky points to have a z component of 1.0, then the function
            // should be GL_EQUAL, or GL_LEQUAL. I have choose GL_LEQUAL for safety.
            // The face culling should be enabled, and the front face is the one to be culled, since
            // we will be INSIDE the sphere. We don't need to modify the default value 
            // of the front face, which is already CCW.
            PipelineState skyPipelineState{};
            skyPipelineState.depthTesting.enabled = true;
            skyPipelineState.depthTesting.function = GL_LEQUAL;
            skyPipelineState.faceCulling.enabled = true;
            skyPipelineState.faceCulling.culledFace = GL_FRONT;
            
            // Load the sky texture (note that we don't need mipmaps since we want to avoid any unnecessary blurring while rendering the sky)
            std::string skyTextureFile = config.value<std::string>("sky", "");
            Texture2D* skyTexture = texture_utils::loadImage(skyTextureFile, false);

            // Setup a sampler for the sky 
            Sampler* skySampler = new Sampler();
            skySampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            skySampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            skySampler->set(GL_TEXTURE_WRAP_S, GL_REPEAT);
            skySampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Combine all the aforementioned objects (except the mesh) into a material 
            this->skyMaterial = new TexturedMaterial();
            this->skyMaterial->shader = skyShader;
            this->skyMaterial->texture = skyTexture;
            this->skyMaterial->sampler = skySampler;
            this->skyMaterial->pipelineState = skyPipelineState;
            this->skyMaterial->tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            this->skyMaterial->alphaThreshold = 1.0f;
            this->skyMaterial->transparent = false;
        }

        // Then we check if there is a postprocessing shader in the configuration
        
        // Comment:
        // Here, we generate the frame buffer, and we bind it to the GL_FRAMEBUFFER so that anything
        // that gets rendered, gets rendered to it, not the default framebuffer.
        // We create the two textures associated with it: for the color buffer and the depth buffer.
        // We create the attachments of the color buffer texture and the depth buffer texture using: glFramebufferTexture2D call.
        // We check the status of completion of the framebuffer, and just print an indicative statement.
        // Then we unbind the framebuffer, and generate the vertex array of the postprocess material, to be later used 
        // when rendering the postprocess material/mesh.
        if(config.contains("postprocess")){
            
            //DONE: (Req 11) Create a framebuffer
            glGenFramebuffers(1, &(this->postprocessFrameBuffer));
            glBindFramebuffer(GL_FRAMEBUFFER, this->postprocessFrameBuffer); 

            //DONE: (Req 11) Create a color and a depth texture and attach them to the framebuffer
            // Hints: The color format can be (Red, Green, Blue and Alpha components with 8 bits for each channel).
            // The depth format can be (Depth component with 24 bits).
            this->colorTarget = texture_utils::empty(GL_RGBA8, this->windowSize);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->colorTarget->getOpenGLName(), 0);

            this->depthTarget = texture_utils::empty(GL_DEPTH_COMPONENT24, this->windowSize);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->depthTarget->getOpenGLName(), 0);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

            //DONE: (Req 11) Unbind the framebuffer just to be safe
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // Create a vertex array to use for drawing the texture
            glGenVertexArrays(1, &postProcessVertexArray);

            // Create a sampler to use for sampling the scene texture in the post processing shader
            Sampler* postprocessSampler = new Sampler();
            postprocessSampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            postprocessSampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            postprocessSampler->set(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            postprocessSampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Create the post processing shader
            ShaderProgram* postprocessShader = new ShaderProgram();
            postprocessShader->attach("assets/shaders/fullscreen.vert", GL_VERTEX_SHADER);
            postprocessShader->attach(config.value<std::string>("postprocess", ""), GL_FRAGMENT_SHADER);
            postprocessShader->link();

            // Create a post processing material
            postprocessMaterial = new TexturedMaterial();
            postprocessMaterial->shader = postprocessShader;
            postprocessMaterial->texture = colorTarget;
            postprocessMaterial->sampler = postprocessSampler;
            // The default options are fine but we don't need to interact with the depth buffer
            // so it is more performant to disable the depth mask
            postprocessMaterial->pipelineState.depthMask = false;
        }
    }

    void ForwardRenderer::destroy(){
        // Delete all objects related to the sky
        if(skyMaterial){
            delete skySphere;
            delete skyMaterial->shader;
            delete skyMaterial->texture;
            delete skyMaterial->sampler;
            delete skyMaterial;
        }
        // Delete all objects related to post processing
        if(postprocessMaterial){
            glDeleteFramebuffers(1, &postprocessFrameBuffer);
            glDeleteVertexArrays(1, &postProcessVertexArray);
            delete colorTarget;
            delete depthTarget;
            delete postprocessMaterial->sampler;
            delete postprocessMaterial->shader;
            delete postprocessMaterial;
        }
    }

    void ForwardRenderer::render(World* world){
        // First of all, we search for a camera and for all the mesh renderers
        CameraComponent* camera = nullptr;
        opaqueCommands.clear();
        transparentCommands.clear();
        for(auto entity : world->getEntities()){
            // If we hadn't found a camera yet, we look for a camera in this entity
            if(!camera) camera = entity->getComponent<CameraComponent>();
            // If this entity has a mesh renderer component
            if(auto meshRenderer = entity->getComponent<MeshRendererComponent>(); meshRenderer){
                // We construct a command from it
                RenderCommand command;
                command.localToWorld = meshRenderer->getOwner()->getLocalToWorldMatrix();
                command.center = glm::vec3(command.localToWorld * glm::vec4(0, 0, 0, 1));
                command.mesh = meshRenderer->mesh;
                command.material = meshRenderer->material;
                // if it is transparent, we add it to the transparent commands list
                if(command.material->transparent){
                    transparentCommands.push_back(command);
                } else {
                // Otherwise, we add it to the opaque command list
                    opaqueCommands.push_back(command);
                }
            }
        }

        // If there is no camera, we return (we cannot render without a camera)
        if(camera == nullptr) return;

        //DONE: (Req 9) Modify the following line such that "cameraForward" contains a vector pointing the camera forward direction
        // HINT: See how you wrote the CameraComponent::getViewMatrix, it should help you solve this one
        
        // Comment:
        // cameraForward is simply the (eye) of the camera, that is, the center the camera is looking at.
        // But it is transformed with the camera's parent entity localToWorldMatrix (the model matrix of the entity).
        // We sort the transparent meshes based on the length between this cameraForward point,
        // and the center of the transparent object.
        // Remember that we draw the transparent objects from the furthest to the nearest.
        glm::vec3 cameraForward = glm::vec3(camera->getOwner()->getLocalToWorldMatrix() * glm::vec4(0, 0, 0, 1)); // eye
        

        // Comment: cameraPosition is used later when setting the uniform camera_position for lit materials.
        glm::vec3 cameraPosition = glm::vec3(camera->getOwner()->getLocalToWorldMatrix() * glm::vec4(0,0,0, 1));
        
        std::sort(transparentCommands.begin(), transparentCommands.end(), [cameraForward](const RenderCommand& first, const RenderCommand& second){
            //DONE: (Req 9) Finish this function
            // HINT: the following return should return true if "first" should be drawn before "second".
            return length(cameraForward - first.center) >= length(cameraForward - second.center);
        });

        //DONE: (Req 9) Get the camera ViewProjection matrix and store it in VP
        glm::mat4 VP = camera->getProjectionMatrix(windowSize) * camera->getViewMatrix();

        //DONE: (Req 9) Set the OpenGL viewport using viewportStart and viewportSize
        glViewport(0, 0, this->windowSize[0], this->windowSize[1]);

        //DONE: (Req 9) Set the clear color to black and the clear depth to 1
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearDepth(1.0f);

        //DONE: (Req 9) Set the color mask to true and the depth mask to true (to ensure the glClear will affect the framebuffer)
        glColorMask(true, true, true, true);
        glDepthMask(true);

        // If there is a postprocess material, bind the framebuffer
        if(postprocessMaterial){
            //DONE: (Req 11) bind the framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, this->postprocessFrameBuffer);
        }

        //DONE: (Req 9) Clear the color and depth buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        //DONE: (Req 9) Draw all the opaque commands
        // Don't forget to set the "transform" uniform to be equal the model-view-projection matrix for each render command
        
        // Comment:
        // This part is fairly simple. We loop over the opaque objects. For each object, 
        // we setup the material, set relevant uniforms, depending on the material types. 
        // Then, we draw the opaque mesh.
        for (auto it = opaqueCommands.begin(); it != opaqueCommands.end(); it++) {
            
            // Obtaining the transform matrix, used for all materials except lit material.
            glm::mat4 transform = VP * (*it).localToWorld;
            ShaderProgram* currentShader = (*it).material->shader;

            // Setting up the material and using the shader.
            (*it).material->setup();
            currentShader->use();
            
            // If it's a lit material, set the light-relevant uniforms.
            if ( dynamic_cast<LitMaterial*>((*it).material) ) {

                // First, setting the number of all light sources within the world.
                currentShader->set("light_count", (int)world->setOfLights.size());

                // Setting sky colors. We are in space, there's no sense in making one different
                // than the other, so I made them all blackish gray.
                currentShader->set("sky.top", glm::vec3(0.1f, 0.1f, 0.1f));
                currentShader->set("sky.horizon", glm::vec3(0.1f, 0.1f, 0.1f));
                currentShader->set("sky.bottom", glm::vec3(0.1f, 0.1f, 0.1f));
                
                int i = 0;

                // Looping over all existent lights components.
                for (auto lightIterator = world->setOfLights.begin(); lightIterator != world->setOfLights.end(); lightIterator++) {
                    
                    // Setting the light's parameters: the type, color, attenuation, and cone_angles. 
                    currentShader->set( our::string_format("lights[%d].type", i), (int)(*lightIterator)->type);
                    currentShader->set(our::string_format("lights[%d].color", i), (*lightIterator)->color);
                    currentShader->set(our::string_format("lights[%d].attenuation", i), (*lightIterator)->attenuation);
                    currentShader->set(our::string_format("lights[%d].cone_angles", i), (*lightIterator)->cone_angles);
                    
                    // Setting the light's position and direction.
                    // The direction is something internal to the light component in case of a SPOT light and a directional light.
                    // In the case of a point light, it's calculated in the shaders.
                    // We get the position from the parent entity.
                    currentShader->set(our::string_format("lights[%d].direction", i), (*lightIterator)->direction);
                    currentShader->set(our::string_format("lights[%d].position", i), (*lightIterator)->getOwner()->localTransform.position);


                    // Setting the M matrix, M_IT matrix, and VP matrix for use in the lit.vert shader.
                    currentShader->set("M", (*it).localToWorld);
                    currentShader->set("M_IT",  glm::transpose(glm::inverse((*it).localToWorld)));
                    currentShader->set("VP", VP);
                    
                    // Setting the camera position.
                    currentShader->set("camera_position", cameraPosition);
                    i++;

                }
            } else {
                // Otherwise, if it's a textured or a tinted material, just set the transform monolith matrix.
                currentShader->set("transform", transform);
            }
            (*it).mesh->draw();
        }

        // If there is a sky material, draw the sky

        // Comment:
        // The following body of the if condition does the following:
        // Sets up the sky material.
        // Obtains the sky model matrix. This matrix should translate every point with the "cameraForward" vector.
        // Now, this vector is created and explained above.
        if(this->skyMaterial){
            //DONE: (Req 10) setup the sky material
            this->skyMaterial->setup();
            
            //DONE: (Req 10) Get the camera position
            // Use cameraForward from above.
            
            //DONE: (Req 10) Create a model matrix for the sky such that it always follows the camera (sky sphere center = camera position)
            glm::mat4 skyModel = glm::translate(glm::mat4(1.0f), cameraForward);

            //DONE: (Req 10) We want the sky to be drawn behind everything (in NDC space, z=1)
            // We can acheive this by multiplying by an extra matrix after the projection but what values should we put in it?

            // Comment:           
            // We need this matrix to preserve all components, but always put 1.0 in the Z component.
            // We utilize the w component for that, putting 1 in it instead of 1 in the z component in the 3rd row.
            // Another thing to note, is that GLM matrices are column-major, so read the columns as rows,
            // and vice versa.
            glm::mat4 alwaysBehindTransform = glm::mat4(
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 1.0f
            );

            //DONE: (Req 10) set the "transform" uniform
            glm::mat4 transform = alwaysBehindTransform *  (VP * skyModel);
            this->skyMaterial->shader->use();
            this->skyMaterial->shader->set("transform", transform);
            
            //DONE: (Req 10) draw the sky sphere
            this->skySphere->draw();

        }

        //DONE: (Req 9) Draw all the transparent commands
        // Don't forget to set the "transform" uniform to be equal the model-view-projection matrix for each render command
        
        // Comment:
        // This is the same as the above loop over the opaque objects.
        for (auto it = transparentCommands.begin(); it != transparentCommands.end(); it++) {
            glm::mat4 transform = VP * (*it).localToWorld;
            (*it).material->shader->use();
            (*it).material->setup();
            (*it).material->shader->set("transform", transform);
            (*it).mesh->draw();
        }


        // If there is a postprocess material, apply postprocessing

        // Comment:
        // What we do in the following is:
        // We unbind the framebuffer (thereby returning to the default framebuffer).
        // We setup the postprocess material, use the shader, bind the vertex array to be used,
        // bind the texture of the post process material, then we draw the vertex array.
        // Note that: the data of the vertices (coordinates and texture coordinates) 
        // actually lie in the shader: assets/shaders/fullscreen.vert (postprocessMaterial->shader)
        // They are 3 vertices of a big triangle (bigger than the whole window).
        if(postprocessMaterial){
            //DONE: (Req 11) Return to the default framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            
            //DONE: (Req 11) Setup the postprocess material and draw the fullscreen triangle
            postprocessMaterial->setup();
            postprocessMaterial->shader->use();
            glBindVertexArray(this->postProcessVertexArray);
            postprocessMaterial->texture->bind();
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
    }
}

