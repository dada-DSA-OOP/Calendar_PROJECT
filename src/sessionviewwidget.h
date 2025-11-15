#ifndef SESSIONVIEWWIDGET_H
#define SESSIONVIEWWIDGET_H

#include <QWidget>
#include <QDate>
#include <QMap>
#include <QTime>

class QGridLayout;
class EventItem;
class TimetableSlotWidget;
class QLabel;

class SessionViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SessionViewWidget(QWidget *parent = nullptr);

    void addEvent(EventItem *event);
    void updateView(const QDate &date);

    // MỚI: Hàm để xóa
    void removeEvent(EventItem *event);

public slots: // <-- THÊM MỚI
    void setTimezoneOffset(int offsetSeconds);

signals:
    // MỚI: Tín hiệu báo cho MainWindow
    void eventClicked(EventItem *item);

private:
    void clearGrid();
    QGridLayout *m_gridLayout;
    TimetableSlotWidget* m_sessionSlots[6][2];
    QLabel* m_dayHeaders[6];
    QDate m_currentMonday;
    QMap<QDate, QList<EventItem*>> m_allEvents;
    int m_timezoneOffsetSeconds;
};

#endif // SESSIONVIEWWIDGET_H
