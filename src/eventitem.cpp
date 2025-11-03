#include "eventitem.h"
#include "calendarview.h"

#include <QPainter>
#include <QTextOption>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>

// THAY ĐỔI: Sửa hàm khởi tạo
EventItem::EventItem(const QString &title, const QColor &color,
                     const QDateTime &startTime, const QDateTime &endTime, QGraphicsItem *parent)
    : QObject(),
    QGraphicsRectItem(parent),
    m_title(title), m_color(color),
    m_startTime(startTime), m_endTime(endTime),
    m_isResizing(false),
    m_ghostItem(nullptr)
{
    setBrush(m_color);
    setPen(Qt::NoPen);
    setFlag(QGraphicsItem::ItemIsMovable);
    setAcceptHoverEvents(true);
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
void EventItem::updateGeometry(double dayWidth, double hourHeight, int subColumn, int totalSubColumns)
{
    // Lấy ngày trong tuần (Thứ Hai = 1, ..., Chủ Nhật = 7)
    int dayOfWeek = m_startTime.date().dayOfWeek(); // 1-7
    int dayIndex = dayOfWeek - 1; // 0-6

    double subColumnWidth = (dayWidth - 10) / totalSubColumns;
    double x = (dayIndex * dayWidth + 5) + (subColumn * subColumnWidth);

    double y_start = (m_startTime.time().hour() * 60 + m_startTime.time().minute()) / 60.0 * hourHeight;
    double y_end = (m_endTime.time().hour() * 60 + m_endTime.time().minute()) / 60.0 * hourHeight;
    double height = y_end - y_start;

    setPos(x, y_start);
    setRect(0, 0, subColumnWidth, height);
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

    // --- THAY ĐỔI: ĐỊNH NGHĨA VÙNG RESIZE ---
    // Giữ nguyên 5px như logic chuột của bạn
    const int resizeHandleHeight = 5;

    // --- THAY ĐỔI: VÙNG VẼ TEXT GIỜ SẼ TRỪ ĐI VÙNG RESIZE ---
    // Chúng ta trừ đi 5px của tay cầm và 5px padding đáy
    QRectF textRect = rect().adjusted(5, 5, -5, -resizeHandleHeight - 5);

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
    if (!view) return;

    // --- THÊM MỚI: KHÔI PHỤC MÀU SẮC ---
    // Khôi phục lại màu sắc cho item gốc, BẤT KỂ là resize hay move
    this->setBrush(m_color);
    this->setPen(Qt::NoPen); // (hoặc QPen(m_color.darker(120)) nếu bạn muốn)

    if (m_isResizing) {
        // Logic resize release giữ nguyên
        m_isResizing = false;
        const double hourHeight = view->getHourHeight();
        int totalMinutes = qRound(rect().height() / hourHeight * 60.0);
        m_endTime = m_startTime.addSecs(totalMinutes * 60);

    } else if (m_ghostItem) {
        // Nếu đang di chuyển (thả ghost item)
        // 1. Lấy vị trí cuối cùng (đã bắt dính) của ghost
        QPointF finalPos = m_ghostItem->pos();

        // 2. Xóa ghost item
        scene()->removeItem(m_ghostItem);
        delete m_ghostItem;
        m_ghostItem = nullptr;

        // 3. XÓA DÒNG NÀY (Không cần nữa):
        // this->setVisible(true);

        // 4. Chạy logic cập nhật thời gian
        const double dayWidth = view->getDayWidth();
        // ... (phần code còn lại của bạn giữ nguyên) ...
        const double hourHeight = view->getHourHeight();
        if (dayWidth <= 0) return;

        int finalDayIndex = qRound((finalPos.x() - 5) / dayWidth);
        finalDayIndex = qBound(0, finalDayIndex, 6);

        const double slotHeight = hourHeight / 4.0;
        int timeSlot = qRound(finalPos.y() / slotHeight);
        int start_minute = timeSlot * 15;
        QTime newTime(start_minute / 60, start_minute % 60);

        QDate monday = view->getMondayOfCurrentWeek();
        QDate newDate = monday.addDays(finalDayIndex);

        long long durationMs = m_startTime.msecsTo(m_endTime);
        m_startTime.setDate(newDate);
        m_startTime.setTime(newTime);
        m_endTime = m_startTime.addMSecs(durationMs);
    }

    // Phát tín hiệu ở cuối cùng để CalendarView sắp xếp lại
    emit eventChanged(this);
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
    // Kiểm tra xem có nhấn vào "tay cầm" resize không
    QRectF handle = QRectF(rect().bottomLeft(), QPointF(rect().right(), rect().bottom() - 5));
    if (handle.contains(event->pos())) {
        m_isResizing = true;
        // Cho phép base class xử lý resize (vì chúng ta không can thiệp)
        QGraphicsRectItem::mousePressEvent(event);
    } else {
        // Đây là thao tác di chuyển (Move)
        m_isResizing = false;

        // --- THAY ĐỔI QUAN TRỌNG ---
        // 1. Ẩn item gốc BẰNG CÁCH LÀM NÓ TRONG SUỐT
        //    Không dùng setVisible(false), vì nó sẽ làm item mất
        //    khả năng nhận sự kiện move/release.
        this->setBrush(Qt::transparent);
        this->setPen(Qt::NoPen); // Đảm bảo viền cũng trong suốt

        // 2. Lưu vị trí nhấn chuột (để kéo mượt)
        m_dragStartOffset = event->pos();

        // 3. Tạo ghost item
        m_ghostItem = new QGraphicsRectItem(this->rect());
        m_ghostItem->setBrush(QColor(100, 100, 100, 100)); // Màu xám mờ
        m_ghostItem->setPen(QPen(Qt::black, 1, Qt::DashLine)); // Viền nét đứt
        m_ghostItem->setZValue(this->zValue() + 1); // Đảm bảo nó nổi lên trên
        scene()->addItem(m_ghostItem);

        // 4. Đặt vị trí ban đầu cho ghost (đã bắt dính)
        updateGhostPosition(this->pos());

        event->accept(); // Chúng ta tự xử lý, không gọi base class
    }
}

void EventItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_isResizing) {
        // Logic resize giữ nguyên như cũ của bạn
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

    } else if (m_ghostItem) {
        // Nếu đang di chuyển (m_ghostItem tồn tại)
        // 1. Tính vị trí mới trên scene (chưa bắt dính)
        QPointF newScenePos = mapToScene(event->pos()) - m_dragStartOffset;

        // 2. Yêu cầu ghost cập nhật vị trí (hàm này sẽ tự bắt dính)
        updateGhostPosition(newScenePos);

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

    const double dayWidth = view->getDayWidth();
    const double hourHeight = view->getHourHeight();
    if (dayWidth <= 0) return;

    // 1. Bắt dính Y (Thời gian)
    const double slotHeight = hourHeight / 4.0; // 15 phút
    int timeSlot = qRound(newScenePos.y() / slotHeight);
    double snappedY = timeSlot * slotHeight;

    // 2. Bắt dính X (Ngày)
    int dayIndex = qRound((newScenePos.x() - 5) / dayWidth);
    dayIndex = qBound(0, dayIndex, 6); // Giới hạn từ 0 (Thứ 2) đến 6 (Chủ Nhật)
    double snappedX = (dayIndex * dayWidth + 5);

    // 3. Cập nhật vị trí cho ghost item
    m_ghostItem->setPos(snappedX, snappedY);
}
