#include "timetableslotwidget.h"
#include "eventitem.h" // Cần để lấy thông tin (màu sắc, tiêu đề)
#include <QVBoxLayout>
#include <QLabel>
#include <QEvent>     // Cần cho QEvent (trong eventFilter)
#include <QVariant>   // Cần để lưu con trỏ EventItem* vào property

// =================================================================================
// === 1. HÀM DỰNG (CONSTRUCTOR)
// =================================================================================

/**
 * @brief Hàm dựng của TimetableSlotWidget.
 * Thiết lập một QFrame với layout dọc (QVBoxLayout) để chứa các nhãn sự kiện.
 */
TimetableSlotWidget::TimetableSlotWidget(QWidget *parent)
    : QFrame(parent)
{
    // 1. Cài đặt style cơ bản cho ô (viền)
    setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    setLineWidth(1);
    setStyleSheet("QFrame { border-top: 1px solid #cccccc; border-left: 1px solid #cccccc; }");

    // 2. Tạo layout dọc (QVBoxLayout)
    m_eventsLayout = new QVBoxLayout(this);
    m_eventsLayout->setContentsMargins(4, 4, 4, 4);
    m_eventsLayout->setSpacing(2);
    // Thêm một "khoảng co giãn" (stretch) ở cuối
    // -> Đẩy tất cả các nhãn sự kiện dồn lên trên cùng của ô
    m_eventsLayout->addStretch(1);

    // 3. Đảm bảo ô có thể co giãn
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumHeight(50); // Chiều cao tối thiểu cho 1 tiết
}

// =================================================================================
// === 2. API CÔNG KHAI (PUBLIC API)
// =================================================================================

/**
 * @brief Thêm một sự kiện vào ô TKB này.
 * Tạo một QLabel rút gọn và thêm vào layout.
 *
 * @note Chỉ hiển thị tối đa 2 sự kiện, sau đó hiển thị "... thêm".
 */
void TimetableSlotWidget::addEvent(EventItem *event)
{
    m_events.append(event); // Thêm vào danh sách dữ liệu (để tham khảo)

    // Chỉ hiển thị 1-2 sự kiện
    if (m_eventsLayout->count() < 2) {
        QString title = event->title();

        QLabel *eventLabel = new QLabel(title);
        eventLabel->setStyleSheet(QString("background-color: %1; color: black; padding: 3px; border-radius: 3px; font-size: 8pt;")
                                      .arg(event->color().name()));
        eventLabel->setWordWrap(true); // Cho phép xuống dòng nếu tiêu đề quá dài

        // --- PHẦN QUAN TRỌNG ĐỂ BẮT CLICK SỰ KIỆN ---
        eventLabel->setCursor(Qt::PointingHandCursor); // (1) Đổi con trỏ chuột

        // (2) Lưu con trỏ EventItem* vào bên trong QLabel
        //     (để khi click có thể biết là click vào sự kiện nào)
        eventLabel->setProperty("eventItem", QVariant::fromValue(event));

        // (3) Đăng ký eventFilter để widget này (TimetableSlotWidget)
        //     có thể "theo dõi" các cú click lên eventLabel
        eventLabel->installEventFilter(this);
        // ---------------

        // Thêm nhãn vào layout (chèn *trước* cái addStretch)
        m_eventsLayout->insertWidget(m_eventsLayout->count() - 1, eventLabel);

    } else if (m_eventsLayout->count() == 2) {
        // Nếu đã đủ 2 sự kiện, hiển thị "... thêm"
        QLabel *moreLabel = new QLabel("... thêm");
        moreLabel->setStyleSheet("font-size: 8pt; color: #555;");

        // Chèn nhãn "... thêm" vào trước addStretch
        m_eventsLayout->insertWidget(m_eventsLayout->count() - 1, moreLabel);
    }
    // Nếu > 2 (tức là đã có "... thêm") thì không làm gì cả
}

/**
 * @brief Xóa SẠCH tất cả sự kiện (chỉ UI) khỏi ô.
 * Được gọi bởi view cha (TimetableView/SessionView) khi update.
 */
void TimetableSlotWidget::clearEvents()
{
    m_events.clear(); // Xóa danh sách dữ liệu

    // Xóa tất cả các widget (QLabel) khỏi layout,
    // (trừ addStretch ở cuối)
    QLayoutItem *child;
    while ((child = m_eventsLayout->takeAt(0)) != nullptr) {
        // Kiểm tra xem nó có phải là widget không (để không xóa stretch item)
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }
}

// =================================================================================
// === 3. HÀM SỰ KIỆN (PROTECTED EVENT HANDLERS)
// =================================================================================

/**
 * @brief Bộ lọc sự kiện, được dùng để bắt cú click lên *event label* (QLabel).
 * @return 'true' nếu đã xử lý sự kiện, 'false' nếu để sự kiện tiếp tục.
 */
bool TimetableSlotWidget::eventFilter(QObject *watched, QEvent *event)
{
    // 1. Chỉ quan tâm đến sự kiện nhấn chuột (thả chuột trái)
    if (event->type() == QEvent::MouseButtonRelease)
    {
        // 2. Kiểm tra xem đối tượng có phải là QLabel không
        if (qobject_cast<QLabel*>(watched))
        {
            // 3. Lấy lại con trỏ EventItem* đã lưu bằng setProperty
            QVariant eventData = watched->property("eventItem");
            if (eventData.isValid()) {
                EventItem *item = eventData.value<EventItem*>();
                if (item) {
                    // 4. Phát tín hiệu eventClicked(EventItem*)
                    //    để view cha (và MainWindow)
                    //    mở dialog SỬA sự kiện này.
                    emit eventClicked(item);
                    return true; // Đã xử lý, dừng sự kiện tại đây
                }
            }
        }
    }

    // Chuyển tiếp tất cả sự kiện khác cho lớp cha xử lý
    return QFrame::eventFilter(watched, event);
}
