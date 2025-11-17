#include "eventitem.h"
#include "calendarview.h" // Cần để cast scene()->views()

#include <QPainter>
#include <QTextOption>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QApplication> // Cần cho QApplication::startDragDistance
#include <QJsonObject>  // Cần cho m_extraData

// =================================================================================
// === 1. HÀM DỰNG (CONSTRUCTOR)
// =================================================================================

/**
 * @brief Hàm dựng của EventItem.
 * (Đã được cập nhật để nhận eventType và extraData)
 */
EventItem::EventItem(const QString &title, const QColor &color,
                     const QDateTime &startTime, const QDateTime &endTime,
                     const QString &description, const QString &showAs,
                     const QString &category, bool isAllDay,
                     const EventDialog::RecurrenceRule &rule,
                     const QString &eventType,
                     const QJsonObject &extraData, // Dữ liệu JSON cho loại sự kiện
                     QGraphicsItem *parent)
    : QObject(),
    QGraphicsRectItem(parent),
    m_title(title), m_color(color),
    m_startTime(startTime), m_endTime(endTime), // Lưu ý: Đây là thời gian UTC
    m_description(description),
    m_showAs(showAs),
    m_category(category),
    m_isAllDay(isAllDay),
    m_rule(rule),
    m_isResizing(false),
    m_ghostItem(nullptr),
    m_isMoving(false),
    m_isFilteredOut(false), // Mặc định là không bị lọc
    m_eventType(eventType),
    m_extraData(extraData)
{
    // Cài đặt cơ bản cho QGraphicsItem
    setBrush(m_color);
    setPen(Qt::NoPen);
    setFlag(QGraphicsItem::ItemIsMovable); // Cho phép item di chuyển
    setAcceptHoverEvents(true);           // Cho phép bắt sự kiện di chuột (hover)

    // --- Xây dựng nội dung cho Tooltip (khi di chuột vào) ---
    QString toolTipText;

    // === ĐỌC TỪ extraData DỰA TRÊN eventType ===
    // (Phần logic này đọc dữ liệu từ QJsonObject m_extraData)
    if (m_eventType == "Cuộc họp") {
        QString status = m_extraData["meetingStatus"].toString();
        QString host = m_extraData["host"].toString();
        QString participants = m_extraData["participants"].toString();

        toolTipText += QString("<b>Trạng thái: %1</b><br>").arg(status);
        if (!host.isEmpty()) {
            toolTipText += QString("<b>Chủ trì:</b> %1<br>").arg(host);
        }
        if (!participants.isEmpty()) {
            QStringList participantsList = participants.split(',');
            toolTipText += "<b>Tham gia:</b><br>";
            for (const QString &p : participantsList) {
                if (!p.trimmed().isEmpty()) {
                    toolTipText += QString("- %1<br>").arg(p.trimmed());
                }
            }
        }
        toolTipText += "<hr>";
    } else if (m_eventType == "Học tập") {
        toolTipText += QString("<b>Môn học: %1</b><br>").arg(m_extraData["subject"].toString());
        toolTipText += QString("<i>Cách thức: %1</i><br>").arg(m_extraData["studyMethod"].toString());
        toolTipText += "<hr>";
    }
    else if (m_eventType == "Ngày lễ") {
        QString scope = m_extraData["holidayScope"].toString();
        toolTipText += QString("<b>Phạm vi: %1</b><br>").arg(scope);
        if (scope == "Tùy chỉnh") {
            toolTipText += QString("<i>Tên: %1</i><br>").arg(m_extraData["customHolidayName"].toString());
        }
        toolTipText += "<hr>";
    }
    else if (m_eventType == "Cuộc hẹn") {
        QString type = m_extraData["appointmentType"].toString();
        toolTipText += QString("<b>Loại: %1</b><br>").arg(type);
        if (type == "Khác") {
            // Sửa lỗi: %f (float) -> %1 (string)
            toolTipText += QString("<i>Tên: %1</i><br>").arg(m_extraData["customAppointmentType"].toString());
        }
        toolTipText += QString("<b>Địa điểm:</b> %1<br>").arg(m_extraData["location"].toString());
        if (m_extraData["isPrivate"].toBool()) {
            toolTipText += "<i>(Riêng tư)</i><br>";
        }
        toolTipText += "<hr>";
    }
    // === KẾT THÚC ĐỌC extraData ===

    // Thêm Tiêu đề và Mô tả (chung cho mọi loại)
    toolTipText += QString("<b>%1</b><br><br>").arg(m_title);
    if (!m_description.isEmpty()) {
        toolTipText += m_description;
    }

    setToolTip(toolTipText);
}

