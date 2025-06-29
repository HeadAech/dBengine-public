#include "Button.h"
#include <InputManager/Input.h>
#include <ResourceManager/ResourceManager.h>
#include "GameObject/GameObject.h"
#include "Shader/Shader.h"
#include <ResourceManager/ResourceManager.h>
#include <Singletons/Ref/Ref.h>

using namespace UI;

Button::Button() {
    name = "Button";
    m_Texture = std::make_shared<Texture>();
    m_HoverTexture = std::make_shared<Texture>();
    
}

Button::~Button() {}

void Button::Render() {
    // gameobject enabled or component enabled
    if (!gameObject->m_enabled || !enabled)
        return;

    auto pShader = GetShader();

    if (!pShader)
        return;

    pShader->Use();

    glm::vec2 bottomLeftPx = GetAnchoredPosition();

    //bottomLeftPx.x = Position.x;
    //bottomLeftPx.y = (float) Ref::ScreenResolution.y - Position.y - Size.y;

    pShader->SetVec2("u_Position", bottomLeftPx);
    pShader->SetVec2("u_Size", Size);
    pShader->SetVec3("u_Color", Color);
    pShader->SetVec2("u_ScreenResolution", Ref::ScreenResolution);
    pShader->SetFloat("u_Emission", Emission);

    pShader->SetBool("u_HasTexture", m_HasTexture);

    if (m_HasTexture)
    {   
        glActiveTexture(GL_TEXTURE0);
        if (m_HoverTexture && isHovered(Ref::MousePosition))
        {
            glBindTexture(GL_TEXTURE_2D, m_HoverTexture->id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, m_Texture->id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        
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

void Button::Update(float deltaTime)
{
    if (m_VAO == 0 || m_VBO == 0)
    {
        setupMesh();
    }
	if (isHovered(Ref::MousePosition))
	{
        if (Input::GetInstance().IsActionPressed("mouse1"))
        {
            if (m_OnClickCallback)
            {
                m_OnClickCallback();
            }
            Pressed = true;
        }
        else
        {
            Pressed = false;
        }
	}
    else
    {
        Pressed = false;
    }
}

void Button::OnClick(std::function<void()> callback) { m_OnClickCallback = callback; }

void Button::SetTexture(std::string_view path)
{
    m_Texture = ResourceManager::GetInstance().LoadTextureFromFile(path.data());
    m_HasTexture = true;
}

void Button::SetTexture(std::shared_ptr<Texture> texture) {
    this->m_Texture = texture;
    m_HasTexture = true;
}

void UI::Button::SetHoverTexture(std::string_view path)
{
    m_HoverTexture = ResourceManager::GetInstance().LoadTextureFromFile(path.data());
}

void UI::Button::SetHoverTexture(std::shared_ptr<Texture> texture)
{
    this->m_HoverTexture = texture;
}

bool UI::Button::HasTexture() { return this->m_HasTexture; }

std::shared_ptr<Texture> UI::Button::GetTexture() { return this->m_Texture; }

std::shared_ptr<Texture> UI::Button::GetHoverTexture()
{
    return m_HoverTexture;
}

void UI::Button::RemoveTexture()
{
    m_Texture = std::make_shared<Texture>();
    m_HasTexture = false;
}

void UI::Button::RemoveHoverTexture()
{
    m_HoverTexture = std::make_shared<Texture>();
}

bool Button::isHovered(glm::vec2& mousePos)
{
    glm::vec2 anchoredPos = GetAnchoredPosition();
    glm::vec2 screenRes = Ref::ScreenResolution;
    glm::vec2 topLeftPos = glm::vec2(anchoredPos.x, screenRes.y - anchoredPos.y - Size.y);

    return (mousePos.x >= topLeftPos.x && mousePos.x <= topLeftPos.x + Size.x &&
        mousePos.y >= topLeftPos.y && mousePos.y <= topLeftPos.y + Size.y);
}

void Button::setupMesh()
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


