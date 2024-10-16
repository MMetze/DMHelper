#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "dmconstants.h"
#include "dmversion.h"
#include "bestiary.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    QString licenseText;

    licenseText += QString("DMHelper is unofficial Fan Content permitted under the Fan Content Policy. Not approved/endorsed by Wizards. Portions of the materials used are property of Wizards of the Coast. ©Wizards of the Coast LLC.\n\n");

    licenseText += QString("All menu icons are derived from tokens provided by Ross from 2-Minute Tabletop, an amazing source of RPG resources: www.2minutetabletop.com under the Creative Commons BY-NC 4.0 (www.creativecommons.org/licenses/by-nc/4.0/)\n\n");

    licenseText += QString("The sample spell tokens included with the spellbook are kindly provided by 2-Minute Tabletop as well!\n\n");

    licenseText += QString("Vectorized dragon logo provided by Mike Rickard from ""I Cast Pod!"", our guide to everything dungeonesque and dragonny (and a great DnD podcast)\n\n");

    licenseText += QString("Quick reference icons provided by www.game-icons.net\n\n");

    licenseText += QString("Simplex Noise GLSL implementation from the LYGIA Shader Library (https://lygia.xyz/generative/snoise), licensed under the Prosperity Liecense here: https://prosperitylicense.com/versions/3.0.0\n\n");
    licenseText += QString("Simplex Noise C++ implementation from Sebastien Rombauts (https://github.com/SRombauts/SimplexNoise) distributed under the MIT License (MIT) (See license file copy at http://opensource.org/licenses/MIT)\n\n");

    licenseText += QString("Quick reference source originally from www.github.com/crobi/dnd5e-quickref provided under the following license:\n");
    licenseText += QString("MIT License\n");
    licenseText += QString("Copyright (c) 2016 Robert Autenrieth\n");
    licenseText += QString("Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the ""Software""), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:\n");
    licenseText += QString("The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.\n");
    licenseText += QString("THE SOFTWARE IS PROVIDED ""AS IS"", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n\n\n");

    licenseText += QString("Content provided in the bestiary, the spellbook and the quick reference tables are from the Wizards of the Coast SRD (Systems Reference Document) published under the OPEN GAME LICENSE Version 1.0a as follows\n\n");
    if(Bestiary::Instance())
        licenseText += Bestiary::Instance()->getLicenseText().join(QString("\n")).append(QString("\n"));

    ui->edtLicenses->setText(licenseText);

    if(DMHelper::DMHELPER_ENGINEERING_VERSION > 0)
        ui->lblVersion->setText(QString::number(DMHelper::DMHELPER_MAJOR_VERSION) + "." + QString::number(DMHelper::DMHELPER_MINOR_VERSION) + "." + QString::number(DMHelper::DMHELPER_ENGINEERING_VERSION) + " (pre-release version)");
    else
        ui->lblVersion->setText(QString::number(DMHelper::DMHELPER_MAJOR_VERSION) + "." + QString::number(DMHelper::DMHELPER_MINOR_VERSION));

    ui->lblBestiaryVersion->setText(Bestiary::getExpectedVersion());
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
