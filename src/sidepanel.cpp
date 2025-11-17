#include "sidepanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation> // Cần cho animation trượt
#include <QScrollArea>      // Cần để bọc nội dung
#include <QSpacerItem>
#include <QGraphicsDropShadowEffect> // Cần cho hiệu ứng đổ bóng
#include <QEasingCurve>    // Cần cho animation mượt mà

/**
 * @brief Hàm dựng (Constructor) của SidePanel.
 * Tạo ra "khung" của panel (Thanh tiêu đề, Nút đóng, Bóng đổ, Animation).
 * @param title Tiêu đề sẽ hiển thị trên thanh tiêu đề (ví dụ: "Trợ giúp").
 * @param parent Widget cha (thường là MainWindow).
 */
SidePanel::SidePanel(const QString &title, QWidget *parent)
    : QWidget(parent)
{
    // Đặt tên đối tượng để có thể style bằng QSS (ví dụ: #sidePanel { ... })
    setObjectName("sidePanel");
    setMinimumWidth(400); // Đặt chiều rộng tối thiểu

    // 1. Tạo hiệu ứng đổ bóng để panel trông "nổi" trên cửa sổ chính
    auto *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(25);
    shadowEffect->setXOffset(0);
    shadowEffect->setYOffset(0);
    shadowEffect->setColor(QColor(0, 0, 0, 100)); // Màu đen mờ
    setGraphicsEffect(shadowEffect);

    // 2. Tạo Layout chính (dọc) cho toàn bộ panel
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 10); // Không lề trên/trái/phải
    m_mainLayout->setSpacing(0);                   // Không khoảng cách

    // 3. Tạo Thanh Tiêu đề (Title Bar) tùy chỉnh
    QWidget *titleBar = new QWidget();
    titleBar->setObjectName("sidePanelTitleBar"); // Đặt tên để style QSS
    titleBar->setLayout(new QHBoxLayout());
    titleBar->layout()->setContentsMargins(15, 8, 8, 8);

    QLabel *titleLabel = new QLabel(title);
    titleLabel->setObjectName("sidePanelTitle"); // Đặt tên để style QSS

    QPushButton *btnClose = new QPushButton("×"); // Nút X (đóng)
    btnClose->setObjectName("sidePanelCloseButton"); // Đặt tên để style QSS
    btnClose->setCursor(Qt::PointingHandCursor);

    // Thêm Tiêu đề, một khoảng trống co giãn, và Nút đóng vào titleBar
    titleBar->layout()->addWidget(titleLabel);
    titleBar->layout()->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    titleBar->layout()->addWidget(btnClose);

    // Thêm titleBar vào layout chính (ở trên cùng)
    m_mainLayout->addWidget(titleBar);

    // 4. Khởi tạo Animation
    // Chúng ta sẽ "lái" (animate) thuộc tính "geometry" (vị trí và kích thước)
    m_animation = new QPropertyAnimation(this, "geometry", this);
    m_animation->setDuration(300); // 300ms (0.3 giây)
    m_animation->setEasingCurve(QEasingCurve::InOutCubic); // Hiệu ứng mượt (chậm-nhanh-chậm)

    // 5. Kết nối Nút đóng
    // Khi nhấn nút 'btnClose', gọi hàm toggleVisibility
    connect(btnClose, &QPushButton::clicked, this, [this, parent](){
        if(parent) {
            // Lấy chiều cao của topBar (để panel trượt ra không đè lên)
            QWidget* topBar = parent->findChild<QWidget*>("topBar");
            int topBarHeight = topBar ? topBar->height() : 0;
            toggleVisibility(parent->geometry(), topBarHeight);
        }
    });
}

/**
 * @brief "Bơm" (Inject) nội dung (đã được tạo ở MainWindow) vào panel.
 * Hàm này bọc layout nội dung đó vào một QScrollArea.
 * @param contentLayout Layout chứa nội dung (ví dụ: các QGroupBox Trợ giúp).
 */
