#include "calendarview.h"
#include "eventitem.h"

#include <QPainter>
#include <QDateTime>
#include <algorithm> // Dùng cho std::sort
#include <QSet>
#include <QTimer>     // Dùng để vẽ đường "thời gian hiện tại"
#include <QScrollBar> // Dùng để truy cập thanh cuộn
#include <QResizeEvent> // (Bao gồm đầy đủ, dù .h đã có)

// =================================================================================
// === 1. HÀM DỰNG (CONSTRUCTOR)
// =================================================================================

CalendarView::CalendarView(QWidget *parent)
    : QGraphicsView(parent)
    , m_days(7) // Mặc định 7 ngày
    , m_hourHeight(60.0) // Mặc định 60px/giờ
    , m_timezoneOffsetSeconds(QDateTime::currentDateTime().offsetFromUtc())
{
    // m_scene là "tấm vải" (canvas) ảo.
    // Tất cả các EventItem sẽ được thêm vào scene này.
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);

    setRenderHint(QPainter::Antialiasing); // Vẽ mượt mà
    setDragMode(QGraphicsView::NoDrag); // Không cho phép kéo scene (chỉ kéo item)
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    // Cập nhật kích thước scene ban đầu
    updateSceneRect();

    // --- TẠO TIMER ĐỂ CẬP NHẬT "ĐƯỜNG KẺ THỜI GIAN HIỆN TẠI" ---
    m_timer = new QTimer(this);
    // Kết nối tín hiệu timeout() của timer với slot update() của QGraphicsView
    // (update() sẽ kích hoạt drawForeground chạy lại)
    connect(m_timer, &QTimer::timeout, this, QOverload<>::of(&CalendarView::update));
    m_timer->start(60000); // Cập nhật mỗi 60 giây (1 phút)
}

// =================================================================================
// === 2. PUBLIC API (HÀM ĐIỀU KHIỂN TỪ MAINWINDOW)
// =================================================================================

/**
 * @brief Đặt số lượng ngày hiển thị (1, 3, 5, 7).
 * @param days Số ngày
 */
void CalendarView::setNumberOfDays(int days)
{
    if (days < 1) days = 1; // Đảm bảo luôn có ít nhất 1 ngày
    m_days = days;

    // Cập nhật lại Scene Rect, vì getDayWidth() phụ thuộc vào m_days
    updateSceneRect();
}

/**
 * @brief Đặt "Tỉ lệ thời gian" (Time Scale), hay còn gọi là mức "zoom" dọc.
 * @param minutes Số phút cho mỗi "khối" 60px (ví dụ: 60, 30, 15).
 */
void CalendarView::setTimeScale(int minutes)
{
    if (minutes <= 0) minutes = 60;

    // 1. Tính toán chiều cao mới cho 1 giờ
    // Ví dụ: 30 phút/khối -> m_hourHeight = (60/30) * 60 = 120px/giờ
    // Ví dụ: 15 phút/khối -> m_hourHeight = (60/15) * 60 = 240px/giờ
    m_hourHeight = (60.0 / minutes) * 60.0;

    // 2. Cập nhật lại Scene Rect (vì tổng chiều cao 24*m_hourHeight đã thay đổi)
    updateSceneRect();

    // 3. Cập nhật lại vị trí/kích thước của tất cả sự kiện
    updateViewForDateRange(m_currentMonday);
}

/**
 * @brief Đặt chênh lệch múi giờ (so với UTC).
 * @param offsetSeconds Số giây chênh lệch.
 */
void CalendarView::setTimezoneOffset(int offsetSeconds)
{
    m_timezoneOffsetSeconds = offsetSeconds;
    // Yêu cầu vẽ lại toàn bộ, vì tất cả sự kiện
    // có thể sẽ bị dịch chuyển (ví dụ: 7h UTC -> 14h VN | 7h UTC -> 2h Mỹ)
    updateViewForDateRange(m_currentMonday);
}

