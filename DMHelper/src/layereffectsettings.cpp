#include "layereffectsettings.h"
#include "ui_layereffectsettings.h"
#include "colorpushbutton.h"

LayerEffectSettings::LayerEffectSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LayerEffectSettings),
    _timerId(0)
{
    ui->setupUi(this);

    ui->btnColorizeColor->setColor(Qt::white);
    ui->btnColorizeColor->setRotationVisible(false);

    connect(ui->btnColorizeColor, &ColorPushButton::colorChanged, this, &LayerEffectSettings::effectColorChanged);
    connect(ui->spinWidth, qOverload<int>(&QSpinBox::valueChanged), this, &LayerEffectSettings::setEffectWidth);
    connect(ui->slideWidth, &QSlider::valueChanged, this, &LayerEffectSettings::setEffectWidth);
    connect(ui->spinHeight, qOverload<int>(&QSpinBox::valueChanged), this, &LayerEffectSettings::setEffectHeight);
    connect(ui->slideHeight, &QSlider::valueChanged, this, &LayerEffectSettings::setEffectHeight);
    connect(ui->spinThickness, qOverload<int>(&QSpinBox::valueChanged), this, &LayerEffectSettings::setEffectThickness);
    connect(ui->slideThickness, &QSlider::valueChanged, this, &LayerEffectSettings::setEffectThickness);
    connect(ui->spinDirection, qOverload<int>(&QSpinBox::valueChanged), this, &LayerEffectSettings::setEffectDirection);
    connect(ui->dialDirection, &QDial::valueChanged, this, &LayerEffectSettings::handleDialDirectionChanged);
    connect(ui->spinSpeed, qOverload<int>(&QSpinBox::valueChanged), this, &LayerEffectSettings::setEffectSpeed);
    connect(ui->slideSpeed, &QSlider::valueChanged, this, &LayerEffectSettings::setEffectSpeed);
    connect(ui->spinMorph, qOverload<int>(&QSpinBox::valueChanged), this, &LayerEffectSettings::setEffectMorph);
    connect(ui->slideMorph, &QSlider::valueChanged, this, &LayerEffectSettings::setEffectMorph);

    connect(this, &LayerEffectSettings::effectWidthChanged, this, &LayerEffectSettings::startRedrawTimer);
    connect(this, &LayerEffectSettings::effectHeightChanged, this, &LayerEffectSettings::startRedrawTimer);
    connect(this, &LayerEffectSettings::effectThicknessChanged, this, &LayerEffectSettings::startRedrawTimer);
    connect(this, &LayerEffectSettings::effectDirectionChanged, this, &LayerEffectSettings::startRedrawTimer);
    connect(this, &LayerEffectSettings::effectSpeedChanged, this, &LayerEffectSettings::startRedrawTimer);
    connect(this, &LayerEffectSettings::effectMorphChanged, this, &LayerEffectSettings::startRedrawTimer);
    connect(this, &LayerEffectSettings::effectColorChanged, this, &LayerEffectSettings::startRedrawTimer);
}

LayerEffectSettings::~LayerEffectSettings()
{
    if(_timerId)
    {
        killTimer(_timerId);
        _timerId = 0;
    }

    delete ui;
}

int LayerEffectSettings::effectWidth() const
{
    return ui->spinWidth->value();
}

int LayerEffectSettings::effectHeight() const
{
    return ui->spinHeight->value();
}

int LayerEffectSettings::effectThickness() const
{
    return ui->spinThickness->value();
}

int LayerEffectSettings::effectDirection() const
{
    return ui->spinDirection->value();
}

int LayerEffectSettings::effectSpeed() const
{
    return ui->spinSpeed->value();
}

int LayerEffectSettings::effectMorph() const
{
    return ui->spinMorph->value();
}

QColor LayerEffectSettings::effectColor() const
{
    return ui->btnColorizeColor->getColor();
}

