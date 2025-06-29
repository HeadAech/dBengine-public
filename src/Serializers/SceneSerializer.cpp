
#include "SceneSerializer.h"
#include "MaterialSerializer.h"
#include <Components/Control/Text/Text.h>
#include <Components/Control/Sprite/Sprite.h>
#include <Singletons/Ref/Ref.h>

namespace Serialize {


    SceneSerializer::SceneSerializer() {}

    /// <summary>
    /// returns a static instance of SceneSerializer
    /// </summary>
    /// <returns></returns>
    SceneSerializer &SceneSerializer::GetInstance() {
        static SceneSerializer instance;
        return instance;
    }

    Ref &ref = Ref::GetInstance();

    /// <summary>
    /// Sets current used scene for deserialization
    /// </summary>
    /// <param name="scene"></param>
    void SceneSerializer::SetScene(Scene *scene) { this->scene = scene; };

    /// <summary>
    /// Serializes given scene to given path
    /// </summary>
    /// <param name="scene">: scene to save</param>
    /// <param name="filepath">: where to save given scene</param>
    bool SceneSerializer::Serialize(Scene *scene, const std::string &filePath) {
        TimerHelper timer("SceneSerializer::Serialize");
        SetScene(scene);
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "sceneName";
        out << YAML::Value << scene->name;
        out << YAML::Key << "uuid";
        out << YAML::Value << scene->uuid;
        out << YAML::Key << "sceneCamera";
        out << YAML::Value << YAML::BeginSeq;
        if (auto comp = scene->sceneCameraObject.get()) {
            if(!m_SerializeGameObject(out, comp)){
                return false;
            }
        }
        out << YAML::EndSeq;
        out << YAML::Key << "rootGameObject";
        out << YAML::Value << YAML::BeginSeq;
        scene->sceneRootObject.get()->pathToScene = filePath;
        if(!m_SerializeGameObject(out, scene->sceneRootObject.get())){
            return false;
        }

        out << YAML::EndSeq;
        out << YAML::Key << "LibraryAnimations";
        out << YAML::Value<<YAML::BeginMap;
        for (auto &animation: AnimationLibrary::GetInstance().GetAllAnimations()){
            out << YAML::Key << "Animation";
            out << YAML::Value << YAML::BeginMap;
            out << YAML::Key << "animationName";
            out << YAML::Value << animation.second->GetName();
            out << YAML::Key << "animationPath";
            out << YAML::Value << animation.second->GetAnimationPath();
            out << YAML::EndMap;
        }

        out << YAML::EndMap;
        std::ofstream fout(filePath);
        fout << out.c_str();
        return true;
    }

    /// <summary>
    /// Serializes game object and it's children
    /// </summary>
    /// <param name="out">: YAML emiter</param>
    /// <param name="gameObject">: gameobject from which serialization should be started [mostly rootGameObject] </param>
    bool SceneSerializer::m_SerializeGameObject(YAML::Emitter &out, GameObject *gameObject) {
        out << YAML::BeginMap;
        if (gameObject->isScene && gameObject->pathToScene != "") {
            out << YAML::Key << "isScene";
            out << YAML::Value << true;
            out << YAML::Key << "pathToString";
            out << YAML::Value << gameObject->pathToScene;
        }
        out << YAML::Key << "name";
        out << YAML::Value << gameObject->name;
        out << YAML::Key << "m_enabled";
        out << YAML::Value << gameObject->m_enabled;
        out << YAML::Key << "transform";
       
        m_SerializeTransformComponent(out, gameObject);

        if (gameObject == scene->sceneRootObject.get() || (!gameObject->isScene)) {
            out << YAML::Key << "children";
            out << YAML::BeginSeq;
            for (auto &go: gameObject->children) {
                if (!m_SerializeGameObject(out, go.get())) {
                    EngineDebug::GetInstance().PrintError("My sincerest apologies, the gameObject: " + go->name + " of " + gameObject->name + " seems broken to me.");
                    return false;
                }
            }
            out << YAML::EndSeq;
           
        }

        out << YAML::Key << "components";
        out << YAML::BeginSeq;
        for (auto &component: gameObject->components) {
            if (!m_SerializeComponent(out, component.get())) {
                EngineDebug::GetInstance().PrintError("My sincerest apologies, the gameObject: " + component->name +
                                                      " of " + gameObject->name + " seems broken to me.");
                return false;
            }
        }
        out << YAML::EndSeq;
        out << YAML::Key << "uuid";
        out << YAML::Value << gameObject->GetUUID();
        out << YAML::EndMap;
        return true;
    }

