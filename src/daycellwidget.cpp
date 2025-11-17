#include "daycellwidget.h"
#include "eventitem.h" // Cần để lấy thông tin (màu sắc, tiêu đề)
#include <QVBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QDebug>
#include <QEvent>
#include <QVariant> // Cần để lưu con trỏ EventItem* vào property

// =================================================================================
// === 1. HÀM DỰNG (CONSTRUCTOR)
// =================================================================================

/**
 * @brief Hàm dựng của DayCellWidget.
 * Thiết lập layout ban đầu, bao gồm m_dateLabel (số ngày),
 * m_eventsContainer (danh sách sự kiện) và
 * m_compactEventsIndicator (dấu chấm tròn '●' khi thu gọn).
 */
DayCellWidget::DayCellWidget(QWidget *parent)
    : QFrame(parent)
{
    // Thiết lập style cơ bản cho ô (viền)
    setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    setLineWidth(1);
    setStyleSheet("QFrame { border-top: 1px solid #cccccc; border-left: 1px solid #cccccc; }");

    // Layout chính (dọc)
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(2);

    // Nhãn hiển thị số ngày (ví dụ: "15")
    m_dateLabel = new QLabel("1");
    m_dateLabel->setAlignment(Qt::AlignTop | Qt::AlignRight);
    m_dateLabel->setStyleSheet("font-weight: 500;");

    // Container cho danh sách sự kiện (để dễ dàng ẩn/hiện)
    m_eventsContainer = new QWidget;
    m_eventsLayout = new QVBoxLayout(m_eventsContainer);
    m_eventsLayout->setContentsMargins(0, 0, 0, 0);
    m_eventsLayout->setSpacing(2);

    // Dấu chấm '●' (indicator) khi thu gọn
    // (Đây là một QLabel rỗng được bo tròn bằng QSS)
    m_compactEventsIndicator = new QLabel("");
    m_compactEventsIndicator->setFixedSize(8, 8);
    m_compactEventsIndicator->setStyleSheet(
        "QLabel { "
        "  border-radius: 4px; "        // Nửa chiều rộng/cao để thành hình tròn
        "  background-color: #555555; " // Màu xám đậm mặc định
        "}"
        );
    m_compactEventsIndicator->setAlignment(Qt::AlignCenter);
    m_compactEventsIndicator->setVisible(false); // Ẩn đi ban đầu

    // Thêm các widget vào layout chính
    layout->addWidget(m_dateLabel);
    layout->addWidget(m_eventsContainer, 1); // Cho phép layout sự kiện co giãn
    layout->addWidget(m_compactEventsIndicator, 1); // Dấu chấm cũng co giãn

    setCursor(Qt::PointingHandCursor);
    setMinimumHeight(60); // Đảm bảo ô không bị bóp
}

// =================================================================================
// === 2. CÁC HÀM CÔNG KHAI (PUBLIC FUNCTIONS / SLOTS)
// =================================================================================

/**
 * @brief Đặt ngày cho ô này và áp dụng style.
 * @param date Ngày (ví dụ: 1, 2, 3...)
 * @param isCurrentMonth 'true' nếu ngày này thuộc tháng đang xem (màu đen),
 * 'false' nếu thuộc tháng trước/sau (màu xám).
 */
void DayCellWidget::setDate(const QDate &date, bool isCurrentMonth)
{
    m_date = date;
    m_dateLabel->setText(QString::number(date.day()));

    if (date == QDate::currentDate()) {
        // Style đặc biệt cho "Hôm nay"
        m_dateLabel->setStyleSheet("font-weight: bold; color: #0078d7; padding: 2px 5px; background-color: #e5f3ff; border-radius: 4px;");
    } else if (isCurrentMonth) {
        // Style cho ngày trong tháng
        m_dateLabel->setStyleSheet("font-weight: 500; color: black;");
    } else {
        // Style cho ngày mờ (tháng trước/sau)
        m_dateLabel->setStyleSheet("font-weight: 400; color: #aaaaaa;");
    }

    clearEvents(); // Xóa sự kiện cũ khi đặt ngày mới
}

