#ifndef FILE_EXPLORER_H
#define FILE_EXPLORER_H

#include "imgui.h"
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

/// <summary>
/// Class responsible for managing and displaying a file explorer interface.
/// </summary>
class FileExplorer {

	public:
    FileExplorer();
    ~FileExplorer() = default;

    /// <summary>
    /// Draws the file explorer interface.
    /// </summary>
    void Draw();

    /// <summary>
    /// Sets the current path for the file explorer.
    /// </summary>
    /// <param name="path">Path to set (string)</param>
    void SetCurrentPath(const std::string &path);

    /// <summary>
    /// Returns the current path of the file explorer.
    /// </summary>
    /// <returns>Current path (string)</returns>
    std::string GetSelectedFilePath() const;

    /// <summary>
    /// Returns the selected directory path of the file explorer.
    /// </summary>
    /// <returns>Directory path (string)</returns>
    std::string GetSelectedDirectoryPath() const;

    /// <summary>
    /// Returns the singleton instance of the FileExplorer class.
    /// </summary>
    /// <returns>Singleton (FileExplorer)</returns>
    FileExplorer &GetInstance();

    private:

        fs::path currentPath;
        std::vector<fs::directory_entry> entries;

        void refreshEntries();

        void drawDirectoryTree(const fs::path& directory);

        void drawNewFilePopUp();
        void drawDeletePopUp();
        void drawNewFolderPopUp();

        const char* getExtensionIcon(const fs::path& filename);

        char newFileName[256] = "";
        bool showNewFilePopup = false;
        fs::path targetPathForNewFile;

        bool showDeletePopup = false;
        fs::path targetPathForDelete;

        bool showNewFolderPopup = false;
        char newFolderName[256] = {};
        fs::path targetPathForNewFolder;

        void openInExternalExplorer(const fs::path &path);
};

#endif // !FILE_EXPLORER_H
