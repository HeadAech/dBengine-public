
#include "dBengine/dBengine.h"

int main(int argc, char** argv)
{
    dBengine engine;

    for (int i = 1; i < argc; ++i)
    {
        std::string a = argv[i];
        if (a == "--editor")
        {
            engine.Settings.EditorEnabled = true;
        }
        else if (a == "--no-editor")
        {
            engine.Settings.EditorEnabled = false;
        }
        else if (a == "--help" || a == "-h")
        {
            std::cout << "Usage: dBengine [--editor] [--no-editor]\n";
            return 0;
        }
        else if (a == "--scene" && i + 1 < argc)
        {
            engine.SceneToLoadOnStart = argv[i + 1];
        }
    }
    return engine.Run();
}