/**
 * @brief Thêm một sự kiện vào ô ngày này.
 * Hàm này thêm EventItem* vào danh sách m_events VÀ
 * tạo một QLabel (nhãn) tương ứng để hiển thị trong m_eventsLayout.
 *
 * @note Chỉ hiển thị tối đa 3 sự kiện, sau đó hiển thị "... thêm".
 * @note Cập nhật màu của m_compactEventsIndicator (dấu chấm)
 * theo màu của sự kiện ĐẦU TIÊN.
 */
void DayCellWidget::addEvent(EventItem *event)
{
    m_events.append(event); // Thêm vào danh sách dữ liệu

    // --- Cập nhật màu cho dấu chấm (nếu đây là sự kiện đầu tiên) ---
    if (m_events.size() == 1)
    {
        QColor firstEventColor = event->color();
        m_compactEventsIndicator->setStyleSheet(
            QString("QLabel { "
                    "  border-radius: 4px; "
                    "  background-color: %1; "
                    "}")
                .arg(firstEventColor.name(QColor::HexRgb))
            );
    }

    // --- Logic tạo QLabel hiển thị ---
    if (m_eventsLayout->count() < 3) {
        // Hiển thị 3 sự kiện đầu tiên
        QString title = event->title();
        if (title.length() > 20) {
            title = title.left(17) + "..."; // Rút gọn tiêu đề
        }

        QLabel *eventLabel = new QLabel(title);
        eventLabel->setStyleSheet(QString("background-color: %1; color: black; padding: 2px; border-radius: 3px; font-size: 8pt;")
                                      .arg(event->color().name()));

        // --- PHẦN QUAN TRỌNG ĐỂ BẮT CLICK SỰ KIỆN ---
        eventLabel->setCursor(Qt::PointingHandCursor); // (1) Đổi con trỏ chuột

        // (2) Lưu con trỏ EventItem* vào bên trong QLabel
        //     để khi click có thể biết là click vào sự kiện nào.
        eventLabel->setProperty("eventItem", QVariant::fromValue(event));

        // (3) Đăng ký eventFilter để widget này (DayCellWidget)
        //     có thể "theo dõi" các cú click lên eventLabel
        eventLabel->installEventFilter(this);
        // ---------------

        m_eventsLayout->addWidget(eventLabel);
    } else if (m_eventsLayout->count() == 3) {
        // Nếu đã đủ 3 sự kiện, hiển thị "... thêm"
        QLabel *moreLabel = new QLabel("... thêm");
        moreLabel->setStyleSheet("font-size: 8pt; color: #555;");
        m_eventsLayout->addWidget(moreLabel);
    }
    // Nếu > 4 thì không làm gì cả

    // Cập nhật lại hiển thị (thu gọn/đầy đủ)
    updateEventDisplay(this->size());
}

/**
 * @brief Xóa một sự kiện khỏi ô.
 * Do việc xóa 1 QLabel phức tạp, chiến lược ở đây là:
 * 1. Xóa sự kiện khỏi m_events (danh sách dữ liệu).
 * 2. Xóa TẤT CẢ QLabel khỏi m_eventsLayout (dọn dẹp UI).
 * 3. Chạy lại addEvent() cho các sự kiện còn lại để "vẽ lại" UI.
 */
