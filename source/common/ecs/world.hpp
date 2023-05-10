#pragma once

#include "components/camera.hpp"
#include "components/light.hpp"
#include "components/mesh-renderer.hpp"
#include "entity.hpp"
#include "json/json.hpp"
#include <iostream>
#include <ostream>
#include <unordered_set>


namespace our {

// This class holds a set of entities
class World {
  std::unordered_set<Entity *>
      entities; // These are the entities held by this world
  std::unordered_set<Entity *>
      markedForRemoval; // These are the entities that are awaiting to be
                        // deleted when deleteMarkedEntities is called
public:
  World() = default;

  std::unordered_set<LightComponent *> setOfLights;
  std::unordered_set<Entity *> setOfSpaceArtifacts;
  // Those hold the far right and the far left points of the track.
  glm::vec3 tracksFarLeft = glm::vec3(std::numeric_limits<float>::max(), 0, 0), tracksFarRight = glm::vec3(std::numeric_limits<float>::min(), 0, 0);

  // This will deserialize a json array of entities and add the new entities to
  // the current world If parent pointer is not null, the new entities will be
  // have their parent set to that given pointer If any of the entities has
  // children, this function will be called recursively for these children
  void deserialize(const nlohmann::json &data, Entity *parent = nullptr);

  // This adds an entity to the entities set and returns a pointer to that
  // entity WARNING The entity is owned by this world so don't use "delete" to
  // delete it, instead, call "markForRemoval" to put it in the
  // "markedForRemoval" set. The elements in the "markedForRemoval" set will be
  // removed and deleted when "deleteMarkedEntities" is called.
  Entity *add() {
    // DONE: (Req 8) Create a new entity, set its world member variable to this,
    //  and don't forget to insert it in the suitable container.
    Entity *newEntity = new Entity();
    newEntity->world = this;
    entities.insert(newEntity);
    return newEntity;
  }

  // This returns and immutable reference to the set of all entites in the
  // world.
  const std::unordered_set<Entity *> &getEntities() { return entities; }

  // This marks an entity for removal by adding it to the "markedForRemoval"
  // set. The elements in the "markedForRemoval" set will be removed and deleted
  // when "deleteMarkedEntities" is called.
  void markForRemoval(Entity *entity) {
    // DONE: (Req 8) If the entity is in this world, add it to the
    // "markedForRemoval" set.
    if (auto it = entities.find(entity); it != entities.end()) {

      // If it contains a light component, remove it from the set of light
      // components as well.
      if (our::LightComponent *light = (*it)->getComponent<LightComponent>(); light) {
        setOfLights.erase(light);
      }
      
      markedForRemoval.insert(*it);
      entities.erase(*it);
    }
  }

  // This removes the elements in "markedForRemoval" from the "entities" set.
  // Then each of these elements are deleted.
  void deleteMarkedEntities() {
    // DONE: (Req 8) Remove and delete all the entities that have been marked
    // for removal
    for (auto it = markedForRemoval.begin(); it != markedForRemoval.end(); it++) delete *it;
    markedForRemoval.clear();
  }

  // This deletes all entities in the world
  void clear() {
    // DONE: (Req 8) Delete all the entites and make sure that the containers
    // are empty
    deleteMarkedEntities();
    for (auto it = entities.begin(); it != entities.end(); it++) {
      delete *it;
    }
    
    entities.clear();
    setOfSpaceArtifacts.clear();
    setOfLights.clear();
  }

  // Since the world owns all of its entities, they should be deleted alongside
  // it.
  ~World() { clear(); }

  // The world should not be copyable
  World(const World &) = delete;
  World &operator=(World const &) = delete;
};

} // namespace our