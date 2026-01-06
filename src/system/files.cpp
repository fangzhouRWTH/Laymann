#include "files.h"

#ifdef PLATFORM_WINDOWS

#elif defined(PLATFORM_LINUX)
    #include <unistd.h>
    #include <limits.h>
#elif defined(PLATFORM_MACOS)

#endif

namespace lmv{
std::string getExePath() {
#ifdef PLATFORM_WINDOWS
    return std::string();
#elif defined(PLATFORM_LINUX)
    char path[PATH_MAX] = {0};
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    return std::string(path, (count > 0) ? count : 0);
#elif defined(PLATFORM_MACOS)
    return std::string();
#else
    return std::string();
#endif
}

std::string getExeFolderPath()
{
    auto path = getExePath();
    auto i = path.rfind("/");
    auto p = path.substr(0,i);
    return p;
}

std::string getShaderPath()
{
    auto path = getExeFolderPath();

    return path + "/shaders";
}
}