    /// <summary>
    /// Serializes components of gameObjects, REMEMBER TO BEGIN MAP, IF YOU WILL NOT BEGIN IT, IT WILL DESTROY THE SERIALIZATION
    /// </summary>
    /// <param name="out">: YAML emitter</param>
    /// <param name="component">: component which is going to be serialized</param>
    bool SceneSerializer::m_SerializeComponent(YAML::Emitter &out, Component *component) {

        out << YAML::BeginMap;

        // cast it, [we could use name, but i will not risk it. [use either component name or getcomponent]
        // someone can change the name someday.
        if (auto *comp = dynamic_cast<Animator *>(component)) {
            out << YAML::Key << "Animator";
            out << YAML::BeginMap;
            m_SerializeComponentDefaultData(out, component);

            out << YAML::Key << "CurrentAnimation";
            if (comp->GetCurrentAnimation()) {
                out << YAML::Value << comp->GetCurrentAnimation()->GetName();
            } else {
                out << YAML::Value << "";
            }
            out << YAML::Key << "Playing";
            out << YAML::Value << comp->IsPlaying();
            out << YAML::Key << "m_Blending";
            out << YAML::Value << comp->IsBlending();
            out << YAML::Key << "m_BlendFactor";
            out << YAML::Value << comp->GetBlendFactor();
            out << YAML::Key << "animations";
            out << YAML::Value << YAML::BeginMap;
            for (auto animation: comp->GetAllAnimations()) {
                out << YAML::Key << "animation";
                out << YAML::Value << YAML::BeginMap;
                out << YAML::Key << "animationName";
                out << YAML::Value << animation->GetName();
                out << YAML::Key << "animationPath";
                out << YAML::Value << animation->GetAnimationPath();
                out << YAML::EndMap;
            }
            out << YAML::EndMap;

            out << YAML::Key << "animationTransitions";
            out << YAML::Value << YAML::BeginMap;
            for (auto transition: comp->GetAnimationTransitions()){
                out << YAML::Key << "transition";    
                out << YAML::Value << transition;
            }
            out << YAML::EndMap;
            

        } else if (auto *comp = dynamic_cast<AudioListener *>(component)) {
            out << YAML::Key << "AudioListener";
            out << YAML::BeginMap;
            m_SerializeComponentDefaultData(out, component);

            out << YAML::Key << "listener_pos";
            out << YAML::Value << comp->listener_pos;
            out << YAML::Key << "listener_vel";
            out << YAML::Value << comp->listener_vel;
            out << YAML::Key << "listener_forward";
            out << YAML::Value << comp->listener_forward;
            out << YAML::Key << "listener_up";
            out << YAML::Value << comp->listener_up;

        } else if (auto *comp = dynamic_cast<AudioSource *>(component)) {
            out << YAML::Key << "AudioSource";
            out << YAML::BeginMap;
            m_SerializeComponentDefaultData(out, component);
            // there must be a way to know what music should be played
            // ex. in gothic [maybe check the area? and onarea play the stuff?
            // ex. in DD / bg3 same stuff
            
            out << YAML::Key << "pos";
            out << YAML::Value << comp->GetPos();
            out << YAML::Key << "bankPath";
            out << YAML::Value << comp->GetBankPath();
            out << YAML::Key << "eventNames";
            out << YAML::Value << comp->eventNames;
            out << YAML::Key << "isAllPaused";
            out << YAML::Value << comp->isAllPaused;
            out << YAML::Key << "flag";
            out << YAML::Value << static_cast<int>(comp->flag);
            out << YAML::Key << "affectedByDSP";
            out << YAML::Value << comp->affectedByDSP;
            out << YAML::Key << "activeEvents";
            out << YAML::Value << YAML::BeginSeq;
            for (const auto *instance: comp->activeEvents) {
                if (instance) {
                    FMOD::Studio::EventDescription *desc = nullptr;
                    instance->getDescription(&desc);
                    if (desc) {
                        char path[512];
                        int retrieved = 0;
                        desc->getPath(path, sizeof(path), &retrieved);

                        out << YAML::BeginMap;
                        out << YAML::Key << "eventName" << YAML::Value << path;

                        FMOD_STUDIO_PLAYBACK_STATE state;
                        instance->getPlaybackState(&state);
                        out << YAML::Key << "isPlaying" << YAML::Value
                            << (state == FMOD_STUDIO_PLAYBACK_PLAYING || state == FMOD_STUDIO_PLAYBACK_SUSTAINING);
                        bool paused = false;
                        instance->getPaused(&paused);
                        out << YAML::Key << "isPaused" << YAML::Value << paused;

                        float volume = 0.0f;
                        instance->getVolume(&volume);
                        out << YAML::Key << "volume" << YAML::Value << volume;
                        float pitchValue = 0.0f;
                        instance->getPitch(&pitchValue);
                        out << YAML::Key << "pitch" << YAML::Value << pitchValue;

                        out << YAML::Key << "parameters" << YAML::Value << YAML::BeginSeq;
                        int paramCount = 0;
                        desc->getParameterDescriptionCount(&paramCount);
                        for (int i = 0; i < paramCount; ++i) {
                            FMOD_STUDIO_PARAMETER_DESCRIPTION paramDesc;
                            if (desc->getParameterDescriptionByIndex(i, &paramDesc) == FMOD_OK) {
                                float value = 0.0f;
                                instance->getParameterByName(paramDesc.name, &value);
                                out << YAML::BeginMap;
                                out << YAML::Key << "name" << YAML::Value << paramDesc.name;
                                out << YAML::Key << "value" << YAML::Value << value;
                                out << YAML::EndMap;
                            }
                        }
                        out << YAML::EndSeq;

                        out << YAML::EndMap;
                    }
                }
            }
            out << YAML::EndSeq;

        } else if (auto *comp = dynamic_cast<ThirdPersonCamera*>(component)) {
            out << YAML::Key << "ThirdPersonCamera";
            out << YAML::BeginMap;
            m_SerializeComponentDefaultData(out, component);

            out << YAML::Key << "Front";
            out << YAML::Value << comp->Front;
            out << YAML::Key << "Up";
            out << YAML::Value << comp->Up;
            out << YAML::Key << "Right";
            out << YAML::Value << comp->Right;
            out << YAML::Key << "WorldUp";
            out << YAML::Value << comp->WorldUp;
            out << YAML::Key << "Yaw";
            out << YAML::Value << comp->Yaw;
            out << YAML::Key << "Pitch";
            out << YAML::Value << comp->Pitch;
            out << YAML::Key << "MovementSpeed";
            out << YAML::Value << comp->MovementSpeed;
            out << YAML::Key << "MouseSensitivity";
            out << YAML::Value << comp->MouseSensitivity;
            out << YAML::Key << "Zoom";
            out << YAML::Value << comp->Zoom;
            out << YAML::Key << "aspectRatio";
            out << YAML::Value << comp->aspectRatio;
            out << YAML::Key << "NearPlane";
            out << YAML::Value << comp->NearPlane;
            out << YAML::Key << "FarPlane";
            out << YAML::Value << comp->FarPlane;

            out << YAML::Key << "orbitDistance";
            out << YAML::Value << comp->orbitDistance;
            out << YAML::Key << "orbitYaw";
            out << YAML::Value << comp->orbitYaw;
            out << YAML::Key << "orbitPitch";
            out << YAML::Value << comp->orbitPitch;
            out << YAML::Key << "targetHeightOffset";
            out << YAML::Value << comp->targetHeightOffset;
            out << YAML::Key << "minOrbitPitch";
            out << YAML::Value << comp->minOrbitPitch;
            out << YAML::Key << "maxOrbitPitch";
            out << YAML::Value << comp->maxOrbitPitch;
            out << YAML::Key << "minDistance";
            out << YAML::Value << comp->minDistance;
            out << YAML::Key << "maxDistance";
            out << YAML::Value << comp->maxDistance;
            out << YAML::Key << "positionSmoothing";
            out << YAML::Value << comp->positionSmoothing;
            out << YAML::Key << "rotationSmoothing";
            out << YAML::Value << comp->rotationSmoothing;
            out << YAML::Key << "Pos";
            out << YAML::Value << comp->Pos;

        } else if (auto *comp = dynamic_cast<Camera *>(component)) {
            out << YAML::Key << "Camera";
            out << YAML::BeginMap;
            m_SerializeComponentDefaultData(out, component);

            out << YAML::Key << "Front";
            out << YAML::Value << comp->Front;
            out << YAML::Key << "Up";
            out << YAML::Value << comp->Up;
            out << YAML::Key << "Right";
            out << YAML::Value << comp->Right;
            out << YAML::Key << "WorldUp";
            out << YAML::Value << comp->WorldUp;
            out << YAML::Key << "Yaw";
            out << YAML::Value << comp->Yaw;
            out << YAML::Key << "Pitch";
            out << YAML::Value << comp->Pitch;
            out << YAML::Key << "MovementSpeed";
            out << YAML::Value << comp->MovementSpeed;
            out << YAML::Key << "MouseSensitivity";
            out << YAML::Value << comp->MouseSensitivity;
            out << YAML::Key << "Zoom";
            out << YAML::Value << comp->Zoom;
            out << YAML::Key << "aspectRatio";
            out << YAML::Value << comp->aspectRatio;
            out << YAML::Key << "NearPlane";
            out << YAML::Value << comp->NearPlane;
            out << YAML::Key << "FarPlane";
            out << YAML::Value << comp->FarPlane;

        } else if (auto *comp = dynamic_cast<Hitbox *>(component)) {
            out << YAML::Key << "Hitbox";
            out << YAML::BeginMap;
            m_SerializeComponentDefaultData(out, component);
            out << YAML::Key << "m_shapeType";
            out << YAML::Value << (int) comp->GetShapeType();
            out << YAML::Key << "m_boxSize";
            out << YAML::Value << comp->GetBoxSize();
            out << YAML::Key << "m_capsuleRadius";
            out << YAML::Value << comp->GetCapsuleRadius();
            out << YAML::Key << "m_capsuleHeight";
            out << YAML::Value << comp->GetCapsuleHeight();
            out << YAML::Key << "m_color";
            out << YAML::Value << comp->GetColor();
            out << YAML::Key << "m_isVisible";
            out << YAML::Value << comp->IsVisible();
            out << YAML::Key << "m_WorldMin";
            out << YAML::Value << comp->GetWorldMin();
            out << YAML::Key << "m_WorldMax";
            out << YAML::Value << comp->GetWorldMax();
            out << YAML::Key << "m_IsCollisionArea";
            out << YAML::Value << comp->GetIsCollisionArea();
            out << YAML::Key << "m_collisionMask";
            out << YAML::Value << static_cast<int>(comp->GetCollisionMask());
            out << YAML::Key << "m_collisionLayer";
            out << YAML::Value << static_cast<int>(comp->GetCollisionLayer());
            out << YAML::Key << "m_collisionPositionOffset";
            out << YAML::Value << comp->GetPositionOffset();
            out << YAML::Key << "m_isActive";
            out << YAML::Value << comp->IsActive();
            out << YAML::Key << "m_hitboxName";
            out << YAML::Value << comp->GetName();
            out << YAML::Key << "m_damage";
            out << YAML::Value << comp->GetDamage();
            out << YAML::Key << "m_oneHitPerTarget";
            out << YAML::Value << comp->IsOneHitPerTarget();

            // out << YAML::Key << "m_aabbDirty"; always dirty after load.
        } else if (auto *comp = dynamic_cast<CollisionShape *>(component)) {
            out << YAML::Key << "CollisionShape";
            out << YAML::BeginMap;
            m_SerializeComponentDefaultData(out, component);
            out << YAML::Key << "m_shapeType";
            out << YAML::Value << (int) comp->GetShapeType();
            out << YAML::Key << "m_boxSize";
            out << YAML::Value << comp->GetBoxSize();
            out << YAML::Key << "m_capsuleRadius";
            out << YAML::Value << comp->GetCapsuleRadius();
            out << YAML::Key << "m_capsuleHeight";
            out << YAML::Value << comp->GetCapsuleHeight();
            out << YAML::Key << "m_color";
            out << YAML::Value << comp->GetColor();
            out << YAML::Key << "m_isVisible";
            out << YAML::Value << comp->IsVisible();
            out << YAML::Key << "m_WorldMin";
            out << YAML::Value << comp->GetWorldMin();
            out << YAML::Key << "m_WorldMax";
            out << YAML::Value << comp->GetWorldMax();
            out << YAML::Key << "m_IsCollisionArea";
            out << YAML::Value << comp->GetIsCollisionArea();
            out << YAML::Key << "m_collisionMask";
            out << YAML::Value << static_cast<int> (comp->GetCollisionMask());
            out << YAML::Key << "m_collisionLayer";
            out << YAML::Value << static_cast<int> (comp->GetCollisionLayer());
            out << YAML::Key << "m_collisionPositionOffset";
            out << YAML::Value << comp->GetPositionOffset();

            // out << YAML::Key << "m_aabbDirty"; always dirty after load.
        } else if (auto *comp = dynamic_cast<DirectionalLight *>(component)) {
            out << YAML::Key << "DirectionalLight";
            out << YAML::BeginMap;
            m_SerializeComponentDefaultData(out, comp);
            m_SerializeLightComponentDefaultData(out, comp);
            
            out << YAML::Key << "direction";
            out << YAML::Value << comp->direction;
        } else if (auto *comp = dynamic_cast<PointLight *>(component)) {
            out << YAML::Key << "PointLight";
            out << YAML::BeginMap;
            m_SerializeComponentDefaultData(out, comp);
            m_SerializeLightComponentDefaultData(out, comp);

            out << YAML::Key << "constant";
            out << YAML::Value << comp->constant;
            out << YAML::Key << "linear";
            out << YAML::Value << comp->linear;
            out << YAML::Key << "quadratic";
            out << YAML::Value << comp->quadratic;

            out << YAML::Key << "castShadows";
            out << YAML::Value << comp->castShadows;

            out << YAML::Key << "shadowDistance";
            out << YAML::Value << comp->shadowDistance;

        } else if (auto *comp = dynamic_cast<SpotLight *>(component)) {
            out << YAML::Key << "SpotLight";
            out << YAML::BeginMap;
            m_SerializeComponentDefaultData(out, component);
            m_SerializeLightComponentDefaultData(out, comp);

            out << YAML::Key << "constant";
            out << YAML::Value << comp->constant;
            out << YAML::Key << "linear";
            out << YAML::Value << comp->linear;
            out << YAML::Key << "quadratic";
            out << YAML::Value << comp->quadratic;
            out << YAML::Key << "direction";
            out << YAML::Value << comp->direction;
            out << YAML::Key << "innerCutOff";
            out << YAML::Value << comp->innerCutOff;
            out << YAML::Key << "outerCutOff";
            out << YAML::Value << comp->outerCutOff;

            out << YAML::Key << "castShadows";
            out << YAML::Value << comp->castShadows;

            out << YAML::Key << "shadowDistance";
            out << YAML::Value << comp->shadowDistance;

        } else if (auto *comp = dynamic_cast<LuaComponent *>(component)) {
            out << YAML::Key << "LuaComponent";
            out << YAML::BeginMap;
            m_SerializeComponentDefaultData(out, component);

            out << YAML::Key << "scriptPath";
            out << YAML::Value << comp->scriptPath;
            out << YAML::Key << "scriptName";
            out << YAML::Value << comp->scriptName;

        } else if (auto *comp = dynamic_cast<MeshInstance *>(component)) {
            out << YAML::Key << "MeshInstance";
            out << YAML::BeginMap;
            m_SerializeComponentDefaultData(out, component);

            out << YAML::Key << "directory";
            out << YAML::Value << comp->directory;
            out << YAML::Key << "m_modelPath";
            out << YAML::Value << comp->m_modelPath;
            out << YAML::Key << "MaterialBase";
            MaterialSerializer::GetInstance().SerializeForModel(out, &comp->model->Material);
            if (comp->MaterialOverride){
                out << YAML::Key << "MaterialOverride";
                MaterialSerializer::GetInstance().SerializeForModel(out, comp->MaterialOverride.get());
            }
            if (comp->isUsingVolumetric()){
                out << YAML::Key << "density";
                out << YAML::Value << comp->density;
                out << YAML::Key << "samples";
                out << YAML::Value << comp->samples;
                out << YAML::Key << "fogColor";
                out << YAML::Value << comp->fogColor;
                out << YAML::Key << "scattering";
                out << YAML::Value << comp->scattering;
            }

        } else if (auto *comp = dynamic_cast<TextRenderer *>(component)) {
            out << YAML::Key << "TextRenderer";
            out << YAML::BeginMap;
            m_SerializeComponentDefaultData(out, component);

            out << YAML::Key << "text";
            out << YAML::Value << comp->text;
            out << YAML::Key << "font_size";
            out << YAML::Value << comp->fontSize;
            out << YAML::Key << "scale";
            out << YAML::Value << comp->scale;
            out << YAML::Key << "color";
            out << YAML::Value << comp->color;
            out << YAML::Key << "position";
            out << YAML::Value << comp->position;

        } else if (auto *comp = dynamic_cast<WorldEnvironment *>(component)) {
            out << YAML::Key << "WorldEnvironment";
            out << YAML::BeginMap;
            m_SerializeComponentDefaultData(out, component);
            out << YAML::Key << "skyboxPath";
            out << YAML::Value << comp->m_skyboxPath;
            out << YAML::Key << "m_skyType";
            out << YAML::Value << comp->GetSkyType();
            out << YAML::Key << "irradianceStrength";
            out << YAML::Value << comp->irradianceStrength;
            out << YAML::Key << "sampleDelta";
            out << YAML::Value << comp->sampleDelta;
        } else if (auto *comp = dynamic_cast<PhysicsBody *>(component)) {
            out << YAML::Key << "PhysicsBody";
            out << YAML::BeginMap;
            m_SerializeComponentDefaultData(out, component);
            out << YAML::Key << "mass";
            out << YAML::Value << comp->mass;
            out << YAML::Key << "velocity";
            out << YAML::Value << comp->velocity;
            out << YAML::Key << "useGravity";
            out << YAML::Value << comp->useGravity;
            out << YAML::Key << "isStatic";
            out << YAML::Value << comp->isStatic;
            out << YAML::Key << "restitution";
            out << YAML::Value << comp->restitution;
            out << YAML::Key << "invMass";
            out << YAML::Value << comp->invMass;
            out << YAML::Key << "accumulatedForces";
            out << YAML::Value << comp->accumulatedForces;
            out << YAML::Key << "linearDamping";
            out << YAML::Value << comp->linearDamping;
            out << YAML::Key << "isGrounded";
            out << YAML::Value << comp->isGrounded;
            out << YAML::Key << "stabilizationPosition";
            out << YAML::Value << comp->stabilizationPosition;
        } else if (auto *comp = dynamic_cast<ParticleSystem *>(component)) {
            out << YAML::Key << "ParticleSystem";
            out << YAML::BeginMap;
            m_SerializeComponentDefaultData(out, component);
            out << YAML::Key << "EmissionShape";
            out << YAML::Value << (int)comp->EmissionShape;
            out << YAML::Key << "DrawEmissionShape";
            out << YAML::Value << comp->DrawEmissionShape;
            out << YAML::Key << "CubeSize";
            out << YAML::Value << comp->CubeSize;
            out << YAML::Key << "SphereRadius";
            out << YAML::Value << comp->SphereRadius;
            out << YAML::Key << "SphereSurfaceOnly";
            out << YAML::Value << comp->SphereSurfaceOnly;
            out << YAML::Key << "Emitting";
            out << YAML::Value << comp->Emitting;
            out << YAML::Key << "MaxParticles";
            out << YAML::Value << comp->MaxParticles;
            out << YAML::Key << "SpawnRate";
            out << YAML::Value << comp->SpawnRate;
            out << YAML::Key << "OneShot";
            out << YAML::Value << comp->OneShot;
            out << YAML::Key << "MinLifeTime";
            out << YAML::Value << comp->MinLifeTime;
            out << YAML::Key << "MaxLifeTime";
            out << YAML::Value << comp->MaxLifeTime;
            out << YAML::Key << "MinSize";
            out << YAML::Value << comp->MinSize;
            out << YAML::Key << "MaxSize";
            out << YAML::Value << comp->MaxSize;
            out << YAML::Key << "Gravity";
            out << YAML::Value << comp->Gravity;
            out << YAML::Key << "Albedo";
            out << YAML::Value << comp->Albedo;
            out << YAML::Key << "MinInitialVelocity";
            out << YAML::Value << comp->MinInitialVelocity;
            out << YAML::Key << "MaxInitialVelocity";
            out << YAML::Value << comp->MaxInitialVelocity;
            out << YAML::Key << "CameraPosition";
            out << YAML::Value << comp->CameraPosition;
            out << YAML::Key << "BlendingAdditive";
            out << YAML::Value << comp->BlendingAdditive;
            out << YAML::Key << "EmissionMultiplier";
            out << YAML::Value << comp->EmissionMultiplier;
            out << YAML::Key << "UseLocalSpace";
            out << YAML::Value << comp->UseLocalSpace;
            out << YAML::Key << "m_Initialized"; // idk if it should not be always false
            out << YAML::Value << comp->m_Initialized;
            out << YAML::Key << "m_Texture";
            out << YAML::Value << *comp->m_Texture;
            out << YAML::Key << "BillboardY";
            out << YAML::Value << comp->BillboardY;
            out << YAML::Key << "ScaleOverTime";
            out << YAML::Value << comp->ScaleOverTime;
            out << YAML::Key << "BeginScaling";
            out << YAML::Value << comp->BeginScaling;
            out << YAML::Key << "FadeOverTime";
            out << YAML::Value << comp->FadeOverTime;
            out << YAML::Key << "BeginFading";
            out << YAML::Value << comp->BeginFading;

        } else if (auto *comp = dynamic_cast<Tag *>(component)) {
            out << YAML::Key << "Tag";
            out << YAML::BeginMap;
            m_SerializeComponentDefaultData(out, component);
            out << YAML::Key << "Name";
            out << YAML::Value << comp->Name;
        } else if (auto *comp = dynamic_cast<Timer *>(component)) {
            out << YAML::Key << "Timer";
            out << YAML::BeginMap;
            m_SerializeComponentDefaultData(out, component);
            out << YAML::Key << "OneShot";
            out << YAML::Value << comp->OneShot;
            out << YAML::Key << "HasStarted";
            out << YAML::Value << comp->HasStarted;
            out << YAML::Key << "HasTimedOut";
            out << YAML::Value << comp->HasTimedOut;
            out << YAML::Key << "TimeoutMs";
            out << YAML::Value << comp->TimeoutMs;
            out << YAML::Key << "ElapsedMs";
            out << YAML::Value << comp->ElapsedMs;
            out << YAML::Key << "SignalMessage";
            out << YAML::Value << comp->SignalMessage;
        } else if (auto *comp = dynamic_cast<UI::Button *>(component)){
            out << YAML::Key << "Button";
            out << YAML::BeginMap;
            out << YAML::Key << "Color";
            out << YAML::Value << comp->Color;
            out << YAML::Key << "m_HasTexture";
            out << YAML::Value << comp->HasTexture();
            if (comp->HasTexture()) {
                out << YAML::Key << "m_Texture";
                out << YAML::Value << *comp->GetTexture().get();
            }
            if (comp->GetHoverTexture()->id != 0)
            {
                out << YAML::Key << "m_HoverTexture";
                out << YAML::Value << *comp->GetHoverTexture().get();
            }

            m_SerializeComponentDefaultData(out, comp);
            m_SerializeControlComponentDefaultData(out, comp);
        } else if (auto *comp = dynamic_cast<UI::Text *>(component)) {
            out << YAML::Key << "Text";
            out << YAML::BeginMap;
            out << YAML::Key << "Text";
            out << YAML::Value << comp->GetText();
            out << YAML::Key << "FontSize";
            out << YAML::Value << comp->GetFontSize();
            out << YAML::Key << "Color";
            out << YAML::Value << comp->GetColor();
            m_SerializeComponentDefaultData(out, comp);
            m_SerializeControlComponentDefaultData(out, comp);
        
        }
        else if (auto* comp = dynamic_cast<UI::Sprite*>(component))
        {
            out << YAML::Key << "Sprite";
            out << YAML::BeginMap;
            out << YAML::Key << "ModulateColor";
            out << YAML::Value << comp->ModulateColor;
            out << YAML::Key << "m_HasTexture";
            out << YAML::Value << comp->HasTexture();
            if (comp->HasTexture())
            {
                out << YAML::Key << "m_Texture";
                out << YAML::Value << *comp->GetTexture().get();
            }
            out << YAML::Key << "Clipping";
            out << YAML::Value << comp->Clipping;
            m_SerializeComponentDefaultData(out, comp);
            m_SerializeControlComponentDefaultData(out, comp);

        }
        else if (auto *comp = dynamic_cast<PlayerController *>(component)) {
            out << YAML::Key << "PlayerController";
            out << YAML::BeginMap;
            out << YAML::Key << "moveSpeed";
            out << YAML::Value << comp->moveSpeed;
            out << YAML::Key << "acceleration";
            out << YAML::Value << comp->acceleration;
            out << YAML::Key << "deceleration";
            out << YAML::Value << comp->deceleration;
            out << YAML::Key << "maxSpeed";
            out << YAML::Value << comp->maxSpeed;
            m_SerializeComponentDefaultData(out, comp);
        } else if (auto *comp = dynamic_cast<NavigationMesh *>(component)) {
            out << YAML::Key << "NavigationMesh";
            out << YAML::BeginMap;
            out << YAML::Key << "floorObjectUUID";
            out << YAML::Value << comp->GetFloor()->gameObject->GetUUID();
            out << YAML::Key << "obstaclesObjectsUUID";
            out << YAML::BeginSeq;
            for (const auto &obstacle: comp->GetObstacles()) {
                out << obstacle->gameObject->GetUUID();
            }
            out << YAML::EndSeq;
            m_SerializeComponentDefaultData(out, comp);
        } else if (auto *comp = dynamic_cast<AIAgent *>(component)) {
            out << YAML::Key << "AIAgent";
            out << YAML::BeginMap;
            out << YAML::Key << "isPaused";
            out << YAML::Value << comp->IsPaused();
            out << YAML::Key << "agentPosition";
            out << YAML::Value << comp->GetPosition();
            out << YAML::Key << "speed";
            out << YAML::Value << comp->GetSpeed();
            out << YAML::Key << "maxSpeed";
            out << YAML::Value << comp->GetMaxSpeed();
            out << YAML::Key << "maxForce";
            out << YAML::Value << comp->GetMaxForce();
            out << YAML::Key << "mass";
            out << YAML::Value << comp->GetMass();
            out << YAML::Key << "targetPos";
            out << YAML::Value << comp->GetTargetPos();
            out << YAML::Key << "lineOfSightDistance";
            out << YAML::Value << comp->GetLineOfSightDistance();
            out << YAML::Key << "stoppingDistance";
            out << YAML::Value << comp->GetStoppingDistance();
            out << YAML::Key << "circleRadius";
            out << YAML::Value << comp->GetCircleRadius();
            out << YAML::Key << "circleAngle";
            out << YAML::Value << comp->GetCircleAngle();
            out << YAML::Key << "circleDirection";
            out << YAML::Value << comp->GetCircleDirection();
            m_SerializeComponentDefaultData(out, comp);
        } else if (auto *comp = dynamic_cast<NavigationTarget *>(component)) {
            out << YAML::Key << "NavigationTarget";
            out << YAML::BeginMap;
            out << YAML::Key << "targetPosition";
            out << YAML::Value << comp->GetPosition();
            m_SerializeComponentDefaultData(out, comp);
        } else if (auto *comp = dynamic_cast<AISystem *>(component)) {
            out << YAML::Key << "AISystem";
            out << YAML::BeginMap;
            out << YAML::Key << "m_navMeshObjectUUID";
            out << YAML::Value << comp->GetNavigationMesh()->gameObject->GetUUID();
            out << YAML::Key << "agentsObjectsUUID";
            out << YAML::BeginSeq;
            for (const auto &agent: comp->GetAgents()) {
                out << agent->gameObject->GetUUID();
            }
            out << YAML::EndSeq;
            out << YAML::Key << "targetObjectUUID";
            out << YAML::Value << comp->GetTarget()->gameObject->GetUUID();
            m_SerializeComponentDefaultData(out, comp);
        }
        
        
        
        else {
            EngineDebug::GetInstance().PrintError("Yo pal, there is no such component implemented to serialize: " + component->name + " of " + component->gameObject->name);
            return false;
        }

        out << YAML::EndMap;
        out << YAML::EndMap;
        return true;
    }

