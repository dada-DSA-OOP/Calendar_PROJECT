#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "calendarview.h"
#include "dayheader.h"
#include "timeruler.h"
#include "eventdialog.h"
#include "sidepanel.h"
#include "funnytipwidget.h"
#include "settingsdialog.h"
#include "monthviewwidget.h"
#include "daycellwidget.h"
#include "eventitem.h"
#include "timetableviewwidget.h"
#include "timetableslotwidget.h"
#include "sessionviewwidget.h"

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
#include <QStyle>
#include <QCalendarWidget>
#include <QTextCharFormat>
#include <QScrollArea>
#include <QGroupBox>
#include <QRandomGenerator>
#include <QRadioButton>
#include <QTextEdit>
#include <QMessageBox>
#include <QGraphicsBlurEffect>
#include <QRegularExpression>
#include <QListWidget>
#include <QLineEdit>

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    bool compact = width() < 900; //Nếu cửa sổ < 900px thì ẩn chữ
    setMinimumHeight(600);

    // === BẮT ĐẦU PHẦN SỬA LỖI ===
    // Duyệt qua từng trang (toolbar) trong QStackedWidget
    for (int i = 0; i < m_toolbarStack->count(); ++i) {
        QWidget *toolbarPage = m_toolbarStack->widget(i);
        if (toolbarPage) {
            // Tìm các nút chỉ trong trang toolbar đó
            const auto buttons = toolbarPage->findChildren<QToolButton*>();
            for (auto button : buttons) {
                button->setToolButtonStyle(compact ? Qt::ToolButtonIconOnly : Qt::ToolButtonTextBesideIcon);
            }

            // BƯỚC QUAN TRỌNG NHẤT: Báo cho layout rằng nó cần tính toán lại từ đầu
            if (toolbarPage->layout()) {
                toolbarPage->layout()->invalidate();
            }
        }
    }
    // === KẾT THÚC PHẦN SỬA LỖI ===

    // Giữ vị trí của help panel khi resize cửa sổ
    if (m_helpPanel && !m_helpPanel->isHidden()) {
        int panelWidth = m_helpPanel->width();
        m_helpPanel->setGeometry(width() - panelWidth, m_topBar->height(), panelWidth, height() - m_topBar->height());
    }
    if (m_funnyTipWidget) {
        m_funnyTipWidget->reposition();
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Màn hình nhỏ nhất có thể co lại
    setMinimumWidth(700);
    resize(1250, 800);     // rộng 1200px, cao 800px
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX); // cho phép phóng to tự do

    auto addShadowEffect = [](QWidget *widget) {
        auto *effect = new QGraphicsDropShadowEffect;
        effect->setBlurRadius(15);
        effect->setXOffset(0);
        effect->setYOffset(2);
        effect->setColor(QColor(0, 0, 0, 80));
        widget->setGraphicsEffect(effect);
    };

    // === Nút 3 gạch (hamburger) ===
    m_btnSidebarToggle = new QToolButton(this);
    m_btnSidebarToggle->setIcon(QIcon(":/resource/icons/menu.png"));
    m_btnSidebarToggle->setToolTip("Mở/Đóng Lịch nhỏ");
    m_btnSidebarToggle->setCursor(Qt::PointingHandCursor);
    m_btnSidebarToggle->setFixedSize(50, 50);

    // ===== Tab bar =====
    QTabBar *tabBar = new QTabBar(this);
    tabBar->addTab("Trang chủ");
    tabBar->addTab("Dạng xem");
    tabBar->addTab("Trợ giúp");
    tabBar->setExpanding(false);
    tabBar->setDrawBase(false);

    tabBar->setMinimumWidth(tabBar->minimumSizeHint().width());

    // ===== Toolbar Stack =====
    //QStackedWidget *toolbarStack = new QStackedWidget(this);
    m_toolbarStack = new QStackedWidget(this);

    // --- Nút "Bộ lọc" có menu thả --- //
    QToolButton *btnFilter = new QToolButton;
    btnFilter->setText("  Bộ lọc   ▼");
    btnFilter->setIcon(QIcon(":/resource/icons/filter.png"));
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
        {"Red category",    ":/resource/icons/red_tag.png"},
        {"Orange category", ":/resource/icons/orange_tag.png"},
        {"Yellow category", ":/resource/icons/yellow_tag.png"},
        {"Green category",  ":/resource/icons/green_tag.png"},
        {"Blue category",   ":/resource/icons/blue_tag.png"},
        {"Purple category", ":/resource/icons/purple_tag.png"}
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
        b->setIcon(QIcon(":/resource/icons/filter.png"));
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
    btnNewEvent->setIcon(QIcon(":/resource/icons/calendar.png"));
    btnNewEvent->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btnNewEvent->setCursor(Qt::PointingHandCursor);
    // THAY ĐỔI: Chuyển từ MenuButtonPopup sang chế độ nút bấm thường
    // btnNewEvent->setPopupMode(QToolButton::MenuButtonPopup);
    btnNewEvent->setObjectName("btnNewEvent");

    // Menu thả xuống
    QMenu *newEventMenu = new QMenu(btnNewEvent);
    QAction *actNewMail = newEventMenu->addAction(QIcon(":/resource/icons/message.png"), "Thư");
    QAction *actNewEvent = newEventMenu->addAction(QIcon(":/resource/icons/calendarEvent.png"), "Sự kiện");
    newEventMenu->setObjectName("eventMenu");
    btnNewEvent->setMenu(newEventMenu);
    addShadowEffect(newEventMenu);

    homeLayout->addWidget(btnNewEvent);

    // --- Nút "Ngày" có menu thả ---
    QToolButton *btnDay = new QToolButton;
    btnDay->setText("  Ngày");
    btnDay->setIcon(QIcon(":/resource/icons/7days.png"));
    btnDay->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btnDay->setCursor(Qt::PointingHandCursor);
    btnDay->setPopupMode(QToolButton::InstantPopup);
    btnDay->setObjectName("btnDay");

    QMenu *dayMenu = new QMenu(btnDay);
    QAction *actOneDay = dayMenu->addAction("1 Ngày");
    QAction *actThreeDay = dayMenu->addAction("3 Ngày");
    QAction *actFiveDay = dayMenu->addAction("5 Ngày");
    QAction *actSevenDay = dayMenu->addAction("7 Ngày");
    dayMenu->setObjectName("dayMenu");
    addShadowEffect(dayMenu);

    // Dùng lambda để gọi slot mới với số ngày tương ứng
    connect(actOneDay, &QAction::triggered, this, [this](){ onDisplayDaysChanged(1); });
    connect(actThreeDay, &QAction::triggered, this, [this](){ onDisplayDaysChanged(3); });
    connect(actFiveDay, &QAction::triggered, this, [this](){ onDisplayDaysChanged(5); });
    connect(actSevenDay, &QAction::triggered, this, [this](){ onDisplayDaysChanged(7); });

    btnDay->setMenu(dayMenu);
    homeLayout->addWidget(btnDay);

    QToolButton *btnWorkWeek = makeBtn("Tuần làm việc", ":/resource/icons/workWeek.png");
    connect(btnWorkWeek, &QToolButton::clicked, this, &MainWindow::showWorkWeek);
    homeLayout->addWidget(btnWorkWeek);

    QToolButton *btnWeek = makeBtn("Tuần", ":/resource/icons/week.png");
    connect(btnWeek, &QToolButton::clicked, this, &MainWindow::showFullWeek);
    homeLayout->addWidget(btnWeek);

    QToolButton *btnMonth = makeBtn("Tháng", ":/resource/icons/month.png");
    connect(btnMonth, &QToolButton::clicked, this, &MainWindow::showMonthView); // <-- THÊM CONNECT
    homeLayout->addWidget(btnMonth);

    QToolButton *btnSplitView = makeBtn("Thời khóa biểu", ":/resource/icons/split.png");
    btnSplitView->setPopupMode(QToolButton::InstantPopup); // Chuyển thành menu

    // Tạo menu dùng chung
    QMenu *splitViewMenu = new QMenu(this);
    QAction *actPerSlot = splitViewMenu->addAction("Xem theo Tiết");
    QAction *actPerSession = splitViewMenu->addAction("Xem theo Buổi");

    connect(actPerSlot, &QAction::triggered, this, &MainWindow::showTimetableView);
    connect(actPerSession, &QAction::triggered, this, &MainWindow::showSessionView);

    btnSplitView->setMenu(splitViewMenu);
    homeLayout->addWidget(btnSplitView);

    //Gạch dọc chia
    homeLayout->addWidget(makeSeparator());

    homeLayout->addWidget(makeFilterButton());

    //Gạch dọc chia
    homeLayout->addWidget(makeSeparator());

    homeLayout->addWidget(makeBtn("In", ":/resource/icons/printer.png"));
    homeLayout->addStretch();

    m_toolbarStack->addWidget(homePage);

    // --- View toolbar ---
    QWidget *viewPage = new QWidget;
    QHBoxLayout *viewLayout = new QHBoxLayout(viewPage);
    viewLayout->setContentsMargins(10, 6, 10, 6);
    viewLayout->setSpacing(10);

    // --- Nút "Ngày" ---
    QToolButton *btnDayView = new QToolButton;
    btnDayView->setText("  Ngày");
    btnDayView->setIcon(QIcon(":/resource/icons/7days.png"));
    btnDayView->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btnDayView->setCursor(Qt::PointingHandCursor);
    btnDayView->setPopupMode(QToolButton::InstantPopup);
    btnDayView->setObjectName("btnDayView");

    btnDayView->setMenu(dayMenu); // Gán menu dùng chung
    viewLayout->addWidget(btnDayView);

    // --- Các nút còn lại ---
    QToolButton *btnWorkWeekView = makeBtn("Tuần làm việc", ":/resource/icons/workWeek.png");
    connect(btnWorkWeekView, &QToolButton::clicked, this, &MainWindow::showWorkWeek);
    viewLayout->addWidget(btnWorkWeekView);

    QToolButton *btnWeekView = makeBtn("Tuần", ":/resource/icons/week.png");
    connect(btnWeekView, &QToolButton::clicked, this, &MainWindow::showFullWeek);
    viewLayout->addWidget(btnWeekView);

    QToolButton *btnMonthView = makeBtn("Tháng", ":/resource/icons/month.png");
    connect(btnMonthView, &QToolButton::clicked, this, &MainWindow::showMonthView); // <-- THÊM CONNECT
    viewLayout->addWidget(btnMonthView);

    QToolButton *btnSplitView_View = makeBtn("Thời khóa biểu", ":/resource/icons/split.png");
    btnSplitView_View->setPopupMode(QToolButton::InstantPopup); // Chuyển thành menu
    btnSplitView_View->setMenu(splitViewMenu); // Dùng chung menu
    viewLayout->addWidget(btnSplitView_View);

    viewLayout->addWidget(makeBtn("Lưu dạng xem", ":/resource/icons/save.png"));

    // --- Nút "Tỉ lệ thời gian" ---
    m_btnTimeScale = new QToolButton;
    m_btnTimeScale->setText("  Tỉ lệ thời gian  ▼"); // <-- THAY ĐỔI 1: Thêm mũi tên
    m_btnTimeScale->setIcon(QIcon(":/resource/icons/timeScale.png"));
    m_btnTimeScale->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_btnTimeScale->setCursor(Qt::PointingHandCursor);
    m_btnTimeScale->setPopupMode(QToolButton::InstantPopup); // <-- THAY ĐỔI 2: Chế độ popup
    m_btnTimeScale->setObjectName("btnTimeScale");

    // Tạo menu thả xuống
    QMenu *timeScaleMenu = new QMenu(m_btnTimeScale);
    QAction *act60min = timeScaleMenu->addAction("60 phút - Ít chi tiết");
    QAction *act30min = timeScaleMenu->addAction("30 phút");
    QAction *act15min = timeScaleMenu->addAction("15 phút");
    QAction *act10min = timeScaleMenu->addAction("10 phút");
    QAction *act6min  = timeScaleMenu->addAction("6 phút");
    QAction *act5min  = timeScaleMenu->addAction("5 phút - Nhiều chi tiết");

    connect(act60min, &QAction::triggered, this, [this](){ onTimeScaleChanged(60); });
    connect(act30min, &QAction::triggered, this, [this](){ onTimeScaleChanged(30); });
    connect(act15min, &QAction::triggered, this, [this](){ onTimeScaleChanged(15); });
    connect(act10min, &QAction::triggered, this, [this](){ onTimeScaleChanged(10); });
    connect(act6min, &QAction::triggered, this, [this](){ onTimeScaleChanged(6); });
    connect(act5min, &QAction::triggered, this, [this](){ onTimeScaleChanged(5); });

    // Gán menu vào nút
    m_btnTimeScale->setMenu(timeScaleMenu);

    // Thêm nút vào layout
    viewLayout->addWidget(m_btnTimeScale);

    timeScaleMenu->setObjectName("timeScaleMenu");
    addShadowEffect(timeScaleMenu);

    // Gán menu vào nút
    m_btnTimeScale->setMenu(timeScaleMenu);

    viewLayout->addWidget(makeSeparator());

    viewLayout->addWidget(makeFilterButton());

    viewLayout->addWidget(makeSeparator());

    QToolButton *btnSettings = makeBtn("Cài đặt", ":/resource/icons/setting.png");
    connect(btnSettings, &QToolButton::clicked, this, &MainWindow::openSettingsDialog);
    viewLayout->addWidget(btnSettings);

    viewLayout->addStretch();

    m_toolbarStack->addWidget(viewPage);

    // --- Help toolbar ---
    QWidget *helpPage = new QWidget;
    QHBoxLayout *helpLayout = new QHBoxLayout(helpPage);
    helpLayout->setContentsMargins(10, 6, 10, 6);
    helpLayout->setSpacing(10);

    QToolButton *btnShowHelp = makeBtn("Trợ giúp", ":/resource/icons/question.png");
    connect(btnShowHelp, &QToolButton::clicked, this, &MainWindow::toggleHelpPanel);
    helpLayout->addWidget(btnShowHelp);

    QToolButton *btnTips = makeBtn("Mẹo", ":/resource/icons/lightbulb.png");
    connect(btnTips, &QToolButton::clicked, this, &MainWindow::toggleTipsPanel);
    helpLayout->addWidget(btnTips);

    QToolButton *btnSupport = makeBtn("Hỗ trợ", ":/resource/icons/support.png");
    connect(btnSupport, &QToolButton::clicked, this, &MainWindow::toggleSupportPanel); // <-- THÊM DÒNG NÀY
    helpLayout->addWidget(btnSupport);

    QToolButton *btnFeedback = makeBtn("Phản hồi", ":/resource/icons/feedback.png");
    connect(btnFeedback, &QToolButton::clicked, this, &MainWindow::toggleFeedbackPanel); // <-- THÊM DÒNG NÀY
    helpLayout->addWidget(btnFeedback);

    helpLayout->addWidget(makeSeparator());
    QToolButton *btnGithub = makeBtn("Đi tới Github", ":/resource/icons/github.png");
    connect(btnGithub, &QToolButton::clicked, this, []() {
        QDesktopServices::openUrl(QUrl("https://github.com/dada-DSA-OOP/Calendar_PROJECT"));
    });
    helpLayout->addWidget(btnGithub);
    helpLayout->addStretch();
    m_toolbarStack->addWidget(helpPage);

    // ===== Menu cố định =====
    m_topBar = new QWidget;
    m_topBar->setObjectName("topBar");
    QVBoxLayout *topLayout = new QVBoxLayout(m_topBar);
    topLayout->setContentsMargins(0,0,0,0);
    topLayout->setSpacing(0);
    // Gộp nút 3 gạch và tabBar chung hàng
    QWidget *tabBarContainer = new QWidget;
    QHBoxLayout *tabBarLayout = new QHBoxLayout(tabBarContainer);
    tabBarLayout->setContentsMargins(8, 0, 0, 0);
    tabBarLayout->setSpacing(8);
    tabBarLayout->addWidget(m_btnSidebarToggle);
    tabBarLayout->addWidget(tabBar);
    tabBarLayout->addStretch(1);
    topLayout->addWidget(tabBarContainer);
    topLayout->addWidget(m_toolbarStack);
    QFrame *separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Plain);
    separator->setStyleSheet("color: #dcdcdc;");
    topLayout->addWidget(separator);

    // ===============================================================
    // === BẮT ĐẦU PHẦN SỬA LỖI: TẠO NỘI DUNG CHO CÁC PANEL ===
    // ===============================================================

    // --- 1. Chuẩn bị nội dung cho Help Panel ---
    QWidget *helpContentWidget = new QWidget;
    QVBoxLayout *helpContentLayout = new QVBoxLayout(helpContentWidget);
    auto makeHelpLabel = [](const QString &text) {
        QLabel *label = new QLabel(text);
        label->setWordWrap(true);
        return label;
    };

    QGroupBox *helpGb1 = new QGroupBox("Tạo Sự kiện Mới");
    helpGb1->setLayout(new QVBoxLayout);
    helpGb1->layout()->addWidget(makeHelpLabel("- Nhấn nút 'Sự kiện mới' ở tab 'Trang chủ'.\n- Điền đầy đủ thông tin và chọn 'OK'."));
    helpContentLayout->addWidget(helpGb1);

    QGroupBox *helpGb2 = new QGroupBox("Điều hướng Lịch");
    helpGb2->setLayout(new QVBoxLayout);
    helpGb2->layout()->addWidget(makeHelpLabel("- Dùng các nút mũi tên ◀, ▶ để chuyển tuần.\n- Nhấn 'Hôm nay' để quay về tuần hiện tại.\n- Nhấn vào tên tháng để chọn ngày bất kỳ."));
    helpContentLayout->addWidget(helpGb2);

    QGroupBox *helpGb3 = new QGroupBox("Tương tác với Sự kiện");
    helpGb3->setLayout(new QVBoxLayout);
    helpGb3->layout()->addWidget(makeHelpLabel("- Kéo thả để di chuyển sự kiện sang ngày/giờ khác.\n- Kéo cạnh dưới của sự kiện để thay đổi thời gian kết thúc."));
    helpContentLayout->addWidget(helpGb3);
    helpContentLayout->addStretch();

    // --- 2. Chuẩn bị nội dung cho Tips Panel ---
    QWidget *tipsContentWidget = new QWidget;
    QVBoxLayout *tipsContentLayout = new QVBoxLayout(tipsContentWidget);

    QGroupBox *tipGb1 = new QGroupBox("Đặt làm lịch mặc định");
    tipGb1->setLayout(new QVBoxLayout);
    tipGb1->layout()->addWidget(makeHelpLabel("Tính năng này sẽ sớm được cập nhật để bạn có thể quản lý tất cả sự kiện từ một nơi duy nhất!"));
    tipsContentLayout->addWidget(tipGb1);

    QGroupBox *tipGb2 = new QGroupBox("Thay đổi múi giờ");
    tipGb2->setLayout(new QVBoxLayout);
    tipGb2->layout()->addWidget(makeHelpLabel("Đi du lịch? Vào 'Cài đặt' để thay đổi múi giờ, đảm bảo bạn không bao giờ bị trễ hẹn dù đang ở bất cứ đâu."));
    tipsContentLayout->addWidget(tipGb2);
    tipsContentLayout->addStretch();

    // --- TẠO FUNNY TIP WIDGET BẰNG LỚP MỚI ---
    m_funnyTipWidget = new FunnyTipWidget(this);
    m_funnyTipWidget->start();

    // --- 3. Chuẩn bị nội dung cho Support Panel ---
    QWidget *supportContentWidget = new QWidget;
    QVBoxLayout *supportContentLayout = new QVBoxLayout(supportContentWidget);
    supportContentLayout->setSpacing(15);

    // -- Khung cảnh báo thu thập dữ liệu --
    QGroupBox *dataGroupBox = new QGroupBox("Thu thập dữ liệu chẩn đoán");
    QVBoxLayout *dataLayout = new QVBoxLayout(dataGroupBox);
    dataLayout->setSpacing(10);

    QLabel *warningLabel = new QLabel("Để cải thiện ứng dụng, chúng tôi có thể thu thập dữ liệu sử dụng ẩn danh. Dữ liệu này không chứa thông tin cá nhân. Bạn có đồng ý không?");
    warningLabel->setWordWrap(true);
    dataLayout->addWidget(warningLabel);

    QRadioButton *agreeButton = new QRadioButton("Đồng ý");
    QRadioButton *disagreeButton = new QRadioButton("Không, cảm ơn");
    agreeButton->setChecked(true); // Mặc định là đồng ý

    QHBoxLayout *radioLayout = new QHBoxLayout;
    radioLayout->addWidget(agreeButton);
    radioLayout->addWidget(disagreeButton);
    radioLayout->addStretch();
    dataLayout->addLayout(radioLayout);
    supportContentLayout->addWidget(dataGroupBox);

    // -- Khung gửi phản hồi --
    QGroupBox *feedbackGroupBox = new QGroupBox("Gửi phản hồi cho chúng tôi");
    QVBoxLayout *feedbackLayout = new QVBoxLayout(feedbackGroupBox);
    feedbackLayout->setSpacing(10);

    QTextEdit *feedbackTextEdit = new QTextEdit;
    feedbackTextEdit->setPlaceholderText("Nhập phản hồi của bạn ở đây...");
    feedbackLayout->addWidget(feedbackTextEdit);

    QPushButton *submitButton = new QPushButton("Gửi đi");
    submitButton->setObjectName("submitButton");
    submitButton->setCursor(Qt::PointingHandCursor);

    // Kết nối nút Gửi đi
    connect(submitButton, &QPushButton::clicked, this, [this, feedbackTextEdit]() {
        QMessageBox::information(this, "Đã gửi", "Cảm ơn bạn đã gửi phản hồi!");
        feedbackTextEdit->clear();
    });

    feedbackLayout->addWidget(submitButton, 0, Qt::AlignRight);
    supportContentLayout->addWidget(feedbackGroupBox);

    supportContentLayout->addStretch();

    // --- 4. Tạo các panel và gán nội dung ---
    m_helpPanel = new SidePanel("Trợ giúp", this);
    m_helpPanel->setContentLayout(helpContentLayout);
    m_helpPanel->hide();

    m_tipsPanel = new SidePanel("Mẹo & Thủ thuật", this);
    m_tipsPanel->setContentLayout(tipsContentLayout);
    m_tipsPanel->hide();

    // TẠO SUPPORT PANEL
    m_supportPanel = new SidePanel("Hỗ trợ", this);
    m_supportPanel->setContentLayout(supportContentLayout);
    m_supportPanel->hide();

    // --- 4. Chuẩn bị nội dung cho Feedback Panel ---
    QWidget *feedbackContentWidget = new QWidget;
    QVBoxLayout *feedbackContentLayout = new QVBoxLayout(feedbackContentWidget);
    feedbackContentLayout->setSpacing(15);

    // -- Khung lựa chọn loại phản hồi --
    QGroupBox *typeGroupBox = new QGroupBox("Bạn muốn chia sẻ điều gì?");
    QVBoxLayout *typeLayout = new QVBoxLayout(typeGroupBox);

    QRadioButton *positiveRadio = new QRadioButton("Tôi có một lời khen");
    QRadioButton *negativeRadio = new QRadioButton("Tôi không thích một điều gì đó");
    QRadioButton *bugRadio = new QRadioButton("Tôi nghĩ tôi đã tìm thấy một lỗi");
    positiveRadio->setChecked(true);

    typeLayout->addWidget(positiveRadio);
    typeLayout->addWidget(negativeRadio);
    typeLayout->addWidget(bugRadio);
    feedbackContentLayout->addWidget(typeGroupBox);

    // -- Khu vực nhập liệu động với QStackedWidget --
    QStackedWidget *stackedWidget = new QStackedWidget;

    // Hàm trợ giúp để tạo một trang nhập liệu
    auto createFeedbackPage = [&](const QString &placeholder) {
        QWidget *page = new QWidget;
        QVBoxLayout *layout = new QVBoxLayout(page);
        QTextEdit *textEdit = new QTextEdit;
        textEdit->setPlaceholderText(placeholder);
        QPushButton *submitButton = new QPushButton("Gửi");
        submitButton->setObjectName("submitButton");
        submitButton->setCursor(Qt::PointingHandCursor);

        connect(submitButton, &QPushButton::clicked, this, [this, textEdit]() {
            QMessageBox::information(this, "Đã gửi", "Cảm ơn bạn đã chia sẻ phản hồi!");
            textEdit->clear();
        });

        layout->addWidget(textEdit);
        layout->addWidget(submitButton, 0, Qt::AlignRight);
        return page;
    };

    // Tạo 3 trang tương ứng
    stackedWidget->addWidget(createFeedbackPage("Hãy cho chúng tôi biết bạn thích điều gì..."));
    stackedWidget->addWidget(createFeedbackPage("Chúng tôi có thể cải thiện điều gì?"));
    stackedWidget->addWidget(createFeedbackPage("Vui lòng mô tả lỗi bạn gặp phải..."));

    feedbackContentLayout->addWidget(stackedWidget);

    // Kết nối các radio button để chuyển trang
    connect(positiveRadio, &QRadioButton::toggled, [=](bool checked){
        if (checked) stackedWidget->setCurrentIndex(0);
    });
    connect(negativeRadio, &QRadioButton::toggled, [=](bool checked){
        if (checked) stackedWidget->setCurrentIndex(1);
    });
    connect(bugRadio, &QRadioButton::toggled, [=](bool checked){
        if (checked) stackedWidget->setCurrentIndex(2);
    });

    feedbackContentLayout->addStretch();

    // --- 5. Tạo các panel và gán nội dung ---
    // ... (code tạo help/tips/support panel giữ nguyên)

    // TẠO FEEDBACK PANEL
    m_feedbackPanel = new SidePanel("Gửi phản hồi", this);
    m_feedbackPanel->setContentLayout(feedbackContentLayout);
    m_feedbackPanel->hide();

    // === MAIN CONTENT ===

    // -- BƯỚC 1: KHỞI TẠO TẤT CẢ CÁC WIDGET CẦN THIẾT --

    // Thanh điều hướng và layout của nó
    QWidget *dateNavBar = new QWidget;
    dateNavBar->setObjectName("dateNavBar");
    QHBoxLayout *dateNavLayout = new QHBoxLayout(dateNavBar);
    dateNavLayout->setContentsMargins(10, 5, 10, 5);
    dateNavLayout->setSpacing(8);

    // Các nút và label trên thanh điều hướng
    m_btnPrevWeek = new QPushButton;
    m_btnPrevWeek->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    m_btnPrevWeek->setObjectName("navButton");

    QPushButton *btnToday = new QPushButton("Hôm nay");
    btnToday->setObjectName("navButton");

    m_btnNextWeek = new QPushButton;
    m_btnNextWeek->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    m_btnNextWeek->setObjectName("navButton");

    // THAY THẾ BẰNG KHỐI CODE SAU:
    m_dateNavButton = new QPushButton;
    m_dateNavButton->setObjectName("dateNavButton"); // Đặt tên để style QSS
    m_dateNavButton->setCursor(Qt::PointingHandCursor);

    // Tạo lịch popup
    m_calendarPopup = new QCalendarWidget(this);

    // Tên thứ và dịch tháng/năm sang Tiếng Việt
    m_calendarPopup->setLocale(QLocale(QLocale::Vietnamese));

    m_calendarPopup->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);

    // --- THÊM ĐOẠN CODE NÀY ĐỂ HIGHLIGHT NGÀY HIỆN TẠI ---
    QTextCharFormat todayFormat;
    // Tạo một viền màu xanh bao quanh ngày hôm nay
    todayFormat.setTextOutline(QPen(QColor("#0078d7"), 1));
    m_calendarPopup->setDateTextFormat(QDate::currentDate(), todayFormat);

    // Dùng QWidgetAction để đưa QCalendarWidget vào menu
    QWidgetAction *calendarAction = new QWidgetAction(this);
    calendarAction->setDefaultWidget(m_calendarPopup);

    // Tạo menu để chứa calendar
    QMenu *calendarMenu = new QMenu(m_dateNavButton);
    calendarMenu->addAction(calendarAction);
    m_dateNavButton->setMenu(calendarMenu);

    // Vùng chứa lịch và layout của nó
    QWidget *calendarContainer = new QWidget;
    QGridLayout *grid = new QGridLayout(calendarContainer);

    // === Sidebar lịch nhỏ ===
    m_sidebarCalendar = new QWidget(this);
    m_sidebarCalendar->setObjectName("sidebarCalendar");
    m_sidebarCalendar->setFixedWidth(0);  // ban đầu ẩn

    QVBoxLayout *sidebarLayout = new QVBoxLayout(m_sidebarCalendar);
    sidebarLayout->setContentsMargins(10, 10, 10, 10);

    QCalendarWidget *miniCalendar = new QCalendarWidget(m_sidebarCalendar);
    miniCalendar->setFixedSize(200, 220);
    miniCalendar->setLocale(QLocale(QLocale::Vietnamese));
    miniCalendar->setFirstDayOfWeek(Qt::Monday);
    miniCalendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    sidebarLayout->addWidget(miniCalendar);
    // === PHẦN GHI CHÚ DƯỚI LỊCH NHỎ ===
    QLabel *noteTitle = new QLabel("📝 Ghi chú");
    noteTitle->setStyleSheet("font-weight: bold; margin-top:10px;");

    // Ô nhập + nút thêm
    QTextEdit *noteInput = new QTextEdit;
    noteInput->setPlaceholderText("Thêm việc cần làm...");
    noteInput->setObjectName("noteInput");
    noteInput->setMaximumHeight(65); // Giới hạn chiều cao, ví dụ: tương đương 3 dòng

    QPushButton *btnAddNote = new QPushButton("+");
    btnAddNote->setObjectName("btnAddNote"); // <-- THÊM TÊN OBJECT
    btnAddNote->setCursor(Qt::PointingHandCursor);
    btnAddNote->setToolTip("Thêm công việc");

    QHBoxLayout *addLayout = new QHBoxLayout;
    addLayout->setContentsMargins(0,0,0,0);
    addLayout->setSpacing(5);
    addLayout->addWidget(noteInput);
    addLayout->addWidget(btnAddNote);

    // Danh sách công việc
    QListWidget *todoList = new QListWidget;
    todoList->setObjectName("todoList"); // <-- THÊM TÊN OBJECT
    todoList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    todoList->verticalScrollBar()->setStyleSheet(
        "QScrollBar::sub-line:vertical {"
        "    border: none;"
        "    background: none;"
        "    height: 0px;"
        "}"
        "QScrollBar::add-line:vertical {"
        "    border: none;"
        "    background: none;"
        "    height: 0px;"
        "}"
    );

    noteInput->verticalScrollBar()->setStyleSheet(
        "QScrollBar::sub-line:vertical {"
        "    border: none;"
        "    background: none;"
        "    height: 0px;"
        "}"
        "QScrollBar::add-line:vertical {"
        "    border: none;"
        "    background: none;"
        "    height: 0px;"
        "}"
        );
    // XÓA HẾT STYLESHEET Ở ĐÂY

    // XÓA DÒNG sidebarLayout->addWidget(miniCalendar); BỊ LẶP
    sidebarLayout->addWidget(noteTitle);
    sidebarLayout->addLayout(addLayout);
    sidebarLayout->addWidget(todoList, 1);

    // ===== HÀM THÊM CÔNG VIỆC (ĐÃ TỐI ƯU HÓA) =====
    auto addTodoItem = [=]() {
        QString text = noteInput->toPlainText().trimmed();
        if (text.isEmpty()) return;

        QListWidgetItem *item = new QListWidgetItem(todoList);
        QWidget *itemWidget = new QWidget;

        // Giảm lề và khoảng cách một chút để có thêm không gian
        QHBoxLayout *itemLayout = new QHBoxLayout(itemWidget);
        itemLayout->setContentsMargins(8, 4, 4, 4); // Lề trái, trên, phải, dưới
        itemLayout->setSpacing(6);

        // --- CÁC WIDGET CON ---
        QCheckBox *check = new QCheckBox;
        QLabel *todoLabel = new QLabel(text);
        todoLabel->setWordWrap(true);

        // **SỬA LỖI 1: BÁO CHO LAYOUT BIẾT LABEL CÓ THỂ BỊ CO LẠI TỐI ĐA**
        todoLabel->setMinimumWidth(0);
        todoLabel->setMaximumWidth(100);

        // **SỬA LỖI 2: ĐẶT CHÍNH SÁCH KÍCH THƯỚC ĐỂ NÚT XÓA KHÔNG BỊ CO LẠI**
        QPushButton *btnDel = new QPushButton("×");
        btnDel->setObjectName("btnDeleteTodo");
        btnDel->setCursor(Qt::PointingHandCursor);
        btnDel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); // Nút luôn có kích thước cố định

        // --- THÊM WIDGET VÀO LAYOUT ---
        itemLayout->addWidget(check);
        itemLayout->addWidget(todoLabel, 1); // Tham số 1 để label lấp đầy không gian
        itemLayout->addWidget(btnDel);
        itemWidget->setLayout(itemLayout);

        // --- GÁN VÀO LIST ---
        item->setSizeHint(itemWidget->sizeHint());
        todoList->addItem(item);
        todoList->setItemWidget(item, itemWidget);
        noteInput->clear();

        // --- KẾT NỐI TÍN HIỆU (connect) ---
        // (Toàn bộ phần connect giữ nguyên như cũ, không cần thay đổi)
        connect(check, &QCheckBox::checkStateChanged, [=](Qt::CheckState state){
            bool completed = (state == Qt::Checked);
            if (completed) {
                todoLabel->setText(QString("<p style='white-space: normal; word-break: break-all; text-align: justify;'>%1</p>").arg(text.toHtmlEscaped()));
                todoLabel->setStyleSheet("color: #999; text-decoration: line-through;");
                check->setStyleSheet("QCheckBox::indicator:checked { image: url(:/resource/icons/check-green.png); }");
                itemWidget->setStyleSheet("background-color: #f0f0f0;");
            } else {
                todoLabel->setText(QString("<p style='white-space: normal; word-break: break-all; text-align: justify;'>%1</p>").arg(text.toHtmlEscaped()));
                todoLabel->setStyleSheet("");
                check->setStyleSheet("");
                itemWidget->setStyleSheet("");
            }
        });

        connect(btnDel, &QPushButton::clicked, [=]() {
            int row = todoList->row(item);
            delete todoList->takeItem(row);
        });
    };

    // --- Thêm bằng nút hoặc phím Enter ---
    connect(btnAddNote, &QPushButton::clicked, this, addTodoItem);

    // THÊM DÒNG KẾT NỐI NÀY
    connect(miniCalendar, &QCalendarWidget::clicked, this, &MainWindow::onDateSelectedFromPopup);

    grid->setSpacing(0);
    grid->setContentsMargins(0, 0, 0, 0);

    // Các thành phần của lịch
    m_dayHeader = new DayHeader;
    m_dayHeader->setObjectName("dayHeaderWidget");

    m_timeRuler = new TimeRuler;
    m_timeRuler->setObjectName("timeRulerWidget");
    m_timeRuler->setFixedWidth(60); // Cố định chiều rộng của widget

    m_calendarView = new CalendarView;
    m_calendarView->setObjectName("mainCalendarView");

    m_calendarCorner = new QWidget;
    m_calendarCorner->setObjectName("calendarCornerWidget");
    m_calendarCorner->setFixedSize(60, 60);


    // -- BƯỚC 2: THÊM CÁC WIDGET VÀO LAYOUT --

    // TẠO PAGE CHO CHẾ ĐỘ XEM THÁNG
    m_monthView = new MonthViewWidget;
    m_monthView->setObjectName("monthViewWidget");

    // --- THÊM MỚI: TẠO PAGE CHO CHẾ ĐỘ XEM TKB ---
    m_timetableView = new TimetableViewWidget;
    m_timetableView->setObjectName("timetableViewWidget");

    // --- THÊM MỚI: TẠO PAGE CHO CHẾ ĐỘ XEM BUỔI ---
    m_sessionView = new SessionViewWidget;
    m_sessionView->setObjectName("sessionViewWidget");

    // TẠO STACKED WIDGET
    m_viewStack = new QStackedWidget;
    m_viewStack->addWidget(m_calendarView);      // Page 0 (Timeline)
    m_viewStack->addWidget(m_monthView);         // Page 1 (Tháng)
    m_viewStack->addWidget(m_timetableView);     // Page 2 (TKB Tiết)
    m_viewStack->addWidget(m_sessionView);       // Page 3 (TKB Buổi)

    // Thêm vào thanh điều hướng
    dateNavLayout->addWidget(m_btnPrevWeek);
    dateNavLayout->addWidget(btnToday);
    dateNavLayout->addWidget(m_btnNextWeek);
    dateNavLayout->addWidget(m_dateNavButton, 1);

    // Thêm vào lưới lịch (Layout 'grid' của 'calendarContainer')
    grid->addWidget(m_calendarCorner, 0, 0); // (Hàng 0, Cột 0)
    grid->addWidget(m_dayHeader, 0, 1);      // (Hàng 0, Cột 1)
    //grid->addWidget(ruler, 1, 0);
    //grid->addWidget(m_calendarView, 1, 1);
    grid->addWidget(m_timeRuler, 1, 0);      // (Hàng 1, Cột 0)
    grid->addWidget(m_viewStack, 1, 1);      // (Hàng 1, Cột 1)

    // Đồng bộ cột và hàng
    grid->setColumnStretch(1, 1); // Cho phép Cột 1 (chứa header và stack) co giãn
    grid->setRowStretch(1, 1);    // Cho phép Hàng 1 (chứa ruler và stack) co giãn

    // Thêm vào layout chính của cửa sổ
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_topBar, 0, Qt::AlignTop);
    mainLayout->addWidget(dateNavBar);

    // Bọc calendarContainer và sidebar chung layout ngang
    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->setContentsMargins(0,0,0,0);
    contentLayout->setSpacing(0);
    contentLayout->addWidget(m_sidebarCalendar);
    contentLayout->addWidget(calendarContainer, 1);

    mainLayout->addLayout(contentLayout, 1);

    setCentralWidget(central);


    // -- BƯỚC 3: KẾT NỐI TÍN HIỆU (SIGNALS & SLOTS) --

    connect(m_calendarView->horizontalScrollBar(), &QScrollBar::valueChanged, m_dayHeader, &DayHeader::setScrollOffset);
    connect(m_calendarView->verticalScrollBar(), &QScrollBar::valueChanged, m_timeRuler, &TimeRuler::setScrollOffset);
    connect(tabBar, &QTabBar::currentChanged, this, [=](int index) {
        m_toolbarStack->setCurrentIndex(index);
    });
    connect(m_btnPrevWeek, &QPushButton::clicked, this, &MainWindow::showPreviousWeek);
    connect(m_btnNextWeek, &QPushButton::clicked, this, &MainWindow::showNextWeek);
    connect(btnToday, &QPushButton::clicked, this, &MainWindow::showToday);
    connect(btnNewEvent, &QToolButton::clicked, this, &MainWindow::onNewEventClicked);
    connect(m_calendarPopup, &QCalendarWidget::clicked, this, &MainWindow::onDateSelectedFromPopup);

    connect(btnShowHelp, &QToolButton::clicked, this, &MainWindow::toggleHelpPanel);
    connect(btnTips, &QToolButton::clicked, this, &MainWindow::toggleTipsPanel);


    // -- BƯỚC 4: THÊM DỮ LIỆU MẪU --
    // ...
    QDate monday = QDate::currentDate().addDays(-(QDate::currentDate().dayOfWeek() - 1));

    m_calendarView->addEvent("Toán rời rạc", QColor("#a7d7f9"), QDateTime(monday, QTime(7, 0)), QDateTime(monday, QTime(11, 30))); // Sửa lại endTime
    m_monthView->addEvent(new EventItem("Toán rời rạc", QColor("#a7d7f9"), QDateTime(monday, QTime(7, 0)), QDateTime(monday, QTime(11, 30))));
    m_timetableView->addEvent(new EventItem("Toán rời rạc", QColor("#a7d7f9"), QDateTime(monday, QTime(7, 0)), QDateTime(monday, QTime(11, 30))));
    m_sessionView->addEvent(new EventItem("Toán rời rạc", QColor("#a7d7f9"), QDateTime(monday, QTime(7, 0)), QDateTime(monday, QTime(11, 30))));

    QDate tuesday = monday.addDays(1);
    m_calendarView->addEvent("Lập trình HĐT", QColor("#a7d7f9"), QDateTime(tuesday, QTime(13, 0)), QDateTime(tuesday, QTime(17, 30))); // Sửa lại endTime
    m_monthView->addEvent(new EventItem("Lập trình HĐT", QColor("#a7d7f9"), QDateTime(tuesday, QTime(13, 0)), QDateTime(tuesday, QTime(17, 30))));
    m_timetableView->addEvent(new EventItem("Lập trình HĐT", QColor("#a7d7f9"), QDateTime(tuesday, QTime(13, 0)), QDateTime(tuesday, QTime(17, 30))));
    m_sessionView->addEvent(new EventItem("Lập trình HĐT", QColor("#a7d7f9"), QDateTime(tuesday, QTime(13, 0)), QDateTime(tuesday, QTime(17, 30))));

    tabBar->setCurrentIndex(0);
    m_toolbarStack->setCurrentIndex(0);

    // Gọi hàm này ở cuối cùng, sau khi mọi thứ đã được tạo
    showToday();
    m_helpPanel->hide();
    m_tipsPanel->hide();

    // --- THÊM MỚI: Tự động cuộn đến 6 giờ sáng ---
    // Lấy thanh cuộn dọc từ CalendarView
    QScrollBar *verticalScrollBar = m_calendarView->verticalScrollBar();
    // Tính toán vị trí pixel tương ứng với 6 giờ sáng
    int scrollToPosition = 7.5 * m_calendarView->getHourHeight();
    // Đặt giá trị cho thanh cuộn
    verticalScrollBar->setValue(scrollToPosition);

    // === Kết nối nút 3 gạch ===
    connect(m_btnSidebarToggle, &QToolButton::clicked, this, [this]() {
        int startWidth = m_sidebarCalendar->width();
        int endWidth = m_sidebarVisible ? 0 : 220;
        QPropertyAnimation *anim = new QPropertyAnimation(m_sidebarCalendar, "minimumWidth");
        anim->setDuration(250);
        anim->setStartValue(startWidth);
        anim->setEndValue(endWidth);
        anim->setEasingCurve(QEasingCurve::InOutCubic);
        anim->start(QAbstractAnimation::DeleteWhenStopped);

        m_sidebarVisible = !m_sidebarVisible;
    });

    // Đặt độ trong suốt mặc định cho lịch
    setCalendarTransparency(true);

    changeBackgroundImage(2, QString(), QColor());
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ----- CÁC HÀM LOGIC MỚI -----

