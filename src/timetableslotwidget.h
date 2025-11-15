#ifndef TIMETABLESLOTWIDGET_H
#define TIMETABLESLOTWIDGET_H

#include <QFrame>
#include <QList>

class QVBoxLayout;
class EventItem;

class TimetableSlotWidget : public QFrame
{
    Q_OBJECT // <-- MỚI: BẮT BUỘC PHẢI CÓ

public:
    explicit TimetableSlotWidget(QWidget *parent = nullptr);
    void addEvent(EventItem *event);
    void clearEvents();

signals:
    // MỚI: Tín hiệu để báo cho MainWindow/SessionViewWidget
    void eventClicked(EventItem *item);

protected:
    // MỚI: Hàm để bắt sự kiện click trên các QLabel con
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QVBoxLayout *m_eventsLayout;
    QList<EventItem*> m_events;
};

#endif // TIMETABLESLOTWIDGET_H
