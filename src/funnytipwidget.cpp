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
    iconLabel->setPixmap(QPixmap("resource/icons/smile.png").scaled(15, 15, Qt::KeepAspectRatio, Qt::SmoothTransformation));

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
            "Uống đủ nước nhé. Lịch của bạn sẽ không nhắc đâu!",
            "Cách tốt nhất để nhớ sinh nhật vợ là quên nó một lần.",
            "Nếu bạn thấy stress, hãy thử lật ngược lịch lại xem sao.",
            "Đừng bao giờ lên lịch một cuộc họp có tên 'ngày mai'.",
            "Nhớ đứng dậy vươn vai nhé. Cái ghế của bạn sẽ không tự đẩy bạn ra đâu!",
            "Lịch này không thể thêm 'Thời gian cho bản thân'. Bạn phải tự làm điều đó.",
            "Cười lên một chút nào. Bàn phím của bạn không thể gõ ra một nụ cười được.",
            "Ngủ đủ giấc nhé. Tách cà phê sáng mai sẽ không thể thay thế cho những giờ đã mất đâu.",
            "Gọi cho người thân đi. Danh bạ điện thoại chỉ lưu số chứ không lưu lại tình cảm.",
            "Deadline là cách tuyệt vời để bắt đầu công việc... vào ngày mai.",
            "Tôi không lười, tôi đang ở chế độ tiết kiệm năng lượng.",
            "Hãy đối xử tốt với chính mình. Bảng báo cáo công việc sẽ không ôm bạn một cái đâu.",
            "Nghỉ một lát đi. To-do list của bạn vẫn sẽ ở đó chờ thôi.",
            "Cho mắt nhìn ra xa một chút đi. Màn hình của bạn sẽ không tự động mờ đi để nhắc bạn đâu.",
            "Tháo tai nghe ra và lắng nghe. Sự im lặng cũng có một giai điệu rất riêng.",
            "Hít một hơi thật sâu. Dòng thông báo có thể đợi, nhưng lá phổi của bạn thì không.",
            "Ra ngoài đi dạo một lát đi. Công cụ tìm kiếm có thể cho bạn câu trả lời, nhưng không khí trong lành mới cho bạn ý tưởng.",
            "Trò chuyện với một người bạn thật sự đi. Mạng xã hội sẽ không hỏi han hôm nay bạn cảm thấy thế nào đâu.",
            "Hãy tha thứ cho một lỗi sai nhỏ của bản thân. Nút 'Ctrl z' không có tác dụng với cuộc sống này."
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