    bool SceneSerializer::m_SerializeTransformComponent(YAML::Emitter &out, GameObject *gameObject) { 
        out << YAML::BeginMap;
        out << YAML::Key << "position";
        out << YAML::Value << gameObject->transform.position;
        out << YAML::Key << "scale";
        out << YAML::Value << gameObject->transform.scale;
        out << YAML::Key << "rotation";
        out << YAML::Value << gameObject->transform.rotation;
        out << YAML::Key << "modelMatrix";
        out << YAML::Value << gameObject->transform.modelMatrix;
        out << YAML::Key << "globalPosition";
        out << YAML::Value << gameObject->transform.globalPosition;
        out << YAML::Key << "m_isDirty";
        out << YAML::Value << gameObject->transform.m_isDirty;
        out << YAML::EndMap;
        return true;
    }

    /// <summary>
    /// Serializes default parameters for each of the components
    /// </summary>
    /// <param name="out">: YAML emmiter</param>
    /// <param name="component">: component that is going to be serialized</param>
    bool SceneSerializer::m_SerializeComponentDefaultData(YAML::Emitter &out, Component *component) {
        out << YAML::Key << "ComponentName";
        out << YAML::Value << component->name;
        out << YAML::Key << "Enabled";
        out << YAML::Value << component->enabled;
        return true;
    }