// Sửa hàm updateCalendarDisplay
void MainWindow::updateCalendarDisplay()
{
    int daysToShow;
    QDate endDate; // Biến để lưu ngày kết thúc

    // KIỂM TRA XEM ĐANG Ở VIEW NÀO
    if (m_viewStack->currentWidget() == m_monthView) {
        // Chế độ xem tháng
        daysToShow = 7; // Header luôn là 7 ngày
        endDate = m_currentMonday.addDays(6); // 7 ngày, bắt đầu từ m_currentMonday

        // Yêu cầu month view cập nhật (ví dụ: khi nhấn tới/lui)
        m_monthView->updateView(m_currentMonday);

    } else if (m_viewStack->currentWidget() == m_timetableView) {
        // Chế độ xem TKB
        daysToShow = 6; // Luôn là 6 ngày (T2-T7)
        endDate = m_currentMonday.addDays(daysToShow - 1);
        m_timetableView->updateView(m_currentMonday);

    } else if (m_viewStack->currentWidget() == m_sessionView) {
        // Chế độ xem TKB Buổi
        daysToShow = 6; // Luôn là 6 ngày (T2-T7)
        endDate = m_currentMonday.addDays(daysToShow - 1);
        m_sessionView->updateView(m_currentMonday);

    } else {
        // Chế độ xem timeline (Ngày/Tuần)
        daysToShow = m_calendarView->getNumberOfDays();
        endDate = m_currentMonday.addDays(daysToShow - 1);

        // Yêu cầu calendar view cập nhật
        m_calendarView->updateViewForDateRange(m_currentMonday);
    }

    // --- PHẦN BỊ THIẾU LÀ Ở ĐÂY ---
    // Khai báo và gán giá trị cho dateRangeText
    QString dateRangeText;
    QLocale viLocale(QLocale::Vietnamese);

    if (m_currentMonday.month() == endDate.month()) {
        dateRangeText = viLocale.monthName(m_currentMonday.month()) + ", " + m_currentMonday.toString("yyyy");
    } else {
        dateRangeText = viLocale.monthName(m_currentMonday.month()) + " - " + viLocale.monthName(endDate.month()) + ", " + m_currentMonday.toString("yyyy");
    }

    // Giờ dòng này sẽ hết lỗi
    m_dateNavButton->setText(dateRangeText);
    m_calendarPopup->setSelectedDate(m_currentMonday);

    m_dayHeader->updateDates(m_currentMonday);
    // --- KẾT THÚC PHẦN SỬA ---
}

