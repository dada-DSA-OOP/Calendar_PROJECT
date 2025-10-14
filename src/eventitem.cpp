#include "eventitem.h"
#include "calendarview.h"

#include <QPainter>
#include <QTextOption>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>

// Sửa lại hàm khởi tạo để event thay đổi kích thước theo màn hình main
EventItem::EventItem(const QString &title, const QColor &color, int day,
                     const QTime &startTime, const QTime &endTime, QGraphicsItem *parent)
    : QObject(), // <-- Thêm dòng này để gọi constructor của QObject
    QGraphicsRectItem(parent), // Hàm này đã có
    m_title(title), m_color(color),
    m_day(day), m_startTime(startTime), m_endTime(endTime),
    m_isResizing(false)
{
    setBrush(m_color);
    setPen(Qt::NoPen);

    //Qt cho phép set cờ để dễ dàng kéo thả
    setFlag(QGraphicsItem::ItemIsMovable);
    setAcceptHoverEvents(true); // <-- Bật hover events
}

// Thêm hàm mới này - logic tính toán được chuyển vào đây
void EventItem::updateGeometry(double dayWidth, double hourHeight, int subColumn, int totalSubColumns)
{
    // Tính toán chiều rộng và vị trí x của cột phụ
    double subColumnWidth = (dayWidth - 10) / totalSubColumns; // -10 là padding
    double x = (day() * dayWidth + 5) + (subColumn * subColumnWidth);

    // Phần tính toán y và height giữ nguyên
    double y_start = (m_startTime.hour() * 60 + m_startTime.minute()) / 60.0 * hourHeight;
    double y_end = (m_endTime.hour() * 60 + m_endTime.minute()) / 60.0 * hourHeight;
    double height = y_end - y_start;

    setPos(x, y_start);
    setRect(0, 0, subColumnWidth, height);
}

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

void EventItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_isResizing) {
        // Nếu đang resize, thì kết thúc resize
        m_isResizing = false;

        // Cập nhật lại thời gian kết thúc (m_endTime)
        auto *view = qobject_cast<CalendarView*>(scene()->views().first());
        if (!view) return;

        const double hourHeight = view->getHourHeight();
        int totalMinutes = qRound(rect().height() / hourHeight * 60.0);
        long long durationMs = totalMinutes * 60 * 1000;
        m_endTime = m_startTime.addMSecs(durationMs);

    } else {
        // Nếu không, xử lý như một thao tác di chuyển (move)
        // Code này được copy và điều chỉnh từ phiên bản trước
        QGraphicsRectItem::mouseReleaseEvent(event);

        auto *view = qobject_cast<CalendarView*>(scene()->views().first());
        if (!view) return;

        const double dayWidth = view->getDayWidth();
        const double hourHeight = view->getHourHeight();
        if (dayWidth <= 0) return;

        QPointF currentPos = pos();
        int finalDay = qRound((currentPos.x() - 5) / dayWidth);
        finalDay = qBound(0, finalDay, 6);
        double finalX = finalDay * dayWidth + 5;

        const double slotHeight = hourHeight / 4.0;
        int timeSlot = qRound(currentPos.y() / slotHeight);
        double finalY = timeSlot * slotHeight;

        setPos(finalX, finalY);

        m_day = finalDay;

        int start_minute = qRound(finalY / hourHeight * 60.0);
        QTime newStartTime(start_minute / 60, start_minute % 60);

        long long duration = m_startTime.msecsTo(m_endTime);
        m_startTime = newStartTime;
        m_endTime = m_startTime.addMSecs(duration);
    }
    emit eventChanged(this); // <-- Thêm dòng này vào cuối hàm
}

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
