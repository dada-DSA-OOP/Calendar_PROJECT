#ifndef DAYCELLWIDGET_H
#define DAYCELLWIDGET_H

#include <QFrame>
#include <QDate>
#include <QList>

class QLabel;
class QVBoxLayout;
class EventItem;

class DayCellWidget : public QFrame
{
    Q_OBJECT

public:
    explicit DayCellWidget(QWidget *parent = nullptr);
    void setDate(const QDate &date, bool isCurrentMonth);
    void addEvent(EventItem *event);
    void clearEvents();

signals:
    void cellClicked(const QDate &date, const QList<EventItem*> &events);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

private:
    void updateEventDisplay(const QSize &newSize);

    QLabel *m_dateLabel;
    QVBoxLayout *m_eventsLayout;
    QWidget *m_eventsContainer;

    QLabel *m_compactEventsIndicator; // Dấu chấm "●" khi thu nhỏ

    QDate m_date;
    QList<EventItem*> m_events;

    const int m_compactThreshold = 75; // 75px chiều cao
};

#endif // DAYCELLWIDGET_H