void MainWindow::showPreviousWeek()
{
    m_currentMonday = m_currentMonday.addDays(-7);
    updateCalendarDisplay();
}

void MainWindow::showNextWeek()
{
    m_currentMonday = m_currentMonday.addDays(7);
    updateCalendarDisplay();
}

void MainWindow::showToday()
{
    m_btnTimeScale->setEnabled(true);
    // --- THÊM CÁC DÒNG NÀY ĐỂNÓ TRỞ THÀNH HÀM CHUYỂN VIEW HOÀN CHỈNH ---
    m_viewStack->setCurrentWidget(m_calendarView);
    m_timeRuler->setVisible(true);
    m_calendarCorner->setVisible(true);

    // ĐÂY LÀ DÒNG QUAN TRỌNG NHẤT SỬA LỖI CỦA BẠN
    m_dayHeader->setRightMargin(m_calendarView->verticalScrollBar()->width());
    // --- KẾT THÚC THÊM MỚI ---

    m_dayHeader->setVisible(true);

    QDate today = QDate::currentDate();
    // Tính ngày thứ Hai của tuần hiện tại
    m_currentMonday = today.addDays(-(today.dayOfWeek() - 1));

    // Reset lại số ngày về 7
    m_calendarView->setNumberOfDays(7);
    m_dayHeader->setNumberOfDays(7);

    updateCalendarDisplay();
}

