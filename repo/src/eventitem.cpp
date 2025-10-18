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
    m_isResizing(false)
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

    // Vẽ tiêu đề sự kiện
    painter->setPen(Qt::white);
    QFont font = painter->font();
    font.setPixelSize(12);
    painter->setFont(font);

    // Căn lề và cho phép xuống dòng nếu cần
    QTextOption textOption;
    textOption.setAlignment(Qt::AlignTop | Qt::AlignLeft);
    textOption.setWrapMode(QTextOption::WordWrap);

    // Thêm một chút padding
    QRectF textRect = rect().adjusted(5, 5, -5, -5);
    painter->drawText(textRect, m_title, textOption);
}

QVariant EventItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionChange && scene()) {
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
    }

    return QGraphicsRectItem::itemChange(change, value);
}


// THAY ĐỔI: Logic cập nhật khi thả chuột
void EventItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    auto *view = qobject_cast<CalendarView*>(scene()->views().first());
    if (!view) return;

    if (m_isResizing) {
        m_isResizing = false;
        const double hourHeight = view->getHourHeight();

        // Tính tổng số phút từ chiều cao mới
        int totalMinutes = qRound(rect().height() / hourHeight * 60.0);
        // Cập nhật thời gian kết thúc
        m_endTime = m_startTime.addSecs(totalMinutes * 60);

    } else {
        QGraphicsRectItem::mouseReleaseEvent(event);

        const double dayWidth = view->getDayWidth();
        const double hourHeight = view->getHourHeight();
        if (dayWidth <= 0) return;

        QPointF currentPos = pos();
        // Tính toán chỉ số ngày mới (0-6)
        int finalDayIndex = qRound((currentPos.x() - 5) / dayWidth);
        finalDayIndex = qBound(0, finalDayIndex, 6);

        // Tính toán thời gian bắt đầu mới
        const double slotHeight = hourHeight / 4.0;
        int timeSlot = qRound(currentPos.y() / slotHeight);
        int start_minute = timeSlot * 15;
        QTime newTime(start_minute / 60, start_minute % 60);

        // Lấy ngày bắt đầu của tuần từ CalendarView
        QDate monday = view->getMondayOfCurrentWeek();
        QDate newDate = monday.addDays(finalDayIndex);

        // Tính toán khoảng thời gian của sự kiện
        long long durationMs = m_startTime.msecsTo(m_endTime);

        // Cập nhật lại thời gian bắt đầu và kết thúc
        m_startTime.setDate(newDate);
        m_startTime.setTime(newTime);
        m_endTime = m_startTime.addMSecs(durationMs);
    }

    emit eventChanged(this);
}


// ... các hàm hover và mouse press/move không đổi ...
void EventItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    // Xác định một vùng nhỏ ở cạnh dưới làm "tay cầm" resize
    QRectF handle = QRectF(rect().bottomLeft(), QPointF(rect().right(), rect().bottom() - 5));
    if (handle.contains(event->pos())) {
        setCursor(Qt::SizeVerCursor);
    } else {
        setCursor(Qt::ArrowCursor);
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
    } else {
        // Nếu không thì đây là thao tác di chuyển
        QGraphicsRectItem::mousePressEvent(event);
    }
}

void EventItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_isResizing) {
        auto *view = qobject_cast<CalendarView*>(scene()->views().first());
        if (!view) return;

        // Lấy chiều cao mới từ vị trí chuột
        double newHeight = event->pos().y();

        // Bắt dính chiều cao vào các mốc 15 phút
        const double hourHeight = view->getHourHeight();
        const double slotHeight = hourHeight / 4.0;
        int timeSlots = qRound(newHeight / slotHeight);
        double snappedHeight = timeSlots * slotHeight;

        // Giới hạn chiều cao tối thiểu là 15 phút
        if (snappedHeight < slotHeight) {
            snappedHeight = slotHeight;
        }

        // Cập nhật lại hình chữ nhật của item
        setRect(0, 0, rect().width(), snappedHeight);
    } else {
        // Nếu không phải resize thì là di chuyển
        QGraphicsRectItem::mouseMoveEvent(event);
    }
}
