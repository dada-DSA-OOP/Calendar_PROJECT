#ifndef TIMERULER_H
#define TIMERULER_H

#include <QWidget>
#include <QTime>

class TimeRuler : public QWidget
{
    Q_OBJECT
public:
    explicit TimeRuler(QWidget *parent = nullptr);

    void setHourHeight(double height);

public slots:
    void setScrollOffset(int y);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_scrollOffset;
    double m_hourHeight;
};

#endif // TIMERULER_H
