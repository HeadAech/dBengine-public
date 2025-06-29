#include "Sprite.h"
#include "ResourceManager/ResourceManager.h"
#include "Singletons/Ref/Ref.h"
#include "Helpers/fonts/IconsFontAwesome4.h"

using namespace UI;

Sprite::Sprite()
{
    name = "Sprite";
    icon = ICON_FA_PICTURE_O;
    m_Texture = std::make_shared<Texture>();
    //setupMesh();
}

Sprite::~Sprite()
{}

void Sprite::Render()
{
    // gameobject enabled or component enabled
    if (!gameObject->m_enabled || !enabled)
        return;

    auto pShader = GetShader();

    pShader->Use();

    glm::vec2 bottomLeftPx = GetAnchoredPosition();

    pShader->SetVec2("u_Position", bottomLeftPx);
    pShader->SetVec2("u_Size", Size);
    pShader->SetVec4("u_Color", ModulateColor);
    pShader->SetVec2("u_ScreenResolution", Ref::ScreenResolution);
    pShader->SetFloat("u_Emission", Emission);
    pShader->SetFloat("u_Rotation", Rotation);

    pShader->SetFloat("u_ClipLeft", Clipping.x);
    pShader->SetFloat("u_ClipRight", Clipping.y);
    pShader->SetFloat("u_ClipTop", Clipping.w);
    pShader->SetFloat("u_ClipBottom", Clipping.z);


    pShader->SetBool("u_HasTexture", m_HasTexture);

    if (m_HasTexture)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_Texture->id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        pShader->SetInt("u_Texture", 0);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glDisable(GL_DEPTH_TEST);
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    //glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void Sprite::Update(float deltaTime)
{
    if (m_VAO == 0 || m_VBO == 0)
        setupMesh();
}

void Sprite::SetTexture(std::string_view path) {
    m_Texture = ResourceManager::GetInstance().LoadTextureFromFile(path.data());
    m_HasTexture = true;
}

void Sprite::SetTexture(std::shared_ptr<Texture> texture) {
    this->m_Texture = texture;
    m_HasTexture = true;
}

bool Sprite::HasTexture() { return this->m_HasTexture; }

std::shared_ptr<Texture> Sprite::GetTexture() { return this->m_Texture; }

void Sprite::RemoveTexture() {
    m_Texture = std::make_shared<Texture>();
    m_HasTexture = false;
}

void Sprite::setupMesh()
{
    float quadVertices[] = {
     //  X      Y       U     V
      0.0f,   0.0f,   0.0f, 1.0f,
      1.0f,   0.0f,   1.0f, 1.0f,
      1.0f,   1.0f,   1.0f, 0.0f,

      0.0f,   0.0f,   0.0f, 1.0f,
      1.0f,   1.0f,   1.0f, 0.0f,
      0.0f,   1.0f,   0.0f, 0.0f
    };
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
