#include <QtCore/QTimer>

#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>

#include "hled.h"

class HLed::Private
{
public:
    Private()
        : darkerFactor(300), color(Qt::green),
        isOn(true), offPixmap(0), onPixmap(0)
    {
        offColor = color.darker(darkerFactor);
        timer = new QTimer;
        timer->setInterval(250);
    }

    ~Private()
    {
        delete offPixmap;
        delete onPixmap;
        delete timer;
    }

    int darkerFactor;
    QColor color;
    bool isOn;

    QColor offColor;
    QPixmap *offPixmap;
    QPixmap *onPixmap;

    HLed::State state;
    QTimer * timer;
};

HLed::HLed(QWidget *parent)
    :QWidget(parent), m_d(new Private)
{
    setColor(Qt::red);
    setState(Blink);
    connect(m_d->timer, SIGNAL(timeout()), this, SLOT(blinkToggle()));
}

HLed::~HLed()
{
    delete m_d;
}

QColor HLed::color() const
{
    return m_d->color;
}

HLed::State HLed::state() const
{
    return m_d->state;
}

int HLed::darkerFactor() const
{
    return m_d->darkerFactor;
}

void HLed::setColor(const QColor &color)
{
    if (m_d->color == color)
        return;

    if (m_d->onPixmap) {
        delete m_d->onPixmap;
        m_d->onPixmap = 0;
    }

    if (m_d->offPixmap) {
        delete m_d->offPixmap;
        m_d->offPixmap = 0;
    }

    m_d->color = color;
    m_d->offColor = color.darker(m_d->darkerFactor);

    update();
}

void HLed::setState(State state)
{
    if (m_d->state == state)
        return;

    if (Blink == state) {
        m_d->timer->start();
    }
    else {
        m_d->timer->stop();
        m_d->isOn = (state==On) ? true : false;
    }
    m_d->state = state;
    update();
}

void HLed::setDarkerFactor(int darkerFactor)
{
    if (m_d->darkerFactor == darkerFactor)
        return;

    m_d->darkerFactor = darkerFactor;
    m_d->offColor = m_d->color.darker(darkerFactor);

    update();
}

QSize HLed::sizeHint() const
{
    return QSize(20, 20);
}

QSize HLed::minimumSizeHint() const
{
    return QSize(16, 16);
}

void HLed::toggle()
{
    if (m_d->state == Blink) {
        return;
    }

    setState(m_d->state == On ? Off : On);
    update();
}

void HLed::turnOn(bool on)
{
    setState(on ? On : Off);
}

void HLed::turnOff(bool off)
{
    turnOn(!off);
}

void HLed::blink()
{
    setState(Blink);
}

void HLed::blinkToggle()
{
    m_d->isOn = !m_d->isOn;
    update();
}

void HLed::paintEvent(QPaintEvent* /* event*/)
{
    if (paintCachedPixmap())
    {
        return;
    }

    int width = ledWidth();
    const int scale = 3;
    QPixmap *tmpPixmap = 0;

    width *= scale;
    tmpPixmap = new QPixmap(width + scale*2, width + scale*2);
    tmpPixmap->fill(palette().color(backgroundRole()));
    tmpPixmap->fill(QColor(Qt::transparent));

    QPainter paint;
    paint.begin(tmpPixmap);
    paint.setRenderHint(QPainter::Antialiasing);

    QColor color = m_d->isOn ? m_d->color : m_d->offColor;

    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    brush.setColor(color);

    paint.setBrush(brush);
    // draw plain
    paint.drawEllipse(scale, scale, width-scale*2, width-scale*2);

    QPen pen;
    pen.setWidth(2 * scale);

    int pos = width / 5 + 1;
    int lightWidth = width * 2 / 3;
    int lightQuote = 130 * 2 / (lightWidth ? lightWidth : 1) + 100;

    // draw bright spot
    while (lightWidth) {
        color = color.lighter(lightQuote);
        pen.setColor(color);
        paint.setPen(pen);
        paint.drawEllipse(pos, pos, lightWidth, lightWidth);
        lightWidth--;

        if (!lightWidth) {
            break;
        }

        paint.drawEllipse(pos, pos, lightWidth, lightWidth);
        lightWidth--;

        if (!lightWidth) {
            break;
        }

        paint.drawEllipse(pos, pos, lightWidth, lightWidth);
        pos++;
        lightWidth--;
    }


    pen.setWidth(2*scale + 1);
    brush.setStyle(Qt::NoBrush);
    paint.setBrush(brush);

    int angle = -720;
    color = palette().color(QPalette::Light);

    for (int arc=120; arc<2880; arc+=240) {
        pen.setColor(color);
        paint.setPen(pen);
        int w = width - pen.width()/2 - scale + 1;
        paint.drawArc(pen.width()/2, pen.width()/2, w, w, angle+arc, 240);
        paint.drawArc(pen.width()/2, pen.width()/2, w, w, angle-arc, 240);
        color = color.darker(110);
    }

    paint.end();

    QPixmap *&dest = (m_d->isOn ? m_d->onPixmap : m_d->offPixmap);
    QImage i = tmpPixmap->toImage();
    width /= scale;
    i = i.scaled(width, width, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    delete tmpPixmap;

    dest = new QPixmap(QPixmap::fromImage(i));
    paint.begin(this);
    paint.drawPixmap(0, 0, *dest);
    paint.end();
}

bool HLed::paintCachedPixmap()
{
    if (m_d->isOn && m_d->onPixmap) {
        QPainter paint(this);
        paint.drawPixmap(0, 0, *m_d->onPixmap);
        return true;
    }
    if (!m_d->isOn && m_d->offPixmap) {
        QPainter paint(this);
        paint.drawPixmap(0, 0, *m_d->offPixmap);
        return true;
    }
    return false;
}

int HLed::ledWidth() const
{
    int width = qMin(this->width(), this->height());
    width -= 2;
    return width > 0 ? width : 0;
}

