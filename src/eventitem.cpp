#include "eventitem.h"
#include "calendarview.h"

#include <QPainter>
#include <QTextOption>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QApplication>
#include <QJsonObject>

// THAY ĐỔI: Sửa hàm khởi tạo
EventItem::EventItem(const QString &title, const QColor &color,
                     const QDateTime &startTime, const QDateTime &endTime,
                     const QString &description, const QString &showAs,
                     const QString &category, bool isAllDay,
                     const EventDialog::RecurrenceRule &rule,
                     const QString &eventType,
                     const QJsonObject &extraData,
                     QGraphicsItem *parent)
    : QObject(),
    QGraphicsRectItem(parent),
    m_title(title), m_color(color),
    m_startTime(startTime), m_endTime(endTime),
    m_description(description),
    m_showAs(showAs),
    m_category(category),
    m_isAllDay(isAllDay),
    m_rule(rule),
    m_isResizing(false),
    m_ghostItem(nullptr),
    m_isMoving(false),
    m_isFilteredOut(false),
    m_eventType(eventType),
    m_extraData(extraData)
{
    setBrush(m_color);
    setPen(Qt::NoPen);
    setFlag(QGraphicsItem::ItemIsMovable); // Flag này vẫn cần
    setAcceptHoverEvents(true);

    // 1. Tạo nội dung cho tooltip
    QString toolTipText;

    // === SỬA ĐỔI: ĐỌC TỪ extraData ===
    // 2. Thêm thông tin cuộc họp (nếu có)
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
            toolTipText += QString("<i>Tên: %f</i><br>").arg(m_extraData["customAppointmentType"].toString());
        }
        toolTipText += QString("<b>Địa điểm:</b> %1<br>").arg(m_extraData["location"].toString());
        if (m_extraData["isPrivate"].toBool()) {
            toolTipText += "<i>(Riêng tư)</i><br>";
        }
        toolTipText += "<hr>";
    }
    // (Sau này bạn có thể 'else if (m_eventType == "Học tập") { ... }' ở đây)
    // === KẾT THÚC SỬA ĐỔI ===

    // 3. Thêm tiêu đề
    toolTipText += QString("<b>%1</b><br><br>").arg(m_title);

    // 4. Thêm mô tả
    if (!m_description.isEmpty()) {
        toolTipText += m_description;
    }

    setToolTip(toolTipText);
}

// THAY ĐỔI: Thêm 2 hàm setter
void EventItem::setStartTime(const QDateTime &startTime)
{
    m_startTime = startTime;
}

void EventItem::setEndTime(const QDateTime &endTime)
{
    m_endTime = endTime;
}


// THAY ĐỔI: Logic updateGeometry giờ sẽ tính toán ngày dựa trên QDateTime
void EventItem::updateGeometry(double dayWidth, double hourHeight, int dayIndex, int col, int totalCols, int displayOffsetSeconds)
{
    // === BẮT ĐẦU LOGIC MỚI ===

    // 1. Chuyển đổi thời gian lưu trữ (UTC) sang múi giờ hiển thị
    // (m_startTime và m_endTime của bạn đang là UTC)
    QDateTime displayStart = m_startTime.toOffsetFromUtc(displayOffsetSeconds);
    QDateTime displayEnd = m_endTime.toOffsetFromUtc(displayOffsetSeconds);

    QTime startTime = displayStart.time();
    QTime endTime = displayEnd.time();

    // 2. Tính toán vị trí Y và Chiều cao
    double yPos = (startTime.hour() * 60 + startTime.minute()) / 60.0 * hourHeight;

    // Tính thời lượng (vẫn dùng secsTo trên UTC để đảm bảo chính xác tuyệt đối)
    qint64 durationSeconds = m_startTime.secsTo(m_endTime);
    double height = (durationSeconds / 3600.0) * hourHeight;

    // 3. Xử lý các sự kiện kéo dài qua nửa đêm (theo múi giờ hiển thị)
    // (Rất quan trọng khi chuyển từ UTC+7 sang UTC)
    if (displayStart.date() < displayEnd.date()) {
        // Sự kiện bắt đầu hôm nay và kết thúc vào ngày mai
        height = (24.0 * hourHeight) - yPos; // Kéo dài đến hết 24:00
    }
    // (Trường hợp sự kiện bắt đầu hôm qua và kết thúc hôm nay
    // sẽ được CalendarView xử lý bằng cách gọi relayoutEventsForDate
    // cho cả 2 ngày, nên chúng ta không cần xử lý yPos = 0 ở đây)

    // === KẾT THÚC LOGIC MỚI ===


    // --- PHẦN LOGIC CŨ (giữ nguyên) ---

    // Tính toán vị trí X
    double colWidth = dayWidth / totalCols;
    double xPos = (dayIndex * dayWidth) + (col * colWidth);

    // Cập nhật hình dạng
    setPos(xPos, yPos);
    setRect(0, 0, colWidth - 5, height); // Trừ 5px để tạo khoảng hở
}

