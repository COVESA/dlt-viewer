#ifndef HLED_H
#define HLED_H

#include <QWidget>

class QColor;
class QTimer;

class HLed : public QWidget
{
    Q_OBJECT
public:
    enum State {Off, On, Blink};
    HLed(QWidget *parent = 0);
    ~HLed();

    QColor color() const;
    State state() const;
    int darkerFactor() const;

    void setColor(const QColor & color);
    void setState(State state);
    void setDarkerFactor(int darkerFactor);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

public slots:
    void toggle();
    void turnOn(bool on=true);
    void turnOff(bool off=true);
    void blink();

private slots:
    void blinkToggle();

protected:
    void paintEvent(QPaintEvent*);
    bool paintCachedPixmap();
    int ledWidth() const;
    void paintSunken();

private:
    class Private;
    Private * const m_d;
};

#endif // HLED_H