    bool SceneSerializer::m_SerializeLightComponentDefaultData(YAML::Emitter &out, Light *component) {
        out << YAML::Key << "diffuse";
        out << YAML::Value << component->diffuse;
        out << YAML::Key << "specular";
        out << YAML::Value << component->specular;
        out << YAML::Key << "ambient";
        out << YAML::Value << component->ambient;
        out << YAML::Key << "intensity";
        out << YAML::Value << component->intensity;
        out << YAML::Key << "ambientIntensity";
        out << YAML::Value << component->ambientIntensity;

        return true;
    }

    bool SceneSerializer::m_SerializeControlComponentDefaultData(YAML::Emitter &out, UI::Control *component) {
        out << YAML::Key << "Position";
        out << YAML::Value << component->Position;
        out << YAML::Key << "Size";
        out << YAML::Value << component->Size;
        out << YAML::Key << "Rotation";
        out << YAML::Value << component->Rotation;
        out << YAML::Key << "Emission";
        out << YAML::Value << component->Emission;
        out << YAML::Key << "anchor";
        out << YAML::Value << static_cast<int>(component->anchor);
        return true;
    }

    bool SceneSerializer::DeserializeToScene(Scene *scene, const std::string &filepath) {
        TimerHelper timer("SceneSerializer::Deserialize");
        SetScene(scene);
        std::ifstream stream(filepath);
        std::stringstream strStream;
        strStream << stream.rdbuf();

        YAML::Node data = YAML::Load(strStream.str());
        if (!data["sceneName"] || !data["rootGameObject"] || !data["sceneCamera"]) {
            EngineDebug::GetInstance().PrintError("LOADED YAML VERSION IS NOT ACCEPTED! [SceneSerializer::DeserializeToScene]");
            return false;
        }
        scene->Path = filepath;
        scene->name = data["sceneName"].as<std::string>();
        std::string uuid = data["uuid"].as<std::string>();

        scene->ReassignSignals(uuid);

        AnimationLibrary& animLib = AnimationLibrary::GetInstance();
        auto animations = data["LibraryAnimations"];
        if (animations && animations.IsMap()){
            for (const auto& animation: animations){
                std::string name = animation.second["animationName"].as<std::string>();
                std::string path = animation.second["animationPath"].as<std::string>();
                animLib.AddAnimation(name, path);
            }
        }

        auto rootGameObject = data["rootGameObject"];
        if (rootGameObject) {
            m_DeserializeGameObject(rootGameObject[0], scene->sceneRootObject.get());
        }

        auto sceneCameraSeq = data["sceneCamera"];
        auto sceneCamera = sceneCameraSeq[0];
        {
            std::string name = sceneCamera["name"].as<std::string>();
            bool m_enabled = sceneCamera["m_enabled"].as<bool>();

            auto deserializedGameObject = std::make_unique<GameObject>(name);
            deserializedGameObject->m_enabled = m_enabled;

            m_DeserializeTransformComponent(sceneCamera, deserializedGameObject.get());

            auto deserializedChildren = sceneCamera["children"];
            if (deserializedChildren && deserializedChildren.IsSequence()) {
                for (auto child: deserializedChildren) {
                    if (child.IsMap()) {
                        if (!m_DeserializeGameObject(child, deserializedGameObject.get())) {
                            return false;
                        }
                    }
                }
            }

            auto deserializedComponents = sceneCamera["components"];
            if (deserializedComponents && deserializedComponents.IsSequence()) {
                for (auto component: deserializedComponents) {
                    if (component.IsMap() && component.size() == 1) {
                        auto it = component.begin();
                        std::string type = it->first.as<std::string>();
                        auto node = it->second;
                        if (!m_DeserializeComponent(node, deserializedGameObject.get(), type)) {

                            return false;
                        }
                    }
                }
            }

            scene->sceneCameraObject = std::move(deserializedGameObject);
        }

        return true;
    }