void MainWindow::onNewEventClicked()
{
    EventDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // Lấy dữ liệu từ dialog
        QString title = dialog.title();
        QDateTime start = dialog.startDateTime();
        QDateTime end = dialog.endDateTime();
        QColor color = dialog.categoryColor(); // <-- THAY ĐỔI: Lấy màu từ danh mục

        m_calendarView->addEvent(title, color, start, end);
        m_monthView->addEvent(new EventItem(title, color, start, end));
        m_timetableView->addEvent(new EventItem(title, color, start, end));
        m_sessionView->addEvent(new EventItem(title, color, start, end));

        // Thêm sự kiện vào CalendarView
        m_calendarView->addEvent(title, color, start, end);

        // Cập nhật lại view để hiển thị sự kiện mới (quan trọng)
        // Cần đảm bảo view đang hiển thị đúng tuần chứa sự kiện
        QDate eventDate = start.date();
        int daysUntilMonday = eventDate.dayOfWeek() - 1;
        QDate mondayOfEventWeek = eventDate.addDays(-daysUntilMonday);

        // Nếu tuần của sự kiện khác tuần hiện tại, chuyển view đến tuần đó
        if (m_currentMonday != mondayOfEventWeek) {
            m_currentMonday = mondayOfEventWeek;
        }

        updateCalendarDisplay(); // Hàm này sẽ cập nhật cả header và view
    }
}

