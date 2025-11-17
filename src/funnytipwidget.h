#ifndef FUNNYTIPWIDGET_H
#define FUNNYTIPWIDGET_H

#include <QWidget>
#include <QStringList> // Cần cho m_tipDeck (bộ bài chứa các mẹo)

// --- Forward Declarations (Khai báo trước) ---
class QLabel;
class QPropertyAnimation;

/**
 * @brief Lớp FunnyTipWidget hiển thị một mẹo/thông báo vui ngắn
 * ở dưới cùng của cửa sổ cha.
 *
 * Widget này tự quản lý một vòng lặp:
 * Fade-in -> Hiển thị (5 giây) -> Fade-out -> Lặp lại với mẹo mới.
 */
class FunnyTipWidget : public QWidget
{
    // Q_OBJECT là bắt buộc để sử dụng signals và slots
    Q_OBJECT

public:
    // === 1. HÀM CÔNG KHAI (PUBLIC API) ===

    /**
     * @brief Hàm dựng (Constructor).
     * @param parent Widget cha (thường là MainWindow).
     */
    explicit FunnyTipWidget(QWidget *parent = nullptr);

    /**
     * @brief Tính toán lại vị trí của widget (để căn giữa-dưới).
     * Thường được gọi khi cửa sổ cha thay đổi kích thước.
     */
    void reposition();

    /**
     * @brief Bắt đầu vòng lặp hiển thị mẹo (thường được gọi một lần bởi MainWindow).
     */
    void start();

private slots:
    // === 2. SLOT NỘI BỘ (PRIVATE SLOTS) ===

    /**
     * @brief Hàm logic chính của vòng lặp:
     * Lấy mẹo mới, fade-in, chờ, fade-out, và tự gọi lại chính nó.
     */
    void updateAndShowNextTip();

private:
    // === 3. BIẾN THÀNH VIÊN (MEMBER VARIABLES) ===

    QLabel *m_tipLabel;               // Nhãn hiển thị văn bản mẹo.
    QPropertyAnimation *m_animation;  // Đối tượng xử lý animation fade-in/fade-out.
    QWidget* m_parentWidget;          // Con trỏ tới cửa sổ cha (để tính toán vị trí).
    QStringList m_tipDeck;            // "Bộ bài" chứa các mẹo. Khi hết, nó sẽ được xào lại.
};

#endif // FUNNYTIPWIDGET_H