// =================================================================================
// === 3. QUẢN LÝ SỰ KIỆN (PUBLIC EVENT MANAGEMENT)
// =================================================================================

/**
 * @brief Thêm một EventItem* (đã được tạo bởi MainWindow) vào Scene.
 */
void CalendarView::addEvent(EventItem *item)
{
    // Khi event thay đổi (kéo/thả/resize xong)
    connect(item, &EventItem::eventChanged, this, [this](EventItem *changedItem){
        // Yêu cầu sắp xếp lại layout (chỉ cho ngày đó,
        // nhưng gọi cho cả tuần cho đơn giản)
        updateViewForDateRange(m_currentMonday);
    });

    // Khi click vào event
    connect(item, &EventItem::clicked, this, &CalendarView::eventClicked);

    // Khi kéo event (phát tín hiệu lên MainWindow để xử lý logic)
    connect(item, &EventItem::eventDragged, this, &CalendarView::onInternalEventDragged);

    m_scene->addItem(item); // Thêm item đồ họa vào scene
    item->hide(); // Ẩn nó đi, chờ updateViewForDateRange quyết định
}

/**
 * @brief Xóa một EventItem* khỏi Scene.
 * (MainWindow chịu trách nhiệm delete con trỏ)
 */
void CalendarView::removeEvent(EventItem *item)
{
    if (item) {
        m_scene->removeItem(item);
    }
}

/**
 * @brief HÀM QUAN TRỌNG NHẤT: Cập nhật toàn bộ view.
 * Hàm này được gọi khi:
 * 1. Đổi tuần (showNextWeek/showPreviousWeek).
 * 2. Thay đổi số ngày (onDisplayDaysChanged).
 * 3. Thay đổi Time Scale.
 * 4. Thay đổi Múi giờ.
 * 5. Thêm/Sửa/Xóa sự kiện (gián tiếp qua addEvent/eventChanged).
 */
void CalendarView::updateViewForDateRange(const QDate &monday)
{
    m_currentMonday = monday;
    if (!m_currentMonday.isValid()) return;

    QDate endDate = m_currentMonday.addDays(m_days - 1);

    // BƯỚC 1: LẶP QUA TẤT CẢ ITEM, ẨN HOẶC HIỆN
    // (Đây là logic lọc và hiển thị)
    for (QGraphicsItem *item : m_scene->items()) {
        if (auto eventItem = qgraphicsitem_cast<EventItem*>(item)) {

            // Lấy ngày hiển thị (đã chuyển sang Local time)
            QDate eventDate = eventItem->startTime().toOffsetFromUtc(m_timezoneOffsetSeconds).date();

            // Đọc trạng thái BỘ LỌC (từ MainWindow)
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
        }
    }

    // BƯỚC 2: SẮP XẾP LẠI LAYOUT CHO TỪNG NGÀY
    // (Hàm này sẽ tự động bỏ qua các item đã bị hide ở Bước 1)
    for (int i = 0; i < m_days; ++i) {
        relayoutEventsForDate(m_currentMonday.addDays(i), i);
    }

    viewport()->update(); // Yêu cầu vẽ lại (repaint)
}

// =================================================================================
// === 4. HÀM SỰ KIỆN QT (PROTECTED QT EVENT HANDLERS)
// =================================================================================

/**
 * @brief Được Qt gọi khi kích thước của View (cửa sổ) thay đổi.
 */
void CalendarView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    // Cập nhật lại kích thước Scene (đặc biệt là chiều rộng)
    updateSceneRect();
    // Cập nhật lại layout của các sự kiện
    // (vì getDayWidth() đã thay đổi)
    updateViewForDateRange(m_currentMonday);
}

/**
 * @brief Được Qt gọi để vẽ NỀN (GRID) - Vẽ *trước* khi vẽ sự kiện.
 */
