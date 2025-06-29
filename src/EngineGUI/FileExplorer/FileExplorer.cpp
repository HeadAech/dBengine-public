#include "FileExplorer.h"
#include "dBengine/EngineDebug/EngineDebug.h"
#include "dBengine/EngineSettings/EngineSettings.h"
#include "Helpers/fonts/IconsFontAwesome4.h"
#include "Signal/Signals.h"
#include <imgui_internal.h>
#include <EngineGUI/EngineGUI.h>
#include "ResourceManager/ResourceManager.h"

FileExplorer::FileExplorer() { 
    currentPath = fs::path(RES_DIR);
    refreshEntries();
}

void FileExplorer::refreshEntries() {
    entries.clear();

    try {
        for (const auto &entry: fs::directory_iterator(currentPath)) {
            entries.push_back(entry);
        }

        
    } catch (const fs::filesystem_error &err) {
        EngineDebug::GetInstance().PrintError("Error reading directory: " + std::string(err.what()));
    
    }

    std::sort(entries.begin(), entries.end(), [](const fs::directory_entry &a, const fs::directory_entry &b) {
        if (a.is_directory() != b.is_directory()) {
            return a.is_directory(); // directories first
        }
        return a.path().filename().string() < b.path().filename().string(); // alphabetically
    });
}

void FileExplorer::Draw() { 
    ImGui::Begin("File Explorer", 0, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse); 

       ImGui::BeginChild("Header", ImVec2(0, 40), false,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);


    if (ImGui::Button(ICON_FA_LEVEL_UP) && currentPath.has_parent_path()) {
        currentPath = currentPath.parent_path();
        refreshEntries();
    }
    
    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();
    ImGui::TextUnformatted(currentPath.string().c_str());
    ImGui::Separator();
    ImGui::EndChild();

    ImGui::BeginChild("FileList", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    // window context menu
    if (ImGui::BeginPopupContextWindow("window_context", ImGuiPopupFlags_MouseButtonRight)) {
        if (ImGui::MenuItem(ICON_FA_FILE_O " Create File")) {
            showNewFilePopup = true;
            targetPathForNewFile = currentPath;
        }

        if (ImGui::MenuItem(ICON_FA_FOLDER " Create Folder")) {
            showNewFolderPopup = true;
            targetPathForNewFolder = currentPath;
        }

        ImGui::EndPopup();
    }

    for (int i = 0; i < entries.size(); i++) {
        auto entry = entries[i];
        const bool &isDirectory = entry.is_directory();
        const std::string &name = entry.path().filename().string();
        const std::string &fullPath = entry.path().lexically_normal().generic_string();
        const std::string &extension = entry.path().filename().extension().string();

        ImGui::PushID(fullPath.c_str());

        if (isDirectory) {
            if (ImGui::Selectable((ICON_FA_FOLDER "  " + name).c_str(), false)) {
                currentPath = entry;
                refreshEntries();
            }
        } else {
            std::string icon = getExtensionIcon(entry.path().filename());
            
            if (ImGui::Selectable((icon + "  " + name).c_str(), false)) {
                ImGui::OpenPopup("ContextMenu");
            }
            
        }
        // Begin drag-drop source for any entry
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            const std::string payloadPath = entry.path().string();
            ImGui::SetDragDropPayload("DND_PATH", payloadPath.c_str(), payloadPath.size() + 1);
            ImGui::TextUnformatted(payloadPath.c_str());
            ImGui::EndDragDropSource();
        }

        // per item context menu
        if (ImGui::BeginPopupContextItem("ContextMenu")) {

            if (extension == ".lua" || extension == ".vert" || extension == ".frag" || extension == ".vs" ||
                extension == ".fs" || extension == ".geom" || extension == ".txt" || extension == ".yaml") {
                if (ImGui::MenuItem(ICON_FA_PENCIL " Edit")) {
                    Signals::Editor_SetScriptPathToEdit.emit(fullPath);
                }
            }

            if (extension == ".jpeg" || extension == ".jpg" || extension == ".png")
            {
                if (ImGui::MenuItem(ICON_FA_EYE " Preview"))
                {
                    PreviewPanel &previewPanel = EngineGUI::GetInstance().GetPreviewPanel();
                    previewPanel.SetImage(ResourceManager::GetInstance().LoadTextureFromFile(fullPath.c_str())->id);
                    previewPanel.Open();
                }
                if (extension == ".yaml") {
                    if (ImGui::MenuItem(ICON_FA_FILE "Load Scene")) {
                        Signals::FileToScene.emit(fullPath);
                    }
                }
            }

            if (ImGui::MenuItem(ICON_FA_CLIPBOARD " Copy path")) {
                ImGui::SetClipboardText(fullPath.c_str());
            }

            if (ImGui::MenuItem(ICON_FA_EXTERNAL_LINK " Show In System Explorer")) {
                if (isDirectory) {
                    openInExternalExplorer( entry.path());
                } else {
                    openInExternalExplorer(entry.path().parent_path().string());
                }
            }

            ImGui::SeparatorText("File");

            if (isDirectory && ImGui::MenuItem(ICON_FA_FILE_O " Create File In Folder")) {
                targetPathForNewFile = entry.path();
                showNewFilePopup = true;
            }

             if (ImGui::MenuItem(ICON_FA_TRASH " Delete")) {
                showDeletePopup = true;
                targetPathForDelete = entry.path();
            }

            ImGui::EndPopup();
        }

        ImGui::PopID();
    }

    ImGui::EndChild();
    ImGui::End();

    if (showNewFilePopup) {
        ImGui::OpenPopup("Create New File");
        showNewFilePopup = false;
        strncpy(newFileName, "", sizeof(newFileName));
    }

    if (showDeletePopup) {
        ImGui::OpenPopup("Confirm Delete");
        showDeletePopup = false;
    }

    if (showNewFolderPopup) {
        ImGui::OpenPopup("Create New Folder");
        showNewFolderPopup = false;
        strncpy(newFolderName, "", sizeof(newFolderName));
    }

    drawNewFilePopUp();
    drawDeletePopUp();
    drawNewFolderPopUp();
}