QString EventItem::title() const
{
    return m_title;
}

QColor EventItem::color() const
{
    return m_color;
}

// ... hàm paint và itemChange không thay đổi ...
void EventItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // Vẽ hình chữ nhật bo góc với màu nền
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(m_color);
    painter->setPen(QPen(m_color.darker(120))); // Viền đậm hơn một chút
    painter->drawRoundedRect(rect(), 5, 5);

    if (m_eventType == "Cuộc họp") {
        QColor statusColor;
        QString status = m_extraData["meetingStatus"].toString();
        if (status == "Đã xác nhận") {
            statusColor = QColor("#8cbb63"); // Xanh lá cây
        } else if (status == "Đã hủy") {
            statusColor = QColor("#d9534f"); // Đỏ
        } else { // "Dự kiến"
            statusColor = QColor("#f0ad4e"); // Vàng cam
        }

        // Vẽ một thanh dọc rộng 4px bên trái, bo góc
        // Hơi thụt vào 1px so với viền để giữ độ bo của nền
        QRectF statusBarRect = rect().adjusted(1, 1, 0, -1);
        statusBarRect.setWidth(4);
        painter->setBrush(statusColor);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(statusBarRect, 3, 3);
    }

    // --- THAY ĐỔI: ĐỊNH NGHĨA VÙNG RESIZE ---
    // Giữ nguyên 5px như logic chuột của bạn
    const int resizeHandleHeight = 5;

    // --- THAY ĐỔI: VÙNG VẼ TEXT GIỜ SẼ TRỪ ĐI VÙNG RESIZE ---
    // Chúng ta trừ đi 5px của tay cầm và 5px padding đáy
    // Dịch lề trái: 5px (bình thường) hoặc 12px (nếu là cuộc họp)
    int leftPadding = (m_eventType == "Cuộc họp") ? 12 : 5;
    QRectF textRect = rect().adjusted(leftPadding, 5, -5, -resizeHandleHeight - 5);

    // Vẽ tiêu đề sự kiện
    painter->setPen(Qt::black);
    QFont font = painter->font();
    font.setPixelSize(12);
    painter->setFont(font);

    // Căn lề và cho phép xuống dòng nếu cần
    QTextOption textOption;
    textOption.setAlignment(Qt::AlignTop | Qt::AlignLeft);
    textOption.setWrapMode(QTextOption::WordWrap);

    painter->drawText(textRect, m_title, textOption);

    // --- THÊM MỚI: VẼ TAY NẮM RESIZE (PHẦN THIẾT KẾ DỄ NHẬN BIẾT) ---
    if (rect().height() > 20) // Chỉ vẽ nếu item đủ cao
    {
        // Định nghĩa vùng tay cầm
        QRectF handleRect(rect().left(), rect().bottom() - resizeHandleHeight, rect().width(), resizeHandleHeight);

        // Vẽ 3 dấu chấm ở giữa
        QPoint center = handleRect.center().toPoint();

        // Lấy màu tối hơn từ màu nền
        QPen dotPen(m_color.darker(150), 2, Qt::DotLine);
        painter->setPen(dotPen);

        int y = center.y(); // Căn y cho các dấu chấm
        painter->drawPoint(center.x() - 4, y);
        painter->drawPoint(center.x(), y);
        painter->drawPoint(center.x() + 4, y);
    }
}