void CalendarView::drawBackground(QPainter *painter, const QRectF &rect)
{
    // Vẽ nền trắng/trong suốt (do QSS quyết định)
    QGraphicsView::drawBackground(painter, rect);

    const double dayWidth = getDayWidth();
    if (dayWidth <= 0) return;

    // --- Vẽ các đường kẻ ngang (chia giờ) ---
    painter->setPen(QPen(Qt::lightGray, 1, Qt::SolidLine));
    for (int hour = 1; hour < 24; ++hour) {
        const double y = hour * m_hourHeight;
        painter->drawLine(QPointF(rect.left(), y), QPointF(rect.right(), y));
    }

    // --- Vẽ các đường kẻ dọc (chia ngày) ---
    painter->setPen(QPen(Qt::gray, 1, Qt::SolidLine));
    for (int day = 1; day < m_days; ++day) {
        const double x = day * dayWidth;
        painter->drawLine(QPointF(x, rect.top()), QPointF(x, rect.bottom()));
    }
}

/**
 * @brief Được Qt gọi để vẽ TIỀN CẢNH - Vẽ *sau* khi vẽ sự kiện.
 * Dùng để vẽ "Đường kẻ thời gian hiện tại".
 */
void CalendarView::drawForeground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawForeground(painter, rect);

    // Lấy thời gian hiện tại THEO MÚI GIỜ ĐÃ CHỌN
    QDateTime now = QDateTime::currentDateTime().toOffsetFromUtc(m_timezoneOffsetSeconds);
    QDate today = now.date();

    // 1. Kiểm tra xem "hôm nay" có đang được hiển thị không
    if (today < m_currentMonday || today > m_currentMonday.addDays(m_days - 1)) {
        return; // Không cần vẽ nếu "hôm nay" không có trên lịch
    }

    double dayWidth = getDayWidth();
    if (dayWidth <= 0) return;

    // 2. Tính toán vị trí
    // Tọa độ Y (chiều dọc)
    double y = (now.time().hour() * 60 + now.time().minute()) / 60.0 * m_hourHeight;
    // Tọa độ X (chiều ngang)
    int todayIndex = m_currentMonday.daysTo(today); // todayIndex = 0, 1, 2...
    double x_today_start = todayIndex * dayWidth;
    double x_today_end = x_today_start + dayWidth;

    // 3. Vẽ đường chấm (cho các ngày không phải hôm nay)
    QPen dottedPen(QColor("#0078d7"), 1, Qt::DotLine);
    painter->setPen(dottedPen);
    if (x_today_start > 0) {
        painter->drawLine(QPointF(0, y), QPointF(x_today_start, y));
    }
    if (x_today_end < sceneRect().width()) {
        painter->drawLine(QPointF(x_today_end, y), QPointF(sceneRect().width(), y));
    }

    // 4. Vẽ đường đậm (cho ngày hôm nay)
    QPen solidPen(QColor("#0078d7"), 2, Qt::SolidLine);
    painter->setPen(solidPen);
    painter->drawLine(QPointF(x_today_start, y), QPointF(x_today_end, y));

    // 5. Vẽ hình tròn ở đầu đường kẻ
    painter->setBrush(QColor("#0078d7"));
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(QPointF(x_today_start, y), 6, 6); // Vòng ngoài
    painter->setBrush(QColor("#FFFFFF"));
    painter->drawEllipse(QPointF(x_today_start, y), 4, 4); // Vòng trong (màu trắng)
}

// =================================================================================
// === 5. CORE PRIVATE LOGIC (LAYOUT ALGORITHM)
// =================================================================================

/**
 * @brief HÀM QUAN TRỌNG: Sắp xếp các sự kiện bị chồng chéo trong 1 ngày.
 * Đây là thuật toán "Column Packing".
 * @param date Ngày cần sắp xếp.
 * @param dayIndex Cột của ngày (0 = cột đầu tiên, 1 = cột thứ hai...).
 */
