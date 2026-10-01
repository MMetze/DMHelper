#ifndef DMHTHEME_H
#define DMHTHEME_H

#include <QString>
#include <QStringList>

class QWidget;

class DMHTheme
{
public:
    enum ThemeType
    {
        ThemeType_Classic = 0,
        ThemeType_Light,
        ThemeType_Dark,
        ThemeType_System,

        ThemeType_Count
    };

    // Applies style, palette and application stylesheet. Safe to call repeatedly to switch themes live.
    static void apply(ThemeType themeType);
    static ThemeType currentTheme();
    static bool isClassic();

    // Files found here override the built-in theme files; current built-in copies are kept in <directory>/defaults.
    static void setUserThemeDirectory(const QString& directory);
    static QString userThemeDirectory();
    static QStringList themeFileNames();

    // Scroll-area viewports ignore stylesheet background images, so Classic parchment is applied via QPalette::Base.
    static void setParchmentBase(QWidget* widget);

    static QString themeName(ThemeType themeType);
    static ThemeType themeFromName(const QString& name);
};

#endif // DMHTHEME_H
