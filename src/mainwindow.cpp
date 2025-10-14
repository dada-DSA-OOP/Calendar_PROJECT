#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "calendarview.h"
#include "dayheader.h"
#include "timeruler.h"

#include <QTabBar>
#include <QStackedWidget>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QFrame>
#include <QMenu>
#include <QAction>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QCheckBox>
#include <QPushButton>
#include <QWidgetAction>
#include <QGraphicsDropShadowEffect>
#include <QDesktopServices>
#include <QUrl>
#include <QGridLayout>
#include <QScrollBar>

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    bool compact = width() < 900; //Nếu cửa sổ < 900px thì ẩn chữ
    setMinimumHeight(600);

    const auto buttons = this->findChildren<QToolButton*>();
    for (auto button : buttons) {
        button->setToolButtonStyle(compact ? Qt::ToolButtonIconOnly : Qt::ToolButtonTextBesideIcon);
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Màn hình nhỏ nhất có thể co lại
    setMinimumWidth(600);
    resize(1200, 800);     // rộng 1200px, cao 800px
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX); // cho phép phóng to tự do

    auto addShadowEffect = [](QWidget *widget) {
        auto *effect = new QGraphicsDropShadowEffect;
        effect->setBlurRadius(15);
        effect->setXOffset(0);
        effect->setYOffset(2);
        effect->setColor(QColor(0, 0, 0, 80));
        widget->setGraphicsEffect(effect);
    };

    // ===== Tab bar =====
    QTabBar *tabBar = new QTabBar(this);
    tabBar->addTab("Trang chủ");
    tabBar->addTab("Dạng xem");
    tabBar->addTab("Trợ giúp");
    tabBar->setExpanding(false);
    tabBar->setDrawBase(false);

    // ===== Toolbar Stack =====
    QStackedWidget *toolbarStack = new QStackedWidget(this);

    // --- Nút "Bộ lọc" có menu thả --- //
    QToolButton *btnFilter = new QToolButton;
    btnFilter->setText("  Bộ lọc  ▼");
    btnFilter->setIcon(QIcon("resource/icons/filter.png"));
    btnFilter->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btnFilter->setCursor(Qt::PointingHandCursor);
    btnFilter->setPopupMode(QToolButton::InstantPopup);
    btnFilter->setObjectName("btnFilter");

    QMenu *filterMenu = new QMenu(btnFilter);
    addShadowEffect(filterMenu);

    QAction *actAppointment = filterMenu->addAction("Cuộc hẹn");
    actAppointment->setCheckable(true);
    actAppointment->setChecked(true);


    // === Các mục có submenu === //
    QMenu *menuMeetings = new QMenu("Cuộc họp", filterMenu);
    addShadowEffect(menuMeetings);
    QAction *actClearAll = menuMeetings->addAction("Bỏ chọn tất cả");
    QObject::connect(actClearAll, &QAction::triggered, menuMeetings, [menuMeetings]() {
        const auto actions = menuMeetings->actions();
        for (QAction *a : actions)
            if (a->isCheckable()) a->setChecked(false);
    });
    menuMeetings->addSeparator();
    QAction *header1 = menuMeetings->addAction("Tôi là người tổ chức");
    header1->setEnabled(false);
    header1->setFont(QFont("Segoe UI", 9, QFont::Bold));
    const QStringList meetingTypes = {"Đã gửi", "Bản thảo"};
    for (const QString &t : meetingTypes) {
        QAction *a = menuMeetings->addAction("  " + t);
        a->setCheckable(true);
        a->setChecked(true);
    }
    menuMeetings->addSeparator();
    QAction *header2 = menuMeetings->addAction("Tôi là người dự");
    header2->setEnabled(false);
    header2->setFont(QFont("Segoe UI", 9, QFont::Bold));
    const QStringList attendeeStatuses = {"Đã chấp nhận", "Đã từ chối", "Dự định", "Đã hủy bỏ", "Chưa trả lời"};
    for (const QString &t : attendeeStatuses) {
        QAction *a = menuMeetings->addAction("  " + t);
        a->setCheckable(true);
        a->setChecked(true);
    }

    QMenu *menuCategory = new QMenu("Thể loại", filterMenu);
    addShadowEffect(menuCategory);

    // Nút "Bỏ chọn tất cả"
    QAction *actUncheckCategory = menuCategory->addAction("Bỏ chọn tất cả");
    QObject::connect(actUncheckCategory, &QAction::triggered, menuCategory, [menuCategory]() {
        const auto categoryActions = menuCategory->actions();
        for (QAction *a : categoryActions)
            if (a->isCheckable()) a->setChecked(false);
    });
    menuCategory->addSeparator();

    // "Chưa được phân loại"
    QAction *actUncategorized = menuCategory->addAction("Chưa được phân loại");
    actUncategorized->setCheckable(true);
    actUncategorized->setChecked(true);
    menuCategory->addSeparator();

    // Danh sách thể loại có icon màu
    struct Category {
        QString name;
        QString colorIcon;
    };

    QList<Category> categories = {
        {"Red category",    "resource/icons/red_tag.png"},
        {"Orange category", "resource/icons/orange_tag.png"},
        {"Yellow category", "resource/icons/yellow_tag.png"},
        {"Green category",  "resource/icons/green_tag.png"},
        {"Blue category",   "resource/icons/blue_tag.png"},
        {"Purple category", "resource/icons/purple_tag.png"}
    };

    menuCategory->setObjectName("menuCategory");

    for (const auto &cat : categories) {
        QAction *a = menuCategory->addAction(QIcon(cat.colorIcon), cat.name);
        a->setCheckable(true);
        a->setChecked(true);
    }


    QMenu *menuDisplayAs = new QMenu("Hiển thị như", filterMenu);
    addShadowEffect(menuDisplayAs);

    // ---- Bỏ chọn tất cả ----
    QAction *actUncheckDisplayAs = menuDisplayAs->addAction("Bỏ chọn tất cả");
    QObject::connect(actUncheckDisplayAs, &QAction::triggered, menuDisplayAs, [menuDisplayAs]() {
        const auto actionList = menuDisplayAs->actions();
        for (QAction *a : actionList)
            if (a->isCheckable()) a->setChecked(false);
    });
    menuDisplayAs->addSeparator();

    // ---- Danh sách lựa chọn ----
    QStringList displayAs = {
        "Rảnh",
        "Làm việc ở nơi khác",
        "Dự định",
        "Bận",
        "Vắng mặt"
    };

    for (const QString &opt : displayAs) {
        QAction *a = menuDisplayAs->addAction(opt);
        a->setCheckable(true);
        a->setChecked(true);
    }


    QMenu *menuRepeat = new QMenu("Lặp lại", filterMenu);
    addShadowEffect(menuRepeat);

    // ---- Danh sách lựa chọn ----
    QStringList displayRepeat = {
        "Đơn",
        "Chuỗi"
    };

    for (const QString &opt : displayRepeat) {
        QAction *a = menuRepeat->addAction(opt);
        a->setCheckable(true);
        a->setChecked(true);
    }

    QMenu *menuDirect = new QMenu("Trực tiếp", filterMenu);
    addShadowEffect(menuDirect);

    // ---- Danh sách lựa chọn ----
    QStringList displayDirect = {
        "Đã yêu cầu",
        "Không yêu cầu"
    };

    for (const QString &opt : displayDirect) {
        QAction *a = menuDirect->addAction(opt);
        a->setCheckable(true);
        a->setChecked(true);
    }

    // Thêm các mục tick + menu con
    filterMenu->addMenu(menuMeetings);
    filterMenu->addMenu(menuCategory);
    filterMenu->addMenu(menuDisplayAs);
    filterMenu->addMenu(menuRepeat);
    filterMenu->addMenu(menuDirect);

    btnFilter->setMenu(filterMenu);

    // --- Hàm tiện ích: tạo bản sao nút Bộ lọc --- //
    auto makeFilterButton = [&](QWidget *parent = nullptr) {
        QToolButton *b = new QToolButton(parent);
        b->setText("  Bộ lọc  ▼");
        b->setIcon(QIcon("resource/icons/filter.png"));
        b->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        b->setCursor(Qt::PointingHandCursor);
        b->setPopupMode(QToolButton::InstantPopup);
        b->setMenu(filterMenu); // dùng chung menu
        return b;
    };

    // --- Home toolbar ---
    QWidget *homePage = new QWidget;
    QHBoxLayout *homeLayout = new QHBoxLayout(homePage);
    homeLayout->setContentsMargins(10, 6, 10, 6);
    homeLayout->setSpacing(10);

    //Hàm thêm icon đầu nút
    auto makeBtn = [](const QString &text, const QString &icon = QString()) {
        QToolButton *btn = new QToolButton;
        btn->setText("  " + text);
        if (!icon.isEmpty()) btn->setIcon(QIcon(icon));  // ← quan trọng
        btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setObjectName(text); // để dễ style QSS nếu cần
        return btn;
    };

    //Hỗ trợ homeLayout->addWidget(makeSeparator());
    auto makeSeparator = []() {
        QFrame *line = new QFrame;
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Plain);
        line->setStyleSheet("color: #e0e0e0;"); // màu xám nhẹ
        return line;
    };

    // --- Nút chính "Sự kiện mới" ---
    QToolButton *btnNewEvent = new QToolButton;
    btnNewEvent->setText("  Sự kiện mới");
    btnNewEvent->setIcon(QIcon("resource/icons/calendar.png")); // ← thêm icon lịch (đặt trong build/.../resource/icons)
    btnNewEvent->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btnNewEvent->setCursor(Qt::PointingHandCursor);
    btnNewEvent->setPopupMode(QToolButton::MenuButtonPopup);
    btnNewEvent->setObjectName("btnNewEvent");

    // Menu thả xuống
    QMenu *newEventMenu = new QMenu(btnNewEvent);
    QAction *actNewMail = newEventMenu->addAction(QIcon("resource/icons/message.png"), "Thư");
    QAction *actNewEvent = newEventMenu->addAction(QIcon("resource/icons/calendarEvent.png"), "Sự kiện");
    newEventMenu->setObjectName("eventMenu");
    btnNewEvent->setMenu(newEventMenu);
    addShadowEffect(newEventMenu);

    homeLayout->addWidget(btnNewEvent);

    // --- Nút "Ngày" có menu thả ---
    QToolButton *btnDay = new QToolButton;
    btnDay->setText("  Ngày");
    btnDay->setIcon(QIcon("resource/icons/7days.png"));
    btnDay->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btnDay->setCursor(Qt::PointingHandCursor);
    btnDay->setPopupMode(QToolButton::MenuButtonPopup);
    btnDay->setObjectName("btnDay");

    QMenu *dayMenu = new QMenu(btnDay);
    QAction *actOneDay = dayMenu->addAction("1 Ngày");
    QAction *actTwoDay = dayMenu->addAction("2 Ngày");
    QAction *actThreeDay = dayMenu->addAction("3 Ngày");
    QAction *actFourDay = dayMenu->addAction("4 Ngày");
    QAction *actFiveDay = dayMenu->addAction("5 Ngày");
    QAction *actSixDay = dayMenu->addAction("6 Ngày");
    QAction *actSevenDay = dayMenu->addAction("7 Ngày");
    dayMenu->setObjectName("dayMenu");
    addShadowEffect(dayMenu);

    btnDay->setMenu(dayMenu);

    homeLayout->addWidget(btnDay);

    homeLayout->addWidget(makeBtn("Tuần làm việc", "resource/icons/workWeek.png"));
    homeLayout->addWidget(makeBtn("Tuần", "resource/icons/week.png"));
    homeLayout->addWidget(makeBtn("Tháng", "resource/icons/month.png"));
    homeLayout->addWidget(makeBtn("Dạng xem tách", "resource/icons/split.png"));

    //Gạch dọc chia
    homeLayout->addWidget(makeSeparator());

    homeLayout->addWidget(makeFilterButton());

    //Gạch dọc chia
    homeLayout->addWidget(makeSeparator());

    homeLayout->addWidget(makeBtn("In", "resource/icons/printer.png"));
    homeLayout->addStretch();

    toolbarStack->addWidget(homePage);

    // --- View toolbar ---
    QWidget *viewPage = new QWidget;
    QHBoxLayout *viewLayout = new QHBoxLayout(viewPage);
    viewLayout->setContentsMargins(10, 6, 10, 6);
    viewLayout->setSpacing(10);

    // --- Nút "Ngày" ---
    QToolButton *btnDayView = new QToolButton;
    btnDayView->setText("  Ngày");
    btnDayView->setIcon(QIcon("resource/icons/7days.png"));
    btnDayView->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btnDayView->setCursor(Qt::PointingHandCursor);
    btnDayView->setPopupMode(QToolButton::MenuButtonPopup);
    btnDayView->setObjectName("btnDayView");

    btnDayView->setMenu(dayMenu); // Gán menu dùng chung
    viewLayout->addWidget(btnDayView);

    // --- Các nút còn lại ---
    viewLayout->addWidget(makeBtn("Tuần làm việc", "resource/icons/workWeek.png"));
    viewLayout->addWidget(makeBtn("Tuần", "resource/icons/week.png"));
    viewLayout->addWidget(makeBtn("Tháng", "resource/icons/month.png"));
    viewLayout->addWidget(makeBtn("Lưu dạng xem", "resource/icons/save.png"));

    // --- Nút "Tỉ lệ thời gian" ---
    QToolButton *btnTimeScale = new QToolButton;
    btnTimeScale->setText("  Tỉ lệ thời gian  ▼"); // <-- THAY ĐỔI 1: Thêm mũi tên
    btnTimeScale->setIcon(QIcon("resource/icons/timeScale.png"));
    btnTimeScale->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btnTimeScale->setCursor(Qt::PointingHandCursor);
    btnTimeScale->setPopupMode(QToolButton::InstantPopup); // <-- THAY ĐỔI 2: Chế độ popup
    btnTimeScale->setObjectName("btnTimeScale");

    // Tạo menu thả xuống
    QMenu *timeScaleMenu = new QMenu(btnTimeScale);
    QAction *act60min = timeScaleMenu->addAction("60 phút - Ít chi tiết");
    QAction *act30min = timeScaleMenu->addAction("30 phút");
    QAction *act15min = timeScaleMenu->addAction("15 phút");
    QAction *act10min = timeScaleMenu->addAction("10 phút");
    QAction *act6min  = timeScaleMenu->addAction("6 phút");
    QAction *act5min  = timeScaleMenu->addAction("5 phút - Nhiều chi tiết");

    timeScaleMenu->setObjectName("timeScaleMenu");
    addShadowEffect(timeScaleMenu);

    // Gán menu vào nút
    btnTimeScale->setMenu(timeScaleMenu);

    // Thêm nút vào layout
    viewLayout->addWidget(btnTimeScale);

    timeScaleMenu->setObjectName("timeScaleMenu");
    addShadowEffect(timeScaleMenu);

    // Gán menu vào nút
    btnTimeScale->setMenu(timeScaleMenu);

    // Thêm nút vào layout
    viewLayout->addWidget(btnTimeScale);

    viewLayout->addWidget(makeSeparator());

    viewLayout->addWidget(makeFilterButton());

    viewLayout->addWidget(makeSeparator());

    viewLayout->addWidget(makeBtn("Cài đặt", "resource/icons/setting.png"));

    viewLayout->addStretch();

    toolbarStack->addWidget(viewPage);

    // --- Help toolbar ---
    QWidget *helpPage = new QWidget;
    QHBoxLayout *helpLayout = new QHBoxLayout(helpPage);
    helpLayout->setContentsMargins(10, 6, 10, 6);
    helpLayout->setSpacing(10); // Thêm khoảng cách giữa các nút

    // Sử dụng lại hàm makeBtn đã tạo ở trên
    // Lưu ý: Đường dẫn icon "resource/icons/..." là ví dụ,
    // bạn cần chuẩn bị các file icon này.
    helpLayout->addWidget(makeBtn("Trợ giúp", "resource/icons/question.png"));
    helpLayout->addWidget(makeBtn("Mẹo", "resource/icons/lightbulb.png"));
    helpLayout->addWidget(makeBtn("Hỗ trợ", "resource/icons/support.png"));
    helpLayout->addWidget(makeBtn("Phản hồi", "resource/icons/feedback.png"));
    helpLayout->addWidget(makeBtn("Xem chẩn đoán", "resource/icons/diagnostics.png"));

    // Gạch dọc chia
    helpLayout->addWidget(makeSeparator());

    helpLayout->addWidget(makeBtn("dadaCal cho thiết bị di động", "resource/icons/mobile.png"));
    // 1. Tạo nút và lưu vào biến
    QToolButton *btnGithub = makeBtn("Đi tới Github", "resource/icons/github.png");

    // 2. Gắn link vào sự kiện click
    connect(btnGithub, &QToolButton::clicked, this, []() {
        QDesktopServices::openUrl(QUrl("https://github.com/dada-DSA-OOP/Calendar_PROJECT")); // <-- Thay link Github của bạn ở đây
    });

    // 3. Thêm nút vào layout
    helpLayout->addWidget(btnGithub);

    helpLayout->addStretch(); // Đẩy các nút về bên trái
    toolbarStack->addWidget(helpPage);

    // ===== Menu cố định =====
    QWidget *topBar = new QWidget;
    QVBoxLayout *topLayout = new QVBoxLayout(topBar);
    topLayout->setContentsMargins(0,0,0,0);
    topLayout->setSpacing(0);
    topLayout->addWidget(tabBar);
    topLayout->addWidget(toolbarStack);

    QFrame *separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Plain);
    separator->setStyleSheet("color: #dcdcdc;");
    topLayout->addWidget(separator);

    // ===== Main content =====
    // Thay thế toàn bộ đoạn code cũ của "Main content" bằng đoạn dưới đây
    QWidget *calendarContainer = new QWidget;
    QGridLayout *grid = new QGridLayout(calendarContainer);
    grid->setSpacing(0);
    grid->setContentsMargins(0, 0, 0, 0);

    DayHeader *header = new DayHeader;
    TimeRuler *ruler = new TimeRuler;
    CalendarView *view = new CalendarView;

    // Widget trống ở góc trên bên trái
    QWidget *corner = new QWidget;
    corner->setStyleSheet("background-color: #f5f5f5; border-bottom: 1px solid #dcdcdc; border-right: 1px solid #dcdcdc;");
    corner->setFixedSize(60, 60);

    grid->addWidget(corner, 0, 0);
    grid->addWidget(header, 0, 1);
    grid->addWidget(ruler, 1, 0);
    grid->addWidget(view, 1, 1);

    // ----- KẾT NỐI THANH CUỘN -----
    // Kết nối cuộn ngang
    connect(view->horizontalScrollBar(), &QScrollBar::valueChanged,
            header, &DayHeader::setScrollOffset);

    // Kết nối cuộn dọc
    connect(view->verticalScrollBar(), &QScrollBar::valueChanged,
            ruler, &TimeRuler::setScrollOffset);

    // Cập nhật dayWidth khi cửa sổ resize
    connect(view, &CalendarView::viewResized, this, [view, header](){
        header->setDayWidth(view->getDayWidth());
    });
    header->setDayWidth(view->getDayWidth());


    // ===== Tổng layout =====
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(topBar, 0, Qt::AlignTop);
    // mainLayout->addWidget(calendarView, 1); // <-- XÓA DÒNG NÀY
    mainLayout->addWidget(calendarContainer, 1); // <-- THAY BẰNG DÒNG NÀY
    setCentralWidget(central);

    // THÊM CÁC SỰ KIỆN MẪU
    // Ngày 0 (Thứ hai)
    view->addEvent("Họp team dự án A", QColor("#3a87ad"), 0, QTime(9, 0), QTime(10, 30));
    view->addEvent("Họp nhanh với sếp", QColor("#e09445"), 0, QTime(10, 0), QTime(11, 0));

    // Ngày 1 (Thứ ba)
    // Hai sự kiện giống hệt nhau
    view->addEvent("Event X", QColor("#8cbb63"), 1, QTime(14, 0), QTime(15, 0));
    view->addEvent("Event Y", QColor("#af67de"), 1, QTime(14, 0), QTime(15, 0));

    // Ngày 2 (Thứ tư)
    view->addEvent("Event A", QColor("#8cbb63"), 2, QTime(10, 0), QTime(11, 30));
    view->addEvent("Event B", QColor("#3a87ad"), 2, QTime(10, 15), QTime(11, 0));
    view->addEvent("Event C", QColor("#e09445"), 2, QTime(10, 45), QTime(12, 0));

    setCentralWidget(central);

    // ===== Kết nối =====
    connect(tabBar, &QTabBar::currentChanged, this, [=](int index) {
        toolbarStack->setCurrentIndex(index);
    });

    tabBar->setCurrentIndex(0);
    toolbarStack->setCurrentIndex(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}