    /// <summary>
    /// Deserializes gameObjects
    /// </summary>
    /// <param name="node">: YAML Node [data of gameObject] </param>
    /// <param name="gameObject">: gameObject to which data is going to be deserialized</param>
    bool SceneSerializer::m_DeserializeGameObject(const YAML::Node &node, GameObject *gameObject, bool isAppended, bool overwriteUUID, const std::string &realChildUUID) {
        if (isAppended) {
            std::string path = node["pathToString"].as<std::string>();
        } 

        GameObject *deserializedGameObject;

        std::string name = node["name"].as<std::string>();
        std::string uuid; 
        if (overwriteUUID){
            uuid = UUID::generateUUID();
        } else {
            if (!realChildUUID.empty()) {
                uuid = realChildUUID;
            } else {
                uuid = node["uuid"] ? node["uuid"].as<std::string>() : UUID::generateUUID();
            }
        }
        gameObject->uuid = uuid;
        
        {
            bool m_enabled = node["m_enabled"].as<bool>();

            if (!gameObject->parent) {
                deserializedGameObject = scene->sceneRootObject.get();
                deserializedGameObject->uuid = uuid;
                deserializedGameObject->name = name; 
            } else {
                deserializedGameObject = gameObject;

            }
            
            m_DeserializeTransformComponent(node, deserializedGameObject);
            deserializedGameObject->m_enabled = m_enabled;
            

            auto deserializedChildren = node["children"];
            if (deserializedChildren && deserializedChildren.IsSequence()) {
                for (auto child: deserializedChildren) {
                    if (child.IsMap()) {
                        std::string name = child["name"].as<std::string>();
                        
                        GameObject *deserializedChild = new GameObject(name);
                        if (child["pathToString"]) {
                            std::string pathToString = child["pathToString"].as<std::string>();
                            std::string childName = child["name"].as<std::string>();
                            childName = scene->CheckGameObjectNameSiblings(childName, deserializedGameObject);
                            if (pathToString != "") {
                                DeserializeToMap(scene, pathToString);
                                //GameObject *parentChildTemp = new GameObject(childName);
                                GameObject *realChild = new GameObject(childName);
                                gameObject->AddChild(realChild);
                                //parentChildTemp->AddChild(realChild);
                                gameObject->children.back()->pathToScene = pathToString;
                                gameObject->children.back()->isScene = true;
                                //m_DeserializeTransformComponent(child, parentChildTemp);
                                
                                
                                //auto deserializedComponents = child["components"];
                                //if (deserializedComponents && deserializedComponents.IsSequence()) {
                                //    for (auto component: deserializedComponents) {
                                //        if (component.IsMap() && component.size() == 1) {
                                //            auto it = component.begin();
                                //            std::string type = it->first.as<std::string>();
                                //            auto node = it->second;
                                //            if (!m_DeserializeComponent(node, parentChildTemp, type)) {

                                //                return false;
                                //            }
                                //        }
                                //    }
                                //}
                                std::string uuid;
                                if (child["uuid"]){
                                    uuid = child["uuid"].as<std::string>();
                                } else{
                                    uuid = UUID::generateUUID();
                                }
                                if (!m_DeserializeGameObject(_rootObjects[pathToString][0], realChild, true, overwriteUUID, uuid)) {
                                    EngineDebug::GetInstance().PrintError("THERE IS SOMETHING WRONG WITH PREFAB LOADING TO MAIN SCENES! GOname: " + realChild->name);
                                    return false;
                                }
                                m_DeserializeTransformComponent(child, realChild);
                                continue;
                            }
                        }
                        else {
                            m_DeserializeTransformComponent(child, deserializedChild);
                            gameObject->AddChild(deserializedChild);
                        }
                        if (isAppended){
                            if(!m_DeserializeGameObject(child, deserializedChild, false, true)){
                                EngineDebug::GetInstance().PrintError("THERE IS SOMETHING WRONG WITH LOADING TO MAIN SCENES! GOname: " + child["name"].as<std::string>());
                                return false;
                            }
                        }
                        else if (!m_DeserializeGameObject(child, deserializedChild,isAppended,overwriteUUID)) {
                            EngineDebug::GetInstance().PrintError("THERE IS SOMETHING WRONG WITH LOADING TO MAIN SCENES! GOname: " + child["name"].as<std::string>());
                            return false;
                        }
                    }
                }
            }

            auto deserializedComponents = node["components"];
            if (deserializedComponents && deserializedComponents.IsSequence()) {
                for (auto component: deserializedComponents) {
                    if (component.IsMap() && component.size() == 1) {
                        auto it = component.begin();
                        std::string type = it->first.as<std::string>();
                        auto node = it->second;
                        if (!m_DeserializeComponent(node, deserializedGameObject, type)) {
                            EngineDebug::GetInstance().PrintError(
                                    "THERE IS SOMETHING WRONG WITH LOADING GO&COMPONENT TO MAIN SCENE! GOname: " +
                                    deserializedGameObject->name + "COMPONENT NAME: " + type);
                            return false;
                        }
                    }
                }
            }
            return true;
        }
    }

