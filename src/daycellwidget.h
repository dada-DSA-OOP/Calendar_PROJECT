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
    Q_OBJECT // <-- MỚI: BẮT BUỘC PHẢI CÓ

public:
    explicit DayCellWidget(QWidget *parent = nullptr);
    void setDate(const QDate &date, bool isCurrentMonth);
    void addEvent(EventItem *event);
    void clearEvents();

    // MỚI: Hàm để MonthViewWidget gọi khi xóa
    void removeEvent(EventItem *event);

    // MỚI: Hàm để lấy ngày (cần cho removeEvent)
    QDate date() const { return m_date; }

signals:
    void cellClicked(const QDate &date, const QList<EventItem*> &events);

    // MỚI: Tín hiệu khi một sự kiện cụ thể được nhấn
    void eventClicked(EventItem *item);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    // MỚI: Hàm để bắt sự kiện click trên các QLabel con
    bool eventFilter(QObject *watched, QEvent *event) override;

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
