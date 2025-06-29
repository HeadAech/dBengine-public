#include "MaterialPanel.h"
#include <ResourceManager/ResourceManager.h>
#include <Helpers/Colors/Colors.h>
#include <Helpers/fonts/IconsFontAwesome4.h>
#include <ImGuiFileDialog.h>
#include <Serializers/MaterialSerializer.h>

MaterialPanel::MaterialPanel()
{
	SetName("Material");

}

void MaterialPanel::Draw()
{
	if (m_Visible || !p_MeshInstance)
		return;


	if (Panel::CollapsingHeader("Material"))
	{
		if (!p_Material)
		{
			ImGui::Text("No original material.");
		}
		else
		{
			drawEditProperties(p_Material);
		}
		
	}
	// original material tooltip
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		Panel::ColoredText("Warning!", Colors::DarkRed);
		ImGui::Text("Modifying mesh's original material will affect every instance of that model!");
		ImGui::EndTooltip();
	}
	

	// override material

	if (p_MeshInstance->MaterialOverride)
	{
		if (ImGui::Button(ICON_FA_LIST " Resource"))
		{
			ImGui::OpenPopup("MaterialOverrideContextMenu");
		}
	}
	else
	{
		// add material override
		if (ImGui::Button(ICON_FA_PLUS ""))
		{
			p_MeshInstance->MaterialOverride = std::make_shared<Material>();
		}
	}
	

	auto pMaterialOverride = p_MeshInstance->MaterialOverride;
	
	ImGui::SameLine();

	ImGui::BeginDisabled(pMaterialOverride == nullptr);
	if (Panel::CollapsingHeader("Material Override"))
	{

		if (pMaterialOverride)
		{
			if (m_ResourceModified)
			{
				Panel::ColoredText("-- Resource Modified! --", Colors::DarkRed);
			}
			ImGui::Text("Resource: ");
			ImGui::SameLine();
            ImGui::Text(pMaterialOverride->name.c_str());

			

			// material override existing
			drawEditProperties(pMaterialOverride.get());
		}
	}
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_PATH"))
		{
			if (payload->DataSize > 0)
			{
				std::string path = std::string(static_cast<const char*>(payload->Data), payload->DataSize - 1);

				loadMaterial(path);
			}
		}
		ImGui::EndDragDropTarget();
	}
	if (ImGui::BeginPopupContextItem("MaterialOverrideContextMenu"))
	{
		if (ImGui::MenuItem(ICON_FA_FOLDER " Load resource..."))
		{
			// loading saved resource logic
            IGFD::FileDialogConfig config;
            config.path = "./res/materials";
			ImGuiFileDialog::Instance()->OpenDialog("LoadMaterial", "Load material...", "Resource files (*.res *.yaml){.res,.yaml}", config);
		}

		if (ImGui::MenuItem(ICON_FA_FLOPPY_O " Save as..."))
		{
			// saving material logic
            IGFD::FileDialogConfig config;
            config.path = "./res/materials";
			ImGuiFileDialog::Instance()->OpenDialog("SaveMaterial", "Save material...", ".res,.yaml", config);

		}

		ImGui::Separator();
		if (ImGui::MenuItem(ICON_FA_TIMES " Remove"))
		{
			p_MeshInstance->MaterialOverride = nullptr;
		}

		ImGui::EndPopup();
	}
	
	ImGui::EndDisabled();

	// save dialog
	if (ImGuiFileDialog::Instance()->Display("SaveMaterial", NULL, ImVec2(400, 300)))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			std::string retPath = ImGuiFileDialog::Instance()->GetFilePathName();
			std::filesystem::path path = std::filesystem::path(retPath);
			if (pMaterialOverride)
			{
				saveMaterial(path.string(), pMaterialOverride.get());
			}
		}
		ImGuiFileDialog::Instance()->Close();
	}

	// load dialog
	if (ImGuiFileDialog::Instance()->Display("LoadMaterial", NULL, ImVec2(400, 300)))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
            std::string retPath = ImGuiFileDialog::Instance()->GetFilePathName();
            std::filesystem::path path = std::filesystem::path(retPath);
            if (pMaterialOverride) {
                loadMaterial(path.string());
            }
		}
		ImGuiFileDialog::Instance()->Close();
	}
}