    /// <summary>
    /// Deserializes component based on their type
    /// </summary>
    /// <param name="node">: YAML Node [data of gameObject]</param>
    /// <param name="gameObject">: gameObject to which data component is going to be deserialized and added</param>
    /// <param name="type">: Type (derived class) of component that will be deserialized</param>
    bool SceneSerializer::m_DeserializeComponent(const YAML::Node &node, GameObject *gameObject,
                                               const std::string &type) {


        if (type == "Animator") {
            auto *comp = gameObject->AddComponent<Animator>();

            m_DeserializeComponentDefaultData(node, comp);

            comp->SetBlending(node["m_Blending"].as<bool>());
            comp->SetBlendFactor(node["m_BlendFactor"].as<float>());
            AnimationLibrary &m_AnimationLibrary = AnimationLibrary::GetInstance();
            auto animations = node["animations"];
            if (animations && animations.IsMap()) {
                for (const auto &animation: animations) {
                    std::string name = animation.second["animationName"].as<std::string>();
                    std::string path = animation.second["animationPath"].as<std::string>();
                    m_AnimationLibrary.AddAnimation(name, path);
                    std::shared_ptr<Animation> anim = m_AnimationLibrary.GetAnimation(name);
                    comp->AddAnimation(anim);
                }
            }
            comp->SetCurrentAnimation(node["CurrentAnimation"].as<std::string>());
            if (node["Playing"].as<bool>()) {
                comp->Play();
            }
            
            auto transitions = node["animationTransitions"];
            if (transitions && transitions.IsMap()) {
                for (const auto &transition : transitions){
                    AnimationTransition animTrans = transition.second.as<AnimationTransition>();
                    comp->AddAnimationTransition(animTrans);
                }
            }


        } else if (type == "AudioListener") {
            glm::vec3 pos = node["listener_pos"].as<glm::vec3>();
            glm::vec3 forward = node["listener_forward"].as<glm::vec3>();
            glm::vec3 up = node["listener_up"].as<glm::vec3>();
            auto *comp = gameObject->AddComponent<AudioListener>(pos,forward,up);

            m_DeserializeComponentDefaultData(node, comp);
            
            comp->listener_vel = node["listener_vel"].as<glm::vec3>();
            
            
        } else if (type == "AudioSource") {
            std::string bankPath = node["bankPath"].as<std::string>();
            int flag = node["flag"].as<int>();
            auto *comp = gameObject->AddComponent<AudioSource>(bankPath, flag);
            m_DeserializeComponentDefaultData(node, comp);
            comp->SetPos(node["pos"].as<glm::vec3>());

            comp->bankPath = node["bankPath"].as<std::string>();
            if (comp->bank) {
                comp->bank->unload();
                comp->bank = nullptr;
            }
            
            comp->eventNames = node["eventNames"] ? node["eventNames"].as<std::vector<std::string>>() : std::vector<std::string>();
            comp->isAllPaused = node["isAllPaused"] ? node["isAllPaused"].as<bool>() : false;
            
            flag = static_cast<FMOD_STUDIO_LOAD_BANK_FLAGS>(node["flag"].as<int>());
            ref.ERRCHECK(ref.fmodSystem->loadBankFile(comp->bankPath.c_str(), flag, &comp->bank));
            comp->affectedByDSP = node["affectedByDSP"] ? node["affectedByDSP"].as<bool>() : false;
            if (node["activeEvents"]) {
                
                for (const auto &eventData: node["activeEvents"]) {
                    std::string eventName = eventData["eventName"].as<std::string>();
                    comp->AddToEvents(eventName);
                    if (eventData["isPlaying"].as<bool>()){
                        comp->Play(eventName);
                        comp->SetPitchAll(eventData["pitch"].as<float>());
                        comp->SetVolumeAll(eventData["volume"].as<float>());
                    }
                }
            }

        } else if (type == "ThirdPersonCamera") {
            auto aspectRatio = node["aspectRatio"].as<float>();
            auto *comp = gameObject->AddComponent<ThirdPersonCamera>(aspectRatio);

            m_DeserializeComponentDefaultData(node, comp);
            comp->Front = node["Front"].as<glm::vec3>();
            comp->Up = node["Up"].as<glm::vec3>();
            comp->Right = node["Right"].as<glm::vec3>();
            comp->WorldUp = node["WorldUp"].as<glm::vec3>();
            comp->Yaw = node["Yaw"].as<float>();
            comp->Pitch = node["Pitch"].as<float>();
            comp->MovementSpeed = node["MovementSpeed"].as<float>();
            comp->MouseSensitivity = node["MouseSensitivity"].as<float>();
            comp->Zoom = node["Zoom"].as<float>();

            comp->NearPlane = node["NearPlane"].as<float>();
            comp->FarPlane = node["FarPlane"].as<float>();

            comp->orbitDistance = node["orbitDistance"].as<float>();
            comp->orbitYaw = node["orbitYaw"].as<float>();
            comp->orbitPitch = node["orbitPitch"].as<float>();
            comp->targetHeightOffset = node["targetHeightOffset"].as<float>();
            comp->minOrbitPitch = node["minOrbitPitch"].as<float>();
            comp->maxOrbitPitch = node["maxOrbitPitch"].as<float>();
            comp->minDistance = node["minDistance"].as<float>();
            comp->maxDistance = node["maxDistance"].as<float>();
            comp->positionSmoothing = node["positionSmoothing"].as<float>();
            comp->rotationSmoothing = node["rotationSmoothing"].as<float>();
            comp->Pos = node["Pos"].as<glm::vec3>();

        } else if (type == "Camera") {
            auto aspectRatio = node["aspectRatio"].as<float>();
            auto *comp = gameObject->AddComponent<Camera>(aspectRatio);

            m_DeserializeComponentDefaultData(node, comp);
            comp->Front = node["Front"].as<glm::vec3>();
            comp->Up = node["Up"].as<glm::vec3>();
            comp->Right = node["Right"].as<glm::vec3>();
            comp->WorldUp = node["WorldUp"].as<glm::vec3>();
            comp->Yaw = node["Yaw"].as<float>();
            comp->Pitch = node["Pitch"].as<float>();
            comp->MovementSpeed = node["MovementSpeed"].as<float>();
            comp->MouseSensitivity = node["MouseSensitivity"].as<float>();
            comp->Zoom = node["Zoom"].as<float>();

            comp->NearPlane = node["NearPlane"].as<float>();
            comp->FarPlane = node["FarPlane"].as<float>();

        } else if (type == "Hitbox") {
            std::string hitboxName = node["m_hitboxName"].as<std::string>();
            auto *comp = gameObject->AddComponent<Hitbox>(hitboxName);
            comp->SetShapeType(node["m_shapeType"].as<int>());
            comp->SetBoxSize(node["m_boxSize"].as<glm::vec3>());
            comp->SetCapsuleParams(node["m_capsuleRadius"].as<float>(), node["m_capsuleHeight"].as<float>());
            comp->SetColor(node["m_color"].as<glm::vec3>());
            comp->SetVisible(node["m_isVisible"].as<bool>());
            comp->SetWorldMin(node["m_WorldMin"].as<glm::vec3>());
            comp->SetWorldMax(node["m_WorldMax"].as<glm::vec3>());
            comp->MarkAABBDirty();
            comp->SetIsCollisionArea(node["m_IsCollisionArea"] ? node["m_IsCollisionArea"].as<bool>() : false);
            comp->SetCollisionMask(node["m_collisionMask"] ? node["m_collisionMask"].as<uint8_t>() : (uint8_t) 0);
            comp->SetCollisionLayer(node["m_collisionLayer"] ? node["m_collisionLayer"].as<uint8_t>() : (uint8_t) 0);
            comp->SetPositionOffset(node["m_collisionPositionOffset"] ? node["m_collisionPositionOffset"].as<glm::vec3>() : glm::vec3(0.0));
            comp->SetActive(node["m_isActive"] ? node["m_isActive"].as<bool>() : true);
            comp->SetDamage(node["m_damage"] ? node["m_damage"].as<float>() : 10.0);
            comp->SetOneHitPerTarget(node["m_oneHitPerTarget"] ? node["m_oneHitPerTarget"].as<bool>() : true);
            m_DeserializeComponentDefaultData(node, comp);
        } else if (type == "CollisionShape") {
            auto *comp = gameObject->AddComponent<CollisionShape>();
            comp->SetShapeType(node["m_shapeType"].as<int>());
            comp->SetBoxSize(node["m_boxSize"].as<glm::vec3>());
            comp->SetCapsuleParams(node["m_capsuleRadius"].as<float>(), node["m_capsuleHeight"].as<float>());
            comp->SetColor(node["m_color"].as<glm::vec3>());
            comp->SetVisible(node["m_isVisible"].as<bool>());
            comp->SetWorldMin(node["m_WorldMin"].as<glm::vec3>());
            comp->SetWorldMax(node["m_WorldMax"].as<glm::vec3>());
            comp->MarkAABBDirty();
            comp->SetIsCollisionArea(node["m_IsCollisionArea"] ? node["m_IsCollisionArea"].as<bool>() : false);
            comp->SetCollisionMask(node["m_collisionMask"] ? node["m_collisionMask"].as<uint8_t>() : (uint8_t)0);
            comp->SetCollisionLayer(node["m_collisionLayer"] ? node["m_collisionLayer"].as<uint8_t>() : (uint8_t)0);
            comp->SetPositionOffset(
                    node["m_collisionPositionOffset"] ? node["m_collisionPositionOffset"].as<glm::vec3>() : glm::vec3(0.0));
            m_DeserializeComponentDefaultData(node, comp);

        } else if (type == "DirectionalLight") {
            auto *comp = gameObject->AddComponent<DirectionalLight>();

            m_DeserializeComponentDefaultData(node, comp);
            m_DeserializeLightComponentDefaultData(node, comp);
            comp->direction = node["direction"].as<glm::vec3>();

        } else if (type == "PointLight") {
            auto *comp = gameObject->AddComponent<PointLight>();

            m_DeserializeComponentDefaultData(node, comp);
            m_DeserializeLightComponentDefaultData(node, comp);
            comp->constant = node["constant"].as<float>();
            comp->linear = node["linear"].as<float>();
            comp->quadratic = node["quadratic"].as<float>();

            if (node["castShadows"])
            {
                comp->castShadows = node["castShadows"].as<bool>();
            }

            if (node["shadowDistance"])
            {
                comp->shadowDistance = node["shadowDistance"].as<float>();
            }

        } else if (type == "SpotLight") {
            auto *comp = gameObject->AddComponent<SpotLight>();

            m_DeserializeComponentDefaultData(node, comp);
            m_DeserializeLightComponentDefaultData(node, comp);
            comp->constant = node["constant"].as<float>();
            comp->linear = node["linear"].as<float>();
            comp->quadratic = node["quadratic"].as<float>();
            comp->direction = node["direction"].as<glm::vec3>();
            comp->innerCutOff = node["innerCutOff"].as<float>();
            comp->outerCutOff = node["outerCutOff"].as<float>();

            if (node["castShadows"])
            {
                comp->castShadows = node["castShadows"].as<bool>();
            }

            if (node["shadowDistance"])
            {
                comp->shadowDistance = node["shadowDistance"].as<float>();
            }

        } else if (type == "LuaComponent") {
            std::string scriptPath = node["scriptPath"].as<std::string>();
            auto *comp = gameObject->AddComponent<LuaComponent>(scriptPath);
            m_DeserializeComponentDefaultData(node, comp);

        } else if (type == "MeshInstance") {
            auto *comp = gameObject->AddComponent<MeshInstance>();

            m_DeserializeComponentDefaultData(node, comp);
            comp->directory = node["directory"].as<std::string>();
            comp->m_modelPath = node["m_modelPath"].as<std::string>();
            //comp->m_instanced = node["m_instanced"].as<bool>();
            comp->LoadModel(comp->m_modelPath);
            /*if (node["MaterialBase"]) {
                MaterialSerializer::GetInstance().DeserializeForModel(node["MaterialBase"], &comp->model->Material);
            }*/
            if (node["MaterialOverride"]) {
                if (!comp->MaterialOverride) {
                    comp->MaterialOverride = std::make_shared<Material>();
                }
                MaterialSerializer::GetInstance().DeserializeForModel(node["MaterialOverride"], comp->MaterialOverride.get());
            }
            if (node["density"]) {
                comp->useVolumetric(true);
                comp->density = node["density"].as<float>();
                comp->samples = node["samples"].as<int>();
                comp->fogColor = node["fogColor"].as<glm::vec3>();
                comp->scattering = node["scattering"].as<float>();
            }

        } else if (type == "TextRenderer") {
            auto text = node["text"].as<std::string>();
            auto position = node["position"].as<glm::vec2>();
            auto fontSize = node["font_size"].as<float>();
            auto color = node["color"].as<glm::vec3>();
            auto *comp = gameObject->AddComponent<TextRenderer>(text, position, fontSize, color);

            m_DeserializeComponentDefaultData(node, comp);
            comp->scale = node["scale"].as<glm::vec2>();

        } else if (type == "WorldEnvironment") {
            if (scene->m_WorldEnvironment != nullptr){
                EngineDebug::GetInstance().PrintInfo("ENVIRO ALREADY EXISTS! I WON'T CREATE ANOTHER! ");
                return true;
            }
            auto skyboxPath = node["skyboxPath"].as<std::string>();
            auto *comp = gameObject->AddComponent<WorldEnvironment>(skyboxPath);
            comp->SetSkyType(node["m_skyType"].as<int>());
            
            comp->irradianceStrength = node["irradianceStrength"] ? node["irradianceStrength"].as<float>() : 1.0f;
            comp->sampleDelta = node["sampleDelta "] ? node["sampleDelta "].as<float>() : 0.01f;

            m_DeserializeComponentDefaultData(node, comp);

        } else if (type == "PhysicsBody") {
            auto mass = node["mass"].as<float>();
            auto useGravity = node["useGravity"].as<bool>();
            auto *comp = gameObject->AddComponent<PhysicsBody>(mass, useGravity);
            comp->velocity = node["velocity"].as<glm::vec3>();
            comp->isStatic = node["isStatic"].as<bool>();
            comp->restitution = node["restitution"].as<float>();
            comp->invMass = node["invMass"].as<float>();
            comp->accumulatedForces = node["accumulatedForces"].as<glm::vec3>();
            comp->linearDamping = node["linearDamping"].as<float>();
            comp->isGrounded = node["isGrounded"].as<bool>();
            comp->stabilizationPosition = node["stabilizationPosition"].as<glm::vec3>();
            m_DeserializeComponentDefaultData(node, comp);

        } else if (type == "ParticleSystem") {
            auto *comp = gameObject->AddComponent<ParticleSystem>();
            comp->EmissionShape = static_cast<EmissionShapeType>(node["EmissionShape"].as<int>());
            comp->DrawEmissionShape = node["DrawEmissionShape"].as<bool>();
            comp->CubeSize = node["CubeSize"].as<glm::vec3>();
            comp->SphereRadius = node["SphereRadius"].as<float>();
            comp->SphereSurfaceOnly = node["SphereSurfaceOnly"].as<bool>();
            comp->Emitting = node["Emitting"].as<bool>();
            comp->SetMaxParticles(node["MaxParticles"].as<unsigned int>());
            comp->SpawnRate = node["SpawnRate"].as<float>();
            comp->OneShot = node["OneShot"].as<bool>();
            comp->MinLifeTime = node["MinLifeTime"].as<float>();
            comp->MaxLifeTime = node["MaxLifeTime"].as<float>();
            comp->MinSize = node["MinSize"].as<float>();
            comp->MaxSize = node["MaxSize"].as<float>();
            comp->Gravity = node["Gravity"].as<glm::vec3>();
            comp->Albedo = node["Albedo"].as<glm::vec4>();
            comp->MinInitialVelocity = node["MinInitialVelocity"].as<glm::vec3>();
            comp->MaxInitialVelocity = node["MaxInitialVelocity"].as<glm::vec3>();
            comp->CameraPosition = node["CameraPosition"].as<glm::vec3>();
            comp->BlendingAdditive = node["BlendingAdditive"].as<bool>();
            comp->EmissionMultiplier = node["EmissionMultiplier"].as<float>();
            comp->UseLocalSpace = node["UseLocalSpace"].as<bool>();
            comp->SetShader(dBrender::GetInstance().GetParticlesShader());
            std::shared_ptr<Texture> pTexture = ResourceManager::GetInstance().LoadTextureFromFile( node["m_Texture"].as<Texture>().path);
            comp->m_Texture = pTexture;
            comp->m_Initialized = false; //check if correct.
            if (node["BillboardY"])
            {
                comp->BillboardY = node["BillboardY"].as<bool>();
            }

            if (node["ScaleOverTime"])
            {
                comp->ScaleOverTime = node["ScaleOverTime"].as<bool>();
            }

            if (node["BeginScaling"])
            {
                comp->BeginScaling = node["BeginScaling"].as<float>();
            }

            if (node["FadeOverTime"])
            {
                comp->FadeOverTime = node["FadeOverTime"].as<bool>();
            }

            if (node["BeginFading"]) {
                comp->BeginFading = node["BeginFading"].as<float>();
            }
            m_DeserializeComponentDefaultData(node, comp);

        } else if (type == "Tag") {
            auto Name = node["Name"].as<std::string>();
            auto *comp = gameObject->AddComponent<Tag>(Name);
            m_DeserializeComponentDefaultData(node, comp);

        } else if (type == "Timer") {
            float TimeoutMs = node["TimeoutMs"].as<float>();
            bool OneShot = node["OneShot"].as<bool>();
            std::string SignalMessage = node["SignalMessage"].as<std::string>();
            auto *comp = gameObject->AddComponent<Timer>(TimeoutMs, SignalMessage, OneShot);
            comp->HasStarted = node["HasStarted"].as<bool>();
            comp->HasTimedOut = node["HasTimedOut"].as<bool>();
            comp->ElapsedMs = node["ElapsedMs"].as<float>();
            m_DeserializeComponentDefaultData(node, comp);
            
        } else if (type == "Button"){
            auto *comp = gameObject->AddComponent<UI::Button>();
            comp->Color = node["Color"].as<glm::vec3>();
            if (node["m_HasTexture"].as<bool>()) {
                std::shared_ptr<Texture> pTexture = ResourceManager::GetInstance().LoadTextureFromFile( node["m_Texture"].as<Texture>().path);
                comp->SetTexture(pTexture);
            }
            if (node["m_HoverTexture"])
            {
                if (node["m_HoverTexture"].as<Texture>().id != 0)
                {
                    std::shared_ptr<Texture> pHoverTexture = ResourceManager::GetInstance().LoadTextureFromFile(
                        node["m_HoverTexture"].as<Texture>().path);
                    comp->SetHoverTexture(pHoverTexture);
                }
            }
            
            m_DeserializeControlComponentDefaultData(node, comp);
            m_DeserializeComponentDefaultData(node, comp);
        } else if (type == "Text") {
            auto *comp = gameObject->AddComponent<UI::Text>();
            comp->SetText(node["Text"].as<std::string>());
            comp->SetFontSize(node["FontSize"].as<float>());
            comp->SetColor(node["Color"].as<glm::vec3>());
            m_DeserializeControlComponentDefaultData(node, comp);
            m_DeserializeComponentDefaultData(node, comp);
        }
        else if (type == "Sprite")
        {
            auto* comp = gameObject->AddComponent<UI::Sprite>();
            comp->ModulateColor = node["ModulateColor"].as<glm::vec4>();
            if (node["m_HasTexture"].as<bool>())
            {
                std::shared_ptr<Texture> pTexture = ResourceManager::GetInstance().LoadTextureFromFile(node["m_Texture"].as<Texture>().path);
                comp->SetTexture(pTexture);
            }
            if (node["Clipping"])
            {
                comp->Clipping = node["Clipping"].as<glm::vec4>();
            }
            m_DeserializeControlComponentDefaultData(node, comp);
            m_DeserializeComponentDefaultData(node, comp);
        }
        else if (type == "PlayerController"){
            auto *comp = gameObject->AddComponent<PlayerController>();
            comp->moveSpeed = node["moveSpeed"].as<float>();
            comp->acceleration = node["acceleration"].as<float>();
            comp->deceleration = node["deceleration"].as<float>();
            comp->maxSpeed = node["maxSpeed"].as<float>();
        } else if (type == "NavigationMesh") {
            auto *comp = gameObject->AddComponent<NavigationMesh>();
            if (comp) {
                if (node["floorObjectUUID"]) {
                    comp->floorUUID = node["floorObjectUUID"].as<std::string>();
                }
                if (node["obstaclesObjectsUUID"]) {
                    YAML::Node obstaclesNode = node["obstaclesObjectsUUID"];
                    if (obstaclesNode.IsSequence()) {
                        for (const auto &uuidNode: obstaclesNode) {
                            std::string uuid = uuidNode.as<std::string>();
                            comp->obstaclesUUIDS.push_back(uuid);
                        }
                    }
                }
                m_DeserializeComponentDefaultData(node, comp);
            }
        } else if (type == "AIAgent") {
            auto *comp = gameObject->AddComponent<AIAgent>();
            if (comp) {
                bool pauseFlag = node["isPaused"].as<bool>();
                if (pauseFlag) {
                    comp->Pause();
                } else {
                    comp->Resume();
                }
                comp->SetPosition(node["agentPosition"].as<glm::vec3>());
                comp->SetSpeed(node["speed"].as<float>());
                comp->SetMaxSpeed(node["maxSpeed"].as<float>());
                comp->SetMaxForce(node["maxForce"].as<float>());
                comp->SetMass(node["mass"].as<float>());
                comp->SetTargetPos(node["targetPos"].as<glm::vec3>());
                comp->SetLineOfSightDistance(node["lineOfSightDistance"].as<float>());
                comp->SetStoppingDistance(node["stoppingDistance"].as<float>());
                comp->SetCircleRadius(node["circleRadius"].as<float>());
                comp->SetCircleAngle(node["circleAngle"].as<float>());
                comp->SetCircleDirection(node["circleDirection"].as<float>());

                m_DeserializeComponentDefaultData(node, comp);
            }
        } else if (type == "NavigationTarget") {
            auto *comp = gameObject->AddComponent<NavigationTarget>();
            if (comp) {
                comp->SetPosition(node["targetPosition"].as<glm::vec3>());

                m_DeserializeComponentDefaultData(node, comp);
            }
        } else if (type == "AISystem") {
            auto *comp = gameObject->AddComponent<AISystem>();
            if (comp) {
                if(node["m_navMeshObjectUUID"]) {
                    comp->m_navMeshUUID = node["m_navMeshObjectUUID"].as<std::string>();
                }
                if (node["agentsObjectsUUID"]) {
                    YAML::Node agentsNode = node["agentsObjectsUUID"];
                    if (agentsNode.IsSequence()) {
                        for (const auto &uuidNode: agentsNode) {
                            comp->m_agentUUIDs.push_back(uuidNode.as<std::string>());
                        }
                    }
                }
                if (node["targetObjectUUID"]) {
                    comp->m_targetUUID = node["targetObjectUUID"].as<std::string>();
                }
                m_DeserializeComponentDefaultData(node, comp);
            }
        }
        
        else {
            EngineDebug::GetInstance().PrintError("Yo pal, there is no such component implemented to DEserialize: " + type);
            return false;
        }

        return true;
    }

/// <summary>
    /// Deserializes default parameters for each of the components
    /// </summary>
    /// <param name="node">: YAML node [data of component]</param>
    /// <param name="component">: component that is going to be serialized</param>
    bool SceneSerializer::m_DeserializeComponentDefaultData(const YAML::Node &node, Component *component) {
        component->name = node["ComponentName"].as<std::string>();
        component->enabled = node["Enabled"].as<bool>();
        return true;
    }

