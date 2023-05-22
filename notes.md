Entity => Transform localTransform => position, rotation, scale
Entity has a set of components.
Components (info of spatial position is in the parent entity)


Mesh (has a vertex array => vertices in  the object space).
MeshRenderer (Is a component) and has a mesh as a data member.
MultipleMeshesRendererComponent
Entity has a child MeshRenderer (in the set of components)
World has a set of entities.


Camera Component (Has all the details relating to the camera)
FreeCameraComponent (has only the speed sensitivities)


FreeCameraControllerSystem => Updates the position of the camera according to the input. (WASD) 
camera->getOwner()->localTransform.position += FreeCameraControllerComponent->speedSensitivity * (0.5, 0.0, 0.0) 

== 


If an entity 2 is a child of an entity 1,
Mode matrix = M2 * M1

deserialization of assets => first step
deserialization of component => i use asset x => access through assets[assetname] of assetloader.


World deserialization => entity deserialization => component deserialization

Layers:

    Assets (Mesh, MultipleMesh, Texture2D)
    Components (MeshRendererComponent, MultipleMeshesRendererComponent) 



if time.since = 0.0 (either speed is not in effect, or is just collected and not in effect)

    if speed just collected (speed.inEffect = true)
        Just returned from the above collisionSystem.update call
        Update and set the effects and everything. (FOV, postprocess)

    if speed not collected (speed.inEffect = false)
        Don't do anything


else => speed is in effect
    check if expired
        return everything to the normal state
    otherwise
        continue as is




setOfCelestialOrbs
planets, stars, moon: celestial orbs
                                الأجرام السماوية


Randomize number of planets.
Randomize number of stars.
Give a planet a moon with 1/3 probability.

Requiement: minimum distance between any two celestial orbs الاجرام السماوية (planets, stars, moon)

Generate a new orb, with a new randomized position, calculate the distance between it and every other orb
If the distance requirement is violated < threshold : regeneration of randomized position.