void MaterialPanel::drawProperty(std::string_view propertyName, std::shared_ptr<Texture> pTexture, Material* pMaterial)
{
	ImGui::PushID(propertyName.data() + pTexture->id);

	if (ImGui::TreeNode(propertyName.data()))
	{
		ImGui::Text("Texture: ");
		ImGui::SameLine();
		ImGui::ImageButton("##TextureMatEditBtn", (ImTextureID)pTexture->id, ImVec2(32, 32));

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_PATH"))
			{
				if (payload->DataSize > 0)
				{
					std::string path = std::string(static_cast<const char*>(payload->Data), payload->DataSize - 1);

					std::shared_ptr<Texture> texture = ResourceManager::GetInstance().LoadTextureFromFile(path);
					pMaterial->SetTexture(texture, propertyName);
					if (p_MeshInstance->MaterialOverride)
					{
						if (pMaterial->UUID == p_MeshInstance->MaterialOverride->UUID)
						{
                            m_ResourceModified = true;
						}
					}
                    
				}
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::TreePop();
	}

	

	ImGui::PopID();
}

void MaterialPanel::drawEditProperties(Material* pMaterial)
{
	ImGui::PushID(pMaterial->UUID.c_str());

	std::string type = pMaterial->GetMaterialTypeName();

	ImGui::Text(("Type: " + type).c_str());

	ImGui::Separator();

	//diffuse
	drawProperty("Diffuse", pMaterial->diffuse, pMaterial);

	//normal
	drawProperty("Normal", pMaterial->normal, pMaterial);

	//metallic
	drawProperty("Metallic", pMaterial->metallic, pMaterial);

	//roughness
	drawProperty("Roughness", pMaterial->roughness, pMaterial);

	//ao
	drawProperty("Ambient Occlusion", pMaterial->ambientOcclusion, pMaterial);

	//height
	drawProperty("Height", pMaterial->height, pMaterial);

	//specular
	drawProperty("Specular", pMaterial->specular, pMaterial);

	//emissive
	drawProperty("Emissive", pMaterial->emissive, pMaterial);

	ImGui::Separator();
	ImGui::Text("Triplanar: ");
	ImGui::SameLine();
	ImGui::Checkbox("##TriplanarCheckbox", &pMaterial->Triplanar);

	if (pMaterial->Triplanar)
	{
		ImGui::Text("Tiling scale: ");
		ImGui::SameLine();
		ImGui::PushItemWidth(-1);
		ImGui::DragFloat("##TriplanarScale", &pMaterial->TriplanarTilingScale, 0.1f);
		ImGui::PopItemWidth();
	}

	ImGui::PopID();

}

void MaterialPanel::saveMaterial(const std::string& path, Material* pMaterial)
{
    materialSerializer.Serialize(path, pMaterial);
    EngineDebug::GetInstance().PrintInfo("Material saved to: " + path);
}

void MaterialPanel::loadMaterial(const std::string& path)
{
    std::shared_ptr<Material> oldMaterial = p_MeshInstance->MaterialOverride;
    p_MeshInstance->MaterialOverride = std::make_shared<Material>();
    if (materialSerializer.Deserialize(path, p_MeshInstance->MaterialOverride.get())) {
        EngineDebug::GetInstance().PrintInfo("Material loaded from: " + path);
    } else {
        EngineDebug::GetInstance().PrintError("Failed to load material from: " + path);
        p_MeshInstance->MaterialOverride = oldMaterial;
    }

	if (p_MeshInstance)
	{
		m_ResourceModified = false;
        m_ResourceName = std::filesystem::path(path).filename().string();
	}
}


void MaterialPanel::Open()
{
	m_Visible = true;
}

void MaterialPanel::Close()
{
	m_Visible = false;
}

void MaterialPanel::SetMaterial(Material* pMaterial)
{
	p_Material = pMaterial;
}

void MaterialPanel::SetMeshInstance(MeshInstance* pMesh)
{
	p_MeshInstance = pMesh;
}
