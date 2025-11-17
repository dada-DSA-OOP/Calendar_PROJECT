#include "mainwindow.h" // Bao gồm tệp header của cửa sổ chính

#include <QApplication> // Bao gồm lớp quản lý ứng dụng
#include <QFile>        // Cần để đọc tệp stylesheet từ tài nguyên
#include <QIcon>        // (Thêm) Cần để đặt icon cho cửa sổ

/**
 * @brief Hàm main() - Điểm khởi đầu (Entry Point) của ứng dụng.
 */
int main(int argc, char *argv[])
{
    // 1. Khởi tạo đối tượng QApplication
    //    (Bắt buộc phải có cho mọi ứng dụng Qt Widgets)
    QApplication a(argc, argv);

    // 2. Tải tệp Stylesheet (QSS) từ tài nguyên (resources.qrc)
    QFile styleFile(":/resource/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        // Nếu mở tệp thành công, đọc toàn bộ nội dung
        // và áp dụng nó làm stylesheet chung cho toàn bộ ứng dụng
        a.setStyleSheet(styleFile.readAll());
        styleFile.close(); // (Đóng tệp sau khi đọc xong)
    }

    // 3. Tạo đối tượng cửa sổ chính (MainWindow)
    MainWindow w;

    // 4. Cài đặt các thuộc tính cho cửa sổ
    w.setWindowTitle("dadaCalendar"); // Đặt tiêu đề cửa sổ
    w.setWindowIcon(QIcon(":/resource/icons/app_icon.png")); // Đặt icon

    // 5. Hiển thị cửa sổ chính
    w.show();

    // 6. Bắt đầu vòng lặp sự kiện (event loop) của ứng dụng
    //    Ứng dụng sẽ chạy và chờ đợi các hành động (click, gõ phím...)
    //    cho đến khi cửa sổ chính được đóng.
    return a.exec();
}
