#include "SkirmishMenuModel.h"

namespace rwe
{
    bool SkirmishMenuModel::SelectedMapInfo::operator==(const SkirmishMenuModel::SelectedMapInfo& rhs) const
    {
        return name == rhs.name
            && description == rhs.description
            && size == rhs.size
            && minimap == rhs.minimap;
    }

    bool SkirmishMenuModel::SelectedMapInfo::operator!=(const SkirmishMenuModel::SelectedMapInfo& rhs) const
    {
        return !(rhs == *this);
    }

    SkirmishMenuModel::SelectedMapInfo::SelectedMapInfo(
        const std::string& name,
        const std::string& description,
        const std::string& size,
        const Sprite& minimap)
        : name(name),
          description(description),
          size(size),
          minimap(minimap)
    {
    }
}
