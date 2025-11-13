// MIT License
//
// Copyright (c)  "2025" Talik A. Kasozi
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

//
// Created by talik on 5/12/2025.
//

#include <QFile>
#include <QDir>

#include <Config.h>
#include <error_codes.h>

Config::Config() : mainStyles(new MainStyles),
                   windowConfig(new WindowConfig),
                   appIcons(new AppIcons)
{
}

Config& Config::singleton()
{
    static Config instance; // Created once, thread-safe since C++11
    if (!instance.isSetup)
    {
        Config::loadConfig(&instance);
        instance.isSetup = true;
    }
    return instance;
}

void Config::loadConfig(Config* _this)
{
    QFile config(":/config/main_config");
    if (!config.open(QIODevice::ReadOnly))
    {
        // Add this line to see all available resources at the root.
        exit(ErrorCode::ERROR_FILE_NOT_FOUND);
    }

    QDomDocument configDoc;
    if (!configDoc.setContent(&config))
    {
        config.close();
        qDebug() << "Error parsing XML config.";
        exit(ErrorCode::ERROR_INVALID_FORMAT);
    }

    // Get the root element (configuration)
    QDomElement root = configDoc.documentElement();
    if (root.tagName() != "configuration")
    {
        exit(ErrorCode::ERROR_INVALID_FORMAT);
    }

    QDomNodeList appTitleList = root.elementsByTagName("Title");
    if (appTitleList.count() > 0)
    {
        // Extracts the first element from the appTitleList QDomNodeList and converts it to a QDomElement.
        // This element typically represents the application's title node in the XML configuration.
        QDomElement appTitleElement = appTitleList.at(0).toElement();
        _this->title = QString(appTitleElement.text());
    }
    else
    {
        qDebug() << "Error: Configuration is missing a 'Title'.";
        exit(ErrorCode::ERROR_INVALID_FORMAT);
    }

    QDomNodeList appVersionList = root.elementsByTagName("Version");
    if (appVersionList.count() > 0)
    {
        QDomElement appVersionElement = appVersionList.at(0).toElement();
        _this->version = QString(appVersionElement.text());
    }
    else
    {
        // Error: Configuration is missing a 'Version'.;
        exit(ErrorCode::ERROR_INVALID_FORMAT);
    }

    QDomNodeList appLogoList = root.elementsByTagName("AppLogo");
    if (appLogoList.count() > 0)
    {
        QDomElement appLogoElement = appLogoList.at(0).toElement();
        _this->appLogo = QIcon::fromTheme(appLogoElement.text());
    }
    else
    {
        qDebug() << "Error: Configuration is missing a 'AppLogo'.";
        exit(ErrorCode::ERROR_INVALID_FORMAT);
    }

    QDomNodeList windowList = root.elementsByTagName("window");
    if (windowList.count() > 0)
    {
        QDomElement windowElement = windowList.at(0).toElement();
        _this->processWindowAttr(windowElement);
    }
    else
    {
        // "Error: Configuration is missing a 'window'.";
        exit(ErrorCode::ERROR_INVALID_FORMAT);
    }

    if (const QDomNodeList appIconsList = root.elementsByTagName("AppIcons"); appIconsList.count() > 0)
    {
        const QDomElement appIconsElement = appIconsList.at(0).toElement();
        _this->processAppIconsAttr(appIconsElement);
    }
    else
    {
        qDebug() << "Error: Configuration is missing a 'AppIcons'.";
        exit(ErrorCode::ERROR_INVALID_FORMAT);
    }

    if (const QDomNodeList mainStylesList = root.elementsByTagName("Styles"); mainStylesList.count() > 0)
    {
        const QDomElement mainStylesElement = mainStylesList.at(0).toElement();
        _this->processStyles(mainStylesElement);
    }
    else
    {
        // "Error: Configuration is missing a 'Styles'.";
        exit(ErrorCode::ERROR_INVALID_FORMAT);
    }

    config.close();
}

void Config::processWindowAttr(const QDomElement& element) const
{
    const auto attrs = element.attributes();
    for (int i = 0; i < attrs.size(); ++i)
    {
        QString attrName = attrs.item(i).nodeName();
        QString attrValue = attrs.item(i).nodeValue();

        if (attrName == "minWidth")
        {
            windowConfig->minWidth = attrValue.toInt();
        }
        else if (attrName == "minHeight")
        {
            windowConfig->minHeight = attrValue.toInt();
        }
        else if (attrName == "normalSize")
        {
            windowConfig->normalSize = attrValue.toInt();
        }
        else if (attrName == "minimizeIcon")
        {
            windowConfig->minimizeIcon = QIcon::fromTheme(attrValue);
        }
        else if (attrName == "maximizeIcon")
        {
            windowConfig->maximizeIcon = QIcon::fromTheme(attrValue);
        }
        else if (attrName == "restoreIcon")
        {
            windowConfig->restoreIcon = QIcon::fromTheme(attrValue);
        }
        else if (attrName == "closeIcon")
        {
            windowConfig->closeIcon = QIcon::fromTheme(attrValue);
        }
    }
}

