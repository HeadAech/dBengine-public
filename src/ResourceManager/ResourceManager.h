#pragma once

#include <glad/glad.h>
#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <stb_image.h>
#include <memory>
#include <Components/MeshInstance/MeshInstance.h>
#include <mutex>
#include <queue>
#include <future>
#include <string>

class Material;
class Texture;
class Model;

/// <summary>
/// Product of the asynchronous texture loading job.
/// </summary>
struct TextureLoadJob
{
    std::string Path;
    int Width, Height, Channels;
    std::vector<unsigned char> PixelData;
};

/// <summary>
/// Product of the asynchronous model loading job.
/// </summary>
struct ModelLoadJob
{
    std::string Path;
};

/// <summary>
/// Class responsible for managing resources such as textures and models.
/// </summary>
class ResourceManager {

    std::unordered_map<std::string, std::shared_ptr<Model>> m_LoadedModels;
    //TODO: Track subscribers for future memory free.
    std::unordered_map<std::string, int> m_ModelSubscribers;

    std::unordered_map<std::string, std::shared_ptr<Texture>> m_LoadedTextures;
    std::unordered_map<std::string, int> m_TextureSubscribers;

    void loadMesh(const std::string &path);

    std::vector<std::future<void>> m_Futures;

    std::queue<TextureLoadJob> m_TextureJobs;
    std::queue<ModelLoadJob> m_ModelJobs;

public:


	ResourceManager() = default;

    /// <summary>
    /// Returns the singleton instance of the ResourceManager.
    /// </summary>
    /// <returns>Singleton (ResourceManager)</returns>
    static ResourceManager &GetInstance();
    ResourceManager(const ResourceManager &) = delete;
    ResourceManager &operator=(const ResourceManager &) = delete;

	/// <summary>
	/// Returns a handle to a loaded texture.
    /// <para>
    ///     If texture is not currently loaded, it calls for an asynchronous operation of loading it.
    ///     Once the texture is loaded, all subsequent calls to this function will return the same texture
    ///     and handle will become valid.
    /// </para>
	/// </summary>
	/// <param name="filename">Path to the file</param>
	/// <param name="fromMemory">Is the texture embedded in a file like .fbx?</param>
	/// <param name="data">Assimp data of the file which the texture is embeded in (only valid if fromMemory is true)</param>
	/// <param name="width">Size of the data (only valid if fromMemory is true)</param>
	/// <returns>Handle to texture (shared pointer)</returns>
    std::shared_ptr<Texture> LoadTextureFromFile(std::string filename, bool fromMemory = false, std::vector<unsigned char> data = {}, unsigned int width = 0);

    /// <summary>
    /// Returns a handle to a loaded model.
    /// <para>
    ///     If model is not currently loaded, it calls for an asynchronous operation of loading it.
    ///     Once the model is loaded, all subsequent calls to this function will return the same model
    ///     and handle will become valid.
    /// </para>
    /// </summary>
    /// <param name="path">Path to the file</param>
    /// <returns>Handle to model (shared pointer)</returns>
    std::shared_ptr<Model> LoadMeshFromFile(const std::string &path);

    /// <summary>
    /// Processes all pending texture loading jobs.
    /// <para>
    ///     <para>OpenGL is not thread-safe, so this function should be called from the main thread.</para>
    ///     Every pending texture loading job will be processed and the corresponding texture will be loaded into
    ///     OpenGL.
    /// </para>
    /// </summary>
    void ProcessPendingTextures();

    /// <summary>
    /// Processes all pending model loading jobs.
    /// <para>
    ///     <para>OpenGL is not thread-safe, so this function should be called from the main thread.</para>
    ///     Every pending model loading job will be processed and the corresponding model will be loaded into
    ///     OpenGL.
    /// </para>
    /// </summary>
    void ProcessPendingModels();

    /// <summary>
    /// Asynchronously loads textures from a file or memory.
    /// <para>
    /// Only for internal use, as it does not return a handle to the texture.
    /// </para>
    /// </summary>
    void LoadTextures_Async(const std::string& path, bool fromMemory = false, std::vector<unsigned char> data = {}, unsigned int width = 0);

    /// <summary>
    /// Asynchronously loads a model from a file.
    /// <para>
    /// Only for internal use, as it does not return a handle to the model.
    /// </para>
    /// </summary>
    void LoadMesh_Async(std::unordered_map<std::string, std::shared_ptr<Model>> *models, std::string filePath);

};

#endif // !RESOURCE_MANAGER_H