QImage LayerEffectSettings::createNoiseImage(const QSize& imageSize, qreal width, qreal height, const QColor& color, qreal thickness)
{
    QImage image(imageSize, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    QColor editColor = color;
    qreal predivWidth = width / (static_cast<qreal>(imageSize.width()) * 20.f);
    qreal predivHeight = height / (static_cast<qreal>(imageSize.height()) * 20.f);

    for(int y = 0; y < image.height(); ++y)
    {
        QRgb *line = reinterpret_cast<QRgb*>(image.scanLine(y));
        for(int x = 0; x < image.width(); ++x)
        {
            float value = noise(static_cast<qreal>(x) * predivWidth, static_cast<qreal>(y) * predivHeight, 0);
            value = value * (1.0 - thickness) + thickness;
            if((value >= 0.0) && (value <= 1.0))
            {
                editColor.setAlphaF(value);
                line[x] = editColor.rgba();
            }
        }
    }

    return image;
}

/**
 * 3D Perlin simplex noise
 *
 * @param[in] x float coordinate
 * @param[in] y float coordinate
 * @param[in] z float coordinate
 *
 * @return Noise value in the range[-1; 1], value of 0 on all integer coordinates.
 */
float LayerEffectSettings::noise(float x, float y, float z)
{
    float n0, n1, n2, n3; // Noise contributions from the four corners

    // Skewing/Unskewing factors for 3D
    static const float F3 = 1.0f / 3.0f;
    static const float G3 = 1.0f / 6.0f;

    // Skew the input space to determine which simplex cell we're in
    float s = (x + y + z) * F3; // Very nice and simple skew factor for 3D
    int i = fastfloor(x + s);
    int j = fastfloor(y + s);
    int k = fastfloor(z + s);
    float t = (i + j + k) * G3;
    float X0 = i - t; // Unskew the cell origin back to (x,y,z) space
    float Y0 = j - t;
    float Z0 = k - t;
    float x0 = x - X0; // The x,y,z distances from the cell origin
    float y0 = y - Y0;
    float z0 = z - Z0;

    // For the 3D case, the simplex shape is a slightly irregular tetrahedron.
    // Determine which simplex we are in.
    int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
    int i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords
    if (x0 >= y0) {
        if (y0 >= z0) {
            i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 1; k2 = 0; // X Y Z order
        } else if (x0 >= z0) {
            i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 0; k2 = 1; // X Z Y order
        } else {
            i1 = 0; j1 = 0; k1 = 1; i2 = 1; j2 = 0; k2 = 1; // Z X Y order
        }
    } else { // x0<y0
        if (y0 < z0) {
            i1 = 0; j1 = 0; k1 = 1; i2 = 0; j2 = 1; k2 = 1; // Z Y X order
        } else if (x0 < z0) {
            i1 = 0; j1 = 1; k1 = 0; i2 = 0; j2 = 1; k2 = 1; // Y Z X order
        } else {
            i1 = 0; j1 = 1; k1 = 0; i2 = 1; j2 = 1; k2 = 0; // Y X Z order
        }
    }

    // A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
    // a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
    // a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
    // c = 1/6.
    float x1 = x0 - i1 + G3; // Offsets for second corner in (x,y,z) coords
    float y1 = y0 - j1 + G3;
    float z1 = z0 - k1 + G3;
    float x2 = x0 - i2 + 2.0f * G3; // Offsets for third corner in (x,y,z) coords
    float y2 = y0 - j2 + 2.0f * G3;
    float z2 = z0 - k2 + 2.0f * G3;
    float x3 = x0 - 1.0f + 3.0f * G3; // Offsets for last corner in (x,y,z) coords
    float y3 = y0 - 1.0f + 3.0f * G3;
    float z3 = z0 - 1.0f + 3.0f * G3;

    // Work out the hashed gradient indices of the four simplex corners
    int gi0 = hash(i + hash(j + hash(k)));
    int gi1 = hash(i + i1 + hash(j + j1 + hash(k + k1)));
    int gi2 = hash(i + i2 + hash(j + j2 + hash(k + k2)));
    int gi3 = hash(i + 1 + hash(j + 1 + hash(k + 1)));

    // Calculate the contribution from the four corners
    float t0 = 0.6f - x0*x0 - y0*y0 - z0*z0;
    if (t0 < 0) {
        n0 = 0.0;
    } else {
        t0 *= t0;
        n0 = t0 * t0 * grad(gi0, x0, y0, z0);
    }
    float t1 = 0.6f - x1*x1 - y1*y1 - z1*z1;
    if (t1 < 0) {
        n1 = 0.0;
    } else {
        t1 *= t1;
        n1 = t1 * t1 * grad(gi1, x1, y1, z1);
    }
    float t2 = 0.6f - x2*x2 - y2*y2 - z2*z2;
    if (t2 < 0) {
        n2 = 0.0;
    } else {
        t2 *= t2;
        n2 = t2 * t2 * grad(gi2, x2, y2, z2);
    }
    float t3 = 0.6f - x3*x3 - y3*y3 - z3*z3;
    if (t3 < 0) {
        n3 = 0.0;
    } else {
        t3 *= t3;
        n3 = t3 * t3 * grad(gi3, x3, y3, z3);
    }
    // Add contributions from each corner to get the final noise value.
    // The result is scaled to stay just inside [-1,1]
    return 32.0f*(n0 + n1 + n2 + n3);
}

void LayerEffectSettings::setEffectWidth(int width)
{
    if((width < 1) || (width > 1000))
        return;

    bool changed = false;

    if(ui->slideWidth->value() != width)
    {
        ui->slideWidth->setValue(width);
        changed = true;
    }

    if(ui->spinWidth->value() != width)
    {
        ui->spinWidth->setValue(width);
        changed = true;
    }

    if(changed)
        emit effectWidthChanged(width);
}

void LayerEffectSettings::setEffectHeight(int height)
{
    if((height < 1) || (height > 1000))
        return;

    bool changed = false;

    if(ui->slideHeight->value() != height)
    {
        ui->slideHeight->setValue(height);
        changed = true;
    }

    if(ui->spinHeight->value() != height)
    {
        ui->spinHeight->setValue(height);
        changed = true;
    }

    if(changed)
        emit effectHeightChanged(height);
}

void LayerEffectSettings::setEffectThickness(int thickness)
{
    if((thickness < 1) || (thickness > 100))
        return;

    bool changed = false;

    if(ui->slideThickness->value() != thickness)
    {
        ui->slideThickness->setValue(thickness);
        changed = true;
    }

    if(ui->spinThickness->value() != thickness)
    {
        ui->spinThickness->setValue(thickness);
        changed = true;
    }

    if(changed)
        emit effectThicknessChanged(thickness);
}

void LayerEffectSettings::setEffectDirection(int direction)
{
    direction = clampTo360(direction);
    if((direction < 0) || (direction > 360))
        return;

    bool changed = false;

    int dialValue = clampTo360(direction - 180);
    if(ui->dialDirection->value() != dialValue)
    {
        ui->dialDirection->setValue(dialValue);
        changed = true;
    }

    if(ui->spinDirection->value() != direction)
    {
        ui->spinDirection->setValue(direction);
        changed = true;
    }

    if(changed)
        emit effectDirectionChanged(direction);
}

void LayerEffectSettings::setEffectSpeed(int speed)
{
    if((speed < 0) || (speed > 100))
        return;

    bool changed = false;

    if(ui->slideSpeed->value() != speed)
    {
        ui->slideSpeed->setValue(speed);
        changed = true;
    }

    if(ui->spinSpeed->value() != speed)
    {
        ui->spinSpeed->setValue(speed);
        changed = true;
    }

    if(changed)
        emit effectSpeedChanged(speed);
}

void LayerEffectSettings::setEffectMorph(int morph)
{
    if((morph < 0) || (morph > 500))
        return;

    bool changed = false;

    if(ui->slideMorph->value() != morph)
    {
        ui->slideMorph->setValue(morph);
        changed = true;
    }

    if(ui->spinMorph->value() != morph)
    {
        ui->spinMorph->setValue(morph);
        changed = true;
    }

    if(changed)
        emit effectMorphChanged(morph);
}

void LayerEffectSettings::setEffectColor(const QColor& color)
{
    ui->btnColorizeColor->setColor(color);
}

void LayerEffectSettings::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    if(!_timerId)
        return;

    killTimer(_timerId);
    _timerId = 0;

    int inverseAverage = 255 - qGray(ui->btnColorizeColor->getColor().rgb());
    QString style = "background-color: rgb(" + QString::number(inverseAverage) + "," + QString::number(inverseAverage) + "," + QString::number(inverseAverage) + ");";
    ui->lblPreview->setStyleSheet(style);

    ui->lblPreview->setPixmap(QPixmap::fromImage(createNoiseImage(QSize(512, 512),
                                                                  static_cast<qreal>(ui->spinWidth->value()) / 10.f,
                                                                  static_cast<qreal>(ui->spinHeight->value()) / 10.f,
                                                                  ui->btnColorizeColor->getColor(),
                                                                  static_cast<qreal>(ui->spinThickness->value()) / 100.f)));
}