void FileExplorer::drawNewFilePopUp() {

    if (ImGui::BeginPopupModal("Create New File", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("File name", newFileName, IM_ARRAYSIZE(newFileName));
        if (ImGui::Button("Create") && strlen(newFileName) > 0) {
            fs::path newFile = targetPathForNewFile / newFileName;
            std::ofstream(newFile.string()); // Create empty file
            refreshEntries(); // update list
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void FileExplorer::drawDeletePopUp() {
    if (ImGui::BeginPopupModal("Confirm Delete", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to delete:\n%s?", targetPathForDelete.string().c_str());
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f)); // red
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.2f, 0.2f, 1.0f)); // brighter red
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.05f, 0.05f, 1.0f)); // darker red

        if (ImGui::Button(ICON_FA_TRASH " Yes, Delete")) {
            try {
                if (fs::is_directory(targetPathForDelete)) {
                    fs::remove_all(targetPathForDelete);
                } else {
                    fs::remove(targetPathForDelete);
                }
                refreshEntries();
            } catch (const std::exception &e) {
                // Optional: display error popup
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(3); // restore colors
        ImGui::SameLine();

        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void FileExplorer::drawNewFolderPopUp() {
    if (ImGui::BeginPopupModal("Create New Folder", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        static std::string validationError;

        ImGui::InputText("Folder name", newFolderName, IM_ARRAYSIZE(newFolderName));

        // Show error if exists
        if (!validationError.empty()) {
            ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "%s", validationError.c_str());
        }

        if (ImGui::Button("Create")) {
            std::string nameStr(newFolderName);

            // Validate folder name
            const std::string illegalChars = "\\/:?\"<>|*.";
            bool isValid = !nameStr.empty() && nameStr.find_first_of(illegalChars) == std::string::npos;

            if (!isValid) {
                validationError = "Invalid folder name! Avoid: \\/:?\"<>|*.";
            } else {
                validationError.clear();
                fs::path newFolder = targetPathForNewFolder / nameStr;
                try {
                    fs::create_directory(newFolder);
                } catch (const std::exception &e) {
                    validationError = std::string("Error: ") + e.what();
                    ImGui::EndPopup();
                    return;
                }
                refreshEntries();
                strncpy(newFolderName, "", sizeof(newFolderName));
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            validationError.clear();
            strncpy(newFolderName, "", sizeof(newFolderName));
            ImGui::CloseCurrentPopup();
            
        }

        ImGui::EndPopup();
    }
}


const char *FileExplorer::getExtensionIcon(const fs::path &filename) {
    fs::path extension = filename.extension();

    if (extension == ".txt")
        return ICON_FA_FILE_TEXT_O;
    if (extension == ".png" || extension == ".jpg" || extension == ".jpeg")
        return ICON_FA_FILE_IMAGE_O;
    if (extension == ".mp3" || extension == ".wav")
        return ICON_FA_FILE_AUDIO_O;
    if (extension == ".lua" || extension == ".cs" || extension == ".cpp" || extension == ".h")
        return ICON_FA_FILE_CODE_O;
    if (extension == ".zip" || extension == ".rar")
        return ICON_FA_FILE_ARCHIVE_O;
    if (extension == ".pdf")
        return ICON_FA_FILE_PDF_O;
    if (extension == ".mp4" || extension == ".avi")
        return ICON_FA_FILE_VIDEO_O;
    if (extension == ".fbx" || extension == ".obj" || extension == ".dae" || extension == ".gltf")
        return ICON_FA_CUBE;
    if (extension == ".scene")
        return ICON_FA_CUBES;
    if (extension == ".res")
        return ICON_FA_DATABASE;
    if (extension == ".json" || extension == ".yaml")
        return ICON_FA_FILE;

    return ICON_FA_FILE_O;
}

#ifdef _WIN32

#include <Windows.h>
#include <shellapi.h>

void FileExplorer::openInExternalExplorer(const fs::path &path) {
    ShellExecuteA(NULL, "explore", path.string().c_str(), NULL, NULL, SW_SHOW);
}

#endif // _WIN32

#if __APPLE__
#include <cstdlib>

void FileExplorer::openInExternalExplorer(const fs::path &path) {
    std::string command = "open -R \"" + path.string() + "\"";
    std::system(command.c_str()); // Execute the open command
}

#endif // __APPLE__

#ifdef __linux__
#include <cstdlib>

void FileExplorer::openInExternalExplorer(const fs::path &path) {
    std::string command = "xdg-open " + path.parent_path().string();
    std::system(command.c_str()); // Open parent directory
}
#endif
