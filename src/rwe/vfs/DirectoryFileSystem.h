#ifndef RWE_DIRECTORYFILESYSTEM_H
#define RWE_DIRECTORYFILESYSTEM_H

#include <rwe/vfs/AbstractVirtualFileSystem.h>

#include <boost/filesystem.hpp>

namespace rwe
{
    class DirectoryFileSystem final : public AbstractVirtualFileSystem
    {
    public:
        explicit DirectoryFileSystem(const std::string& path);

        boost::optional<std::vector<char>> readFile(const std::string& filename) const override;

        std::vector<std::string> getFileNames(const std::string& directory, const std::string& filter) override;

    private:
        boost::filesystem::path path;
    };
}

#endif
