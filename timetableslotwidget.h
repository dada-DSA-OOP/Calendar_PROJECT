#ifndef TIMETABLESLOTWIDGET_H
#define TIMETABLESLOTWIDGET_H

#include <QFrame>
#include <QList>

class QVBoxLayout;
class EventItem;

class TimetableSlotWidget : public QFrame
{
    Q_OBJECT

public:
    explicit TimetableSlotWidget(QWidget *parent = nullptr);
    void addEvent(EventItem *event);
    void clearEvents();

private:
    QVBoxLayout *m_eventsLayout;
    QList<EventItem*> m_events;
};

#endif // TIMETABLESLOTWIDGET_H