void MainWindow::onDateSelectedFromPopup(const QDate &date)
{
    // Tính toán ngày thứ Hai của tuần chứa ngày được chọn
    int daysToMonday = date.dayOfWeek() - 1;
    m_currentMonday = date.addDays(-daysToMonday);

    // --- THÊM MỚI: Reset lại số ngày về 7 ---
    m_calendarView->setNumberOfDays(7);
    m_dayHeader->setNumberOfDays(7);
    // --- KẾT THÚC THÊM MỚI ---

    // Cập nhật lại toàn bộ giao diện
    updateCalendarDisplay();

    // Ẩn menu đi sau khi đã chọn
    m_dateNavButton->menu()->hide();
}

void MainWindow::toggleHelpPanel()
{
    m_tipsPanel->hidePanel(this->geometry(), m_topBar->height());
    m_supportPanel->hidePanel(this->geometry(), m_topBar->height());
    m_feedbackPanel->hidePanel(this->geometry(), m_topBar->height()); // <-- THÊM
    m_helpPanel->toggleVisibility(this->geometry(), m_topBar->height());
}

void MainWindow::toggleTipsPanel()
{
    m_helpPanel->hidePanel(this->geometry(), m_topBar->height());
    m_supportPanel->hidePanel(this->geometry(), m_topBar->height());
    m_feedbackPanel->hidePanel(this->geometry(), m_topBar->height()); // <-- THÊM
    m_tipsPanel->toggleVisibility(this->geometry(), m_topBar->height());
}

