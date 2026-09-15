#include "BuraqFileIconProvider.h"
#include "extensions/ExtensionManager.h"

QIcon BuraqFileIconProvider::icon(const QFileInfo& info) const
{
    if (info.isFile())
    {
        const QIcon customIcon = ExtensionManager::instance().getIconForFile(info.filePath());
        if (!customIcon.isNull())
        {
            return customIcon;
        }
    }

    return QFileIconProvider::icon(info);
}

QIcon BuraqFileIconProvider::icon(const IconType type) const
{
    return QFileIconProvider::icon(type);
}
