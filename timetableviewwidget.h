#ifndef TIMETABLEVIEWWIDGET_H
#define TIMETABLEVIEWWIDGET_H

#include <QWidget>
#include <QDate>
#include <QMap>
#include <QTime>

class QGridLayout;
class EventItem;
class TimetableSlotWidget;
class QLabel;

class TimetableViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TimetableViewWidget(QWidget *parent = nullptr);

    void addEvent(EventItem *event);
    void updateView(const QDate &date); // 'date' là ngày Thứ 2 của tuần

private:
    void clearGrid();

    QGridLayout *m_gridLayout;
    // Lưới 6x10 (T2-T7, 10 tiết)
    TimetableSlotWidget* m_slots[6][10];

    // Header
    QLabel* m_dayHeaders[6]; // T2, T3, ...

    QDate m_currentMonday;
    QMap<QDate, QList<EventItem*>> m_allEvents;

    // Định nghĩa các tiết học
    QList<QTime> m_slotStartTimes;
};

#endif // TIMETABLEVIEWWIDGET_H
