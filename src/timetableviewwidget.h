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
    Q_OBJECT // <-- MỚI: BẮT BUỘC PHẢI CÓ

public:
    explicit TimetableViewWidget(QWidget *parent = nullptr);

    void addEvent(EventItem *event);
    void updateView(const QDate &date); // 'date' là ngày Thứ 2 của tuần

    // MỚI: Thêm hàm removeEvent
    void removeEvent(EventItem *event);

public slots:
    void setTimezoneOffset(int offsetSeconds);

signals:
    // MỚI: Tín hiệu để báo cho MainWindow
    void eventClicked(EventItem *item);

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

    int m_timezoneOffsetSeconds;
};

#endif // TIMETABLEVIEWWIDGET_H