void Config::processAppIconsAttr(const QDomElement& element) const
{
    auto appIconsNodes = element.childNodes();
    for (int i = 0; i < appIconsNodes.size(); ++i)
    {
        QDomElement qDomElement = appIconsNodes.item(i).toElement();
        QString tagName = qDomElement.tagName();
        const QString val = qDomElement.text();

        if (tagName == "settings")
        {
            appIcons->settingsIcon = QIcon::fromTheme(val);
        }
        if (tagName == "folder")
        {
            appIcons->folderIcon = QIcon::fromTheme(val);
        }
        if (tagName == "terminal")
        {
            appIcons->terminalIcon = QIcon::fromTheme(val);
        }
        if (tagName == "playCode")
        {
            appIcons->playCode = QIcon::fromTheme(val);
        }
        if (tagName == "execute")
        {
            appIcons->executeIcon = QIcon::fromTheme(val);
        }
        if (tagName == "executeSelected")
        {
            appIcons->executeSelectedIcon = QIcon::fromTheme(val);
        }
        if (tagName == "addFile")
        {
            appIcons->addFileIcon = QIcon::fromTheme(val);
        }
    }
}

void Config::processStyles(const QDomElement& element) const
{
    auto styleNodes = element.childNodes();
    QDomElement qDomCommonNode;
    for (int i = 0; i < styleNodes.size(); ++i)
    {
        auto temp = styleNodes.item(i).toElement();
        if (temp.tagName() == "commonStyle")
        {
            processStyleBlock(temp, mainStyles->commonStyle);
            break;
        }
    }

    for (int i = 0; i < styleNodes.size(); ++i)
    {
        auto temp = styleNodes.item(i).toElement();
        if (temp.tagName() != "commonStyle")
        {
            if (temp.tagName() == "controlToolBar")
            {
                processStyleBlock(temp, mainStyles->controlToolBar);
            }
            if (temp.tagName() == "toolBar")
            {
                processStyleBlock(temp, mainStyles->toolBar);
            }
            if (temp.tagName() == "statusToolBar")
            {
                processStyleBlock(temp, mainStyles->statusToolBar);
            }
            if (temp.tagName() == "toolBarHover")
            {
                processStyleBlock(temp, mainStyles->toolBarHover);
            }
        }
    }
}

void Config::processStyleBlock(const QDomElement& element, StyleSheetStruct& aStruct) const
{
    // includes styles
    if (element.hasAttribute("inherits") && element.attributes().item(0).nodeValue() == "commonStyle")
    {
        const StyleSheetStruct commonStyle =  mainStyles->commonStyle;
        aStruct.color = commonStyle.color;
        aStruct.backgroundColor = commonStyle.backgroundColor;
        aStruct.padding = commonStyle.padding;
        aStruct.border = commonStyle.border;
        aStruct.height = commonStyle.height;
    }

    const auto styleNodes = element.childNodes();
    for (int i = 0; i < styleNodes.size(); ++i)
    {
        auto temp = styleNodes.item(i).toElement();
        if (temp.tagName() == "backgroundColor")
        {
            aStruct.backgroundColor = QString("background-color:" + temp.text() + ";");
        }
        if (temp.tagName() == "border")
        {
            aStruct.border = QString("border: " + temp.text() + ";");
        }
        if (temp.tagName() == "borderBottom")
        {
            aStruct.borderBottom = QString("border-bottom:" + temp.text() + ";");
        }
        if (temp.tagName() == "borderTop")
        {
            aStruct.borderBottom = QString("border-top:" + temp.text() + ";");
        }
        if (temp.tagName() == "borderColor")
        {
            aStruct.borderColor = QString("border-color:" + temp.text() + ";");
        }
        if (temp.tagName() == "padding")
        {
            aStruct.padding = QString("padding:" + temp.text() + ";");
        }
        if (temp.tagName() == "height")
        {
            aStruct.height = QString("height:" + temp.text() + ";");
        }

        aStruct.styleSheet = aStruct.backgroundColor + aStruct.padding + aStruct.color + aStruct.border + aStruct.height
            + aStruct.borderColor + aStruct.borderBottom;
    }
}