void MainWindow::toggleSupportPanel()
{
    m_helpPanel->hidePanel(this->geometry(), m_topBar->height());
    m_tipsPanel->hidePanel(this->geometry(), m_topBar->height());
    m_feedbackPanel->hidePanel(this->geometry(), m_topBar->height()); // <-- THÊM
    m_supportPanel->toggleVisibility(this->geometry(), m_topBar->height());
}

void MainWindow::toggleFeedbackPanel()
{
    m_helpPanel->hidePanel(this->geometry(), m_topBar->height());
    m_tipsPanel->hidePanel(this->geometry(), m_topBar->height());
    m_supportPanel->hidePanel(this->geometry(), m_topBar->height());
    m_feedbackPanel->toggleVisibility(this->geometry(), m_topBar->height());
}

void MainWindow::openSettingsDialog()
{
    SettingsDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // Cập nhật cách gọi hàm này
        changeBackgroundImage(dialog.selectedBackgroundIndex(),
                              dialog.selectedImagePath(),
                              dialog.selectedSolidColor());

        setCalendarTransparency(dialog.isCalendarTransparent());
    }
}

// Cập nhật hàm này để nhận thêm QColor
void MainWindow::changeBackgroundImage(int index, const QString &imagePath, const QColor &color)
{
    QString style = qApp->styleSheet();

    // Xóa cả hai thuộc tính cũ để tránh xung đột
    style.remove(QRegularExpression("QMainWindow \\{[^\\}]*background-image[^\\}]*\\}"));
    style.remove(QRegularExpression("QMainWindow \\{[^\\}]*background-color[^\\}]*\\}"));

    QString newRule;

    if (index == 14 && color.isValid()) // 15 là ID của màu đơn sắc
    {
        // THÊM LOGIC MỚI: Đặt màu nền
        newRule = QString("QMainWindow { background-color: %1; }").arg(color.name());
    }
    else if (index == 13) // 14 là ID của ảnh tùy chỉnh
    {
        if (!imagePath.isEmpty()) {
            QString formattedPath = imagePath;
            formattedPath.replace("\\", "/");
            newRule = QString("QMainWindow { background-image: url('%1'); background-position: center; background-repeat: no-repeat; background-size: cover; }").arg(formattedPath);
        }
    }
    else // Các ảnh nền mặc định (index từ 0 đến 13)
    {
        // const QString fillStyle = " background-position: center; background-repeat: no-repeat; background-size: cover; ";
        switch (index) {
        case 0: newRule = "QMainWindow { background-image: url(:/resource/images/background1.jpg); background-position: center; }"; break;
        case 1: newRule = "QMainWindow { background-image: url(:/resource/images/background2.jpg); background-position: center; }"; break;
        case 2: newRule = "QMainWindow { background-image: url(:/resource/images/background3.jpg); background-position: center; }"; break;
        case 3: newRule = "QMainWindow { background-image: url(:/resource/images/background4.jpg); background-position: center; }"; break;
        case 4: newRule = "QMainWindow { background-image: url(:/resource/images/background5.jpg); background-position: center; }"; break;
        case 5: newRule = "QMainWindow { background-image: url(:/resource/images/background6.jpg); background-position: center; }"; break;
        case 6: newRule = "QMainWindow { background-image: url(:/resource/images/background7.jpg); background-position: center; }"; break;
        case 7: newRule = "QMainWindow { background-image: url(:/resource/images/background8.jpg); background-position: center; }"; break;
        case 8: newRule = "QMainWindow { background-image: url(:/resource/images/background9.jpg); background-position: center; }"; break;
        case 9: newRule = "QMainWindow { background-image: url(:/resource/images/background10.jpg); background-position: center; }"; break;
        case 10: newRule = "QMainWindow { background-image: url(:/resource/images/background11.jpg); background-position: center; }"; break;
        case 11: newRule = "QMainWindow { background-image: url(:/resource/images/background12.jpg); background-position: center; }"; break;
        case 12: newRule = "QMainWindow { background-image: url(:/resource/images/background13.jpg); background-position: center; }"; break;
        default: // Mặc định quay về nền đầu tiên
            newRule = "QMainWindow { background-image: url(:/resource/images/background.jpg); background-position: center; }";
            break;
        }
    }

    if (!newRule.isEmpty()) {
        style += "\n" + newRule;
    }

    qApp->setStyleSheet(style);
}

