#include "calendarview.h"
#include "eventitem.h"

#include <QPainter>
#include <QDateTime> // <-- THAY ĐỔI: Dùng QDateTime
#include <algorithm>
#include <QSet>
#include <QTimer>

CalendarView::CalendarView(QWidget *parent)
    : QGraphicsView(parent), m_days(7), m_hourHeight(60.0)
    , m_timezoneOffsetSeconds(QDateTime::currentDateTime().offsetFromUtc())
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
    // Cập nhật lại layout khi resize
    updateViewForDateRange(m_currentMonday);
}

void CalendarView::updateSceneRect()
{
    qreal viewWidth = viewport()->width();
    m_scene->setSceneRect(0, 0, viewWidth, 24 * m_hourHeight);
}


// Chuyển sang nhận EventItem*
void CalendarView::addEvent(EventItem *item)
{
    // Khi event thay đổi (kéo/thả/resize)
    connect(item, &EventItem::eventChanged, this, [this](EventItem *changedItem){
        updateViewForDateRange(m_currentMonday);
    });

    // Kết nối tín hiệu click (đã có)
    connect(item, &EventItem::clicked, this, &CalendarView::eventClicked);

    // --- MỚI: Kết nối tín hiệu kéo thả ---
    connect(item, &EventItem::eventDragged, this, &CalendarView::onInternalEventDragged);
    // ------------------------------------

    m_scene->addItem(item);
    item->hide();
}

// MỚI: Hàm để xóa
void CalendarView::removeEvent(EventItem *item)
{
    if (item) {
        m_scene->removeItem(item);
        // Lưu ý: MainWindow sẽ chịu trách nhiệm 'delete item'
        // Ở đây chỉ xóa khỏi scene
    }
}

// THAY ĐỔI: Logic sắp xếp lại các sự kiện trong một ngày cụ thể
void CalendarView::relayoutEventsForDate(const QDate &date, int dayIndex)
{
    double dayWidth = getDayWidth();

    QList<EventItem*> dayEvents;
    for (QGraphicsItem *item : m_scene->items()) {
        if (auto eventItem = qgraphicsitem_cast<EventItem*>(item)) {

            QDateTime utcStartTime = eventItem->startTime();
            QDate displayDate = utcStartTime.toOffsetFromUtc(m_timezoneOffsetSeconds).date();

            if (!eventItem->isFilteredOut() && displayDate == date) {
                dayEvents.append(eventItem);
            }
        }
    }
    if (dayEvents.isEmpty()) return;

    // Sắp xếp các sự kiện (Vẫn dùng UTC để so sánh tuyệt đối)
    std::sort(dayEvents.begin(), dayEvents.end(), [](EventItem* a, EventItem* b) {
        if (a->startTime() != b->startTime()) // So sánh UTC
            return a->startTime() < b->startTime();
        return a->endTime() < b->endTime(); // So sánh UTC
    });

    // Thuật toán chia cột
    QList<QList<EventItem*>> columns;
    columns.append(QList<EventItem*>());

    for (EventItem *event : dayEvents) {
        bool placed = false;
        for (int i = 0; i < columns.size(); ++i) {
            // So sánh thời gian KẾT THÚC (cũng phải dùng UTC)
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

    // Cập nhật hình dạng (Hàm này phải dùng m_displayTimeSpec)
    int totalColumns = columns.size();
    for (int i = 0; i < columns.size(); ++i) {
        for (EventItem *event : columns[i]) {
            // Yêu cầu: Hàm updateGeometry phải được sửa
            // để nhận m_displayTimeSpec
            // event->updateGeometry(dayWidth, m_hourHeight, dayIndex, i, totalColumns);

            // GIẢ ĐỊNH eventitem.cpp có hàm updateGeometryV2
            // Bạn cần sửa eventitem.cpp để thêm logic này
            event->updateGeometry(dayWidth, m_hourHeight, dayIndex, i, totalColumns, m_timezoneOffsetSeconds);
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

    QDate endDate = m_currentMonday.addDays(m_days - 1);

    // 1. Lặp qua tất cả các item, ẩn/hiện tùy theo ngày
    for (QGraphicsItem *item : m_scene->items()) {
        if (auto eventItem = qgraphicsitem_cast<EventItem*>(item)) {

            // === BẮT ĐẦU SỬA LỖI LOGIC ===

            // Lấy ngày hiển thị (display date)
            QDate eventDate = eventItem->startTime().toOffsetFromUtc(m_timezoneOffsetSeconds).date();

            // Đọc trạng thái BỘ LỌC (từ biến mới)
            bool isFiltered = eventItem->isFilteredOut();

            // Kiểm tra xem nó có trong phạm vi ngày không
            bool isInDateRange = (eventDate >= m_currentMonday && eventDate <= endDate);

            // Chỉ HIỆN khi (KHÔNG bị lọc) VÀ (NẰM TRONG TUẦN)
            if (!isFiltered && isInDateRange) {
                eventItem->show();
            } else {
                // ẨN nếu (BỊ LỌC) HOẶC (NẰM NGOÀI TUẦN)
                eventItem->hide();
            }
            // === KẾT THÚC SỬA LỖI LOGIC ===
        }
    }

    // 2. Sắp xếp lại layout cho từng ngày trong tuần
    // (Phần code này giữ nguyên, nó sẽ tự động bỏ qua các item đã bị hide)
    for (int i = 0; i < m_days; ++i) {
        relayoutEventsForDate(m_currentMonday.addDays(i), i);
    }

    viewport()->update();
}

void CalendarView::drawForeground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawForeground(painter, rect);

    QDateTime now = QDateTime::currentDateTime().toOffsetFromUtc(m_timezoneOffsetSeconds);
    QDate today = now.date();

    // Dùng m_days thay vì 6
    if (today < m_currentMonday || today > m_currentMonday.addDays(m_days - 1)) {
        return;
    }

    double dayWidth = getDayWidth();
    if (dayWidth <= 0) return;

    double y = (now.time().hour() * 60 + now.time().minute()) / 60.0 * m_hourHeight;

    int todayIndex = m_currentMonday.daysTo(today);
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

void CalendarView::setNumberOfDays(int days)
{
    if (days < 1) days = 1; // Đảm bảo luôn có ít nhất 1 ngày
    m_days = days;

    // Cập nhật lại Scene Rect, vì getDayWidth() phụ thuộc vào m_days
    updateSceneRect();
}

void CalendarView::setTimeScale(int minutes)
{
    if (minutes <= 0) minutes = 60;

    // 1. Tính toán chiều cao mới cho 1 giờ
    // (60.0 / minutes) là "số 'khối' trong 1 giờ"
    // 60.0 là "chiều cao pixel của 1 'khối'"
    m_hourHeight = (60.0 / minutes) * 60.0;

    // 2. Cập nhật lại Scene Rect (vì tổng chiều cao đã thay đổi)
    updateSceneRect();

    // 3. Cập nhật lại vị trí/kích thước của tất cả sự kiện
    updateViewForDateRange(m_currentMonday);

    // 4. Báo cho TimeRuler biết nó cần vẽ lại
}

void CalendarView::onInternalEventDragged(EventItem *item, const QDateTime &newStartTime, const QDateTime &newEndTime)
{
    // Chỉ cần phát tín hiệu này lên cho MainWindow xử lý
    emit eventDragged(item, newStartTime, newEndTime);
}

void CalendarView::setTimezoneOffset(int offsetSeconds)
{
    m_timezoneOffsetSeconds = offsetSeconds;
    // Tên hàm cập nhật của view này là updateViewForDateRange
    updateViewForDateRange(m_currentMonday);
}