void SidePanel::setContentLayout(QLayout *contentLayout)
{
    // 1. Tạo một vùng cuộn (QScrollArea)
    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true); // Cho phép widget con co giãn theo
    scrollArea->setObjectName("sidePanelScrollArea"); // Đặt tên để style QSS
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // Tắt cuộn ngang

    // 2. Tạo một widget "container" để chứa layout nội dung
    QWidget *contentWidget = new QWidget;
    contentWidget->setLayout(contentLayout); // Đặt layout của MainWindow vào

    // (Style này đảm bảo nền của nội dung là màu trắng,
    // ngay cả khi QSS chính không đặt)
    contentWidget->setStyleSheet("background-color: #ffffff;");

    // 3. Đặt contentWidget làm "con" của scrollArea
    scrollArea->setWidget(contentWidget);

    // 4. Thêm scrollArea (chứa nội dung) vào layout chính của panel
    m_mainLayout->addWidget(scrollArea);
}

/**
 * @brief Hàm logic chính: Trượt panel vào hoặc trượt panel ra.
 * @param parentGeo Kích thước và vị trí của cửa sổ cha (MainWindow).
 * @param topBarHeight Chiều cao của thanh topBar (để panel không đè lên).
 */
void SidePanel::toggleVisibility(const QRect &parentGeo, int topBarHeight)
{
    if (m_animation->state() == QAbstractAnimation::Running) return; // Đang chạy, không làm gì cả

    int panelWidth = this->width();
    int parentWidth = parentGeo.width();
    int parentHeight = parentGeo.height();
    int panelHeight = parentHeight - topBarHeight; // Chiều cao của panel

    if (isHidden()) {
        // --- LOGIC TRƯỢT VÀO (Slide In) ---
        raise(); // Đưa panel lên trên cùng
        show();  // Hiển thị (vẫn ở vị trí cũ)

        // Ngắt kết nối cũ (nếu có)
        disconnect(m_animation, &QAbstractAnimation::finished, this, &QWidget::hide);

        // Bắt đầu: Bên ngoài màn hình (cạnh phải)
        m_animation->setStartValue(QRect(parentWidth, topBarHeight, panelWidth, panelHeight));
        // Kết thúc: Bên trong màn hình (cạnh phải)
        m_animation->setEndValue(QRect(parentWidth - panelWidth, topBarHeight, panelWidth, panelHeight));
    } else {
        // --- LOGIC TRƯỢT RA (Slide Out) ---

        // Kỹ thuật quan trọng: Kết nối tín hiệu 'finished' của animation
        // với slot 'hide' của widget.
        // -> Widget sẽ tự động ẩn SAU KHI animation trượt ra kết thúc.
        connect(m_animation, &QAbstractAnimation::finished, this, &QWidget::hide);

        // Bắt đầu: Vị trí hiện tại (bên trong)
        m_animation->setStartValue(geometry());
        // Kết thúc: Bên ngoài màn hình (cạnh phải)
        m_animation->setEndValue(QRect(parentWidth, topBarHeight, panelWidth, panelHeight));
    }
    m_animation->start(); // Bắt đầu chạy animation
}

/**
 * @brief Hàm "Ép" (Force) ẩn panel (chỉ trượt ra).
 * Được MainWindow sử dụng để đóng các panel khác khi một panel mới được mở.
 */
void SidePanel::hidePanel(const QRect &parentGeo, int topBarHeight)
{
    // Nếu đã ẩn, hoặc đang chạy animation, thì không làm gì cả
    if (isHidden() || m_animation->state() == QAbstractAnimation::Running) return;

    // (Giống như logic "Trượt ra" ở trên)
    connect(m_animation, &QAbstractAnimation::finished, this, &QWidget::hide);
    m_animation->setStartValue(geometry());
    m_animation->setEndValue(QRect(parentGeo.width(), topBarHeight, width(), parentGeo.height() - topBarHeight));
    m_animation->start();
}
