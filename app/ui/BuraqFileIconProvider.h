#ifndef BURAQ_FILE_ICON_PROVIDER_H
#define BURAQ_FILE_ICON_PROVIDER_H

#include <QFileIconProvider>
#include <QFileInfo>
#include <QIcon>

class BuraqFileIconProvider : public QFileIconProvider
{
public:
    explicit BuraqFileIconProvider() = default;
    ~BuraqFileIconProvider() override = default;

    [[nodiscard]] QIcon icon(const QFileInfo& info) const override;
    [[nodiscard]] QIcon icon(IconType type) const override;
};

#endif // BURAQ_FILE_ICON_PROVIDER_H