void DayCellWidget::removeEvent(EventItem *event)
{
    if (!event || !m_events.contains(event)) return;

    // 1. Xóa khỏi danh sách dữ liệu
    m_events.removeAll(event);

    // 2. Xóa tất cả các widget QLabel khỏi layout (dọn dẹp UI)
    QLayoutItem *child;
    while ((child = m_eventsLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    // 3. Chạy lại logic addEvent cho các sự kiện còn lại (Xây dựng lại UI)
    QList<EventItem*> remainingEvents = m_events;
    m_events.clear(); // Xóa list tạm thời để addEvent chạy đúng

    for (EventItem* evt : remainingEvents) {
        addEvent(evt); // Gọi lại addEvent, nó sẽ tự điền lại m_events
    }

    // 4. Cập nhật lại màu sắc của dấu chấm
    if (m_events.isEmpty()) {
        // Trả về màu mặc định nếu không còn sự kiện
        m_compactEventsIndicator->setStyleSheet(
            "QLabel { border-radius: 4px; background-color: #555555; }"
            );
    } else {
        // Lấy màu của sự kiện đầu tiên mới
        QColor firstEventColor = m_events.first()->color();
        m_compactEventsIndicator->setStyleSheet(
            QString("QLabel { border-radius: 4px; background-color: %1; }")
                .arg(firstEventColor.name(QColor::HexRgb))
            );
    }

    updateEventDisplay(this->size());
}

/**
 * @brief Xóa SẠCH tất cả sự kiện (cả dữ liệu và UI) khỏi ô.
 */
void DayCellWidget::clearEvents()
{
    m_events.clear(); // Xóa dữ liệu

    // Trả dấu chấm về màu mặc định
    m_compactEventsIndicator->setStyleSheet(
        "QLabel { "
        "  border-radius: 4px; "
        "  background-color: #555555; "
        "}"
        );

    // Xóa các QLabel khỏi UI
    QLayoutItem *child;
    while ((child = m_eventsLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    updateEventDisplay(this->size());
}

// =================================================================================
// === 3. HÀM SỰ KIỆN (PROTECTED EVENT HANDLERS)
// =================================================================================

/**
 * @brief Được gọi khi widget thay đổi kích thước.
 * Đây là hàm kích hoạt (trigger) cho logic responsive.
 */
void DayCellWidget::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event); // Gọi hàm của lớp cha

    // Gọi hàm kiểm tra xem nên hiển thị (danh sách) hay (dấu chấm)
    updateEventDisplay(event->size());
}

/**
 * @brief Được gọi khi click vào *bản thân ô* (vùng trống).
 * Phát tín hiệu cellClicked(QDate, ...) để MonthViewWidget
 * có thể mở dialog tạo sự kiện MỚI.
 */
void DayCellWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // Phát tín hiệu mang theo Ngày và Danh sách sự kiện (để dialog biết)
        emit cellClicked(m_date, m_events);
    }
    QFrame::mouseReleaseEvent(event);
}

/**
 * @brief Bộ lọc sự kiện, được dùng để bắt cú click lên *event label* (QLabel).
 * @return 'true' nếu đã xử lý sự kiện, 'false' nếu để sự kiện tiếp tục.
 */
bool DayCellWidget::eventFilter(QObject *watched, QEvent *event)
{
    // 1. Chỉ quan tâm đến sự kiện nhấn chuột (thả chuột trái)
    if (event->type() == QEvent::MouseButtonRelease)
    {
        // 2. Kiểm tra xem có phải là QLabel (eventLabel) mà chúng ta theo dõi không
        if (qobject_cast<QLabel*>(watched))
        {
            // 3. Lấy lại con trỏ EventItem* đã lưu bằng setProperty
            QVariant eventData = watched->property("eventItem");
            if (eventData.isValid()) {
                EventItem *item = eventData.value<EventItem*>();
                if (item) {
                    // 4. Phát tín hiệu eventClicked(EventItem*)
                    //    để MonthViewWidget (và MainWindow)
                    //    mở dialog SỬA sự kiện này.
                    emit eventClicked(item);
                    return true; // Đã xử lý, dừng sự kiện tại đây
                }
            }
        }
    }

    // Chuyển tiếp tất cả sự kiện khác cho lớp cha xử lý
    // (ví dụ: click vào "... thêm" cũng sẽ được coi là click vào ô)
    return QFrame::eventFilter(watched, event);
}

// =================================================================================
// === 4. HÀM NỘI BỘ (PRIVATE HELPERS)
// =================================================================================

/**
 * @brief Logic responsive chính: Quyết định hiện danh sách hay dấu chấm.
 * @param newSize Kích thước mới của ô.
 */
void DayCellWidget::updateEventDisplay(const QSize &newSize)
{
    bool hasEvents = !m_events.isEmpty();

    // Kiểm tra xem có nên vào chế độ "thu gọn" (compact) không
    // (Chiều cao nhỏ hơn 75px)
    bool isCompact = (newSize.height() < m_compactThreshold);

    // 1. Hiện/ẩn dấu chấm "●"
    // Chỉ hiện khi: (ở chế độ thu gọn VÀ có sự kiện)
    m_compactEventsIndicator->setVisible(isCompact && hasEvents);

    // 2. Hiện/ẩn danh sách sự kiện đầy đủ
    // Chỉ hiện khi: (KHÔNG ở chế độ thu gọn)
    m_eventsContainer->setVisible(!isCompact);
}