// =================================================================================
// === 2. API CÔNG KHAI (PUBLIC API - GETTERS/SETTERS)
// =================================================================================

// --- Getters (Hàm lấy thông tin) ---
QString EventItem::title() const { return m_title; }
QColor EventItem::color() const { return m_color; }

// --- Setters (Hàm đặt thông tin) ---
// (Dùng bởi MainWindow khi tải dữ liệu)
void EventItem::setStartTime(const QDateTime &startTime) { m_startTime = startTime; }
void EventItem::setEndTime(const QDateTime &endTime) { m_endTime = endTime; }

// =================================================================================
// === 3. LOGIC CỐT LÕI (CORE LOGIC - GEOMETRY & PAINTING)
// =================================================================================

/**
 * @brief HÀM QUAN TRỌNG: Tính toán vị trí và kích thước của item trên scene.
 * @param dayWidth Chiều rộng (pixel) của 1 cột ngày (do CalendarView cung cấp).
 * @param hourHeight Chiều cao (pixel) của 1 giờ (do CalendarView cung cấp).
 * @param dayIndex Cột ngày (0=cột đầu, 1=cột thứ hai...).
 * @param col Cột phụ (dùng khi có sự kiện chồng chéo).
 * @param totalCols Tổng số cột phụ.
 * @param displayOffsetSeconds Múi giờ hiển thị (từ MainWindow).
 */
void EventItem::updateGeometry(double dayWidth, double hourHeight, int dayIndex, int col, int totalCols, int displayOffsetSeconds)
{
    // === LOGIC MỚI: XỬ LÝ MÚI GIỜ ===

    // 1. Chuyển đổi thời gian lưu trữ (UTC) sang múi giờ hiển thị (Local)
    // (m_startTime và m_endTime của item là UTC)
    QDateTime displayStart = m_startTime.toOffsetFromUtc(displayOffsetSeconds);
    QDateTime displayEnd = m_endTime.toOffsetFromUtc(displayOffsetSeconds);

    QTime startTime = displayStart.time();
    QTime endTime = displayEnd.time();

    // 2. Tính toán vị trí Y (dọc) và Chiều cao
    // (Vị trí Y dựa trên giờ/phút của thời gian BẮT ĐẦU hiển thị)
    double yPos = (startTime.hour() * 60 + startTime.minute()) / 60.0 * hourHeight;

    // Tính thời lượng (luôn dùng secsTo trên UTC để đảm bảo chính xác tuyệt đối)
    qint64 durationSeconds = m_startTime.secsTo(m_endTime);
    double height = (durationSeconds / 3600.0) * hourHeight;

    // 3. Xử lý các sự kiện kéo dài qua nửa đêm (theo múi giờ hiển thị)
    if (displayStart.date() < displayEnd.date()) {
        // Nếu sự kiện kết thúc vào ngày mai (theo giờ local)
        // -> Vẽ nó kéo dài đến hết 24:00 của ngày hôm nay
        height = (24.0 * hourHeight) - yPos;
    }
    // (Trường hợp sự kiện bắt đầu hôm qua và kết thúc hôm nay
    // sẽ được CalendarView xử lý ở ngày "hôm qua")

    // === LOGIC CŨ (Tính toán X) ===

    // 4. Tính toán vị trí X (ngang)
    double colWidth = dayWidth / totalCols; // Chiều rộng thực tế (chia cột nếu chồng)
    double xPos = (dayIndex * dayWidth) + (col * colWidth);

    // 5. Cập nhật hình dạng (vị trí và kích thước)
    setPos(xPos, yPos); // Đặt vị trí
    setRect(0, 0, colWidth - 5, height); // Đặt kích thước (trừ 5px để tạo khoảng hở)
}