// THÊM HÀM MỚI NÀY VÀO CUỐI FILE
void MainWindow::setCalendarTransparency(bool transparent)
{
    // Tạo một danh sách tất cả các view cần thay đổi
    QList<QWidget*> views = {m_calendarView, m_monthView, m_timetableView, m_sessionView};

    for (QWidget *view : views) {
        // Đặt thuộc tính [transparent="true"] hoặc [transparent="false"]
        view->setProperty("transparent", transparent);

        // Yêu cầu Qt làm mới lại style của widget
        style()->unpolish(view);
        style()->polish(view);
    }
}

void MainWindow::onDisplayDaysChanged(int days)
{
    m_btnTimeScale->setEnabled(true);
    m_viewStack->setCurrentWidget(m_calendarView);
    m_timeRuler->setVisible(true);
    m_calendarCorner->setVisible(true);

    m_dayHeader->setRightMargin(m_calendarView->verticalScrollBar()->width());

    m_dayHeader->setVisible(true);
    QDate today = QDate::currentDate();

    // 1. Tính toán ngày bắt đầu để "Hôm nay" nằm ở giữa
    // Phép chia số nguyên (integer division) sẽ cho kết quả:
    // 1 ngày -> offset = 1 / 2 = 0 (bắt đầu từ 'hôm nay')
    // 3 ngày -> offset = 3 / 2 = 1 (bắt đầu từ 'hôm qua')
    // 5 ngày -> offset = 5 / 2 = 2 (bắt đầu từ 'hôm kia')
    // 7 ngày -> offset = 7 / 2 = 3 (bắt đầu từ 3 ngày trước)
    int offset = days / 2;
    QDate startDate = today.addDays(-offset);

    // 2. Cập nhật ngày bắt đầu (không còn là thứ Hai nữa)
    m_currentMonday = startDate;

    // 3. Cập nhật số ngày cho CalendarView và DayHeader
    m_calendarView->setNumberOfDays(days);
    m_dayHeader->setNumberOfDays(days);

    // 4. Cập nhật lại toàn bộ hiển thị
    updateCalendarDisplay();
}

