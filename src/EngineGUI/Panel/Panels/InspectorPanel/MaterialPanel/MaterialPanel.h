#ifndef MATERIAL_PANEL_H
#define MATERIAL_PANEL_H

#include "EngineGUI/Panel/Panel.h"
#include <Material/Material.h>
#include "Components/MeshInstance/MeshInstance.h"
#include <Serializers/MaterialSerializer.h>

class MaterialPanel : public Panel
{

	bool m_Visible = false;
    Serialize::MaterialSerializer &materialSerializer = Serialize::MaterialSerializer::GetInstance();

	Material* p_Material;
	MeshInstance* p_MeshInstance;

	void drawProperty(std::string_view propertyName, std::shared_ptr<Texture> pTexture, Material* pMaterial);
	void drawEditProperties(Material* pMaterial);

	void saveMaterial(const std::string& path, Material* pMaterial);
    void loadMaterial(const std::string &path);

	bool m_ResourceModified = false;

	std::string m_ResourceName;

	public:
		
		MaterialPanel();

		void Draw() override;

		void Open();
		void Close();

		void SetMaterial(Material* pMaterial);
		void SetMeshInstance(MeshInstance* pMesh);

};

#endif // !MATERIAL_PANEL_H
