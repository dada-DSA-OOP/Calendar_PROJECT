#ifndef SESSIONVIEWWIDGET_H
#define SESSIONVIEWWIDGET_H

#include <QWidget>
#include <QDate>
#include <QMap>
#include <QTime>

class QGridLayout;
class EventItem;
class TimetableSlotWidget; // Chúng ta có thể tái sử dụng widget ô cũ
class QLabel;

class SessionViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SessionViewWidget(QWidget *parent = nullptr);

    void addEvent(EventItem *event);
    void updateView(const QDate &date); // 'date' là ngày Thứ 2 của tuần

private:
    void clearGrid();

    QGridLayout *m_gridLayout;
    // Lưới 6x2 (T2-T7, Sáng/Chiều)
    TimetableSlotWidget* m_sessionSlots[6][2];

    QLabel* m_dayHeaders[6]; // T2, T3, ...

    QDate m_currentMonday;
    QMap<QDate, QList<EventItem*>> m_allEvents;
};

#endif // SESSIONVIEWWIDGET_H
