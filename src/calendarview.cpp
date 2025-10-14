#include "calendarview.h"
#include "eventitem.h"

#include <QPainter>
#include <QTime>
#include <algorithm>
#include <QSet>

CalendarView::CalendarView(QWidget *parent)
    : QGraphicsView(parent), m_days(7), m_hourHeight(60.0), m_initialLayoutDone(false) // <-- Khởi tạo cờ
{
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);

    // Cài đặt cho View
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::NoDrag);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    connect(this, &CalendarView::viewResized, this, &CalendarView::performInitialLayout);
    updateSceneRect();
}

double CalendarView::getDayWidth() const
{
    if (m_days == 0) return 0;
    return sceneRect().width() / m_days;
}

void CalendarView::drawBackground(QPainter *painter, const QRectF &rect)
{
    // Vẽ nền trắng
    QGraphicsView::drawBackground(painter, rect);

    // Giờ đây DayHeader và TimeRuler sẽ đảm nhiệm việc này
    const double dayWidth = sceneRect().width() / m_days;

    // --- Vẽ các đường kẻ ngang (giờ) ---
    painter->setPen(QPen(Qt::lightGray, 1, Qt::SolidLine));
    for (int hour = 1; hour < 24; ++hour) {
        const double y = hour * m_hourHeight;
        painter->drawLine(QPointF(rect.left(), y), QPointF(rect.right(), y));
    }

    // --- Vẽ các đường kẻ dọc (ngày) ---
    painter->setPen(QPen(Qt::gray, 1, Qt::SolidLine));
    for (int day = 1; day < m_days; ++day) {
        const double x = day * dayWidth;
        painter->drawLine(QPointF(x, rect.top()), QPointF(x, rect.bottom()));
    }
}

void CalendarView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    updateSceneRect();
    emit viewResized(); // Chỉ cần phát tín hiệu là đủ
}

// Hàm này cập nhật kích thước của scene để vừa với chiều rộng của view
// và có chiều cao đủ cho 24 giờ.
void CalendarView::updateSceneRect()
{
    // Lấy chiều rộng của viewport trừ đi thanh cuộn
    qreal viewWidth = viewport()->width();
    m_scene->setSceneRect(0, 0, viewWidth, 24 * m_hourHeight);
}

void CalendarView::addEvent(const QString &title, const QColor &color, int day, const QTime &startTime, const QTime &endTime)
{
    if (day < 0 || day >= m_days) return;

    // Tạo EventItem với đầy đủ thông tin
    EventItem *item = new EventItem(title, color, day, startTime, endTime);

    // Kết nối tín hiệu để tự động sắp xếp lại khi event thay đổi
    connect(item, &EventItem::eventChanged, this, &CalendarView::relayoutDay);

    m_scene->addItem(item);
}

void CalendarView::relayoutDay(EventItem *changedItem)
{
    int day = changedItem->day();
    double dayWidth = getDayWidth();

    // 1. Lấy tất cả sự kiện trong ngày và sắp xếp
    QList<EventItem*> dayEvents;
    for (QGraphicsItem *item : m_scene->items()) {
        if (auto eventItem = qgraphicsitem_cast<EventItem*>(item)) {
            if (eventItem->day() == day) {
                dayEvents.append(eventItem);
            }
        }
    }
    if (dayEvents.isEmpty()) return;

    std::sort(dayEvents.begin(), dayEvents.end(), [](EventItem* a, EventItem* b) {
        if (a->startTime() != b->startTime())
            return a->startTime() < b->startTime();
        return a->endTime() < b->endTime();
    });

    // 2. Thuật toán phân chia cột mới
    QList<QList<EventItem*>> columns;
    columns.append(QList<EventItem*>()); // Bắt đầu với ít nhất một cột

    for (EventItem *event : dayEvents) {
        bool placed = false;
        // Tìm cột đầu tiên mà sự kiện có thể được đặt vào
        for (int i = 0; i < columns.size(); ++i) {
            if (columns[i].isEmpty() || event->startTime() >= columns[i].last()->endTime()) {
                columns[i].append(event);
                placed = true;
                break;
            }
        }
        // Nếu không có cột nào phù hợp, tạo một cột mới
        if (!placed) {
            columns.append({event});
        }
    }

    // 3. Cập nhật hình dạng cho tất cả các sự kiện
    int totalColumns = columns.size();
    for (int i = 0; i < columns.size(); ++i) {
        for (EventItem *event : columns[i]) {
            // Cập nhật vị trí và kích thước dựa trên cột được gán
            event->updateGeometry(dayWidth, m_hourHeight, i, totalColumns);
        }
    }
}

void CalendarView::performInitialLayout()
{
    QSet<int> processedDays;
    for (QGraphicsItem *item : m_scene->items()) {
        if (auto eventItem = qgraphicsitem_cast<EventItem*>(item)) {
            if (!processedDays.contains(eventItem->day())) {
                relayoutDay(eventItem);
                processedDays.insert(eventItem->day());
            }
        }
    }
}