/**
 * @brief Hàm vẽ (Paint) - Được Qt gọi tự động.
 * Chịu trách nhiệm vẽ hình chữ nhật, tiêu đề, và thanh trạng thái.
 */
void EventItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);

    // 1. Vẽ hình chữ nhật nền bo góc
    painter->setBrush(m_color);
    painter->setPen(QPen(m_color.darker(120))); // Viền đậm hơn một chút
    painter->drawRoundedRect(rect(), 5, 5);

    // 2. (MỚI) Vẽ thanh trạng thái (status bar) cho "Cuộc họp"
    if (m_eventType == "Cuộc họp") {
        QColor statusColor;
        QString status = m_extraData["meetingStatus"].toString();
        if (status == "Đã xác nhận") {
            statusColor = QColor("#8cbb63"); // Xanh lá
        } else if (status == "Đã hủy") {
            statusColor = QColor("#d9534f"); // Đỏ
        } else { // "Dự kiến"
            statusColor = QColor("#f0ad4e"); // Vàng cam
        }

        // Vẽ một thanh dọc rộng 4px bên trái
        QRectF statusBarRect = rect().adjusted(1, 1, 0, -1);
        statusBarRect.setWidth(4);
        painter->setBrush(statusColor);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(statusBarRect, 3, 3);
    }

    // 3. Định nghĩa vùng vẽ Text (trừ 5px lề trên, 5px lề trái/phải)
    const int resizeHandleHeight = 5; // Chiều cao vùng resize ở đáy
    // Dịch lề trái: 12px (nếu là cuộc họp) hoặc 5px (bình thường)
    int leftPadding = (m_eventType == "Cuộc họp") ? 12 : 5;
    QRectF textRect = rect().adjusted(leftPadding, 5, -5, -resizeHandleHeight - 5);

    // 4. Vẽ tiêu đề (text)
    painter->setPen(Qt::black);
    QFont font = painter->font();
    font.setPixelSize(12);
    painter->setFont(font);

    QTextOption textOption;
    textOption.setAlignment(Qt::AlignTop | Qt::AlignLeft);
    textOption.setWrapMode(QTextOption::WordWrap); // Cho phép xuống dòng

    painter->drawText(textRect, m_title, textOption);

    // 5. (MỚI) Vẽ tay nắm Resize (3 dấu chấm ở đáy)
    if (rect().height() > 20) // Chỉ vẽ nếu item đủ cao
    {
        QRectF handleRect(rect().left(), rect().bottom() - resizeHandleHeight, rect().width(), resizeHandleHeight);
        QPoint center = handleRect.center().toPoint();
        QPen dotPen(m_color.darker(150), 2, Qt::DotLine);
        painter->setPen(dotPen);
        int y = center.y();
        painter->drawPoint(center.x() - 4, y);
        painter->drawPoint(center.x(), y);
        painter->drawPoint(center.x() + 4, y);
    }
}

// =================================================================================
// === 4. XỬ LÝ SỰ KIỆN TƯƠNG TÁC (INTERACTION EVENT HANDLERS)
// =================================================================================

/**
 * @brief Được Qt gọi khi một thuộc tính (như vị trí) SẮP thay đổi.
 * (Logic bắt dính (snapping) đã được chuyển sang updateGhostPosition)
 */
QVariant EventItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    /*
    if (change == QGraphicsItem::ItemPositionChange && scene()) {
        // ... (Logic bắt dính cũ đã được comment out) ...
    }
    */
    return QGraphicsRectItem::itemChange(change, value);
}

/**
 * @brief Được gọi khi di chuột BÊN TRÊN item (Hover Move).
 */
void EventItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    // Xác định vùng "tay cầm" resize (5px ở đáy)
    QRectF handle = QRectF(rect().bottomLeft(), QPointF(rect().right(), rect().bottom() - 5));

    if (handle.contains(event->pos())) {
        // 1. Nếu ở trên tay cầm -> Đổi trỏ chuột resize
        setCursor(Qt::SizeVerCursor);
    } else {
        // 2. Nếu ở trên thân item -> Đổi trỏ chuột "bàn tay" (chỉ)
        setCursor(Qt::PointingHandCursor);
    }
    QGraphicsRectItem::hoverMoveEvent(event);
}