// TRONG FILE mainwindow.cpp (thêm vào cuối file)

/**
 * @brief Chuyển chế độ xem sang 5 ngày (Thứ 2 - Thứ 6)
 * dựa trên tuần hiện tại đang xem.
 */
void MainWindow::showWorkWeek()
{
    m_btnTimeScale->setEnabled(true);
    m_viewStack->setCurrentWidget(m_calendarView);
    m_timeRuler->setVisible(true);
    m_calendarCorner->setVisible(true);

    m_dayHeader->setRightMargin(m_calendarView->verticalScrollBar()->width());

    m_dayHeader->setVisible(true);
    // 1. Lấy một ngày tham chiếu (ngày đầu tiên đang xem)
    QDate referenceDate = m_currentMonday;

    // 2. Tính ngày thứ Hai của tuần chứa ngày đó
    int daysToMonday = referenceDate.dayOfWeek() - 1;
    m_currentMonday = referenceDate.addDays(-daysToMonday);

    // 3. Đặt số ngày là 5
    m_calendarView->setNumberOfDays(5);
    m_dayHeader->setNumberOfDays(5);

    // 4. Cập nhật lại toàn bộ hiển thị
    updateCalendarDisplay();
}

/**
 * @brief Chuyển chế độ xem về 7 ngày (Thứ 2 - Chủ Nhật)
 * của tuần hiện tại (giống hệt nút "Hôm nay").
 */
void MainWindow::showFullWeek()
{
    showToday();
}

void MainWindow::showMonthView()
{
    m_btnTimeScale->setEnabled(false);
    // 1. Chuyển sang trang xem tháng
    m_viewStack->setCurrentWidget(m_monthView);

    // 2. Ẩn các thành phần của timeline
    m_timeRuler->setVisible(false);
    m_calendarCorner->setVisible(false);

    // 3. Hiển thị DayHeader và đặt 7 ngày
    m_dayHeader->setVisible(true);

    m_dayHeader->setRightMargin(0);

    m_dayHeader->setNumberOfDays(7); // Chế độ xem tháng luôn có 7 ngày header

    // 4. Cập nhật ngày
    // (Đưa về Thứ 2 của tuần hiện tại để header hiển thị đúng)
    QDate today = QDate::currentDate();
    m_currentMonday = today.addDays(-(today.dayOfWeek() - 1));

    m_dayHeader->updateDates(m_currentMonday);
    m_monthView->updateView(today); // Yêu cầu MonthView vẽ tháng hiện tại

    // 5. Cập nhật nhãn tháng/năm
    updateCalendarDisplay(); // Cần sửa hàm này
}

void MainWindow::showTimetableView()
{
    m_btnTimeScale->setEnabled(false);
    // 1. Chuyển sang trang xem TKB
    m_viewStack->setCurrentWidget(m_timetableView);

    // 2. Ẩn các thành phần không cần thiết
    m_timeRuler->setVisible(false);
    m_calendarCorner->setVisible(false);
    m_dayHeader->setVisible(false); // View này có header riêng

    // 3. Cập nhật ngày
    QDate today = QDate::currentDate();
    m_currentMonday = today.addDays(-(today.dayOfWeek() - 1));

    // 4. Cập nhật nhãn tháng/năm
    updateCalendarDisplay();
}

void MainWindow::showSessionView()
{
    m_btnTimeScale->setEnabled(false);
    // 1. Chuyển sang trang xem TKB Buổi
    m_viewStack->setCurrentWidget(m_sessionView);

    // 2. Ẩn các thành phần không cần thiết
    m_timeRuler->setVisible(false);
    m_calendarCorner->setVisible(false);
    m_dayHeader->setVisible(false); // View này có header riêng

    // 3. Cập nhật ngày
    QDate today = QDate::currentDate();
    m_currentMonday = today.addDays(-(today.dayOfWeek() - 1));

    // 4. Cập nhật nhãn tháng/năm
    updateCalendarDisplay();
}

void MainWindow::onTimeScaleChanged(int minutes)
{
    // 1. Áp dụng tỷ lệ mới cho CalendarView
    m_calendarView->setTimeScale(minutes);

    // 2. Lấy chiều cao mới từ CalendarView
    double newHourHeight = m_calendarView->getHourHeight();

    // 3. "Ra lệnh" cho TimeRuler sử dụng chiều cao đó
    m_timeRuler->setHourHeight(newHourHeight);

    // 4. Nếu đang ở view khác, tự động chuyển về view Tuần
    if (m_viewStack->currentWidget() != m_calendarView) {
        showFullWeek();
        // (showFullWeek sẽ tự động gọi setRightMargin, v.v.)
    }

    // 5. Tự động cuộn đến 7 giờ sáng
    QScrollBar *verticalScrollBar = m_calendarView->verticalScrollBar();
    int scrollToPosition = 7.5 * newHourHeight; // Dùng newHourHeight
    verticalScrollBar->setValue(scrollToPosition);
}