QVariant EventItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    /*if (change == QGraphicsItem::ItemPositionChange && scene()) {
        auto *view = qobject_cast<CalendarView*>(scene()->views().first());
        if (!view) {
            return QGraphicsRectItem::itemChange(change, value);
        }

        QPointF newPos = value.toPointF();
        const double hourHeight = view->getHourHeight();

        // --- CHỈ BẮT DÍNH THEO CHIỀU DỌC (GIỜ) ---
        const double slotHeight = hourHeight / 4.0; // 15 phút
        int timeSlot = qRound(newPos.y() / slotHeight);
        double snappedY = timeSlot * slotHeight;

        // Trả về vị trí với X tự do và Y đã bắt dính
        return QPointF(newPos.x(), snappedY);
    }*/

    return QGraphicsRectItem::itemChange(change, value);
}


// THAY ĐỔI: Logic cập nhật khi thả chuột
void EventItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    auto *view = qobject_cast<CalendarView*>(scene()->views().first());
    if (!view) {
        QGraphicsRectItem::mouseReleaseEvent(event);
        return;
    }

    if (m_isResizing) {
        // --- Kết thúc Resize (Giữ nguyên logic cũ) ---
        m_isResizing = false;
        const double hourHeight = view->getHourHeight();
        int totalMinutes = qRound(rect().height() / hourHeight * 60.0);
        m_endTime = m_startTime.addSecs(totalMinutes * 60);

        // TODO: Logic resize cũng nên hỏi "This/All events"
        // Tạm thời, chúng ta chỉ emit eventChanged
        emit eventChanged(this);

    } else if (m_isMoving) {
        // --- BẮT ĐẦU SỬA LỖI: Kết thúc Move (Drag) ---
        m_isMoving = false;
        this->setBrush(m_color); // Khôi phục màu
        this->setPen(Qt::NoPen);

        if (m_ghostItem) {
            QPointF finalPos = m_ghostItem->pos();
            scene()->removeItem(m_ghostItem);
            delete m_ghostItem;
            m_ghostItem = nullptr;

            // --- TÍNH TOÁN THỜI GIAN MỚI ---
            const double dayWidth = view->getDayWidth();
            const double hourHeight = view->getHourHeight();
            if (dayWidth <= 0) return;

            int maxDayIndex = view->getNumberOfDays() - 1;
            int finalDayIndex = qRound((finalPos.x() - 5) / dayWidth);
            finalDayIndex = qBound(0, finalDayIndex, maxDayIndex);

            const double slotHeight = hourHeight / 4.0;
            int timeSlot = qRound(finalPos.y() / slotHeight);
            int start_minute = timeSlot * 15;
            QTime newTime(start_minute / 60, start_minute % 60);

            QDate monday = view->getMondayOfCurrentWeek();
            QDate newDate = monday.addDays(finalDayIndex);

            long long durationMs = m_startTime.msecsTo(m_endTime);

            // TẠO THỜI GIAN MỚI (NHƯNG KHÔNG GÁN)
            QDateTime newStartTime(newDate, newTime);
            QDateTime newEndTime = newStartTime.addMSecs(durationMs);

            // THAY ĐỔI: Không tự mình thay đổi dữ liệu
            // m_startTime.setDate(newDate); (XÓA)
            // m_startTime.setTime(newTime); (XÓA)
            // m_endTime = m_startTime.addMSecs(durationMs); (XÓA)

            // THAY ĐỔI: Phát tín hiệu lên MainWindow
            emit eventDragged(this, newStartTime, newEndTime);
            // (Không emit eventChanged(this) nữa)
        }
        // --- KẾT THÚC SỬA LỖI ---

    } else {
        // --- Đây là CLICK (Giữ nguyên) ---
        emit clicked(this);

        event->accept(); // (1) Báo rằng chúng ta đã xử lý sự kiện này
        return;          // (2) Dừng hàm tại đây, không lan truyền
    }

    // Reset cờ
    m_isResizing = false;
    m_isMoving = false;
    // (Dòng này bây giờ sẽ chỉ được gọi nếu là drag hoặc resize,
    // không bao giờ được gọi nếu là click)
    QGraphicsRectItem::mouseReleaseEvent(event);
}


// ... các hàm hover và mouse press/move không đổi ...
void EventItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    // Xác định một vùng nhỏ ở cạnh dưới làm "tay cầm" resize
    // (Bạn đang dùng 5px, chúng ta giữ nguyên)
    QRectF handle = QRectF(rect().bottomLeft(), QPointF(rect().right(), rect().bottom() - 5));

    if (handle.contains(event->pos())) {
        // 1. Nếu ở trên tay cầm -> Đổi trỏ chuột resize
        setCursor(Qt::SizeVerCursor);
    } else {
        // 2. THAY ĐỔI: Nếu ở trên thân item -> Đổi trỏ chuột "bàn tay"
        setCursor(Qt::PointingHandCursor);
    }

    QGraphicsRectItem::hoverMoveEvent(event);
}

void EventItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    setCursor(Qt::ArrowCursor); // Trả con trỏ về mặc định khi rời khỏi item
    QGraphicsRectItem::hoverLeaveEvent(event);
}

void EventItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // 1. Kiểm tra resize
    QRectF handle = QRectF(rect().bottomLeft(), QPointF(rect().right(), rect().bottom() - 5));
    if (handle.contains(event->pos())) {
        m_isResizing = true;
        m_isMoving = false;
        QGraphicsRectItem::mousePressEvent(event); // Để logic resize gốc xử lý
    } else {
        // 2. Chuẩn bị để di chuyển (hoặc click)
        m_isResizing = false;
        m_isMoving = false;
        m_dragStartOffset = event->pos();
        m_pressPos = event->scenePos(); // Lưu vị trí nhấn (trên scene)

        // Không tạo ghost vội, chỉ chấp nhận sự kiện
        event->accept();
    }
}

void EventItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_isResizing) {
        // Logic resize giữ nguyên
        auto *view = qobject_cast<CalendarView*>(scene()->views().first());
        if (!view) return;
        double newHeight = event->pos().y();
        const double hourHeight = view->getHourHeight();
        const double slotHeight = hourHeight / 4.0;
        int timeSlots = qRound(newHeight / slotHeight);
        double snappedHeight = timeSlots * slotHeight;
        if (snappedHeight < slotHeight) {
            snappedHeight = slotHeight;
        }
        setRect(0, 0, rect().width(), snappedHeight);

    } else {
        // Logic di chuyển (Move)

        // 1. Kiểm tra xem có phải là drag không (đã di chuyển đủ xa)
        if (!m_isMoving && (event->scenePos() - m_pressPos).manhattanLength() > QApplication::startDragDistance()) {
            m_isMoving = true;

            // 2. Tạo ghost item (CHỈ KHI BẮT ĐẦU DRAG)
            this->setBrush(Qt::transparent); // Ẩn item gốc
            this->setPen(Qt::NoPen);

            m_ghostItem = new QGraphicsRectItem(this->rect());
            m_ghostItem->setBrush(QColor(100, 100, 100, 100));
            m_ghostItem->setPen(QPen(Qt::black, 1, Qt::DashLine));
            m_ghostItem->setZValue(this->zValue() + 1);
            scene()->addItem(m_ghostItem);

            // Đặt vị trí ban đầu
            updateGhostPosition(this->pos());
        }

        // 3. Nếu đang trong trạng thái drag (m_isMoving = true)
        if (m_isMoving && m_ghostItem) {
            QPointF newScenePos = mapToScene(event->pos()) - m_dragStartOffset;
            updateGhostPosition(newScenePos);
        }
        event->accept();
    }
}

// Thêm hàm này vào cuối file eventitem.cpp

/**
 * @brief Tính toán và cập nhật vị trí "bắt dính" (snapped) cho ghost item.
 * @param newScenePos Vị trí mới (chưa bắt dính) trên scene.
 */
void EventItem::updateGhostPosition(QPointF newScenePos)
{
    auto *view = qobject_cast<CalendarView*>(scene()->views().first());
    if (!view || !m_ghostItem) return;

    int maxDayIndex = view->getNumberOfDays() - 1; // Lấy số ngày tối đa
    const double dayWidth = view->getDayWidth(); // Lấy dayWidth ở đây
    const double hourHeight = view->getHourHeight();
    if (dayWidth <= 0) return;

    // 1. Bắt dính Y (Thời gian)
    const double slotHeight = hourHeight / 4.0; // 15 phút
    int timeSlot = qRound(newScenePos.y() / slotHeight);
    double snappedY = timeSlot * slotHeight;

    // 2. Bắt dính X (Ngày)
    int dayIndex = qRound((newScenePos.x() - 5) / dayWidth);
    dayIndex = qBound(0, dayIndex, maxDayIndex);
    double snappedX = (dayIndex * dayWidth + 5);

    // 3. Cập nhật vị trí cho ghost item
    m_ghostItem->setPos(snappedX, snappedY);
}
