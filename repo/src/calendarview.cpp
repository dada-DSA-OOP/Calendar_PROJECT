#include "calendarview.h"
#include "eventitem.h"

#include <QPainter>
#include <QDateTime> // <-- THAY ĐỔI: Dùng QDateTime
#include <algorithm>
#include <QSet>
#include <QTimer>

CalendarView::CalendarView(QWidget *parent)
    : QGraphicsView(parent), m_days(7), m_hourHeight(60.0)
{
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);

    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::NoDrag);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    // Không cần connect viewResized tới performInitialLayout nữa
    updateSceneRect();

    // --- THÊM ĐOẠN CODE NÀY VÀO CUỐI HÀM KHỞI TẠO ---
    m_timer = new QTimer(this);
    // Kết nối tín hiệu timeout của timer với slot update của view
    // để vẽ lại giao diện mỗi khi timer kêu
    connect(m_timer, &QTimer::timeout, this, QOverload<>::of(&CalendarView::update));
    m_timer->start(60000); // Cập nhật mỗi 60000ms = 1 phút
}

double CalendarView::getDayWidth() const
{
    if (m_days == 0) return 0;
    return sceneRect().width() / m_days;
}

// ... hàm drawBackground, resizeEvent, updateSceneRect không đổi ...
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
    emit viewResized();
    // Cập nhật lại layout khi resize
    updateViewForDateRange(m_currentMonday);
}

void CalendarView::updateSceneRect()
{
    qreal viewWidth = viewport()->width();
    m_scene->setSceneRect(0, 0, viewWidth, 24 * m_hourHeight);
}


// THAY ĐỔI: Sửa lại hàm addEvent
void CalendarView::addEvent(const QString &title, const QColor &color, const QDateTime &startTime, const QDateTime &endTime)
{
    EventItem *item = new EventItem(title, color, startTime, endTime);

    // Khi event thay đổi (kéo/thả/resize), chúng ta cần relayout lại ngày cũ và ngày mới của nó
    connect(item, &EventItem::eventChanged, this, [this](EventItem *changedItem){
        // Logic này có thể được làm phức tạp hơn để chỉ relayout những ngày cần thiết
        // Nhưng để đơn giản, chúng ta sẽ relayout toàn bộ tuần
        updateViewForDateRange(m_currentMonday);
    });

    m_scene->addItem(item);
    item->hide(); // Mặc định ẩn đi, chỉ hiện khi đúng tuần
}

// THAY ĐỔI: Logic sắp xếp lại các sự kiện trong một ngày cụ thể
void CalendarView::relayoutEventsForDate(const QDate &date)
{
    double dayWidth = getDayWidth();

    QList<EventItem*> dayEvents;
    for (QGraphicsItem *item : m_scene->items()) {
        if (auto eventItem = qgraphicsitem_cast<EventItem*>(item)) {
            // Chỉ lấy các event của ngày đang xét và đang hiển thị
            if (eventItem->isVisible() && eventItem->startTime().date() == date) {
                dayEvents.append(eventItem);
            }
        }
    }
    if (dayEvents.isEmpty()) return;

    // Sắp xếp các sự kiện theo thời gian bắt đầu
    std::sort(dayEvents.begin(), dayEvents.end(), [](EventItem* a, EventItem* b) {
        if (a->startTime() != b->startTime())
            return a->startTime() < b->startTime();
        return a->endTime() < b->endTime();
    });

    // Thuật toán chia cột (giữ nguyên)
    QList<QList<EventItem*>> columns;
    columns.append(QList<EventItem*>());

    for (EventItem *event : dayEvents) {
        bool placed = false;
        for (int i = 0; i < columns.size(); ++i) {
            if (columns[i].isEmpty() || event->startTime() >= columns[i].last()->endTime()) {
                columns[i].append(event);
                placed = true;
                break;
            }
        }
        if (!placed) {
            columns.append({event});
        }
    }

    // Cập nhật hình dạng cho các sự kiện
    int totalColumns = columns.size();
    for (int i = 0; i < columns.size(); ++i) {
        for (EventItem *event : columns[i]) {
            event->updateGeometry(dayWidth, m_hourHeight, i, totalColumns);
        }
    }
}

void CalendarView::performInitialLayout()
{
    // Hàm này sẽ được thay thế bởi updateViewForDateRange
    updateViewForDateRange(m_currentMonday);
}

// THAY ĐỔI: Hàm quan trọng nhất, ẩn/hiện và sắp xếp sự kiện cho tuần mới
void CalendarView::updateViewForDateRange(const QDate &monday)
{
    m_currentMonday = monday;
    if (!m_currentMonday.isValid()) return;

    QDate sunday = m_currentMonday.addDays(6);

    // 1. Lặp qua tất cả các item, ẩn/hiện tùy theo ngày
    for (QGraphicsItem *item : m_scene->items()) {
        if (auto eventItem = qgraphicsitem_cast<EventItem*>(item)) {
            QDate eventDate = eventItem->startTime().date();
            if (eventDate >= m_currentMonday && eventDate <= sunday) {
                eventItem->show();
            } else {
                eventItem->hide();
            }
        }
    }

    // 2. Sắp xếp lại layout cho từng ngày trong tuần
    for (int i = 0; i < 7; ++i) {
        relayoutEventsForDate(m_currentMonday.addDays(i));
    }
}

void CalendarView::drawForeground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawForeground(painter, rect);

    QDateTime now = QDateTime::currentDateTime();
    QDate today = now.date();

    if (today < m_currentMonday || today > m_currentMonday.addDays(6)) {
        return;
    }

    double dayWidth = getDayWidth();
    if (dayWidth <= 0) return;

    double y = (now.time().hour() * 60 + now.time().minute()) / 60.0 * m_hourHeight;

    int todayIndex = today.dayOfWeek() - 1;
    double x_today_start = todayIndex * dayWidth;
    double x_today_end = x_today_start + dayWidth;

    // Vẽ đường chấm màu xanh
    QPen dottedPen(QColor("#0078d7"), 1, Qt::DotLine);
    painter->setPen(dottedPen);

    if (x_today_start > 0) {
        painter->drawLine(QPointF(0, y), QPointF(x_today_start, y));
    }
    if (x_today_end < sceneRect().width()) {
        painter->drawLine(QPointF(x_today_end, y), QPointF(sceneRect().width(), y));
    }

    // Vẽ đường đậm của ngày hôm nay
    QPen solidPen(QColor("#0078d7"), 2, Qt::SolidLine);
    painter->setPen(solidPen);
    painter->drawLine(QPointF(x_today_start, y), QPointF(x_today_end, y));

    // --- THÊM MỚI: Vẽ hình tròn ở đầu đường kẻ ---
    painter->setBrush(QColor("#0078d7")); // Tô màu xanh cho hình tròn
    painter->setPen(Qt::NoPen);          // Không cần viền
    // Vẽ hình tròn với đường kính 12px, căn giữa vào đầu đường kẻ
    painter->drawEllipse(QPointF(x_today_start, y), 6, 6);

    painter->setBrush(QColor("#FFFFFF")); // Tô màu xanh cho hình tròn
    painter->setPen(Qt::NoPen);          // Không cần viền
    // Vẽ hình tròn với đường kính 12px, căn giữa vào đầu đường kẻ
    painter->drawEllipse(QPointF(x_today_start, y), 4, 4);
}