void CalendarView::relayoutEventsForDate(const QDate &date, int dayIndex)
{
    double dayWidth = getDayWidth();

    // 1. Lấy tất cả sự kiện (đã được hiển thị) cho ngày này
    QList<EventItem*> dayEvents;
    for (QGraphicsItem *item : m_scene->items()) {
        if (auto eventItem = qgraphicsitem_cast<EventItem*>(item)) {
            // Chỉ lấy các item đang hiển thị (visible)
            if (eventItem->isVisible()) {
                QDate displayDate = eventItem->startTime().toOffsetFromUtc(m_timezoneOffsetSeconds).date();
                if (displayDate == date) {
                    dayEvents.append(eventItem);
                }
            }
        }
    }
    if (dayEvents.isEmpty()) return;

    // 2. Sắp xếp các sự kiện theo thời gian bắt đầu (dùng UTC để so sánh)
    std::sort(dayEvents.begin(), dayEvents.end(), [](EventItem* a, EventItem* b) {
        if (a->startTime() != b->startTime())
            return a->startTime() < b->startTime();
        return a->endTime() < b->endTime();
    });

    // 3. Thuật toán chia cột
    // (Một danh sách chứa các danh sách cột)
    QList<QList<EventItem*>> columns;
    columns.append(QList<EventItem*>()); // Bắt đầu với 1 cột

    for (EventItem *event : dayEvents) {
        bool placed = false;
        // Tìm cột đầu tiên mà sự kiện này có thể nằm vào
        // (nó không chồng lên sự kiện cuối cùng của cột đó)
        for (int i = 0; i < columns.size(); ++i) {
            if (columns[i].isEmpty() || event->startTime() >= columns[i].last()->endTime()) {
                columns[i].append(event);
                placed = true;
                break;
            }
        }
        // Nếu không tìm được cột nào (bị chồng hết), tạo cột mới
        if (!placed) {
            columns.append({event});
        }
    }

    // 4. Cập nhật hình dạng (Geometry) của từng sự kiện
    int totalColumns = columns.size();
    for (int i = 0; i < columns.size(); ++i) { // i = chỉ số cột
        for (EventItem *event : columns[i]) {
            // Ra lệnh cho EventItem tự cập nhật vị trí và kích thước
            event->updateGeometry(dayWidth, m_hourHeight, dayIndex, i, totalColumns, m_timezoneOffsetSeconds);
        }
    }
}

// =================================================================================
// === 6. PRIVATE HELPER FUNCTIONS
// =================================================================================

/**
 * @brief Tính toán chiều rộng (pixels) của 1 cột ngày.
 */
double CalendarView::getDayWidth() const
{
    if (m_days == 0) return 0;
    // Chiều rộng scene / số ngày = chiều rộng 1 ngày
    return sceneRect().width() / m_days;
}

/**
 * @brief Cập nhật kích thước của Scene (tấm vải ảo).
 */
void CalendarView::updateSceneRect()
{
    // Chiều rộng của Scene LUÔN BẰNG chiều rộng của Viewport (cửa sổ)
    // (Trừ đi thanh cuộn dọc nếu có)
    qreal viewWidth = viewport()->width();

    // Chiều cao của Scene là chiều cao logic
    // (ví dụ: 120px/giờ * 24 giờ = 2880px)
    m_scene->setSceneRect(0, 0, viewWidth, 24 * m_hourHeight);
}

/**
 * @brief Hàm này chỉ được gọi 1 lần trong constructor cũ,
 * nay được thay thế bởi updateViewForDateRange.
 */
void CalendarView::performInitialLayout()
{
    updateViewForDateRange(m_currentMonday);
}

// =================================================================================
// === 7. PRIVATE SLOTS
// =================================================================================

/**
 * @brief Slot này nhận tín hiệu "kéo" từ một EventItem.
 * Nó chỉ đơn giản là "phát lại" (re-emit) tín hiệu đó
 * lên cho MainWindow xử lý.
 */
void CalendarView::onInternalEventDragged(EventItem *item, const QDateTime &newStartTime, const QDateTime &newEndTime)
{
    // Chuyển tín hiệu này lên cho MainWindow
    emit eventDragged(item, newStartTime, newEndTime);
}
