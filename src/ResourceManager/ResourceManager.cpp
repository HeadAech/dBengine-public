#include "ResourceManager.h"
#include "Components/MeshInstance/Model.h"
#include <glad/glad.h>
#include <Helpers/TimerHelper/TimerHelper.h>

static std::mutex s_TextureJobsMutex;
static std::mutex s_ModelJobsMutex;
static std::mutex s_TexturesMutex;

ResourceManager &ResourceManager::GetInstance() {
    static ResourceManager instance;
    return instance;
}

void ResourceManager::LoadTextures_Async(const std::string &path, bool fromMemory, std::vector<unsigned char> data, unsigned int width) {
    int w, h, c;
    unsigned char *fileData;
    stbi_set_flip_vertically_on_load(false);
    if (fromMemory)
    {
        fileData = stbi_load_from_memory(data.data(), static_cast<int>(data.size()), &w, &h, &c, 0);

    }
    else
        fileData = stbi_load(path.c_str(), &w, &h, &c, 0);

    if (fileData) {
        TextureLoadJob job;
        job.Path = path;
        job.Width = w;
        job.Height = h;
        job.Channels = c;
        job.PixelData.assign(fileData, fileData + (w * h * c));
        stbi_image_free(fileData);

        std::lock_guard<std::mutex> lock(s_TextureJobsMutex);
        m_TextureJobs.push(std::move(job));
    } else {
        std::cerr << "Failed to load texture: " << path << ", fromMemory = " << fromMemory << "\n";
    }
}

std::shared_ptr<Texture> ResourceManager::LoadTextureFromFile(std::string filename, bool fromMemory, std::vector<unsigned char> data, unsigned int width) {

    auto it = m_LoadedTextures.find(filename);
    m_TextureSubscribers[filename]++;

    if (it != m_LoadedTextures.end())
        return m_LoadedTextures[filename];

    m_Futures.push_back(std::async(std::launch::async, &ResourceManager::LoadTextures_Async, this, filename, fromMemory, data, width));

    std::lock_guard<std::mutex> lock(s_TexturesMutex);

    m_LoadedTextures[filename] = std::make_shared<Texture>();

    return m_LoadedTextures[filename];
}

static std::mutex s_ModelsMutex;

void ResourceManager::LoadMesh_Async(std::unordered_map<std::string, std::shared_ptr<Model>>* models, std::string filePath)
{
    auto model = Model::LoadModel(filePath);

    {
        std::lock_guard<std::mutex> lock(s_ModelsMutex);

        *(*models)[filePath] = std::move(model);
    }
    
    {
        std::lock_guard<std::mutex> lock(s_ModelJobsMutex);

        ModelLoadJob job;
        job.Path = filePath;

        m_ModelJobs.push(job);
    }
    
    
}

void ResourceManager::loadMesh(const std::string& path)
{
    m_Futures.push_back(std::async(std::launch::async, &ResourceManager::LoadMesh_Async, this, &m_LoadedModels, path));
}

std::shared_ptr<Model> ResourceManager::LoadMeshFromFile(const std::string &path) {

    std::lock_guard<std::mutex> lock(s_ModelsMutex);

    auto it = m_LoadedModels.find(path);
    m_ModelSubscribers[path]++;

    if (it != m_LoadedModels.end()) {
        return it->second;
    }

    std::shared_ptr<Model> placeholder = std::make_shared<Model>();

    m_LoadedModels[path] = placeholder; 
    
    loadMesh(path);

    return placeholder;
}


void ResourceManager::ProcessPendingTextures() 
{
    std::lock_guard<std::mutex> lock(s_TextureJobsMutex);

    while (!m_TextureJobs.empty())
    {
        TextureLoadJob &job = m_TextureJobs.front();

        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        GLenum format = GL_RGBA;
        if (job.Channels == 1)
            format = GL_RED;
        else if (job.Channels == 3)
            format = GL_RGB;
        else if (job.Channels == 4)
            format = GL_RGBA;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, format, job.Width, job.Height, 0, format, GL_UNSIGNED_BYTE, job.PixelData.data());
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);

        std::lock_guard<std::mutex> lock(s_TexturesMutex);
        
        auto texture = m_LoadedTextures[job.Path];
        texture->id = tex;
        texture->path = job.Path;

        EngineDebug::GetInstance().PrintInfo("Texture " + job.Path + " is Ready.");
        m_TextureJobs.pop();
    }

}

void ResourceManager::ProcessPendingModels() 
{

    std::lock_guard<std::mutex> lock(s_ModelJobsMutex);

    while (!m_ModelJobs.empty())
    {
        ModelLoadJob &job = m_ModelJobs.front();

        {
            std::lock_guard<std::mutex> lock(s_ModelsMutex);

            auto model = m_LoadedModels[job.Path];

            for (int i = 0; i < model->Meshes.size(); i++)
            {
                if (model->Meshes[i].Ready == false)
                {
                    model->Meshes[i].SetupMesh();
                }
            }

            model->Ready = true;
            EngineDebug::GetInstance().PrintInfo("Model " + job.Path + " is Ready.");
        }
        

        m_ModelJobs.pop();
    }

}
