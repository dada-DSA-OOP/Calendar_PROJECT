#include "funnytipwidget.h"
#include <QLabel>
#include <QTimer>
#include <QPropertyAnimation>
#include <QHBoxLayout>
#include <QRandomGenerator>
#include <algorithm>

FunnyTipWidget::FunnyTipWidget(QWidget *parent)
    : QWidget(parent), m_parentWidget(parent)
{
    setObjectName("funnyTipWidget");
    setMinimumHeight(50);
    hide();

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 10, 20, 10);

    QLabel *iconLabel = new QLabel;
    iconLabel->setPixmap(QPixmap(":/resource/icons/smile.png").scaled(15, 15, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    m_tipLabel = new QLabel;
    m_tipLabel->setObjectName("funnyTipLabel");

    layout->addWidget(iconLabel);
    layout->addWidget(m_tipLabel);

    // --- SỬA LỖI Ở ĐÂY ---
    m_animation = new QPropertyAnimation(this, "windowOpacity", this);
    m_animation->setDuration(500);      // Animation nhanh hơn (0.5 giây)
    m_animation->setStartValue(0.0);    // Bắt đầu từ trong suốt
    m_animation->setEndValue(1.0);      // Kết thúc ở rõ hoàn toàn
}

// Hàm public để bắt đầu chu kỳ từ MainWindow
void FunnyTipWidget::start()
{
    // Bắt đầu chu kỳ sau 2 giây trì hoãn ban đầu
    QTimer::singleShot(2000, this, &FunnyTipWidget::updateAndShowNextTip);
}

void FunnyTipWidget::reposition()
{
    if (m_parentWidget) {
        int x = (m_parentWidget->width() - width()) / 2;
        int y = m_parentWidget->height() - height() + 13;
        move(x, y);
    }
}

// Vòng lặp logic chính
void FunnyTipWidget::updateAndShowNextTip()
{
    // --- BƯỚC 1: CHUẨN BỊ TIP MỚI ---
    if (m_tipDeck.isEmpty()) {
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
        std::shuffle(m_tipDeck.begin(), m_tipDeck.end(), *QRandomGenerator::global());
    }
    m_tipLabel->setText(m_tipDeck.takeLast());
    adjustSize();
    reposition();

    raise();

    // --- BƯỚC 2: FADE-IN ---
    setWindowOpacity(0.0);
    show();
    m_animation->setDirection(QAbstractAnimation::Forward); // Chạy animation tiến (0.0 -> 1.0)
    m_animation->start();

    // --- BƯỚC 3: SAU KHI FADE-IN XONG, ĐỢI 5 GIÂY ---
    connect(m_animation, &QPropertyAnimation::finished, this, [this]() {
        disconnect(m_animation, &QPropertyAnimation::finished, this, nullptr);

        QTimer::singleShot(5000, this, [this]() {
            // --- BƯỚC 4: SAU KHI ĐỢI XONG, FADE-OUT ---
            m_animation->setDirection(QAbstractAnimation::Backward); // Chạy animation ngược (1.0 -> 0.0)
            m_animation->start();

            // --- BƯỚC 5: SAU KHI FADE-OUT XONG, GỌI LẠI HÀM NÀY ĐỂ HIỆN TIP TIẾP THEO ---
            connect(m_animation, &QPropertyAnimation::finished, this, [this]() {
                disconnect(m_animation, &QPropertyAnimation::finished, this, nullptr);
                hide();
                updateAndShowNextTip(); // Bắt đầu lại chu kỳ
            });
        });
    });
}
