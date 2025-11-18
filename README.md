<p align="center">
  <img src="https://upload.wikimedia.org/wikipedia/commons/0/0b/Qt_logo_2016.svg" width="100"/>
  <img src="https://github.com/user-attachments/assets/5340c406-a3df-4874-9318-98d8bd1e5dd9" width="130"/>
  <img src="https://upload.wikimedia.org/wikipedia/commons/1/18/ISO_C%2B%2B_Logo.svg" width="65"/>
  <img src="https://upload.wikimedia.org/wikipedia/commons/e/ef/CMake_logo.svg" width="75">
</p>
<p align="center">
  <img src="https://img.shields.io/badge/Check-Done-brightgreen?logo=github"/>
  <img src="https://img.shields.io/badge/License-MIT-blue"/>
  <img src="https://img.shields.io/badge/Qt-6.10.0-brightgreen?logo=qt"/>
  <img src="https://img.shields.io/badge/Language-C++ qtwidgets-blue?logo=cplusplus"/>
  <img src="https://img.shields.io/badge/Version-1.0.2-yellow"/>
</p>

# dadaCalendar

[![Build System](https://img.shields.io/badge/Build%20System-CMake-blue.svg?style=for-the-badge&logo=cmake)](https://cmake.org/)

**dadaCalendar** là một ứng dụng lịch desktop hiện đại, được xây dựng bằng C++ và Qt Widgets. Dự án lấy cảm hứng mạnh mẽ từ giao diện và trải nghiệm người dùng của Microsoft Outlook Calendar, kết hợp với các hiệu ứng giao diện người dùng hiện đại như Mica/Acrylic.

Đây là một dự án trong khuôn khổ môn học OOP/DSA, thể hiện khả năng áp dụng các nguyên tắc lập trình hướng đối tượng, cấu trúc dữ liệu và giải thuật vào một ứng dụng thực tế, quy mô lớn.

## Tính Năng Nổi Bật

### 1. Nhiều Chế Độ Xem Linh Hoạt

Ứng dụng hỗ trợ nhiều cách nhìn khác nhau về lịch của bạn, cho phép bạn chuyển đổi tức thì:

* **Chế độ Ngày (Day):** Xem chi tiết 1, 3, 5, hoặc 7 ngày.
* **Chế độ Tuần (Week):** Xem tổng quan 7 ngày trong tuần (T2 - CN).
* **Chế độ Tuần Làm Việc (Work Week):** Tập trung vào 5 ngày làm việc (T2 - T6).
* **Chế độ Tháng (Month):** Xem tổng quan toàn bộ tháng.
* **Chế độ Thời Khóa Biểu (Timetable):**
    * **Xem theo Tiết:** Hiển thị lịch học/làm việc theo từng tiết học (Tiết 1, 2, 3...).
    * **Xem theo Buổi:** Hiển thị lịch theo các buổi (Sáng, Chiều).

### 2. Quản Lý Sự Kiện Trực Quan

* **Tạo sự kiện:** Dễ dàng thêm sự kiện mới với tiêu đề, thời gian bắt đầu/kết thúc, và phân loại màu sắc.
* **Kéo và Thả (Drag & Drop):** Di chuyển sự kiện sang ngày hoặc giờ khác bằng cách kéo thả trực quan.
* **Thay đổi Kích thước (Resize):** Kéo dài hoặc thu ngắn thời lượng sự kiện trực tiếp trên lưới lịch.

### 3. Tùy Chỉnh Giao Diện Mạnh Mẽ

Một trong những điểm nhấn của dadaCalendar là khả năng cá nhân hóa sâu:

* **Thay Đổi Ảnh Nền:**
    * Chọn từ 13 ảnh nền chất lượng cao được tích hợp sẵn.
    * Tải lên ảnh nền tùy chỉnh của riêng bạn.
    * Chọn một màu nền đơn sắc.
* **Hiệu Ứng Trong Suốt (Mica/Acrylic):** Bật/tắt chế độ nền mờ (bán trong suốt), cho phép ảnh nền của máy tính xuyên qua, tạo hiệu ứng chiều sâu hiện đại.
* **Giao Diện Co Giãn (Responsive):** Giao diện tự động điều chỉnh (ví dụ: ẩn/hiện văn bản trên toolbar) khi kích thước cửa sổ thay đổi.

### 4. Tiện Ích Tích Hợp

* **Lịch Nhỏ & Ghi Chú (Sidebar):** Một thanh bên (sidebar) có thể ẩn/hiện, chứa một lịch nhỏ để điều hướng nhanh và một danh sách **To-Do List** tiện dụng.
* **Thay Đổi Tỉ Lệ Thời Gian:** Phóng to hoặc thu nhỏ lưới lịch (từ 60 phút xuống còn 5 phút) để xem chi tiết hơn hoặc tổng quan hơn.
* **Hệ Thống Panel Thông Minh:**
    * **Trợ giúp (Help):** Hướng dẫn sử dụng các tính năng chính.
    * **Mẹo (Tips):** Các mẹo vặt và thủ thuật.
    * **Hỗ trợ & Phản hồi (Support & Feedback):** Tích hợp các biểu mẫu để người dùng gửi phản hồi, báo cáo lỗi hoặc đưa ra đề xuất.
 
### 5. In và Xuất PDF

* Dễ dàng **in và xuất** dạng xem lịch hiện tại của bạn ra file PDF (thông qua nút "In" trên tab Trang chủ).
* Tiện lợi cho việc lưu trữ, quản lý, hoặc gửi thời khóa biểu cho người khác.
* Hỗ trợ in tất cả các dạng xem hiện có của ứng dụng (Tuần, Tháng, TKB).

### 6. Lưu Trữ Nội Bộ và Nhập/Xuất Dữ Liệu

* Ứng dụng hỗ trợ nhập và xuất dữ liệu lịch của bạn ngay trên giao diện "Trang chủ".
* Dữ liệu được lưu trữ nội bộ (local) trong máy của bạn tại thư mục:
    ```bash
    # Trên Windows
    %LOCALAPPDATA%\CalendarApp
    ```
    (Các file `data.json` và `settings.json`)

> **Lưu ý quan trọng:** Hiện tại, dữ liệu chỉ được lưu trên máy. Nếu bạn gỡ cài đặt ứng dụng mà không xuất (export) file lịch, dữ liệu của bạn có thể bị mất.

## Công Nghệ Sử Dụng

* **Ngôn ngữ:** **C++17**
* **Framework:** **Qt 6 / Qt 5** (Module Qt Widgets)
* **Hệ thống Build:** **CMake**
* **Styling:** **Qt Style Sheets (QSS)**.
* **Quản lý tài nguyên:** `resources.qrc` để đóng gói toàn bộ biểu tượng và ảnh nền vào ứng dụng.

## Team dada
* **Lê Quốc Đạt** - Hoàn thiện tab **Trang chủ**, giao diện lịch chính, tối ưu trải nghiệm người dùng.
* **Nguyễn Quốc Đạt** - Hoàn thiện tab **Dạng xem**, sidebar minicalendar, ghi chú, console.
* **Tuấn Kiệt** - Hoàn thiện tab **Trợ giúp**, giao diện hình nền, lưu trữ console.

## Hướng Dẫn Sử Dụng
* Hãy tải bản mới nhất tại [releases](https://github.com/dada-DSA-OOP/Calendar_PROJECT/releases). Giải nén và cài đặt bằng file msi.

## Hướng Dẫn Biên Dịch dành cho dev

Để biên dịch dự án, bạn cần cài đặt **Qt (phiên bản 5 hoặc 6)** và **CMake**.

1.  **Clone kho lưu trữ:**
    ```bash
    git clone [https://github.com/dada-DSA-OOP/Calendar_PROJECT.git](https://github.com/dada-DSA-OOP/Calendar_PROJECT.git)
    cd Calendar_PROJECT
    ```

2.  **Tạo thư mục build:**
    ```bash
    mkdir build
    cd build
    ```

3.  **Chạy CMake:**
    (Hãy chắc chắn rằng Qt đã được thêm vào `PATH` của bạn, hoặc chỉ định đường dẫn đến Qt)
    ```bash
    # Ví dụ cho Qt 6
    cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/gcc_64

    # Hoặc nếu Qt đã ở trong PATH
    cmake ..
    ```

4.  **Biên dịch:**
    ```bash
    # Trên Linux/macOS
    make -j$(nproc)

    # Trên Windows (với MSVC)
    cmake --build . --config Release
    ```

5.  **Chạy ứng dụng:**
    Chạy file thực thi được tạo ra trong thư mục `build` (ví dụ: `CalendarApp.exe` hoặc `./CalendarApp`).

---

## ©️ Bản Quyền MIT

Bản quyền MIT dadaCalendar thuộc về **dada**. Vui lòng tôn trọng bản quyền khi tham khảo hoặc sử dụng mã nguồn.
