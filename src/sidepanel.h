#ifndef SIDEPANEL_H
#define SIDEPANEL_H

#include <QWidget> // Lớp cơ sở (bắt buộc)
#include <QRect>   // (Thêm) Cần cho kiểu dữ liệu QRect trong public slots

// --- Forward Declarations (Khai báo trước) ---
// (Khai báo tên lớp để tránh include .h đầy đủ, tăng tốc độ biên dịch)
class QPropertyAnimation;
class QVBoxLayout;
class QLayout; // (Thêm) Cần cho setContentLayout

/**
 * @brief Lớp SidePanel là một widget "khung" (frame) đa năng.
 *
 * Lớp này cung cấp một panel có thể trượt (slide) vào/ra từ cạnh phải
 * của cửa sổ cha. Nó tự tạo thanh tiêu đề (title bar) và nút đóng,
 * và cho phép "bơm" (inject) một layout nội dung tùy chỉnh (từ MainWindow)
 * vào một vùng có thể cuộn (QScrollArea).
 */
class SidePanel : public QWidget
{
    // Q_OBJECT là bắt buộc để sử dụng signals và slots
    Q_OBJECT

public:
    // === 1. HÀM DỰNG & API CÔNG KHAI ===
    // (Các hàm mà MainWindow sẽ gọi)

    /**
     * @brief Hàm dựng (Constructor).
     * @param title Tiêu đề sẽ hiển thị trên thanh tiêu đề của panel.
     * @param parent Widget cha (thường là MainWindow).
     */
    explicit SidePanel(const QString &title, QWidget *parent = nullptr);

    /**
     * @brief "Bơm" (Inject) một layout nội dung (đã được tạo ở MainWindow) vào panel.
     * Panel sẽ tự động bọc layout này trong một QScrollArea.
     * @param layout Layout chứa nội dung (ví dụ: các QGroupBox Trợ giúp).
     */
    void setContentLayout(QLayout *layout);

public slots:
    // === 2. CÁC SLOT CÔNG KHAI (PUBLIC SLOTS) ===
    // (Các hàm "phản ứng" với MainWindow)

    /**
     * @brief Hàm logic chính: Trượt panel vào hoặc trượt panel ra.
     * @param parentGeo Kích thước và vị trí của cửa sổ cha (MainWindow).
     * @param topBarHeight Chiều cao của thanh topBar (để panel không đè lên).
     */
    void toggleVisibility(const QRect &parentGeo, int topBarHeight);

    /**
     * @brief Hàm "Ép" (Force) ẩn panel (chỉ trượt ra).
     * Dùng để đóng các panel khác khi một panel mới được mở.
     */
    void hidePanel(const QRect &parentGeo, int topBarHeight);

private:
    // === 3. BIẾN THÀNH VIÊN (MEMBER VARIABLES) ===

    /**
     * @brief Đối tượng animation chịu trách nhiệm "lái" thuộc tính "geometry"
     * để tạo hiệu ứng trượt.
     */
    QPropertyAnimation *m_animation;

    /**
     * @brief Layout dọc chính của panel (chứa TitleBar và QScrollArea nội dung).
     */
    QVBoxLayout *m_mainLayout;
};

#endif // SIDEPANEL_H