    bool SceneSerializer::m_DeserializeLightComponentDefaultData(const YAML::Node &node, Light *component) {
        component->diffuse = node["diffuse"].as<glm::vec3>();
        component->specular = node["specular"].as<glm::vec3>();
        component->ambient = node["ambient"].as<glm::vec3>();
        component->intensity = node["intensity"].as<float>();
        component->ambientIntensity = node["ambientIntensity"] ? node["ambientIntensity"].as<float>() : 1.0f;
        return true;
    }

    /// <summary>
    /// Deserializes transform component.
    /// </summary>
    /// <param name="node">: data yaml node</param>
    /// <param name="gameObject">: gameobject where data will be deserialized to</param>
    /// <returns>if succed</returns>
    bool SceneSerializer::m_DeserializeTransformComponent(const YAML::Node &node, GameObject* gameObject){
        auto deserializedTransform = node["transform"];
        if (deserializedTransform) {
            auto &dct = gameObject->transform;
            dct.position = deserializedTransform["position"].as<glm::vec3>();
            dct.scale = deserializedTransform["scale"].as<glm::vec3>();
            dct.rotation = deserializedTransform["rotation"].as<glm::quat>();
            dct.modelMatrix = deserializedTransform["modelMatrix"].as<glm::mat4>();
            dct.globalPosition = deserializedTransform["globalPosition"].as<glm::vec3>();
            dct.m_isDirty = true;
            return true;
        }
        return false;     
    }