void LayerEffectSettings::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    startRedrawTimer();
}

void LayerEffectSettings::handleDialDirectionChanged(int direction)
{
    setEffectDirection(clampTo360(direction + 180));
}

void LayerEffectSettings::startRedrawTimer()
{
    if(_timerId == 0)
        _timerId = startTimer(100);
}

int LayerEffectSettings::clampTo360(int value)
{
    if(value < 0)
        return value + 360;
    else if(value > 360)
        return value - 360;
    else
        return value;
}

/**
 * @file    SimplexNoise.cpp
 * @brief   A Perlin Simplex Noise C++ Implementation (1D, 2D, 3D).
 *
 * Copyright (c) 2014-2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)
 *
 * This C++ implementation is based on the speed-improved Java version 2012-03-09
 * by Stefan Gustavson (original Java source code in the public domain).
 * http://webstaff.itn.liu.se/~stegu/simplexnoise/SimplexNoise.java:
 * - Based on example code by Stefan Gustavson (stegu@itn.liu.se).
 * - Optimisations by Peter Eastman (peastman@drizzle.stanford.edu).
 * - Better rank ordering method by Stefan Gustavson in 2012.
 *
 * This implementation is "Simplex Noise" as presented by
 * Ken Perlin at a relatively obscure and not often cited course
 * session "Real-Time Shading" at Siggraph 2001 (before real
 * time shading actually took on), under the title "hardware noise".
 * The 3D function is numerically equivalent to his Java reference
 * code available in the PDF course notes, although I re-implemented
 * it from scratch to get more readable code. The 1D, 2D and 4D cases
 * were implemented from scratch by me from Ken Perlin's text.
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */

/**
 * Computes the largest integer value not greater than the float one
 *
 * This method is faster than using (int32_t)std::floor(fp).
 *
 * I measured it to be approximately twice as fast:
 *  float:  ~18.4ns instead of ~39.6ns on an AMD APU),
 *  double: ~20.6ns instead of ~36.6ns on an AMD APU),
 * Reference: http://www.codeproject.com/Tips/700780/Fast-floor-ceiling-functions
 *
 * @param[in] fp    float input value
 *
 * @return largest integer value not greater than fp
 */
int32_t LayerEffectSettings::fastfloor(float fp)
{
    int32_t i = static_cast<int32_t>(fp);
    return (fp < i) ? (i - 1) : (i);
}

/**
 * Permutation table. This is just a random jumble of all numbers 0-255.
 *
 * This produce a repeatable pattern of 256, but Ken Perlin stated
 * that it is not a problem for graphic texture as the noise features disappear
 * at a distance far enough to be able to see a repeatable pattern of 256.
 *
 * This needs to be exactly the same for all instances on all platforms,
 * so it's easiest to just keep it as static explicit data.
 * This also removes the need for any initialisation of this class.
 *
 * Note that making this an uint32_t[] instead of a uint8_t[] might make the
 * code run faster on platforms with a high penalty for unaligned single
 * byte addressing. Intel x86 is generally single-byte-friendly, but
 * some other CPUs are faster with 4-aligned reads.
 * However, a char[] is smaller, which avoids cache trashing, and that
 * is probably the most important aspect on most architectures.
 * This array is accessed a *lot* by the noise functions.
 * A vector-valued noise over 3D accesses it 96 times, and a
 * float-valued 4D noise 64 times. We want this to fit in the cache!
 */
static const uint8_t perm[256] = {
    151, 160, 137, 91, 90, 15,
    131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
    190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
    88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
    77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
    102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
    135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
    5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
    223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
    129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
    251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
    49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
    138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
};

/**
 * Helper function to hash an integer using the above permutation table
 *
 *  This inline function costs around 1ns, and is called N+1 times for a noise of N dimension.
 *
 *  Using a real hash function would be better to improve the "repeatability of 256" of the above permutation table,
 * but fast integer Hash functions uses more time and have bad random properties.
 *
 * @param[in] i Integer value to hash
 *
 * @return 8-bits hashed value
 */
uint8_t LayerEffectSettings::hash(int32_t i)
{
    return perm[static_cast<uint8_t>(i)];
}

/**
 * Helper functions to compute gradients-dot-residual vectors (3D)
 *
 * @param[in] hash  hash value
 * @param[in] x     x coord of the distance to the corner
 * @param[in] y     y coord of the distance to the corner
 * @param[in] z     z coord of the distance to the corner
 *
 * @return gradient value
 */
float LayerEffectSettings::grad(int32_t hash, float x, float y, float z)
{
    int h = hash & 15;     // Convert low 4 bits of hash code into 12 simple
    float u = h < 8 ? x : y; // gradient directions, and compute dot product.
    float v = h < 4 ? y : h == 12 || h == 14 ? x : z; // Fix repeats at h = 12 to 15
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

