#ifndef DAYHEADER_H
#define DAYHEADER_H

#include <QWidget>
#include <QDate>

class DayHeader : public QWidget
{
    Q_OBJECT
public:
    explicit DayHeader(QWidget *parent = nullptr);

    void setDayWidth(double width) { m_dayWidth = width; update(); }

public slots:
    void setScrollOffset(int x);
    void updateDates(const QDate& monday); // <-- Thêm

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_scrollOffset;
    double m_dayWidth;
    int m_days;
    QDate m_monday; // <-- Thêm
};

#endif // DAYHEADER_H