    bool SceneSerializer::m_DeserializeControlComponentDefaultData(const YAML::Node &node, UI::Control *component) {
        component->Position = node["Position"].as<glm::vec2>();
        component->Size = node["Size"].as<glm::vec2>();
        component->Emission = node["Emission"].as<float>();
        component->anchor = static_cast<UI::Anchor>(node["anchor"] ? node["anchor"].as<int>() : 0);
        component->Rotation = node["Rotation"] ? node["Rotation"].as<float>() : 0.0f;
        
        return true;
    }

    /// <summary>
    /// Creates gameObject from loaded scene string [yaml format] saved in maps
    /// </summary>
    /// <param name="filepath">: filepath of given scene</param>
    /// <param name="parent">: gameObject to which we want add loaded scene to</param>
    /// <returns> bool: if succeded</returns>
    bool SceneSerializer::createGameObjectFromScene(Scene *scene, const std::string &filePath, GameObject *parent, bool overwriteUUID) {
        SetScene(scene);



        auto it = _rootObjects.find(filePath);
        if (it == _rootObjects.end())
            return false;
        const YAML::Node& rootObjectData = it->second[0];

        std::string name = rootObjectData["name"].as<std::string>();
        name = scene->CheckGameObjectNameSiblings(name, parent);
        GameObject* realChild = new GameObject(name);

        realChild->Disable();
        parent->AddChild(realChild);
        parent->children.back()->pathToScene = filePath;
        parent->children.back()->isScene = true;
        if (m_DeserializeGameObject(rootObjectData, realChild, true, overwriteUUID))
        {
            realChild->Enable();
            return true;
        }
        return false;
    }

    bool SceneSerializer::DeserializeToMap(Scene *scene, const std::string &filePath) {
        SetScene(scene);
        std::ifstream stream(filePath);
        std::stringstream strStream;
        strStream << stream.rdbuf();


        unsigned long long int epochTimeLoaded = Util::getEpochTimeOfAFile(Util::getFileModificationTime(filePath));

        if (_rootObjects.find(filePath) != _rootObjects.end())
        {
            if (_modifyTimes[filePath] < epochTimeLoaded)
            {
                _modifyTimes[filePath] = epochTimeLoaded;

                YAML::Node data = YAML::Load(strStream.str());
                if (!data["rootGameObject"])
                {
                    EngineDebug::GetInstance().PrintError(
                        "LOADED YAML VERSION IS NOT ACCEPTED! [SceneSerializer::DeserializeToMap]");
                    return false;
                }
                AnimationLibrary& animLib = AnimationLibrary::GetInstance();
                auto animations = data["LibraryAnimations"];
                if (animations && animations.IsMap())
                {
                    for (const auto& animation : animations)
                    {
                        std::string name = animation.second["animationName"].as<std::string>();
                        std::string path = animation.second["animationPath"].as<std::string>();
                        animLib.AddAnimation(name, path);
                    }
                }

                auto rootGameObject = data["rootGameObject"];
                _rootObjects[filePath] = rootGameObject;
                std::cout << "STARY JEST NOWSZE JBKC!\n";
                //tu sygnal strzelic o modyfikacji?
            }
        }
        else
        {

            YAML::Node data = YAML::Load(strStream.str());
            if (!data["rootGameObject"])
            {
                EngineDebug::GetInstance().PrintError(
                    "LOADED YAML VERSION IS NOT ACCEPTED! [SceneSerializer::DeserializeToMap]");
                return false;
            }
            AnimationLibrary& animLib = AnimationLibrary::GetInstance();
            auto animations = data["LibraryAnimations"];
            if (animations && animations.IsMap())
            {
                for (const auto& animation : animations)
                {
                    std::string name = animation.second["animationName"].as<std::string>();
                    std::string path = animation.second["animationPath"].as<std::string>();
                    animLib.AddAnimation(name, path);
                }
            }

            auto rootGameObject = data["rootGameObject"];
            _rootObjects[filePath] = rootGameObject;
            _modifyTimes[filePath] = Util::getEpochTimeOfAFile(Util::getFileModificationTime(filePath));
        }



        return true;
    }

} // namespace Serialize
