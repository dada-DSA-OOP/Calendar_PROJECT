#include "funnytipwidget.h"
#include <QLabel>
#include <QTimer>             // Dùng để tạo độ trễ (delay)
#include <QPropertyAnimation> // Dùng để tạo hiệu ứng fade-in/fade-out
#include <QHBoxLayout>
#include <QRandomGenerator>   // Dùng để xáo trộn các mẹo
#include <algorithm>          // Dùng cho std::shuffle

/**
 * @brief Hàm dựng của FunnyTipWidget.
 * @param parent Widget cha (chính là MainWindow).
 */
FunnyTipWidget::FunnyTipWidget(QWidget *parent)
    : QWidget(parent), m_parentWidget(parent)
{
    setObjectName("funnyTipWidget"); // Đặt tên để style QSS
    setMinimumHeight(50);
    hide(); // Ẩn đi lúc đầu

    // --- 1. Cài đặt Layout (Icon + Text) ---
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 10, 20, 10);

    QLabel *iconLabel = new QLabel;
    iconLabel->setPixmap(QPixmap(":/resource/icons/smile.png").scaled(15, 15, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    m_tipLabel = new QLabel; // Nhãn để hiển thị văn bản mẹo
    m_tipLabel->setObjectName("funnyTipLabel");

    layout->addWidget(iconLabel);
    layout->addWidget(m_tipLabel);

    // --- 2. Cài đặt Animation ---
    // (Sửa lỗi: Cần thêm 'this' làm tham số thứ 3)
    // Animation này sẽ thay đổi thuộc tính "windowOpacity" (độ trong suốt)
    m_animation = new QPropertyAnimation(this, "windowOpacity", this);
    m_animation->setDuration(500);      // Tốc độ: 0.5 giây
    m_animation->setStartValue(0.0);    // Bắt đầu từ trong suốt
    m_animation->setEndValue(1.0);      // Kết thúc ở rõ hoàn toàn
}

/**
 * @brief Hàm công khai (public) để MainWindow "kích hoạt" vòng lặp.
 */
void FunnyTipWidget::start()
{
    // Chờ 2 giây sau khi ứng dụng khởi động rồi mới hiện mẹo đầu tiên
    QTimer::singleShot(2000, this, &FunnyTipWidget::updateAndShowNextTip);
}

/**
 * @brief Tính toán lại vị trí (để luôn ở giữa-dưới của cửa sổ cha).
 */
void FunnyTipWidget::reposition()
{
    if (m_parentWidget) {
        int x = (m_parentWidget->width() - width()) / 2; // Căn giữa theo chiều ngang
        int y = m_parentWidget->height() - height() + 13; // Đặt ở đáy (có điều chỉnh)
        move(x, y);
    }
}

/**
 * @brief Vòng lặp logic chính: Lấy mẹo, Fade-in, Chờ, Fade-out, Lặp lại.
 */
void FunnyTipWidget::updateAndShowNextTip()
{
    // --- BƯỚC 1: CHUẨN BỊ MẸO MỚI ---
    if (m_tipDeck.isEmpty()) {
        // Nếu đã hết mẹo, "xào" lại bộ bài
        static const QStringList allTips = {
            "Bạn có thể lên lịch cho mọi thứ, trừ việc uống một cốc nước ngay bây giờ.",
            "Đặt lịch sinh nhật lặp lại hàng năm. Rẻ hơn là mua hoa xin lỗi.",
            "Cách tốt nhất để nhớ sinh nhật vợ là quên nó một lần.",
            "Stress? Thử kéo-thả 'Deadline' sang tuần sau xem (chỉ là thử thôi).",
            "Nếu bạn thấy stress, hãy thử lật ngược lịch lại xem sao.",
            "Sự kiện tên 'Ngày mai' sẽ mãi mãi ở... ngày mai. Hãy đặt tên cụ thể!",
            "Nhớ đứng dậy vươn vai nhé. Cái ghế của bạn sẽ không tự đẩy bạn ra đâu!",
            "Lịch này không thể thêm 'Thời gian cho bản thân'. Bạn phải tự làm điều đó.",
            "Cười lên một chút nào. Bàn phím của bạn không thể gõ ra một nụ cười được.",
            "Cà phê không phải là bản vá lỗi cho một hệ điều hành thiếu ngủ.",
            "Thử gọi cho ai đó bạn yêu quý xem. Một 'cuộc gọi thật' không cần lên lịch.",
            "Deadline là cách tuyệt vời để bắt đầu công việc... vào ngày mai.",
            "Tôi không lười, tôi đang ở 'Chế độ tiết kiệm pin'.",
            "Đừng lo, To-do list của bạn vẫn ở đó sau 5 phút nữa. Nó trung thành lắm.",
            "Nghỉ một lát đi. To-do list của bạn vẫn sẽ ở đó chờ thôi.",
            "Báo cáo của bạn sẽ ổn thôi. Nhưng bạn thì cần một cái ôm. Tự lo đi nhé!",
            "Cho mắt nhìn ra xa một chút đi. Màn hình của bạn sẽ không tự động mờ đi để nhắc bạn đâu.",
            "Quy tắc 20-20-20: Cứ 20 phút, nhìn xa 20 feet, trong 20 giây. Hoặc... cứ nhắm mắt lại là được.",
            "Tháo tai nghe ra và lắng nghe. Sự im lặng cũng có một giai điệu rất riêng.",
            "Hít một hơi thật sâu. Đúng rồi, sâu nữa. Bạn không cần lên lịch cho việc này.",
            "Chatbot cho câu trả lời. Không khí trong lành cho ý tưởng. Hãy chọn cho khôn ngoan.",
            "Cuộc sống không có Ctrl+Z. Nhưng may là nó cũng không có 'Báo cáo lỗi'.",
            "Cách tốt nhất để hoàn thành công việc là bắt đầu.",
            "Đừng bận rộn. Hãy làm việc hiệu quả.",
            "Kế hoạch của hôm qua là nền tảng của hôm nay, không phải trần nhà.",
            "Hoàn thành tốt hơn là hoàn hảo.",
            "Đừng để các cuộc họp trở thành nơi các ý tưởng hay đi vào cõi chết.",
            "Một 'không' lịch sự còn tốt hơn một 'có' miễn cưỡng",
            "Sự sáng tạo chỉ là việc kết nối các dấu chấm. Hãy ra ngoài và thu thập thêm các dấu chấm.",
            "Hộp thư đến là một danh sách việc cần làm do người khác tạo ra cho bạn. Đừng sống trong đó.",
            "Đa nhiệm (Multitasking) chỉ là cơ hội để làm hỏng nhiều việc cùng một lúc.",
            "Nếu mọi thứ đều quan trọng, thì không có gì quan trọng cả. Hãy ưu tiên.",
            "Hãy tò mò, đừng phán xét.",
            "Lỗi lầm không phải là thất bại. Chúng là dữ liệu.",
            "Nghỉ ngơi không phải là lười biếng. Đó là một phần của công việc.",
            "Hãy làm việc thông minh hơn, không phải lúc nào cũng chăm chỉ hơn.",
            "Đừng nhầm lẫn giữa chuyển động và tiến bộ.",
            "Bạn không cần thêm thời gian. Bạn chỉ cần quyết định.",
            "Đóng tab. Tắt thông báo. Làm một việc thôi. Bạn sẽ ngạc nhiên đấy.",
        };
        m_tipDeck = allTips;
        // Xáo trộn danh sách mẹo một cách ngẫu nhiên
        std::shuffle(m_tipDeck.begin(), m_tipDeck.end(), *QRandomGenerator::global());
    }
    // Lấy mẹo cuối cùng từ danh sách (và xóa nó khỏi danh sách)
    m_tipLabel->setText(m_tipDeck.takeLast());
    adjustSize(); // Tự điều chỉnh kích thước widget cho vừa với text
    reposition(); // Đặt lại vị trí

    raise(); // Đảm bảo widget này nằm trên các widget khác

    // --- BƯỚC 2: FADE-IN (Mờ -> Rõ) ---
    setWindowOpacity(0.0); // Đặt về trong suốt
    show(); // Hiển thị widget (vẫn đang trong suốt)
    m_animation->setDirection(QAbstractAnimation::Forward); // Chạy animation tiến (0.0 -> 1.0)
    m_animation->start();

    // --- BƯỚC 3: CHỜ (Sau khi Fade-in xong) ---
    // Kết nối *một lần* với tín hiệu 'finished' của animation
    connect(m_animation, &QPropertyAnimation::finished, this, [this]() {
        // Ngắt kết nối ngay lập-tức để tránh gọi lại
        disconnect(m_animation, &QPropertyAnimation::finished, this, nullptr);

        // Đặt hẹn giờ 5 giây (hiển thị mẹo trong 5 giây)
        QTimer::singleShot(5000, this, [this]() {

            // --- BƯỚC 4: FADE-OUT (Rõ -> Mờ) ---
            m_animation->setDirection(QAbstractAnimation::Backward); // Chạy animation ngược (1.0 -> 0.0)
            m_animation->start();

            // --- BƯỚC 5: LẶP LẠI (Sau khi Fade-out xong) ---
            // Kết nối *một lần* nữa với tín hiệu 'finished'
            connect(m_animation, &QPropertyAnimation::finished, this, [this]() {
                disconnect(m_animation, &QPropertyAnimation::finished, this, nullptr);
                hide(); // Ẩn widget đi
                updateAndShowNextTip(); // Bắt đầu lại chu kỳ (gọi lại hàm này)
            });
        });
    });
}
