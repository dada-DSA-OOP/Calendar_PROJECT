#ifndef DAYHEADER_H
#define DAYHEADER_H

#include <QWidget>
#include <QDate>

class DayHeader : public QWidget
{
    Q_OBJECT
public:
    explicit DayHeader(QWidget *parent = nullptr);

public slots:
    void setScrollOffset(int x);
    void updateDates(const QDate& monday); // <-- Thêm

    void setNumberOfDays(int days);

    void setRightMargin(int margin);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    int m_scrollOffset;
    double m_dayWidth;
    int m_days;
    QDate m_monday; // <-- Thêm
    QDate m_currentMonday;
    int m_rightMargin;
};

#endif // DAYHEADER_H