/**
 * @brief Được gọi khi chuột RỜI KHỎI item (Hover Leave).
 */
void EventItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    setCursor(Qt::ArrowCursor); // Trả con trỏ về mặc định
    QGraphicsRectItem::hoverLeaveEvent(event);
}

/**
 * @brief Được gọi khi NHẤN chuột.
 */
void EventItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // 1. Kiểm tra xem có nhấn vào vùng Resize không
    QRectF handle = QRectF(rect().bottomLeft(), QPointF(rect().right(), rect().bottom() - 5));
    if (handle.contains(event->pos())) {
        m_isResizing = true;
        m_isMoving = false;
        QGraphicsRectItem::mousePressEvent(event); // Để logic resize gốc của Qt xử lý
    } else {
        // 2. Chuẩn bị để di chuyển (Move) hoặc Click
        m_isResizing = false;
        m_isMoving = false; // Chưa di chuyển, có thể chỉ là click
        m_dragStartOffset = event->pos(); // Vị trí nhấn (so với item)
        m_pressPos = event->scenePos(); // Vị trí nhấn (so với scene)
        event->accept(); // Chấp nhận sự kiện
    }
}

/**
 * @brief Được gọi khi KÉO chuột (sau khi đã nhấn).
 */
void EventItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_isResizing) {
        // --- Logic Resize ---
        auto *view = qobject_cast<CalendarView*>(scene()->views().first());
        if (!view) return;
        double newHeight = event->pos().y();
        const double hourHeight = view->getHourHeight();
        const double slotHeight = hourHeight / 4.0; // Bắt dính 15 phút
        int timeSlots = qRound(newHeight / slotHeight);
        double snappedHeight = timeSlots * slotHeight;
        if (snappedHeight < slotHeight) {
            snappedHeight = slotHeight; // Chiều cao tối thiểu là 15 phút
        }
        setRect(0, 0, rect().width(), snappedHeight);

    } else {
        // --- Logic Move (Drag) ---

        // 1. Kiểm tra xem đã "bắt đầu kéo" chưa
        // (Di chuyển đủ xa > startDragDistance)
        if (!m_isMoving && (event->scenePos() - m_pressPos).manhattanLength() > QApplication::startDragDistance()) {
            m_isMoving = true;

            // 2. Tạo "ghost item" (item 'ma' khi kéo)
            this->setBrush(Qt::transparent); // Ẩn item gốc
            this->setPen(Qt::NoPen);

            m_ghostItem = new QGraphicsRectItem(this->rect());
            m_ghostItem->setBrush(QColor(100, 100, 100, 100)); // Màu xám mờ
            m_ghostItem->setPen(QPen(Qt::black, 1, Qt::DashLine)); // Viền đứt
            m_ghostItem->setZValue(this->zValue() + 1); // Nằm trên item gốc
            scene()->addItem(m_ghostItem);

            // Đặt vị trí ban đầu (đã bắt dính)
            updateGhostPosition(this->pos());
        }

        // 3. Nếu đang trong trạng thái kéo -> Cập nhật vị trí ghost
        if (m_isMoving && m_ghostItem) {
            QPointF newScenePos = mapToScene(event->pos()) - m_dragStartOffset;
            updateGhostPosition(newScenePos); // Hàm này sẽ bắt dính
        }
        event->accept();
    }
}

/**
 * @brief Được gọi khi THẢ chuột.
 * Đây là nơi xử lý kết quả của Resize, Move, hoặc Click.
 */
void EventItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    auto *view = qobject_cast<CalendarView*>(scene()->views().first());
    if (!view) {
        QGraphicsRectItem::mouseReleaseEvent(event);
        return;
    }

    if (m_isResizing) {
        // --- Kết thúc Resize ---
        m_isResizing = false;
        const double hourHeight = view->getHourHeight();
        // Tính toán thời gian kết thúc mới
        int totalMinutes = qRound(rect().height() / hourHeight * 60.0);
        m_endTime = m_startTime.addSecs(totalMinutes * 60);

        // Báo cáo thay đổi (MainWindow sẽ xử lý logic 'This/All events')
        emit eventChanged(this);

    } else if (m_isMoving) {
        // --- Kết thúc Move (Drag) ---
        m_isMoving = false;
        this->setBrush(m_color); // Khôi phục màu cho item gốc
        this->setPen(Qt::NoPen);

        if (m_ghostItem) {
            QPointF finalPos = m_ghostItem->pos(); // Lấy vị trí cuối (đã bắt dính)
            scene()->removeItem(m_ghostItem);
            delete m_ghostItem;
            m_ghostItem = nullptr;

            // --- TÍNH TOÁN THỜI GIAN MỚI (DỰA TRÊN VỊ TRÍ GHOST) ---
            const double dayWidth = view->getDayWidth();
            const double hourHeight = view->getHourHeight();
            if (dayWidth <= 0) return;

            // Tính Ngày mới (từ X)
            int maxDayIndex = view->getNumberOfDays() - 1;
            int finalDayIndex = qRound((finalPos.x() - 5) / dayWidth);
            finalDayIndex = qBound(0, finalDayIndex, maxDayIndex); // Đảm bảo không ra ngoài
            QDate newDate = view->getMondayOfCurrentWeek().addDays(finalDayIndex);

            // Tính Giờ mới (từ Y)
            const double slotHeight = hourHeight / 4.0; // 15 phút
            int timeSlot = qRound(finalPos.y() / slotHeight);
            int start_minute = timeSlot * 15;
            QTime newTime(start_minute / 60, start_minute % 60);

            // Tính Thời lượng
            long long durationMs = m_startTime.msecsTo(m_endTime);

            // Tạo QDateTime mới (ở múi giờ Local)
            // (Chúng ta không biết offset ở đây, nhưng CalendarView biết
            // khi nó nhận tín hiệu. Chúng ta chỉ cần tạo QDateTime "ngây thơ")
            QDateTime newStartTime(newDate, newTime);
            QDateTime newEndTime = newStartTime.addMSecs(durationMs);

            // --- THAY ĐỔI KIẾN TRÚC QUAN TRỌNG ---
            // Item KHÔNG tự cập nhật dữ liệu của nó.
            // Nó PHÁT TÍN HIỆU (emit) lên MainWindow.
            emit eventDragged(this, newStartTime, newEndTime);
            // MainWindow sẽ nhận tín hiệu này, hỏi người dùng (nếu lặp),
            // và sau đó gọi lại setStartTime/setEndTime nếu cần.
        }

    } else {
        // --- Đây là CLICK (Không Resize, không Move) ---
        emit clicked(this); // Phát tín hiệu click
        event->accept();    // Báo rằng chúng ta đã xử lý sự kiện
        return;             // Dừng hàm, không lan truyền sự kiện
    }

    // Reset cờ (chỉ khi là Resize hoặc Move)
    m_isResizing = false;
    m_isMoving = false;
    QGraphicsRectItem::mouseReleaseEvent(event);
}

// =================================================================================
// === 5. HÀM TRỢ GIÚP NỘI BỘ (PRIVATE HELPERS)
// =================================================================================

/**
 * @brief Tính toán và cập nhật vị trí "bắt dính" (snapped) cho ghost item.
 * @param newScenePos Vị trí mới (chưa bắt dính) trên scene.
 */
void EventItem::updateGhostPosition(QPointF newScenePos)
{
    auto *view = qobject_cast<CalendarView*>(scene()->views().first());
    if (!view || !m_ghostItem) return;

    int maxDayIndex = view->getNumberOfDays() - 1;
    const double dayWidth = view->getDayWidth();
    const double hourHeight = view->getHourHeight();
    if (dayWidth <= 0) return;

    // 1. Bắt dính Y (Thời gian - 15 phút)
    const double slotHeight = hourHeight / 4.0;
    int timeSlot = qRound(newScenePos.y() / slotHeight);
    double snappedY = timeSlot * slotHeight;

    // 2. Bắt dính X (Ngày)
    int dayIndex = qRound((newScenePos.x() - 5) / dayWidth);
    dayIndex = qBound(0, dayIndex, maxDayIndex); // Giới hạn trong 0 -> maxDayIndex
    double snappedX = (dayIndex * dayWidth + 5); // +5px lề

    // 3. Cập nhật vị trí cho ghost item
    m_ghostItem->setPos(snappedX, snappedY);
}
