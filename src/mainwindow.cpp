#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "calendarview.h"
#include "dayheader.h"
#include "timeruler.h"
#include "eventdialog.h"
#include "eventitem.h"
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
#include <QCloseEvent>
#include <QDateEdit>
#include <QPrinter>
#include <QPainter>
#include <QFileDialog>
#include <QPageLayout>
#include <QPageSize>
#include <QFileInfo>

#include <QFile>
#include <QSaveFile> // An toàn hơn QFile để ghi
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths> // Để tìm thư mục data
#include <QDir>
#include <QListWidget> // Cần cho To-Do list
#include <QTextEdit>   // Cần cho To-Do list
#include <QCheckBox>   // Cần cho To-Do list

// =================================================================================
// === 1. CONSTRUCTOR & DESTRUCTOR
// =================================================================================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_use24HourFormat(true) // <-- Giá trị mặc định (12h)
    , m_timezoneOffsetSeconds(QDateTime::currentDateTime().offsetFromUtc())          // <-- Giá trị mặc định (Giờ địa phương)
    , m_currentBackgroundIndex(2)     // Nền mặc định số 2
    , m_currentImagePath(QString())   // Đường dẫn ảnh tùy chỉnh rỗng
    , m_currentSolidColor(QColor())   // Màu tùy chỉnh rỗng
    , m_isCalendarTransparent(true) // Lịch trong suốt
{
    ui->setupUi(this);

    //Màn hình nhỏ nhất có thể co lại
    setMinimumWidth(700);
    resize(1250, 800);     // rộng 1200px, cao 800px
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX); // cho phép phóng to tự do

    // ===============================================================
    // === 1A. TẠO CÁC HÀM TIỆN ÍCH (HELPER LAMBDAS) ===
    // ===============================================================

    auto addShadowEffect = [](QWidget *widget) {
        auto *effect = new QGraphicsDropShadowEffect;
        effect->setBlurRadius(15);
        effect->setXOffset(0);
        effect->setYOffset(2);
        effect->setColor(QColor(0, 0, 0, 80));
        widget->setGraphicsEffect(effect);
    };

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

    // ===============================================================
    // === 1B. TẠO THANH CÔNG CỤ (TOOLBARS) VÀ TABS ===
    // ===============================================================

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
    m_toolbarStack = new QStackedWidget(this);

    // --- 1B.1. TẠO BỘ LỌC CHUNG (SHARED FILTER MENU) ---
    // (Vì nó được dùng ở nhiều tab)
    QToolButton *btnFilter = new QToolButton;
    btnFilter->setText("  Bộ lọc   ▼");
    btnFilter->setIcon(QIcon(":/resource/icons/filter.png"));
    btnFilter->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btnFilter->setCursor(Qt::PointingHandCursor);
    btnFilter->setPopupMode(QToolButton::InstantPopup);
    btnFilter->setObjectName("btnFilter");

    m_filterMenu = new QMenu(btnFilter);
    addShadowEffect(m_filterMenu);

    // 1. Bộ lọc LOẠI SỰ KIỆN
    QMenu *menuEventTypes = new QMenu("Loại sự kiện", m_filterMenu);
    addShadowEffect(menuEventTypes);
    QStringList eventTypes = {"Sự kiện", "Cuộc họp", "Học tập", "Ngày lễ", "Cuộc hẹn"};
    for (const QString &type : eventTypes) {
        QAction *a = menuEventTypes->addAction(type);
        a->setCheckable(true); a->setChecked(true);
        m_eventTypeActions.append(a);
        connect(a, &QAction::toggled, this, &MainWindow::onFilterChanged);
    }

    // 2. Bộ lọc HỌC TẬP (Cách thức)
    QMenu *menuStudyMethods = new QMenu("Cách thức Học tập", m_filterMenu);
    addShadowEffect(menuStudyMethods);
    QStringList studyMethods = {"Trực tiếp", "Trực tuyến", "Học thêm", "Tự học"};
    for (const QString &method : studyMethods) {
        QAction *a = menuStudyMethods->addAction(method);
        a->setCheckable(true); a->setChecked(true);
        m_studyMethodActions.append(a);
        connect(a, &QAction::toggled, this, &MainWindow::onFilterChanged);
    }

    // 3. Bộ lọc NGÀY LỄ (Phạm vi)
    QMenu *menuHolidayScopes = new QMenu("Phạm vi Ngày lễ", m_filterMenu);
    addShadowEffect(menuHolidayScopes);
    QStringList holidayScopes = {"Quốc tế", "Quốc gia", "Tôn giáo", "Tùy chỉnh"};
    for (const QString &scope : holidayScopes) {
        QAction *a = menuHolidayScopes->addAction(scope);
        a->setCheckable(true); a->setChecked(true);
        m_holidayScopeActions.append(a);
        connect(a, &QAction::toggled, this, &MainWindow::onFilterChanged);
    }

    // 4. Bộ lọc CUỘC HẸN (Loại & Riêng tư)
    QMenu *menuAppointment = new QMenu("Chi tiết Cuộc hẹn", m_filterMenu);
    addShadowEffect(menuAppointment);

    // 4a. Sub-menu Loại
    QMenu *menuAppointmentTypes = new QMenu("Loại cuộc hẹn", menuAppointment);
    addShadowEffect(menuAppointmentTypes);
    QStringList appointmentTypes = {"Hẹn hò", "Gặp mặt", "Học nhóm", "Đi chơi", "Khác"};
    for (const QString &type : appointmentTypes) {
        QAction *a = menuAppointmentTypes->addAction(type);
        a->setCheckable(true); a->setChecked(true);
        m_appointmentTypeActions.append(a);
        connect(a, &QAction::toggled, this, &MainWindow::onFilterChanged);
    }

    // 4b. Sub-menu Riêng tư
    QMenu *menuAppointmentPrivacy = new QMenu("Tính riêng tư", menuAppointment);
    addShadowEffect(menuAppointmentPrivacy);
    QStringList privacyTypes = {"Công khai", "Riêng tư"};
    for (const QString &type : privacyTypes) {
        QAction *a = menuAppointmentPrivacy->addAction(type);
        a->setCheckable(true); a->setChecked(true);
        m_appointmentPrivacyActions.append(a);
        connect(a, &QAction::toggled, this, &MainWindow::onFilterChanged);
    }
    menuAppointment->addMenu(menuAppointmentTypes);
    menuAppointment->addMenu(menuAppointmentPrivacy);

    // 5. Bộ lọc TRẠNG THÁI CUỘC HỌP
    QMenu *menuMeetingStatus = new QMenu("Cuộc họp", m_filterMenu);
    addShadowEffect(menuMeetingStatus);
    QStringList meetingStatuses = {"Dự kiến", "Đã xác nhận", "Đã hủy", "Không phải cuộc họp"};
    for (const QString &status : meetingStatuses) {
        QAction *a = menuMeetingStatus->addAction(status);
        a->setCheckable(true); a->setChecked(true);
        m_meetingStatusActions.append(a);
        connect(a, &QAction::toggled, this, &MainWindow::onFilterChanged);
    }

    // 6. Bộ lọc THẺ/TAG (Category)
    QMenu *menuCategory = new QMenu("Thẻ/Tag", m_filterMenu);
    addShadowEffect(menuCategory);
    // ... (Tạo các action "Bỏ chọn", "Không", và các tag màu) ...
    QAction *actUncheckCategory = menuCategory->addAction("Bỏ chọn tất cả");
    QObject::connect(actUncheckCategory, &QAction::triggered, menuCategory, [menuCategory]() {
        const auto categoryActions = menuCategory->actions();
        for (QAction *a : categoryActions)
            if (a->isCheckable()) a->setChecked(false);
    });
    menuCategory->addSeparator();

    QAction *actUncategorized = menuCategory->addAction("Không");
    actUncategorized->setCheckable(true); actUncategorized->setChecked(true);
    m_categoryActions.append(actUncategorized);
    connect(actUncategorized, &QAction::toggled, this, &MainWindow::onFilterChanged);
    menuCategory->addSeparator();

    struct Category { QString name; QString colorIcon; };
    QList<Category> categories = {
        {"Đỏ",    ":/resource/icons/red_tag.png"},
        {"Cam", ":/resource/icons/orange_tag.png"},
        {"Vàng", ":/resource/icons/yellow_tag.png"},
        {"Xanh lá",  ":/resource/icons/green_tag.png"},
        {"Xanh dương",   ":/resource/icons/blue_tag.png"},
        {"Tím", ":/resource/icons/purple_tag.png"}
    };
    menuCategory->setObjectName("menuCategory");
    for (const auto &cat : categories) {
        QAction *a = menuCategory->addAction(QIcon(cat.colorIcon), cat.name);
        a->setCheckable(true); a->setChecked(true);
        m_categoryActions.append(a);
        connect(a, &QAction::toggled, this, &MainWindow::onFilterChanged);
    }

    // 7. Bộ lọc TRẠNG THÁI (Display As)
    QMenu *menuDisplayAs = new QMenu("Trạng thái", m_filterMenu);
    addShadowEffect(menuDisplayAs);
    // ... (Tạo action "Bỏ chọn" và các trạng thái Rảnh/Bận...) ...
    QAction *actUncheckDisplayAs = menuDisplayAs->addAction("Bỏ chọn tất cả");
    QObject::connect(actUncheckDisplayAs, &QAction::triggered, menuDisplayAs, [menuDisplayAs]() {
        const auto actionList = menuDisplayAs->actions();
        for (QAction *a : actionList)
            if (a->isCheckable()) a->setChecked(false);
    });
    menuDisplayAs->addSeparator();
    QStringList displayAs = { "Rảnh", "Bận", "Dự định", "Làm việc ở nơi khác", "Vắng mặt" };
    for (const QString &opt : displayAs) {
        QAction *a = menuDisplayAs->addAction(opt);
        a->setCheckable(true); a->setChecked(true);
        m_statusActions.append(a);
        connect(a, &QAction::toggled, this, &MainWindow::onFilterChanged);
    }


    // 8. Bộ lọc LẶP LẠI (Recurrence)
    QMenu *menuRepeat = new QMenu("Lặp lại", m_filterMenu);
    addShadowEffect(menuRepeat);
    QStringList displayRepeat = { "Đơn", "Chuỗi" };
    for (const QString &opt : displayRepeat) {
        QAction *a = menuRepeat->addAction(opt);
        a->setCheckable(true); a->setChecked(true);
        m_recurrenceActions.append(a); // Lưu con trỏ
        connect(a, &QAction::toggled, this, &MainWindow::onFilterChanged); // Kết nối
    }

    // Gắn tất cả menu vào Bộ lọc chính
    m_filterMenu->addMenu(menuEventTypes);
    m_filterMenu->addMenu(menuAppointment);
    m_filterMenu->addMenu(menuStudyMethods);
    m_filterMenu->addMenu(menuHolidayScopes);
    m_filterMenu->addMenu(menuMeetingStatus);
    m_filterMenu->addMenu(menuCategory);
    m_filterMenu->addMenu(menuDisplayAs);
    m_filterMenu->addMenu(menuRepeat);

    btnFilter->setMenu(m_filterMenu);

    // --- Hàm tiện ích: tạo bản sao nút Bộ lọc ---
    auto makeFilterButton = [&](QWidget *parent = nullptr) {
        QToolButton *b = new QToolButton(parent);
        b->setText("  Bộ lọc  ▼");
        b->setIcon(QIcon(":/resource/icons/filter.png"));
        b->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        b->setCursor(Qt::PointingHandCursor);
        b->setPopupMode(QToolButton::InstantPopup);
        b->setMenu(m_filterMenu); // DÙNG CHUNG MENU
        return b;
    };

    // --- 1B.2. Toolbar "Trang chủ" ---
    QWidget *homePage = new QWidget;
    QHBoxLayout *homeLayout = new QHBoxLayout(homePage);
    homeLayout->setContentsMargins(10, 6, 10, 6);
    homeLayout->setSpacing(10);

    // --- Nút chính "Sự kiện mới" ---
    QToolButton *btnNewEvent = new QToolButton;
    btnNewEvent->setText("  Sự kiện mới");
    btnNewEvent->setIcon(QIcon(":/resource/icons/calendar.png"));
    btnNewEvent->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btnNewEvent->setCursor(Qt::PointingHandCursor);
    btnNewEvent->setObjectName("btnNewEvent");
    btnNewEvent->setPopupMode(QToolButton::InstantPopup);

    QMenu *newEventMenu = new QMenu(btnNewEvent);
    QAction *actNewEvent = newEventMenu->addAction(QIcon(":/resource/icons/calendarEvent.png"), "  Sự kiện");
    QAction *actNewMeeting = newEventMenu->addAction(QIcon(":/resource/icons/message.png"), "  Cuộc họp");
    QAction *actNewStudy = newEventMenu->addAction(QIcon(":/resource/icons/diagnostics.png"), "  Học tập");
    QAction *actNewHoliday = newEventMenu->addAction(QIcon(":/resource/icons/vacation.png"), "  Ngày lễ");
    QAction *actNewAppointment = newEventMenu->addAction(QIcon(":/resource/icons/mobile.png"), "  Cuộc hẹn");
    connect(actNewMeeting, &QAction::triggered, this, &MainWindow::onNewMeetingClicked);
    connect(actNewEvent, &QAction::triggered, this, &MainWindow::onNewEventClicked);
    connect(actNewStudy, &QAction::triggered, this, &MainWindow::onNewStudyClicked);
    connect(actNewHoliday, &QAction::triggered, this, &MainWindow::onNewHolidayClicked);
    connect(actNewAppointment, &QAction::triggered, this, &MainWindow::onNewAppointmentClicked);
    newEventMenu->setObjectName("eventMenu");
    btnNewEvent->setMenu(newEventMenu);
    addShadowEffect(newEventMenu);

    homeLayout->addWidget(btnNewEvent);

    // --- Nút "Ngày" có menu thả (Dùng chung) ---
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
    connect(btnMonth, &QToolButton::clicked, this, &MainWindow::showMonthView);
    homeLayout->addWidget(btnMonth);

    // --- Nút "TKB" (Dùng chung) ---
    QToolButton *btnSplitView = makeBtn("Thời khóa biểu", ":/resource/icons/split.png");
    btnSplitView->setPopupMode(QToolButton::InstantPopup);
    QMenu *splitViewMenu = new QMenu(this);
    QAction *actPerSlot = splitViewMenu->addAction("Xem theo Tiết");
    QAction *actPerSession = splitViewMenu->addAction("Xem theo Buổi");
    connect(actPerSlot, &QAction::triggered, this, &MainWindow::showTimetableView);
    connect(actPerSession, &QAction::triggered, this, &MainWindow::showSessionView);
    btnSplitView->setMenu(splitViewMenu);

    homeLayout->addWidget(btnSplitView);
    homeLayout->addWidget(makeSeparator());
    homeLayout->addWidget(makeFilterButton()); // Nút Lọc (Bản sao 1)
    homeLayout->addWidget(makeSeparator());

    QToolButton *btnPrint = makeBtn("In", ":/resource/icons/printer.png");
    connect(btnPrint, &QToolButton::clicked, this, &MainWindow::onPrintToPdf);
    homeLayout->addWidget(btnPrint);

    QToolButton *btnImportExport = makeBtn("Nhập/Xuất", ":/resource/icons/save.png");
    btnImportExport->setPopupMode(QToolButton::InstantPopup);
    QMenu *importExportMenu = new QMenu(btnImportExport);
    QAction *actExport = importExportMenu->addAction("Xuất dữ liệu (.json)");
    QAction *actImport = importExportMenu->addAction("Nhập dữ liệu (.json)");
    addShadowEffect(importExportMenu);
    btnImportExport->setMenu(importExportMenu);
    connect(actExport, &QAction::triggered, this, &MainWindow::onExportData);
    connect(actImport, &QAction::triggered, this, &MainWindow::onImportData);

    homeLayout->addWidget(btnImportExport);
    homeLayout->addStretch();
    m_toolbarStack->addWidget(homePage); // THÊM PAGE "TRANG CHỦ" VÀO STACK

    // --- 1B.3. Toolbar "Dạng xem" ---
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
    btnDayView->setMenu(dayMenu); // Dùng chung menu "Ngày"
    viewLayout->addWidget(btnDayView);

    // --- Các nút còn lại ---
    QToolButton *btnWorkWeekView = makeBtn("Tuần làm việc", ":/resource/icons/workWeek.png");
    connect(btnWorkWeekView, &QToolButton::clicked, this, &MainWindow::showWorkWeek);
    viewLayout->addWidget(btnWorkWeekView);

    QToolButton *btnWeekView = makeBtn("Tuần", ":/resource/icons/week.png");
    connect(btnWeekView, &QToolButton::clicked, this, &MainWindow::showFullWeek);
    viewLayout->addWidget(btnWeekView);

    QToolButton *btnMonthView = makeBtn("Tháng", ":/resource/icons/month.png");
    connect(btnMonthView, &QToolButton::clicked, this, &MainWindow::showMonthView);
    viewLayout->addWidget(btnMonthView);

    QToolButton *btnSplitView_View = makeBtn("Thời khóa biểu", ":/resource/icons/split.png");
    btnSplitView_View->setPopupMode(QToolButton::InstantPopup);
    btnSplitView_View->setMenu(splitViewMenu); // Dùng chung menu TKB
    viewLayout->addWidget(btnSplitView_View);

    // --- Nút "Tỉ lệ thời gian" ---
    m_btnTimeScale = new QToolButton;
    m_btnTimeScale->setText("  Tỉ lệ thời gian  ▼");
    m_btnTimeScale->setIcon(QIcon(":/resource/icons/timeScale.png"));
    m_btnTimeScale->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_btnTimeScale->setCursor(Qt::PointingHandCursor);
    m_btnTimeScale->setPopupMode(QToolButton::InstantPopup);
    m_btnTimeScale->setObjectName("btnTimeScale");

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
    timeScaleMenu->setObjectName("timeScaleMenu");
    addShadowEffect(timeScaleMenu);
    m_btnTimeScale->setMenu(timeScaleMenu);

    viewLayout->addWidget(m_btnTimeScale);
    viewLayout->addWidget(makeSeparator());
    viewLayout->addWidget(makeFilterButton()); // Nút Lọc (Bản sao 2)
    viewLayout->addWidget(makeSeparator());

    QToolButton *btnSettings = makeBtn("Cài đặt", ":/resource/icons/setting.png");
    connect(btnSettings, &QToolButton::clicked, this, &MainWindow::openSettingsDialog);
    viewLayout->addWidget(btnSettings);

    viewLayout->addStretch();
    m_toolbarStack->addWidget(viewPage); // THÊM PAGE "DẠNG XEM" VÀO STACK

    // --- 1B.4. Toolbar "Trợ giúp" ---
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
    connect(btnSupport, &QToolButton::clicked, this, &MainWindow::toggleSupportPanel);
    helpLayout->addWidget(btnSupport);

    QToolButton *btnFeedback = makeBtn("Phản hồi", ":/resource/icons/feedback.png");
    connect(btnFeedback, &QToolButton::clicked, this, &MainWindow::toggleFeedbackPanel);
    helpLayout->addWidget(btnFeedback);

    helpLayout->addWidget(makeSeparator());
    QToolButton *btnGithub = makeBtn("Đi tới Github", ":/resource/icons/github.png");
    connect(btnGithub, &QToolButton::clicked, this, []() {
        QDesktopServices::openUrl(QUrl("https://github.com/dada-DSA-OOP/Calendar_PROJECT"));
    });
    helpLayout->addWidget(btnGithub);
    helpLayout->addStretch();
    m_toolbarStack->addWidget(helpPage); // THÊM PAGE "TRỢ GIÚP" VÀO STACK

    // --- 1B.5. Gắn Tabs và Toolbars vào Layout chính ---
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
    topLayout->addWidget(m_toolbarStack); // Stack toolbar
    QFrame *separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Plain);
    separator->setStyleSheet("color: #dcdcdc;");
    topLayout->addWidget(separator);

    // ===============================================================
    // === 1C. TẠO CÁC PANEL BÊN (SIDE PANELS) ===
    // ===============================================================

    // --- 1C.1. Nội dung Panel Trợ giúp ---
    QWidget *helpContentWidget = new QWidget;
    QVBoxLayout *helpContentLayout = new QVBoxLayout(helpContentWidget);
    auto makeHelpLabel = [](const QString &text) {
        QLabel *label = new QLabel(text);
        label->setWordWrap(true);
        return label;
    };
    QGroupBox *helpGb1 = new QGroupBox("Thao Tác Cơ Bản");
    helpGb1->setLayout(new QVBoxLayout);
    helpGb1->layout()->addWidget(makeHelpLabel(
        "<b>Tạo sự kiện mới:</b><br>"
        "Nhấn nút 'Sự kiện mới' (tab Trang chủ) và chọn loại sự kiện (Cuộc họp, Học tập, v.v.). Điền thông tin và nhấn 'Lưu'."
        ));
    helpContentLayout->addWidget(helpGb1);
    // ... (Thêm các QGroupBox khác cho helpGb2, 3, 4, 5, 6, 7, 8) ...
    QGroupBox *helpGb2 = new QGroupBox("Điều hướng Lịch");
    helpGb2->setLayout(new QVBoxLayout);
    helpGb2->layout()->addWidget(makeHelpLabel(
        "<b>Chuyển tuần/tháng:</b><br>"
        "Dùng các nút mũi tên <b>◀</b> (Lùi) và <b>▶</b> (Tới) trên thanh điều hướng.<br><br>"
        "<b>Về ngày hiện tại:</b><br>"
        "Nhấn nút 'Hôm nay'.<br><br>"
        "<b>Chọn ngày cụ thể:</b><br>"
        "Nhấn vào tên tháng (ví dụ: 'Tháng 11, 2025') để mở lịch popup và chọn ngày."
        ));
    helpContentLayout->addWidget(helpGb2);
    QGroupBox *helpGb3 = new QGroupBox("Tương tác với Sự kiện");
    helpGb3->setLayout(new QVBoxLayout);
    helpGb3->layout()->addWidget(makeHelpLabel(
        "<b>Chỉnh sửa nhanh (Kéo thả):</b><br>"
        "- <b>Di chuyển:</b> Kéo thả sự kiện sang ngày/giờ khác.<br>"
        "- <b>Thay đổi thời lượng:</b> Kéo cạnh dưới của sự kiện để tăng/giảm thời gian kết thúc.<br><br>"
        "<b>Chỉnh sửa chi tiết:</b><br>"
        "Nhấn (click) vào một sự kiện để mở dialog chỉnh sửa chi tiết."
        ));
    helpContentLayout->addWidget(helpGb3);
    QGroupBox *helpGb4 = new QGroupBox("Các Chế Độ Xem");
    helpGb4->setLayout(new QVBoxLayout);
    helpGb4->layout()->addWidget(makeHelpLabel(
        "- <b>Ngày/Tuần/Tuần làm việc:</b> Hiển thị lịch theo dạng dòng thời gian (timeline). Bạn có thể thay đổi số ngày xem (1, 3, 5, 7 ngày) từ menu 'Ngày' (tab Trang chủ) hoặc 'Ngày' (tab Dạng xem).<br><br>"
        "- <b>Tháng:</b> Hiển thị tổng quan sự kiện trong cả tháng.<br><br>"
        "- <b>Thời khóa biểu (Tiết/Buổi):</b> Hiển thị các sự kiện 'Học tập' và 'Cuộc họp' được sắp xếp vào các tiết/buổi học cố định (T2-T7)."
        ));
    helpContentLayout->addWidget(helpGb4);
    QGroupBox *helpGb5 = new QGroupBox("Lọc Sự Kiện");
    helpGb5->setLayout(new QVBoxLayout);
    helpGb5->layout()->addWidget(makeHelpLabel(
        "Nhấn nút 'Bộ lọc' (tab Trang chủ hoặc Dạng xem) để mở menu lọc:<br><br>"
        "- <b>Lọc theo Loại:</b> Ẩn/hiện các loại sự kiện chính (Học tập, Ngày lễ, v.v.).<br><br>"
        "- <b>Lọc chi tiết:</b> Ẩn/hiện các sự kiện dựa trên trạng thái (Bận/Rảnh), Thẻ/Tag (Màu sắc), hoặc trạng thái Cuộc họp (Đã xác nhận, v.v.).<br><br>"
        "- <b>Lưu ý:</b> Các bộ lọc con (ví dụ: 'Cách thức Học tập') chỉ hoạt động khi bộ lọc 'Loại sự kiện' (ví dụ: 'Học tập') tương ứng đang được bật."
        ));
    helpContentLayout->addWidget(helpGb5);
    QGroupBox *helpGb6 = new QGroupBox("Tùy Chỉnh Giao Diện");
    helpGb6->setLayout(new QVBoxLayout);
    helpGb6->layout()->addWidget(makeHelpLabel(
        "Nhấn nút 'Cài đặt' (tab Dạng xem) để:<br><br>"
        "- <b>Thay đổi Ảnh nền/Màu nền.</b><br><br>"
        "- <b>Bật/Tắt hiệu ứng trong suốt (Mica) cho lịch.</b><br><br>"
        "- <b>Thay đổi Múi giờ</b> để lịch tự động điều chỉnh khi bạn đi du lịch.<br><br>"
        "- <b>Chuyển đổi định dạng 12/24 giờ</b> cho cột thời gian."
        ));
    helpContentLayout->addWidget(helpGb6);
    QGroupBox *helpGb7 = new QGroupBox("In Lịch ra PDF");
    helpGb7->setLayout(new QVBoxLayout);
    helpGb7->layout()->addWidget(makeHelpLabel(
        "Bạn có thể xuất dạng xem lịch hiện tại (Tuần, Tháng, TKB) ra file PDF để lưu trữ hoặc in ấn.<br><br>"
        "1. Chuyển sang dạng xem bạn muốn in (ví dụ: 'Tháng').<br>"
        "2. Trên tab <b>Trang chủ</b>, nhấn nút <b>'In'</b>.<br>"
        "3. Chọn nơi lưu file PDF của bạn."
        ));
    helpContentLayout->addWidget(helpGb7);
    QGroupBox *helpGb8 = new QGroupBox("Sao lưu & Khôi phục");
    helpGb8->setLayout(new QVBoxLayout);
    helpGb8->layout()->addWidget(makeHelpLabel(
        "Bạn có thể sao lưu toàn bộ lịch (sự kiện và ghi chú) ra file <b>.json</b> và khôi phục lại sau (sử dụng menu 'Nhập/Xuất' trên tab <b>Trang chủ</b>).<br><br>"
        "<b>Xuất (Sao lưu):</b><br>"
        "Chọn 'Xuất dữ liệu'. Thao tác này sẽ tạo một bản sao lưu (file .json) an toàn. Bạn nên làm điều này thường xuyên.<br><br>"
        "<b>Nhập (Khôi phục):</b><br>"
        "Chọn 'Nhập dữ liệu'. <b>LƯU Ý:</b> Thao tác này sẽ <b>XÓA SẠCH</b> toàn bộ dữ liệu hiện tại và thay thế bằng dữ liệu từ file bạn chọn."
        ));
    helpContentLayout->addWidget(helpGb8);


    // --- 1C.2. Nội dung Panel Mẹo & Thủ thuật ---
    QWidget *tipsContentWidget = new QWidget;
    QVBoxLayout *tipsContentLayout = new QVBoxLayout(tipsContentWidget);
    QGroupBox *tipGb1 = new QGroupBox("Đặt làm lịch mặc định");
    tipGb1->setLayout(new QVBoxLayout);
    tipGb1->layout()->addWidget(makeHelpLabel(
        "<b>Sắp có: Tích hợp lịch!</b><br>"
        "Tính năng này đang được phát triển. Sắp tới, bạn có thể đồng bộ hóa lịch này với các lịch khác để quản lý mọi thứ từ một nơi duy nhất!"
        ));
    tipsContentLayout->addWidget(tipGb1);
    // ... (Thêm các QGroupBox khác cho tipGb2, 3, 4, 5) ...
    QGroupBox *tipGb2 = new QGroupBox("Mẹo Cho Người Đi Du Lịch");
    tipGb2->setLayout(new QVBoxLayout);
    tipGb2->layout()->addWidget(makeHelpLabel(
        "<b>Luôn đúng giờ, mọi lúc mọi nơi!</b><br>"
        "Bạn sắp có chuyến bay? Hãy vào 'Cài đặt' (tab Dạng xem) và chọn <b>Múi giờ</b> mới của bạn.<br><br>"
        "Tất cả các sự kiện sẽ tự động dịch chuyển, đảm bảo bạn không bao giờ bị trễ hẹn (hoặc gọi điện về nhà vào lúc 3 giờ sáng!)."
        ));
    tipsContentLayout->addWidget(tipGb2);
    QGroupBox *tipGb3 = new QGroupBox("Lên lịch cho 'Sự Lười Biếng'");
    tipGb3->setLayout(new QVBoxLayout);
    tipGb3->layout()->addWidget(makeHelpLabel(
        "Hãy thử tạo một sự kiện lặp lại vào tối Chủ Nhật tên là <b>'Không làm gì cả'</b>.<br><br>"
        "Đặt trạng thái là 'Rảnh' (ironically) và chọn màu 'Xanh lá' cho thư giãn. Não của bạn cần những cuộc hẹn 'không-làm-gì' này!"
        ));
    tipsContentLayout->addWidget(tipGb3);
    QGroupBox *tipGb4 = new QGroupBox("Mã Hóa Màu Sắc");
    tipGb4->setLayout(new QVBoxLayout);
    tipGb4->layout()->addWidget(makeHelpLabel(
        "Dùng tính năng 'Thẻ/Tag' (Màu sắc) để mã hóa cuộc đời bạn:<br><br>"
        "- <b>Đỏ:</b> Các sự kiện 'nguy hiểm' (ví dụ: Hẹn nha sĩ, Họp gia đình).<br>"
        "- <b>Vàng:</b> Các sự kiện 'có thể hủy' (ví dụ: 'Học nhóm' nhưng bạn biết sẽ không ai đi).<br>"
        "- <b>Tím:</b> Các sự kiện 'bí mật' (ví dụ: 'Dự án thống trị thế giới')."
        ));
    tipsContentLayout->addWidget(tipGb4);
    QGroupBox *tipGb5 = new QGroupBox("Ngày lễ 'Tùy Chỉnh'");
    tipGb5->setLayout(new QVBoxLayout);
    tipGb5->layout()->addWidget(makeHelpLabel(
        "Tạo một sự kiện loại <b>'Ngày lễ'</b>, chọn phạm vi <b>'Tùy chỉnh'</b> và đặt tên là <b>'Ngày Tự Thưởng Của Tôi'</b>.<br><br>"
        "Đặt nó lặp lại vào Thứ Sáu hàng tuần. Đây là ngày lễ quan trọng nhất."
        ));
    tipsContentLayout->addWidget(tipGb5);

    // --- TẠO FUNNY TIP WIDGET ---
    m_funnyTipWidget = new FunnyTipWidget(this);
    m_funnyTipWidget->start();

    // --- 1C.3. Nội dung Panel Hỗ trợ ---
    QWidget *supportContentWidget = new QWidget;
    QVBoxLayout *supportContentLayout = new QVBoxLayout(supportContentWidget);
    supportContentLayout->setSpacing(15);
    // ... (Tạo QGroupBox, QRadioButton, QStackedWidget cho form Hỗ trợ) ...
    QGroupBox *dataGroupBox = new QGroupBox("Thu thập dữ liệu chẩn đoán");
    QVBoxLayout *dataLayout = new QVBoxLayout(dataGroupBox);
    dataLayout->setSpacing(10);
    QLabel *warningLabel = new QLabel("Để cải thiện ứng dụng, chúng tôi có thể thu thập dữ liệu sử dụng ẩn danh. Dữ liệu này không chứa thông tin cá nhân. Bạn có đồng ý không?");
    warningLabel->setWordWrap(true);
    dataLayout->addWidget(warningLabel);
    QRadioButton *agreeButton = new QRadioButton("Đồng ý");
    QRadioButton *disagreeButton = new QRadioButton("Không, cảm ơn");
    agreeButton->setChecked(true);
    QHBoxLayout *radioLayout = new QHBoxLayout;
    radioLayout->addWidget(agreeButton); radioLayout->addWidget(disagreeButton); radioLayout->addStretch();
    dataLayout->addLayout(radioLayout);
    supportContentLayout->addWidget(dataGroupBox);

    QGroupBox *supportTypeGroupBox = new QGroupBox("Tôi cần hỗ trợ về:");
    QVBoxLayout *typeLayout = new QVBoxLayout(supportTypeGroupBox);
    QRadioButton *bugRadio = new QRadioButton("Báo cáo lỗi kỹ thuật");
    QRadioButton *featureRadio = new QRadioButton("Yêu cầu tính năng mới");
    QRadioButton *questionRadio = new QRadioButton("Hỏi đáp / Vấn đề khác");
    bugRadio->setChecked(true);
    typeLayout->addWidget(bugRadio); typeLayout->addWidget(featureRadio); typeLayout->addWidget(questionRadio);
    supportContentLayout->addWidget(supportTypeGroupBox);

    QStackedWidget *supportStackedWidget = new QStackedWidget;
    auto createSupportPage = [&](const QString &placeholder) {
        QWidget *page = new QWidget;
        QVBoxLayout *layout = new QVBoxLayout(page);
        QTextEdit *textEdit = new QTextEdit;
        textEdit->setPlaceholderText(placeholder);
        QPushButton *submitButton = new QPushButton("Gửi yêu cầu");
        submitButton->setObjectName("submitButton");
        submitButton->setCursor(Qt::PointingHandCursor);
        connect(submitButton, &QPushButton::clicked, this, [this, textEdit]() {
            QMessageBox::information(this, "Đã gửi", "Cảm ơn bạn! Yêu cầu hỗ trợ của bạn đã được ghi lại.");
            textEdit->clear();
        });
        layout->addWidget(textEdit);
        layout->addWidget(submitButton, 0, Qt::AlignRight);
        return page;
    };
    supportStackedWidget->addWidget(createSupportPage("Vui lòng mô tả lỗi bạn gặp phải..."));
    supportStackedWidget->addWidget(createSupportPage("Bạn có ý tưởng tuyệt vời nào cho ứng dụng?"));
    supportStackedWidget->addWidget(createSupportPage("Bạn có câu hỏi hoặc vấn đề nào khác...?"));
    supportContentLayout->addWidget(supportStackedWidget);
    connect(bugRadio, &QRadioButton::toggled, [=](bool checked){ if (checked) supportStackedWidget->setCurrentIndex(0); });
    connect(featureRadio, &QRadioButton::toggled, [=](bool checked){ if (checked) supportStackedWidget->setCurrentIndex(1); });
    connect(questionRadio, &QRadioButton::toggled, [=](bool checked){ if (checked) supportStackedWidget->setCurrentIndex(2); });
    supportContentLayout->addStretch();

    // --- 1C.4. Nội dung Panel Phản hồi ---
    QWidget *feedbackContentWidget = new QWidget;
    QVBoxLayout *feedbackContentLayout = new QVBoxLayout(feedbackContentWidget);
    feedbackContentLayout->setSpacing(15);
    // ... (Tạo QGroupBox, QRadioButton, QStackedWidget cho form Phản hồi) ...
    QGroupBox *typeGroupBox_fb = new QGroupBox("Bạn muốn chia sẻ điều gì?");
    QVBoxLayout *typeLayout_fb = new QVBoxLayout(typeGroupBox_fb);
    QRadioButton *positiveRadio_fb = new QRadioButton("Tôi có một lời khen");
    QRadioButton *negativeRadio_fb = new QRadioButton("Tôi không thích một điều gì đó");
    QRadioButton *bugRadio_fb = new QRadioButton("Tôi nghĩ tôi đã tìm thấy một lỗi");
    positiveRadio_fb->setChecked(true);
    typeLayout_fb->addWidget(positiveRadio_fb); typeLayout_fb->addWidget(negativeRadio_fb); typeLayout_fb->addWidget(bugRadio_fb);
    feedbackContentLayout->addWidget(typeGroupBox_fb);

    QStackedWidget *stackedWidget_fb = new QStackedWidget;
    auto createFeedbackPage_fb = [&](const QString &placeholder) {
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
    stackedWidget_fb->addWidget(createFeedbackPage_fb("Hãy cho chúng tôi biết bạn thích điều gì..."));
    stackedWidget_fb->addWidget(createFeedbackPage_fb("Chúng tôi có thể cải thiện điều gì?"));
    stackedWidget_fb->addWidget(createFeedbackPage_fb("Vui lòng mô tả lỗi bạn gặp phải..."));
    feedbackContentLayout->addWidget(stackedWidget_fb);
    connect(positiveRadio_fb, &QRadioButton::toggled, [=](bool checked){ if (checked) stackedWidget_fb->setCurrentIndex(0); });
    connect(negativeRadio_fb, &QRadioButton::toggled, [=](bool checked){ if (checked) stackedWidget_fb->setCurrentIndex(1); });
    connect(bugRadio_fb, &QRadioButton::toggled, [=](bool checked){ if (checked) stackedWidget_fb->setCurrentIndex(2); });
    feedbackContentLayout->addStretch();

    // --- 1C.5. Khởi tạo các SidePanel ---
    m_helpPanel = new SidePanel("Trợ giúp", this);
    m_helpPanel->setContentLayout(helpContentLayout);
    m_helpPanel->hide();

    m_tipsPanel = new SidePanel("Mẹo & Thủ thuật", this);
    m_tipsPanel->setContentLayout(tipsContentLayout);
    m_tipsPanel->hide();

    m_supportPanel = new SidePanel("Hỗ trợ", this);
    m_supportPanel->setContentLayout(supportContentLayout);
    m_supportPanel->hide();

    m_feedbackPanel = new SidePanel("Gửi phản hồi", this);
    m_feedbackPanel->setContentLayout(feedbackContentLayout);
    m_feedbackPanel->hide();

    // ===============================================================
    // === 1D. TẠO NỘI DUNG CHÍNH (LỊCH) ===
    // ===============================================================

    // --- 1D.1. Thanh Điều hướng Ngày (Date Navigation) ---
    QWidget *dateNavBar = new QWidget;
    dateNavBar->setObjectName("dateNavBar");
    QHBoxLayout *dateNavLayout = new QHBoxLayout(dateNavBar);
    dateNavLayout->setContentsMargins(10, 5, 10, 5);
    dateNavLayout->setSpacing(8);

    m_btnPrevWeek = new QPushButton;
    m_btnPrevWeek->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    m_btnPrevWeek->setObjectName("navButton");

    QPushButton *btnToday = new QPushButton("Hôm nay");
    btnToday->setObjectName("navButton");

    m_btnNextWeek = new QPushButton;
    m_btnNextWeek->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    m_btnNextWeek->setObjectName("navButton");

    m_dateNavButton = new QPushButton;
    m_dateNavButton->setObjectName("dateNavButton");
    m_dateNavButton->setCursor(Qt::PointingHandCursor);

    // Tạo lịch popup
    m_calendarPopup = new QCalendarWidget(this);
    m_calendarPopup->setLocale(QLocale(QLocale::Vietnamese));
    m_calendarPopup->setFirstDayOfWeek(Qt::Monday);
    m_calendarPopup->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    QTextCharFormat todayFormat;
    todayFormat.setTextOutline(QPen(QColor("#0078d7"), 1));
    m_calendarPopup->setDateTextFormat(QDate::currentDate(), todayFormat);

    QWidgetAction *calendarAction = new QWidgetAction(this);
    calendarAction->setDefaultWidget(m_calendarPopup);
    QMenu *calendarMenu = new QMenu(m_dateNavButton);
    calendarMenu->addAction(calendarAction);
    m_dateNavButton->setMenu(calendarMenu);

    // Thêm vào thanh điều hướng
    dateNavLayout->addWidget(m_btnPrevWeek);
    dateNavLayout->addWidget(btnToday);
    dateNavLayout->addWidget(m_btnNextWeek);
    dateNavLayout->addWidget(m_dateNavButton, 1);

    // --- 1D.2. Sidebar Trái (Lịch nhỏ & To-Do List) ---
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

    QLabel *noteTitle = new QLabel("📝 Ghi chú");
    noteTitle->setStyleSheet("font-weight: bold; margin-top:10px;");

    m_noteInput = new QTextEdit;
    m_noteInput->setPlaceholderText("Thêm việc cần làm...");
    m_noteInput->setObjectName("noteInput");
    m_noteInput->setMaximumHeight(65);

    QPushButton *btnAddNote = new QPushButton("+");
    btnAddNote->setObjectName("btnAddNote");
    btnAddNote->setCursor(Qt::PointingHandCursor);
    btnAddNote->setToolTip("Thêm công việc");

    QHBoxLayout *addLayout = new QHBoxLayout;
    addLayout->addWidget(m_noteInput);
    addLayout->addWidget(btnAddNote);

    m_todoList = new QListWidget;
    m_todoList->setObjectName("todoList");
    m_todoList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // ... (Xóa style thanh cuộn cho ToDo list) ...
    m_todoList->verticalScrollBar()->setStyleSheet(
        "QScrollBar::sub-line:vertical { border: none; background: none; height: 0px; }"
        "QScrollBar::add-line:vertical { border: none; background: none; height: 0px; }"
        );
    m_noteInput->verticalScrollBar()->setStyleSheet(
        "QScrollBar::sub-line:vertical { border: none; background: none; height: 0px; }"
        "QScrollBar::add-line:vertical { border: none; background: none; height: 0px; }"
        );


    sidebarLayout->addWidget(noteTitle);
    sidebarLayout->addLayout(addLayout);
    sidebarLayout->addWidget(m_todoList, 1);

    // --- 1D.3. Lưới Lịch chính (Main Grid) ---
    QWidget *calendarContainer = new QWidget;
    QGridLayout *grid = new QGridLayout(calendarContainer);
    grid->setSpacing(0);
    grid->setContentsMargins(0, 0, 0, 0);

    m_dayHeader = new DayHeader;
    m_dayHeader->setObjectName("dayHeaderWidget");

    m_timeRuler = new TimeRuler;
    m_timeRuler->setObjectName("timeRulerWidget");
    m_timeRuler->setFixedWidth(100);

    m_calendarView = new CalendarView;
    m_calendarView->setObjectName("mainCalendarView");

    m_calendarCorner = new QWidget;
    m_calendarCorner->setObjectName("calendarCornerWidget");
    m_calendarCorner->setFixedSize(100, 60);

    // --- 1D.4. Stack Chế độ xem (View Stack) ---
    m_monthView = new MonthViewWidget;
    m_monthView->setObjectName("monthViewWidget");

    m_timetableView = new TimetableViewWidget;
    m_timetableView->setObjectName("timetableViewWidget");

    m_sessionView = new SessionViewWidget;
    m_sessionView->setObjectName("sessionViewWidget");

    m_viewStack = new QStackedWidget;
    m_viewStack->addWidget(m_calendarView);      // Page 0 (Timeline)
    m_viewStack->addWidget(m_monthView);         // Page 1 (Tháng)
    m_viewStack->addWidget(m_timetableView);     // Page 2 (TKB Tiết)
    m_viewStack->addWidget(m_sessionView);       // Page 3 (TKB Buổi)

    // Thêm vào lưới lịch (Layout 'grid' của 'calendarContainer')
    grid->addWidget(m_calendarCorner, 0, 0); // (Hàng 0, Cột 0)
    grid->addWidget(m_dayHeader, 0, 1);      // (Hàng 0, Cột 1)
    grid->addWidget(m_timeRuler, 1, 0);      // (Hàng 1, Cột 0)
    grid->addWidget(m_viewStack, 1, 1);      // (Hàng 1, Cột 1)

    // Đồng bộ cột và hàng
    grid->setColumnStretch(1, 1);
    grid->setRowStretch(1, 1);

    // ===============================================================
    // === 1E. SẮP XẾP LAYOUT TRUNG TÂM ===
    // ===============================================================
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_topBar, 0, Qt::AlignTop); // Thanh toolbar trên cùng
    mainLayout->addWidget(dateNavBar);                // Thanh điều hướng ngày

    // Bọc sidebar trái và lưới lịch trong layout ngang
    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->setContentsMargins(0,0,0,0);
    contentLayout->setSpacing(0);
    contentLayout->addWidget(m_sidebarCalendar);      // Sidebar (ẩn ban đầu)
    contentLayout->addWidget(calendarContainer, 1); // Lưới lịch (co giãn)

    mainLayout->addLayout(contentLayout, 1); // Thêm vào layout chính

    setCentralWidget(central);

    // ===============================================================
    // === 1F. KẾT NỐI TÍN HIỆU & KHỞI TẠO (SIGNALS & SLOTS) ===
    // ===============================================================

    // --- Kết nối Tương tác Lịch ---
    connect(m_calendarView, &CalendarView::eventClicked, this, &MainWindow::onEventItemClicked);
    connect(m_monthView, &MonthViewWidget::eventClicked, this, &MainWindow::onEventItemClicked);
    connect(m_timetableView, &TimetableViewWidget::eventClicked, this, &MainWindow::onEventItemClicked);
    connect(m_sessionView, &SessionViewWidget::eventClicked, this, &MainWindow::onEventItemClicked);
    connect(m_calendarView, &CalendarView::eventDragged, this, &MainWindow::onEventItemDragged);

    // --- Kết nối Điều hướng ---
    connect(m_btnPrevWeek, &QPushButton::clicked, this, &MainWindow::showPreviousWeek);
    connect(m_btnNextWeek, &QPushButton::clicked, this, &MainWindow::showNextWeek);
    connect(btnToday, &QPushButton::clicked, this, &MainWindow::showToday);
    connect(m_calendarPopup, &QCalendarWidget::clicked, this, &MainWindow::onDateSelectedFromPopup);
    connect(miniCalendar, &QCalendarWidget::clicked, this, &MainWindow::onDateSelectedFromPopup);

    // --- Kết nối Đồng bộ Thanh cuộn ---
    connect(m_calendarView->horizontalScrollBar(), &QScrollBar::valueChanged, m_dayHeader, &DayHeader::setScrollOffset);
    connect(m_calendarView->verticalScrollBar(), &QScrollBar::valueChanged, m_timeRuler, &TimeRuler::setScrollOffset);

    // --- Kết nối UI Chung ---
    connect(tabBar, &QTabBar::currentChanged, this, [=](int index) {
        m_toolbarStack->setCurrentIndex(index);
    });
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

    // --- Kết nối To-Do List ---
    connect(btnAddNote, &QPushButton::clicked, this, &MainWindow::onAddTodoItem);

    // --- Kết nối Panel Trợ giúp (Đã connect trong 1B.4) ---
    // connect(btnShowHelp, &QToolButton::clicked, this, &MainWindow::toggleHelpPanel);
    // connect(btnTips, &QToolButton::clicked, this, &MainWindow::toggleTipsPanel);

    // ===============================================================
    // === 1G. TẢI DỮ LIỆU VÀ CÀI ĐẶT ===
    // ===============================================================

    tabBar->setCurrentIndex(0);
    m_toolbarStack->setCurrentIndex(0);

    showToday(); // Đặt ngày về hôm nay
    m_helpPanel->hide();
    m_tipsPanel->hide();

    // Tự động cuộn đến 6 giờ sáng
    QScrollBar *verticalScrollBar = m_calendarView->verticalScrollBar();
    int scrollToPosition = 7.5 * m_calendarView->getHourHeight();
    verticalScrollBar->setValue(scrollToPosition);

    // Tải dữ liệu và cài đặt
    initSavePath();
    loadData();
    loadSettings();

    // Áp dụng cài đặt đã tải
    applyTimeSettings();
    onFilterChanged(); // Áp dụng bộ lọc lần đầu
}

MainWindow::~MainWindow()
{
    // Lưu dữ liệu khi đóng
    saveData();

    // Dọn dẹp con trỏ sự kiện
    qDeleteAll(m_allEventItems);
    m_allEventItems.clear();

    delete ui;
}

// =================================================================================
// === 2. PROTECTED EVENT HANDLERS (Sự kiện override từ QMainWindow)
// =================================================================================

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    bool compact = width() < 900; //Nếu cửa sổ < 900px thì ẩn chữ
    setMinimumHeight(600);

    // Duyệt qua từng trang (toolbar) trong QStackedWidget
    for (int i = 0; i < m_toolbarStack->count(); ++i) {
        QWidget *toolbarPage = m_toolbarStack->widget(i);
        if (toolbarPage) {
            // Tìm các nút chỉ trong trang toolbar đó
            const auto buttons = toolbarPage->findChildren<QToolButton*>();
            for (auto button : buttons) {
                button->setToolButtonStyle(compact ? Qt::ToolButtonIconOnly : Qt::ToolButtonTextBesideIcon);
            }

            // Báo cho layout rằng nó cần tính toán lại
            if (toolbarPage->layout()) {
                toolbarPage->layout()->invalidate();
            }
        }
    }

    // Giữ vị trí của help panel khi resize cửa sổ
    if (m_helpPanel && !m_helpPanel->isHidden()) {
        int panelWidth = m_helpPanel->width();
        m_helpPanel->setGeometry(width() - panelWidth, m_topBar->height(), panelWidth, height() - m_topBar->height());
    }
    if (m_funnyTipWidget) {
        m_funnyTipWidget->reposition();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveData(); // Đảm bảo lưu lần cuối
    QMainWindow::closeEvent(event);
}

// =================================================================================
// === 3. PRIVATE SLOTS - UI & NAVIGATION
// =================================================================================

// --- 3A. Slots Chuyển đổi Dạng xem (View Switching) ---

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
    int offset = days / 2;
    QDate startDate = today.addDays(-offset);

    // 2. Cập nhật ngày bắt đầu
    m_currentMonday = startDate;

    // 3. Cập nhật số ngày cho CalendarView và DayHeader
    m_calendarView->setNumberOfDays(days);
    m_dayHeader->setNumberOfDays(days);

    // 4. Cập nhật lại toàn bộ hiển thị
    updateCalendarDisplay();
}

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

void MainWindow::showFullWeek()
{
    showToday(); // Giống hệt nút "Hôm nay"
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
    QDate today = QDate::currentDate();
    m_currentMonday = today.addDays(-(today.dayOfWeek() - 1));

    m_dayHeader->updateDates(m_currentMonday);
    m_monthView->updateView(today); // Yêu cầu MonthView vẽ tháng hiện tại

    // 5. Cập nhật nhãn tháng/năm
    updateCalendarDisplay();
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

// --- 3B. Slots Điều hướng Ngày (Date Navigation) ---

void MainWindow::showPreviousWeek()
{
    if (m_viewStack->currentWidget() == m_monthView) {
        // Nếu là view tháng, lùi 1 tháng
        m_currentMonday = m_currentMonday.addMonths(-1);
    } else {
        // Logic cũ: lùi 1 tuần
        m_currentMonday = m_currentMonday.addDays(-7);
    }
    updateCalendarDisplay();
}

void MainWindow::showNextWeek()
{
    if (m_viewStack->currentWidget() == m_monthView) {
        // Nếu là view tháng, tiến 1 tháng
        m_currentMonday = m_currentMonday.addMonths(1);
    } else {
        // Logic cũ: tiến 1 tuần
        m_currentMonday = m_currentMonday.addDays(7);
    }
    updateCalendarDisplay();
}

void MainWindow::showToday()
{
    m_btnTimeScale->setEnabled(true);
    // Chuyển về view timeline (CalendarView)
    m_viewStack->setCurrentWidget(m_calendarView);
    m_timeRuler->setVisible(true);
    m_calendarCorner->setVisible(true);
    m_dayHeader->setRightMargin(m_calendarView->verticalScrollBar()->width());
    m_dayHeader->setVisible(true);

    QDate today = QDate::currentDate();
    // Tính ngày thứ Hai của tuần hiện tại
    m_currentMonday = today.addDays(-(today.dayOfWeek() - 1));

    // Reset lại số ngày về 7
    m_calendarView->setNumberOfDays(7);
    m_dayHeader->setNumberOfDays(7);

    updateCalendarDisplay();
}

void MainWindow::onDateSelectedFromPopup(const QDate &date)
{
    // Tính toán ngày thứ Hai của tuần chứa ngày được chọn
    int daysToMonday = date.dayOfWeek() - 1;
    m_currentMonday = date.addDays(-daysToMonday);

    // Reset lại số ngày về 7
    m_calendarView->setNumberOfDays(7);
    m_dayHeader->setNumberOfDays(7);

    // Cập nhật lại toàn bộ giao diện
    updateCalendarDisplay();

    // Ẩn menu đi sau khi đã chọn
    m_dateNavButton->menu()->hide();
}

// --- 3C. Slots Toolbar "Tạo mới" ---

void MainWindow::onNewEventClicked()
{
    EventDialog dialog(this);
    dialog.setEventType("Sự kiện");
    dialog.setTimezoneOffset(m_timezoneOffsetSeconds);

    if (dialog.exec() == QDialog::Accepted) {
        if (dialog.getEditResult() == EventDialog::EditResult::Save) {
            addEventFromDialog(dialog);
        }
    }
}

void MainWindow::onNewMeetingClicked()
{
    EventDialog dialog(this);
    dialog.setEventType("Cuộc họp");
    dialog.setTimezoneOffset(m_timezoneOffsetSeconds);

    if (dialog.exec() == QDialog::Accepted) {
        if (dialog.getEditResult() == EventDialog::EditResult::Save) {
            addEventFromDialog(dialog);
        }
    }
}

void MainWindow::onNewStudyClicked()
{
    EventDialog dialog(this);
    dialog.setEventType("Học tập");
    dialog.setTimezoneOffset(m_timezoneOffsetSeconds);
    if (dialog.exec() == QDialog::Accepted && dialog.getEditResult() == EventDialog::EditResult::Save) {
        addEventFromDialog(dialog);
    }
}

void MainWindow::onNewHolidayClicked()
{
    EventDialog dialog(this);
    dialog.setEventType("Ngày lễ");
    dialog.setTimezoneOffset(m_timezoneOffsetSeconds);
    if (dialog.exec() == QDialog::Accepted && dialog.getEditResult() == EventDialog::EditResult::Save) {
        addEventFromDialog(dialog);
    }
}

void MainWindow::onNewAppointmentClicked()
{
    EventDialog dialog(this);
    dialog.setEventType("Cuộc hẹn");
    dialog.setTimezoneOffset(m_timezoneOffsetSeconds);
    if (dialog.exec() == QDialog::Accepted && dialog.getEditResult() == EventDialog::EditResult::Save) {
        addEventFromDialog(dialog);
    }
}

// --- 3D. Slots Cài đặt & Panel bên ---

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
    }

    // 5. Tự động cuộn đến 7 giờ sáng
    QScrollBar *verticalScrollBar = m_calendarView->verticalScrollBar();
    int scrollToPosition = 7.5 * newHourHeight;
    verticalScrollBar->setValue(scrollToPosition);
}

void MainWindow::openSettingsDialog()
{
    SettingsDialog dialog(this);
    dialog.setObjectName("SettingsDialog");

    // Truyền cài đặt hiện tại vào dialog
    dialog.setCurrentSettings(m_use24HourFormat, m_timezoneOffsetSeconds);

    if (dialog.exec() == QDialog::Accepted) {
        // Lấy cài đặt giờ mới
        m_timezoneOffsetSeconds = dialog.getSelectedOffsetSeconds();
        m_use24HourFormat = dialog.is24HourFormat();
        applyTimeSettings(); // Áp dụng cài đặt giờ

        // Áp dụng cài đặt nền
        changeBackgroundImage(dialog.selectedBackgroundIndex(),
                              dialog.selectedImagePath(),
                              dialog.selectedSolidColor());

        // Áp dụng cài đặt trong suốt
        setCalendarTransparency(dialog.isCalendarTransparent());

        // Lưu TẤT CẢ cài đặt vào file settings.json
        saveSettings(&dialog);
    }
}

void MainWindow::toggleHelpPanel()
{
    m_tipsPanel->hidePanel(this->geometry(), m_topBar->height());
    m_supportPanel->hidePanel(this->geometry(), m_topBar->height());
    m_feedbackPanel->hidePanel(this->geometry(), m_topBar->height());
    m_helpPanel->toggleVisibility(this->geometry(), m_topBar->height());
}

void MainWindow::toggleTipsPanel()
{
    m_helpPanel->hidePanel(this->geometry(), m_topBar->height());
    m_supportPanel->hidePanel(this->geometry(), m_topBar->height());
    m_feedbackPanel->hidePanel(this->geometry(), m_topBar->height());
    m_tipsPanel->toggleVisibility(this->geometry(), m_topBar->height());
}

void MainWindow::toggleSupportPanel()
{
    m_helpPanel->hidePanel(this->geometry(), m_topBar->height());
    m_tipsPanel->hidePanel(this->geometry(), m_topBar->height());
    m_feedbackPanel->hidePanel(this->geometry(), m_topBar->height());
    m_supportPanel->toggleVisibility(this->geometry(), m_topBar->height());
}

void MainWindow::toggleFeedbackPanel()
{
    m_helpPanel->hidePanel(this->geometry(), m_topBar->height());
    m_tipsPanel->hidePanel(this->geometry(), m_topBar->height());
    m_supportPanel->hidePanel(this->geometry(), m_topBar->height());
    m_feedbackPanel->toggleVisibility(this->geometry(), m_topBar->height());
}

// =================================================================================
// === 4. PRIVATE SLOTS - CALENDAR INTERACTION
// =================================================================================

void MainWindow::onEventItemClicked(EventItem *item)
{
    if (!item) return;

    EventDialog dialog(this);
    dialog.setTimezoneOffset(m_timezoneOffsetSeconds);

    // 1. Lấy thời gian UTC từ sự kiện
    QDateTime utcStart = item->startTime();
    QDateTime utcEnd = item->endTime();

    // 2. Chuyển đổi UTC về giờ hiển thị (Local Time)
    QDateTime displayStart = convertToDisplayTime(utcStart);
    QDateTime displayEnd = convertToDisplayTime(utcEnd);

    // 3. Nạp dữ liệu vào dialog
    dialog.setEventData(
        item->title(),
        displayStart,
        displayEnd,
        item->color(),
        item->description(),
        item->showAsStatus(),
        item->category(),
        item->isAllDay(),
        item->recurrenceRule(),
        item->eventType(),
        item->extraData()
        );

    // 4. Mở dialog và chờ kết quả
    if (dialog.exec() == QDialog::Accepted) {
        EventDialog::EditResult result = dialog.getEditResult();
        bool isRecurrent = item->recurrenceRule().isRecurrent;

        if (isRecurrent) {
            // 5. Nếu lặp, hiển thị dialog hỏi "Sự kiện nào?"
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("Sự kiện lặp lại");
            msgBox.setIcon(QMessageBox::Question);

            if (result == EventDialog::EditResult::Save) {
                msgBox.setText("Bạn muốn áp dụng thay đổi cho sự kiện nào?");
            } else { // (result == EventDialog::EditResult::Delete)
                msgBox.setText("Bạn muốn xóa sự kiện nào?");
            }

            QAbstractButton* pButtonThis = msgBox.addButton("Chỉ sự kiện này", QMessageBox::ActionRole);
            QAbstractButton* pButtonAll = msgBox.addButton("Chuỗi sự kiện", QMessageBox::ActionRole);
            msgBox.addButton("Hủy bỏ", QMessageBox::RejectRole);

            msgBox.exec();

            // 6. Xử lý kết quả
            if (msgBox.clickedButton() == pButtonThis) {
                if (result == EventDialog::EditResult::Save) {
                    updateSingleEvent(item, dialog); // Tách sự kiện
                } else {
                    deleteSingleEvent(item); // Xóa 1 sự kiện
                }
            } else if (msgBox.clickedButton() == pButtonAll) {
                if (result == EventDialog::EditResult::Save) {
                    updateEventSeries(item, dialog); // Sửa cả chuỗi
                } else {
                    deleteEventSeries(item); // Xóa cả chuỗi
                }
            }
            // (Nếu Hủy bỏ thì không làm gì)

        } else {
            // --- Logic cho sự kiện đơn lẻ (không lặp) ---
            if (result == EventDialog::EditResult::Save) {
                // Đơn giản là xóa cũ, thêm mới
                removeEventFromViews(item);
                addEventFromDialog(dialog);
            } else if (result == EventDialog::EditResult::Delete) {
                removeEventFromViews(item);
            }
        }
    }
}

void MainWindow::onEventItemDragged(EventItem *item, const QDateTime &newStartTime, const QDateTime &newEndTime)
{
    if (!item) return;

    // 1. Tạo Dialog (ảo)
    EventDialog dialog(this);
    dialog.setTimezoneOffset(m_timezoneOffsetSeconds);

    // 2. Lấy thời gian UTC từ sự kiện
    QDateTime utcStart = item->startTime();
    QDateTime utcEnd = item->endTime();

    // 3. Chuyển đổi UTC về giờ hiển thị (Local Time)
    QDateTime displayStart = convertToDisplayTime(utcStart);
    QDateTime displayEnd = convertToDisplayTime(utcEnd);

    // 4. Nạp dữ liệu CŨ
    dialog.setEventData(
        item->title(),
        displayStart,
        displayEnd,
        item->color(),
        item->description(),
        item->showAsStatus(),
        item->category(),
        item->isAllDay(),
        item->recurrenceRule(),
        item->eventType(),
        item->extraData()
        );

    // 5. GHI ĐÈ thời gian MỚI (từ newStartTime, đã là local)
    dialog.setNewStartDateTime(newStartTime);
    dialog.setNewEndDateTime(newEndTime);

    // 6. Kiểm tra lặp
    bool isRecurrent = item->recurrenceRule().isRecurrent;

    if (isRecurrent) {
        // 7. Hiển thị dialog hỏi
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Sự kiện lặp lại");
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText("Bạn muốn áp dụng thay đổi thời gian cho sự kiện nào?");
        QAbstractButton* pButtonThis = msgBox.addButton("Chỉ sự kiện này", QMessageBox::ActionRole);
        QAbstractButton* pButtonAll = msgBox.addButton("Chuỗi sự kiện", QMessageBox::ActionRole);
        msgBox.addButton("Hủy bỏ", QMessageBox::RejectRole);
        msgBox.exec();

        // 8. Xử lý kết quả
        if (msgBox.clickedButton() == pButtonThis) {
            // Tách sự kiện này ra khỏi chuỗi
            EventDialog::RecurrenceRule singleRule = item->recurrenceRule();
            singleRule.isRecurrent = false; // Biến nó thành sự kiện đơn
            dialog.setNewRecurrenceRule(singleRule);

            updateSingleEvent(item, dialog);

        } else if (msgBox.clickedButton() == pButtonAll) {
            // Di chuyển toàn bộ chuỗi
            EventDialog::RecurrenceRule newRule = item->recurrenceRule();
            qint64 dragDeltaDays = item->startTime().date().daysTo(newStartTime.date());
            Qt::DayOfWeek oldDayOfWeek = static_cast<Qt::DayOfWeek>(item->startTime().date().dayOfWeek());
            Qt::DayOfWeek newDayOfWeek = static_cast<Qt::DayOfWeek>(newStartTime.date().dayOfWeek());

            // Cập nhật ngày kết thúc của chuỗi
            newRule.endDate = newRule.endDate.addDays(dragDeltaDays);

            // Cập nhật NGÀY TRONG TUẦN của quy tắc
            if (newRule.days.contains(oldDayOfWeek)) {
                newRule.days.removeAll(oldDayOfWeek);
                if (!newRule.days.contains(newDayOfWeek)) {
                    newRule.days.append(newDayOfWeek);
                }
            }
            dialog.setNewRecurrenceRule(newRule);
            updateEventSeries(item, dialog);
        }
        // (Nếu Hủy bỏ thì không làm gì)

    } else {
        // 9. Sự kiện đơn lẻ (không lặp)
        removeEventFromViews(item);
        addEventFromDialog(dialog);
    }
}

// =================================================================================
// === 5. PRIVATE SLOTS - DATA & FILTERING
// =================================================================================

// --- 5A. Slots Bộ lọc (Filter) ---

void MainWindow::onFilterChanged()
{
    // 1. Cập nhật danh sách Category (Thể loại)
    m_visibleCategories.clear();
    for (QAction *action : m_categoryActions) {
        if (action->isChecked()) {
            m_visibleCategories.insert(action->text().trimmed());
        }
    }

    // 2. Cập nhật danh sách Status (Hiển thị như)
    m_visibleStatuses.clear();
    for (QAction *action : m_statusActions) {
        if (action->isChecked()) {
            m_visibleStatuses.insert(action->text().trimmed());
        }
    }

    // 3. Cập nhật danh sách Recurrence (Lặp lại)
    m_visibleRecurrenceTypes.clear();
    for (QAction *action : m_recurrenceActions) {
        if (action->isChecked()) {
            m_visibleRecurrenceTypes.insert(action->text().trimmed());
        }
    }

    // 4. Cập nhật Trạng thái Cuộc họp
    m_visibleMeetingStatuses.clear();
    for (QAction *action : m_meetingStatusActions) {
        if (action->isChecked()) {
            m_visibleMeetingStatuses.insert(action->text().trimmed());
        }
    }

    // 5. Cập nhật các bộ lọc chính và phụ
    m_visibleEventTypes.clear();
    for (QAction *action : m_eventTypeActions) {
        if (action->isChecked()) m_visibleEventTypes.insert(action->text().trimmed());
    }

    m_visibleStudyMethods.clear();
    for (QAction *action : m_studyMethodActions) {
        if (action->isChecked()) m_visibleStudyMethods.insert(action->text().trimmed());
    }

    m_visibleHolidayScopes.clear();
    for (QAction *action : m_holidayScopeActions) {
        if (action->isChecked()) m_visibleHolidayScopes.insert(action->text().trimmed());
    }

    m_visibleAppointmentTypes.clear();
    for (QAction *action : m_appointmentTypeActions) {
        if (action->isChecked()) m_visibleAppointmentTypes.insert(action->text().trimmed());
    }

    m_visibleAppointmentPrivacy.clear();
    for (QAction *action : m_appointmentPrivacyActions) {
        if (action->isChecked()) m_visibleAppointmentPrivacy.insert(action->text().trimmed());
    }

    // 6. Áp dụng bộ lọc mới
    applyFilters();
}

// --- 5B. Slots In & Nhập/Xuất (Print & Import/Export) ---

void MainWindow::onPrintToPdf()
{
    // 1. Xác định widget
    QWidget *currentView = m_viewStack->currentWidget();
    if (!currentView) return;

    // 2. Hỏi lưu file
    QString defaultFileName = QString("Lich_%1.pdf").arg(QDate::currentDate().toString("ddMMyyyy"));
    QString filePath = QFileDialog::getSaveFileName(this, "Lưu PDF", defaultFileName, "Tệp PDF (*.pdf)");
    if (filePath.isEmpty()) return;

    // 3. Cấu hình máy in
    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setResolution(300);
    printer.setFullPage(true);

    // 4. Cấu hình khổ giấy
    QMarginsF margins(10, 10, 10, 10);
    if (currentView == m_monthView) {
        printer.setPageLayout(QPageLayout(QPageSize(QPageSize::A4), QPageLayout::Portrait, margins));
    } else {
        printer.setPageLayout(QPageLayout(QPageSize(QPageSize::A4), QPageLayout::Landscape, margins));
    }

    // 5. Khởi tạo QPainter
    QPainter painter;
    if (!painter.begin(&printer)) {
        qWarning("Không thể khởi tạo painter cho máy in PDF!");
        return;
    }
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 6. Lấy kích thước trang
    QRect printerRect = printer.pageRect(QPrinter::DevicePixel).toRect();

    // 7. Render (Trường hợp 1: Timeline)
    if (currentView == m_calendarView)
    {
        QGraphicsScene *scene = m_calendarView->scene();
        if (!scene) { painter.end(); return; }

        QWidget *corner = m_calendarCorner;
        QWidget *header = m_dayHeader;

        // Tạo thước ảo để in (vì thước thật có thể bị cuộn)
        TimeRuler printRuler;
        printRuler.set24HourFormat(m_use24HourFormat);
        printRuler.setHourHeight(m_calendarView->getHourHeight());
        printRuler.setTimezoneOffset(m_timezoneOffsetSeconds);
        printRuler.setScrollOffset(0); // In từ đầu (0:00)
        double hourHeight = m_calendarView->getHourHeight();
        double fullHeight = hourHeight * 24.0;
        printRuler.setFixedSize(m_timeRuler->width(), (int)fullHeight);

        // Tính tổng kích thước logic (pixel ảo)
        double dayWidth = m_calendarView->getDayWidth();
        int numDays = m_calendarView->getNumberOfDays();
        double totalDaysWidth = dayWidth * numDays;
        QRect totalWidgetRect(0, 0, corner->width() + (int)totalDaysWidth, corner->height() + (int)fullHeight);

        // Áp dụng biến đổi (Map logic -> vật lý)
        painter.setViewport(printerRect);
        painter.setWindow(totalWidgetRect);

        // VẼ CÁC THÀNH PHẦN NỀN
        corner->render(&painter, QPoint(0, 0));
        painter.save();
        painter.translate(corner->width(), 0);
        header->render(&painter, QPoint(0, 0), QRegion(0, 0, (int)totalDaysWidth, header->height()));
        painter.restore();
        painter.save();
        painter.translate(0, corner->height());
        printRuler.render(&painter, QPoint(0, 0), QRegion(0, 0, printRuler.width(), (int)fullHeight));
        painter.restore();

        // VẼ LƯỚI (Grid)
        painter.save();
        painter.translate(corner->width(), corner->height());
        QPen mainGridPen(QColor(224, 224, 224), 1, Qt::SolidLine);
        QPen subGridPen(QColor(240, 240, 240), 1, Qt::DotLine);
        // Vẽ đường kẻ ngang
        for (int i = 0; i < 24; ++i) {
            double yMain = i * hourHeight;
            painter.setPen(mainGridPen);
            painter.drawLine(QPointF(0, yMain), QPointF(totalDaysWidth, yMain));
            double ySub = yMain + (hourHeight / 2.0);
            painter.setPen(subGridPen);
            painter.drawLine(QPointF(0, ySub), QPointF(totalDaysWidth, ySub));
        }
        // Vẽ đường kẻ dọc
        painter.setPen(mainGridPen);
        for (int i = 1; i <= numDays; ++i) {
            double x = i * dayWidth;
            painter.drawLine(QPointF(x, 0), QPointF(x, fullHeight));
        }
        painter.restore(); // Hoàn tất vẽ lưới

        // VẼ CÁC SỰ KIỆN (NỘI DUNG SCENE)
        painter.save();
        painter.translate(corner->width(), corner->height());
        QRectF sourceRect(0, 0, totalDaysWidth, fullHeight); // Vùng logic cần vẽ
        QRectF targetRect(0, 0, totalDaysWidth, fullHeight); // Vùng đích
        scene->render(&painter, targetRect, sourceRect);
        painter.restore();
    }
    // 7. Render (Trường hợp 2: Các view khác)
    else
    {
        if (printerRect.width() == 0 || printerRect.height() == 0) {
            qWarning("Kích thước máy in không hợp lệ!");
            painter.end();
            return;
        }
        double printerAspectRatio = (double)printerRect.height() / (double)printerRect.width();
        int virtualWidth = 1300;
        int virtualHeight = (int)(virtualWidth * printerAspectRatio);
        QRect widgetRect(0, 0, virtualWidth, virtualHeight);
        painter.setViewport(printerRect);
        painter.setWindow(widgetRect);
        currentView->render(&painter);
    }

    // 8. Hoàn tất
    painter.end();
    QMessageBox::information(this, "Hoàn tất", "Đã xuất PDF thành công!");
}

void MainWindow::onExportData()
{
    // 1. Kiểm tra file nguồn
    if (m_saveFilePath.isEmpty() || !QFile::exists(m_saveFilePath)) {
        QMessageBox::warning(this, "Lỗi", "Không tìm thấy file dữ liệu nguồn để xuất.");
        return;
    }

    // 2. Lấy tên file gốc
    QFileInfo fileInfo(m_saveFilePath);
    QString defaultFileName = fileInfo.fileName();

    // 3. Mở dialog "Save As"
    QString destPath = QFileDialog::getSaveFileName(this, "Xuất dữ liệu lịch", defaultFileName, "Tệp JSON (*.json)");
    if (destPath.isEmpty()) return;

    // 4. Sao chép file
    if (QFile::exists(destPath)) {
        QFile::remove(destPath);
    }
    if (QFile::copy(m_saveFilePath, destPath)) {
        QMessageBox::information(this, "Thành công", "Đã xuất dữ liệu thành công!");
    } else {
        QMessageBox::critical(this, "Lỗi", "Không thể sao chép file. Vui lòng thử lại.");
    }
}

void MainWindow::onImportData()
{
    // 1. Cảnh báo người dùng
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Xác nhận nhập",
                                  "Thao tác này sẽ **GHI ĐÈ** toàn bộ dữ liệu lịch hiện tại của bạn.\n"
                                  "Dữ liệu cũ sẽ bị mất. Bạn có chắc chắn muốn tiếp tục?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) return;

    // 2. Mở dialog "Open"
    QString sourcePath = QFileDialog::getOpenFileName(this, "Nhập dữ liệu lịch", "", "Tệp JSON (*.json)");
    if (sourcePath.isEmpty()) return;

    // 3. Đọc file nguồn
    QFile sourceFile(sourcePath);
    if (!sourceFile.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Lỗi", "Không thể đọc file nguồn được chọn.");
        return;
    }
    QByteArray fileData = sourceFile.readAll();
    sourceFile.close();

    // 4. Ghi đè file dữ liệu hiện tại
    QSaveFile destFile(m_saveFilePath);
    if (destFile.open(QIODevice::WriteOnly)) {
        destFile.write(fileData);
        if (destFile.commit()) {
            // 5. Tải lại dữ liệu
            loadData();
            QMessageBox::information(this, "Thành công", "Đã nhập và tải lại dữ liệu thành công!");
        } else {
            QMessageBox::critical(this, "Lỗi", "Không thể ghi đè file dữ liệu (lỗi commit).");
        }
    } else {
        QMessageBox::critical(this, "Lỗi", "Không thể mở file dữ liệu đích để ghi.");
    }
}

// --- 5C. Slots Sidebar (To-Do List) ---

void MainWindow::onAddTodoItem()
{
    QString text = m_noteInput->toPlainText().trimmed();
    if (text.isEmpty()) return;

    addTodoItem(text, false); // Gọi hàm trợ giúp
    m_noteInput->clear();
    saveData(); // Lưu lại
}

void MainWindow::onTodoItemChanged(int state)
{
    QCheckBox *check = qobject_cast<QCheckBox*>(sender());
    if (!check) return;

    QWidget *itemWidget = check->parentWidget();
    if (!itemWidget) return;

    QLabel *todoLabel = itemWidget->findChild<QLabel*>();
    if (!todoLabel) return;

    // Cập nhật style
    bool completed = (state == Qt::Checked);
    if (completed) {
        todoLabel->setStyleSheet("color: #999; text-decoration: line-through;");
        itemWidget->setStyleSheet("background-color: #f0f0f0;");
    } else {
        todoLabel->setStyleSheet("");
        itemWidget->setStyleSheet("");
    }

    saveData(); // Lưu khi thay đổi
}

void MainWindow::onTodoItemDeleted()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    QVariant itemData = btn->property("listItem");
    if (!itemData.isValid()) return;

    QListWidgetItem *item = itemData.value<QListWidgetItem*>();
    if (item) {
        int row = m_todoList->row(item);
        delete m_todoList->takeItem(row); // Xóa khỏi list
        saveData(); // Lưu lại
    }
}

// =================================================================================
// === 6. CORE LOGIC - DISPLAY & VIEW
// =================================================================================

void MainWindow::updateCalendarDisplay()
{
    int daysToShow;
    QDate endDate; // Biến để lưu ngày kết thúc

    // KIỂM TRA XEM ĐANG Ở VIEW NÀO
    if (m_viewStack->currentWidget() == m_monthView) {
        // Chế độ xem tháng
        daysToShow = 7; // Header luôn là 7 ngày
        endDate = m_currentMonday.addDays(6);
        m_monthView->updateView(m_currentMonday);

    } else if (m_viewStack->currentWidget() == m_timetableView) {
        // Chế độ xem TKB
        daysToShow = 6; // Luôn là 6 ngày (T2-T7)
        endDate = m_currentMonday.addDays(daysToShow - 1);
        m_timetableView->updateView(m_currentMonday);

    } else if (m_viewStack->currentWidget() == m_sessionView) {
        // Chế độ xem TKB Buổi
        daysToShow = 6;
        endDate = m_currentMonday.addDays(daysToShow - 1);
        m_sessionView->updateView(m_currentMonday);

    } else {
        // Chế độ xem timeline (Ngày/Tuần)
        daysToShow = m_calendarView->getNumberOfDays();
        endDate = m_currentMonday.addDays(daysToShow - 1);
        m_calendarView->updateViewForDateRange(m_currentMonday);
    }

    // Cập nhật nhãn tháng/năm trên thanh điều hướng
    QString dateRangeText;
    QLocale viLocale(QLocale::Vietnamese);

    if (m_currentMonday.month() == endDate.month()) {
        dateRangeText = viLocale.monthName(m_currentMonday.month()) + ", " + m_currentMonday.toString("yyyy");
    } else {
        dateRangeText = viLocale.monthName(m_currentMonday.month()) + " - " + viLocale.monthName(endDate.month()) + ", " + m_currentMonday.toString("yyyy");
    }
    m_dateNavButton->setText(dateRangeText);
    m_calendarPopup->setSelectedDate(m_currentMonday);

    // Cập nhật DayHeader (thanh T2, T3, T4...)
    if (m_viewStack->currentWidget() == m_monthView) {
        QDate mondayOfReferenceWeek = m_currentMonday.addDays(-(m_currentMonday.dayOfWeek() - 1));
        m_dayHeader->updateDates(mondayOfReferenceWeek);
    } else if (m_viewStack->currentWidget() == m_calendarView) {
        m_dayHeader->updateDates(m_currentMonday);
    }
}

void MainWindow::applyFilters()
{
    // Lặp qua BẢN GỐC của tất cả sự kiện
    for (EventItem *item : m_allEventItems) {
        if (!item) continue;

        // === BƯỚC 1: KIỂM TRA CÁC BỘ LỌC CHUNG ===
        bool eventTypeMatch = m_visibleEventTypes.contains(item->eventType());
        bool categoryMatch = m_visibleCategories.contains(item->category());
        bool statusMatch = m_visibleStatuses.contains(item->showAsStatus());
        QString itemRecurrenceType = item->recurrenceRule().isRecurrent ? "Chuỗi" : "Đơn";
        bool recurrenceMatch = m_visibleRecurrenceTypes.contains(itemRecurrenceType);

        // === BƯỚC 2: KIỂM TRA BỘ LỌC CON (SUB-FILTER) ===
        bool subFilterMatch = true; // Mặc định là 'true' (vượt qua)

        if (eventTypeMatch)
        {
            if (item->eventType() == "Cuộc họp") {
                QString status = item->extraData().value("meetingStatus").toString("Dự kiến");
                subFilterMatch = m_visibleMeetingStatuses.contains(status);
            }
            else if (item->eventType() == "Học tập") {
                QString method = item->extraData().value("studyMethod").toString("Tự học");
                subFilterMatch = m_visibleStudyMethods.contains(method);
            }
            else if (item->eventType() == "Ngày lễ") {
                QString scope = item->extraData().value("holidayScope").toString("Tùy chỉnh");
                subFilterMatch = m_visibleHolidayScopes.contains(scope);
            }
            else if (item->eventType() == "Cuộc hẹn") {
                QString type = item->extraData().value("appointmentType").toString("Khác");
                bool typeMatch = m_visibleAppointmentTypes.contains(type);
                bool isPrivate = item->extraData().value("isPrivate").toBool(false);
                QString privacy = isPrivate ? "Riêng tư" : "Công khai";
                bool privacyMatch = m_visibleAppointmentPrivacy.contains(privacy);
                subFilterMatch = typeMatch && privacyMatch;
            }
            else if (item->eventType() == "Sự kiện") {
                // Sự kiện thông thường phải khớp với trạng thái "Không phải cuộc họp"
                subFilterMatch = m_visibleMeetingStatuses.contains("Không phải cuộc họp");
            }
        }

        // === BƯỚC 3: LOGIC LỌC CUỐI CÙNG ===
        bool isVisible = eventTypeMatch &&
                         categoryMatch &&
                         statusMatch &&
                         recurrenceMatch &&
                         subFilterMatch;

        item->setFiltered(!isVisible); // Ra lệnh cho item tự ẩn/hiện
    }

    // Yêu cầu các View vẽ lại
    updateCalendarDisplay();
}

void MainWindow::applyTimeSettings()
{
    // 1. Áp dụng cho TimeRuler (cột giờ)
    if (m_timeRuler) {
        m_timeRuler->set24HourFormat(m_use24HourFormat);
        m_timeRuler->setTimezoneOffset(m_timezoneOffsetSeconds);
    }

    // 2. THÔNG BÁO CHO CÁC VIEW
    m_calendarView->setTimezoneOffset(m_timezoneOffsetSeconds);
    m_monthView->setTimezoneOffset(m_timezoneOffsetSeconds);
    m_timetableView->setTimezoneOffset(m_timezoneOffsetSeconds);
    m_sessionView->setTimezoneOffset(m_timezoneOffsetSeconds);

    // 3. Yêu cầu vẽ lại toàn bộ
    updateCalendarDisplay();
}

void MainWindow::changeBackgroundImage(int index, const QString &imagePath, const QColor &color)
{
    QString style = qApp->styleSheet();

    // Xóa cả hai thuộc tính cũ
    style.remove(QRegularExpression("QMainWindow \\{[^\\}]*background-image[^\\}]*\\}"));
    style.remove(QRegularExpression("QMainWindow \\{[^\\}]*background-color[^\\}]*\\}"));

    QString newRule;

    if (index == 14 && color.isValid()) // Màu đơn sắc
    {
        newRule = QString("QMainWindow { background-color: %1; }").arg(color.name());
    }
    else if (index == 13) // Ảnh tùy chỉnh
    {
        if (!imagePath.isEmpty()) {
            QString formattedPath = imagePath;
            formattedPath.replace("\\", "/");
            newRule = QString("QMainWindow { background-image: url('%1'); background-position: center; background-repeat: no-repeat; background-size: cover; }").arg(formattedPath);
        }
    }
    else // Các ảnh nền mặc định
    {
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
        default:
            newRule = "QMainWindow { background-image: url(:/resource/images/background.jpg); background-position: center; }";
            break;
        }
    }

    if (!newRule.isEmpty()) {
        style += "\n" + newRule;
    }
    qApp->setStyleSheet(style);
}

void MainWindow::setCalendarTransparency(bool transparent)
{
    // Tạo một danh sách tất cả các view cần thay đổi
    QList<QWidget*> views = {m_calendarView, m_monthView, m_timetableView, m_sessionView};

    for (QWidget *view : views) {
        // Đặt thuộc tính động [transparent="true"] hoặc [transparent="false"]
        view->setProperty("transparent", transparent);

        // Yêu cầu Qt làm mới lại style của widget
        style()->unpolish(view);
        style()->polish(view);
    }
}

// =================================================================================
// === 7. CORE LOGIC - EVENT MANAGEMENT (CRUD)
// =================================================================================

// --- 7A. Hàm tạo và xóa sự kiện cơ bản ---

void MainWindow::addEventFromDialog(EventDialog &dialog)
{
    QDateTime start = dialog.startDateTime();
    QDateTime end = dialog.endDateTime();
    EventDialog::RecurrenceRule rule = dialog.recurrenceRule();
    bool isAllDayEvent = dialog.isAllDay();

    if (rule.isRecurrent) {
        // Tạo chuỗi sự kiện lặp
        QDate currentDate = start.date();
        QTime startTime = start.time();
        long long durationSecs = start.secsTo(end);

        while (currentDate <= rule.endDate) {
            if (rule.days.contains(static_cast<Qt::DayOfWeek>(currentDate.dayOfWeek()))) {
                QDateTime newStart;
                QDateTime newEnd;
                if (isAllDayEvent) {
                    newStart = QDateTime(currentDate, QTime(0, 0, 0));
                    newEnd = QDateTime(currentDate, QTime(23, 59, 59));
                } else {
                    newStart = QDateTime(currentDate, startTime);
                    newEnd = newStart.addSecs(durationSecs);
                }
                EventItem *item = createEventItemFromDialog(dialog, newStart, newEnd);
                m_calendarView->addEvent(item);
                m_monthView->addEvent(item);
                m_timetableView->addEvent(item);
                m_sessionView->addEvent(item);
            }
            currentDate = currentDate.addDays(1);
        }
    }
    else {
        // Sự kiện đơn lẻ
        EventItem *item = createEventItemFromDialog(dialog, start, end);
        m_calendarView->addEvent(item);
        m_monthView->addEvent(item);
        m_timetableView->addEvent(item);
        m_sessionView->addEvent(item);
    }

    // Cập nhật lại view
    QDate eventDate = start.date();
    int daysUntilMonday = eventDate.dayOfWeek() - 1;
    QDate mondayOfEventWeek = eventDate.addDays(-daysUntilMonday);

    if (m_currentMonday != mondayOfEventWeek) {
        m_currentMonday = mondayOfEventWeek;
    }
    updateCalendarDisplay();
    saveData();
}

EventItem* MainWindow::createEventItemFromDialog(EventDialog &dialog, const QDateTime &start, const QDateTime &end)
{
    // 'start' và 'end' từ dialog là LocalTime. Chuyển sang UTC để lưu trữ.
    QDateTime utcStart = convertToStorageTime(start);
    QDateTime utcEnd = convertToStorageTime(end);

    EventItem* item = new EventItem(
        dialog.title(),
        dialog.eventColor(),
        utcStart, utcEnd,
        dialog.description(),
        dialog.showAsStatus(),
        dialog.category(),
        dialog.isAllDay(),
        dialog.recurrenceRule(),
        dialog.getEventType(),
        dialog.getExtraData()
        );

    m_allEventItems.append(item); // Thêm vào Nguồn sự thật duy nhất
    return item;
}

void MainWindow::removeEventFromViews(EventItem *item)
{
    // 1. Xóa khỏi danh sách chính
    m_allEventItems.removeAll(item);

    // 2. Yêu cầu tất cả các view xóa item này
    m_calendarView->removeEvent(item);
    m_monthView->removeEvent(item);
    m_timetableView->removeEvent(item);
    m_sessionView->removeEvent(item);
}

// --- 7B. Logic xử lý Sự kiện Lặp (Recurrence) ---

void MainWindow::updateSingleEvent(EventItem *oldItem, EventDialog &dialog)
{
    // 1. Xóa sự kiện cũ
    removeEventFromViews(oldItem);
    oldItem->deleteLater();

    // 2. Thêm sự kiện mới (đã được dialog "bẻ gãy" khỏi chuỗi)
    addEventFromDialog(dialog);

    // 3. Logic chuyển tuần (nếu cần)
    QDate eventStartDate = dialog.startDateTime().date();
    int daysToMonday = eventStartDate.dayOfWeek() - 1;
    QDate newMonday = eventStartDate.addDays(-daysToMonday);

    int daysInCurrentView = m_calendarView->getNumberOfDays();
    QDate currentViewEndDate = m_currentMonday.addDays(daysInCurrentView - 1);

    if (eventStartDate < m_currentMonday ||
        eventStartDate > currentViewEndDate ||
        m_viewStack->currentWidget() != m_calendarView)
    {
        m_currentMonday = newMonday;
    }

    // addEventFromDialog đã gọi saveData và updateCalendarDisplay
    // nhưng gọi lại updateCalendarDisplay để đảm bảo view được chuyển
    updateCalendarDisplay();
}

void MainWindow::updateEventSeries(EventItem *oldItem, EventDialog &dialog)
{
    // 1. Lấy quy tắc lặp CŨ (để tìm các sự kiện liên quan)
    EventDialog::RecurrenceRule oldRule = oldItem->recurrenceRule();

    // 2. Tạo danh sách các item cần xóa VÀ TÌM START DATE GỐC
    QList<EventItem*> itemsToDelete;
    QDate oldSeriesStartDate = QDate(9999, 1, 1); // Ngày max

    for (EventItem *item : m_allEventItems) {
        // So sánh 2 quy tắc
        if (item->recurrenceRule().isRecurrent &&
            item->recurrenceRule().endDate == oldRule.endDate &&
            item->recurrenceRule().days == oldRule.days &&
            item->title() == oldItem->title())
        {
            itemsToDelete.append(item);
            // Tìm ngày bắt đầu sớm nhất
            if (item->startTime().date() < oldSeriesStartDate) {
                oldSeriesStartDate = item->startTime().date();
            }
        }
    }
    if (itemsToDelete.isEmpty()) return;

    // 3. Tính toán ngày bắt đầu MỚI
    QDate oldItemDate = oldItem->startTime().date(); // Ngày của item BỊ KÉO (cũ)
    QDate newItemDate = dialog.startDateTime().date(); // Ngày của item BỊ KÉO (mới)
    qint64 daysFromStartToOldItem = oldSeriesStartDate.daysTo(oldItemDate);
    QDate newSeriesStartDate = newItemDate.addDays(-daysFromStartToOldItem);

    // 4. Xóa tất cả sự kiện trong chuỗi cũ
    for (EventItem *item : itemsToDelete) {
        removeEventFromViews(item);
        item->deleteLater();
    }

    // 5. Thêm lại chuỗi sự kiện MỚI, bắt đầu từ newSeriesStartDate
    recreateEventSeries(dialog, newSeriesStartDate);
}

void MainWindow::deleteSingleEvent(EventItem *item)
{
    removeEventFromViews(item);
    item->deleteLater();
    saveData();
    updateCalendarDisplay();
}

void MainWindow::deleteEventSeries(EventItem *item)
{
    EventDialog::RecurrenceRule rule = item->recurrenceRule();

    QList<EventItem*> itemsToDelete;
    for (EventItem *it : m_allEventItems) {
        if (it->recurrenceRule().isRecurrent &&
            it->recurrenceRule().endDate == rule.endDate &&
            it->recurrenceRule().days == rule.days &&
            it->title() == item->title())
        {
            itemsToDelete.append(it);
        }
    }

    // Xóa tất cả
    for (EventItem *it : itemsToDelete) {
        removeEventFromViews(it);
        it->deleteLater();
    }

    saveData();
    updateCalendarDisplay();
}

void MainWindow::recreateEventSeries(EventDialog &dialog, QDate seriesStartDate)
{
    // Lấy thông tin series MỚI từ dialog
    QTime startTime = dialog.startDateTime().time();
    long long durationSecs = dialog.startDateTime().secsTo(dialog.endDateTime());
    EventDialog::RecurrenceRule rule = dialog.recurrenceRule();
    bool isAllDayEvent = dialog.isAllDay();

    // Bắt đầu lặp từ seriesStartDate
    QDate currentDate = seriesStartDate;

    while (currentDate <= rule.endDate) {
        if (rule.days.contains(static_cast<Qt::DayOfWeek>(currentDate.dayOfWeek()))) {
            QDateTime newStart;
            QDateTime newEnd;
            if (isAllDayEvent) {
                newStart = QDateTime(currentDate, QTime(0, 0, 0));
                newEnd = QDateTime(currentDate, QTime(23, 59, 59));
            } else {
                newStart = QDateTime(currentDate, startTime);
                newEnd = newStart.addSecs(durationSecs);
            }
            // Tạo item
            EventItem *item = createEventItemFromDialog(dialog, newStart, newEnd);
            // Thêm vào các view
            m_calendarView->addEvent(item);
            m_monthView->addEvent(item);
            m_timetableView->addEvent(item);
            m_sessionView->addEvent(item);
        }
        currentDate = currentDate.addDays(1);
    }

    saveData();
    updateCalendarDisplay();
}

// --- 7C. Logic Sidebar (To-Do List) ---

void MainWindow::addTodoItem(const QString &text, bool completed)
{
    QListWidgetItem *item = new QListWidgetItem(m_todoList);
    QWidget *itemWidget = new QWidget;
    QHBoxLayout *itemLayout = new QHBoxLayout(itemWidget);
    itemLayout->setContentsMargins(8, 4, 4, 4);
    itemLayout->setSpacing(6);

    QCheckBox *check = new QCheckBox;
    {
        QSignalBlocker blocker(check);
        check->setChecked(completed); // Đặt trạng thái ban đầu
    }

    QLabel *todoLabel = new QLabel(text);
    todoLabel->setWordWrap(true);
    todoLabel->setMinimumWidth(0);
    todoLabel->setMaximumWidth(100);

    QPushButton *btnDel = new QPushButton("×");
    btnDel->setObjectName("btnDeleteTodo");
    btnDel->setCursor(Qt::PointingHandCursor);
    btnDel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    itemLayout->addWidget(check);
    itemLayout->addWidget(todoLabel, 1);
    itemLayout->addWidget(btnDel);
    itemWidget->setLayout(itemLayout);

    item->setSizeHint(itemWidget->sizeHint());
    m_todoList->addItem(item);
    m_todoList->setItemWidget(item, itemWidget);

    // Cập nhật style ban đầu
    if (completed) {
        todoLabel->setStyleSheet("color: #999; text-decoration: line-through;");
        itemWidget->setStyleSheet("background-color: #f0f0f0;");
    }

    // Kết nối với slot của MainWindow
    connect(check, &QCheckBox::checkStateChanged, this, &MainWindow::onTodoItemChanged);
    connect(btnDel, &QPushButton::clicked, this, &MainWindow::onTodoItemDeleted);

    // (Lưu con trỏ item vào checkbox và nút xóa để tìm lại)
    check->setProperty("listItem", QVariant::fromValue(item));
    btnDel->setProperty("listItem", QVariant::fromValue(item));
}

// =================================================================================
// === 8. DATA PERSISTENCE - SAVE/LOAD
// =================================================================================

// --- 8A. Khởi tạo và Lưu/Tải chính ---

void MainWindow::initSavePath()
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir dir(dataPath);
    if (!dir.exists()) {
        dir.mkpath("."); // Tạo thư mục nếu chưa có
    }
    m_saveFilePath = dataPath + "/data.json";
    m_settingsFilePath = dataPath + "/settings.json";
}

void MainWindow::saveData()
{
    QJsonObject rootObject;

    // 1. Lưu tất cả sự kiện
    QJsonArray eventsArray;
    for (EventItem *item : m_allEventItems) {
        eventsArray.append(serializeEvent(item));
    }
    rootObject["events"] = eventsArray;

    // 2. Lưu tất cả To-Do
    QJsonArray todosArray;
    for (int i = 0; i < m_todoList->count(); ++i) {
        QListWidgetItem *item = m_todoList->item(i);
        QWidget *widget = m_todoList->itemWidget(item);
        if (!widget) continue;

        QCheckBox *check = widget->findChild<QCheckBox*>();
        QLabel *label = widget->findChild<QLabel*>();

        if (check && label) {
            QJsonObject todoObject;
            todoObject["text"] = label->text();
            todoObject["completed"] = check->isChecked();
            todosArray.append(todoObject);
        }
    }
    rootObject["todos"] = todosArray;

    // 3. Ghi file
    QJsonDocument saveDoc(rootObject);
    QSaveFile saveFile(m_saveFilePath);
    if (saveFile.open(QIODevice::WriteOnly)) {
        saveFile.write(saveDoc.toJson());
        saveFile.commit();
    } else {
        qWarning() << "Couldn't open save file:" << saveFile.errorString();
    }
}

void MainWindow::loadData()
{
    QFile loadFile(m_saveFilePath);
    if (!loadFile.exists() || !loadFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Couldn't open data file or file doesn't exist.";
        return;
    }

    QByteArray saveData = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    if (loadDoc.isNull() || !loadDoc.isObject()) {
        qWarning() << "Failed to parse JSON data.";
        return;
    }

    QJsonObject rootObject = loadDoc.object();

    // 1. Tải sự kiện
    if (rootObject.contains("events") && rootObject["events"].isArray()) {
        loadEvents(rootObject["events"].toArray());
    }

    // 2. Tải To-Do
    if (rootObject.contains("todos") && rootObject["todos"].isArray()) {
        loadTodos(rootObject["todos"].toArray());
    }

    updateCalendarDisplay(); // Cập nhật lại UI sau khi tải
}

// --- 8B. Hàm trợ giúp Serialize/Deserialize ---

QJsonObject MainWindow::serializeEvent(EventItem *item) const
{
    QJsonObject eventObject;
    eventObject["title"] = item->title();
    eventObject["color"] = item->color().name();
    eventObject["start"] = item->startTime().toString(Qt::ISODate); // Lưu UTC time dạng ISO
    eventObject["end"] = item->endTime().toString(Qt::ISODate);
    eventObject["description"] = item->description();
    eventObject["showAs"] = item->showAsStatus();
    eventObject["category"] = item->category();
    eventObject["isAllDay"] = item->isAllDay();

    // Lưu quy tắc lặp
    QJsonObject recurrenceObject;
    EventDialog::RecurrenceRule rule = item->recurrenceRule();
    recurrenceObject["isRecurrent"] = rule.isRecurrent;
    recurrenceObject["endDate"] = rule.endDate.toString(Qt::ISODate);
    QJsonArray daysArray;
    for (Qt::DayOfWeek day : rule.days) {
        daysArray.append(static_cast<int>(day));
    }
    recurrenceObject["days"] = daysArray;

    eventObject["recurrence"] = recurrenceObject;
    eventObject["eventType"] = item->eventType();
    eventObject["extraData"] = item->extraData(); // Lưu toàn bộ dữ liệu phụ
    return eventObject;
}

void MainWindow::loadEvents(const QJsonArray &eventsArray)
{
    qDeleteAll(m_allEventItems); // Xóa các item cũ
    m_allEventItems.clear();

    for (int i = 0; i < eventsArray.size(); ++i) {
        QJsonObject eventObject = eventsArray[i].toObject();

        QString title = eventObject["title"].toString();
        QColor color(eventObject["color"].toString());
        QDateTime utcStart = QDateTime::fromString(eventObject["start"].toString(), Qt::ISODate);
        QDateTime utcEnd = QDateTime::fromString(eventObject["end"].toString(), Qt::ISODate);
        QString desc = eventObject["description"].toString();
        QString showAs = eventObject["showAs"].toString();
        QString category = eventObject["category"].toString();
        bool isAllDay = eventObject["isAllDay"].toBool();

        // Đọc quy tắc lặp
        EventDialog::RecurrenceRule rule;
        QJsonObject recurrenceObject = eventObject["recurrence"].toObject();
        rule.isRecurrent = recurrenceObject["isRecurrent"].toBool();
        rule.endDate = QDate::fromString(recurrenceObject["endDate"].toString(), Qt::ISODate);
        QJsonArray daysArray = recurrenceObject["days"].toArray();
        for (int j = 0; j < daysArray.size(); ++j) {
            rule.days.append(static_cast<Qt::DayOfWeek>(daysArray[j].toInt()));
        }

        // Đọc loại sự kiện và dữ liệu phụ
        QString eventType = eventObject.value("eventType").toString("Sự kiện"); // Mặc định là "Sự kiện"
        QJsonObject extraData = eventObject.value("extraData").toObject();

        // Logic tương thích ngược (nếu file JSON quá cũ)
        if (!eventObject.contains("eventType") && eventObject.contains("isMeeting")) {
            bool isMeeting_compat = eventObject["isMeeting"].toBool(false);
            if (isMeeting_compat) {
                eventType = "Cuộc họp";
                extraData["host"] = eventObject["host"].toString();
                extraData["participants"] = eventObject["participants"].toString();
                extraData["meetingStatus"] = eventObject["meetingStatus"].toString("Dự kiến");
            }
        }

        // Tạo EventItem
        EventItem *item = new EventItem(title, color, utcStart, utcEnd,
                                        desc, showAs, category, isAllDay, rule,
                                        eventType, extraData
                                        );

        // Thêm vào danh sách chính và các view
        m_allEventItems.append(item);
        m_calendarView->addEvent(item);
        m_monthView->addEvent(item);
        m_timetableView->addEvent(item);
        m_sessionView->addEvent(item);
    }
}

void MainWindow::loadTodos(const QJsonArray &todosArray)
{
    m_todoList->clear(); // Xóa các item cũ

    for (int i = 0; i < todosArray.size(); ++i) {
        QJsonObject todoObject = todosArray[i].toObject();
        QString text = todoObject["text"].toString();
        bool completed = todoObject["completed"].toBool();
        addTodoItem(text, completed); // Gọi hàm trợ giúp
    }
}

// --- 8C. Lưu/Tải Cài đặt (Settings) ---

void MainWindow::saveSettings(SettingsDialog *dialog)
{
    if (!dialog) return;

    QJsonObject settingsObject;

    // 1. Lưu cài đặt thời gian (từ biến thành viên)
    settingsObject["use24HourFormat"] = m_use24HourFormat;
    settingsObject["timezoneOffsetSeconds"] = m_timezoneOffsetSeconds;

    // 2. Lưu cài đặt nền (lấy trực tiếp từ dialog)
    settingsObject["backgroundIndex"] = dialog->selectedBackgroundIndex();
    settingsObject["imagePath"] = dialog->selectedImagePath();
    settingsObject["solidColor"] = dialog->selectedSolidColor().name();
    settingsObject["isTransparent"] = dialog->isCalendarTransparent();

    // 3. Ghi file (settings.json)
    QJsonDocument saveDoc(settingsObject);
    QSaveFile saveFile(m_settingsFilePath);
    if (saveFile.open(QIODevice::WriteOnly)) {
        saveFile.write(saveDoc.toJson());
        saveFile.commit();
    } else {
        qWarning() << "Couldn't open settings file for writing:" << saveFile.errorString();
    }
}

void MainWindow::loadSettings()
{
    QFile loadFile(m_settingsFilePath);
    if (!loadFile.exists() || !loadFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Couldn't open settings file, using defaults.";
        // Nếu file không tồn tại, các giá trị mặc định trong constructor sẽ được dùng
        applyTimeSettings();
        changeBackgroundImage(m_currentBackgroundIndex, m_currentImagePath, m_currentSolidColor);
        setCalendarTransparency(m_isCalendarTransparent);
        return;
    }

    QByteArray settingsData = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(settingsData));
    if (loadDoc.isNull() || !loadDoc.isObject()) {
        qWarning() << "Failed to parse settings.json, using defaults.";
        return;
    }

    QJsonObject settingsObject = loadDoc.object();

    // 1. Tải và áp dụng cài đặt giờ
    m_use24HourFormat = settingsObject.value("use24HourFormat").toBool(m_use24HourFormat);
    int defaultOffset = QDateTime::currentDateTime().offsetFromUtc();
    m_timezoneOffsetSeconds = settingsObject.value("timezoneOffsetSeconds").toInt(defaultOffset);
    // (applyTimeSettings() sẽ được gọi ở cuối constructor)

    // 2. Tải và áp dụng cài đặt nền
    int bgIndex = settingsObject.value("backgroundIndex").toInt(m_currentBackgroundIndex);
    QString imgPath = settingsObject.value("imagePath").toString(m_currentImagePath);
    QColor color(settingsObject.value("solidColor").toString());
    bool isTransparent = settingsObject.value("isTransparent").toBool(m_isCalendarTransparent);

    // Cập nhật biến thành viên để constructor gọi
    m_currentBackgroundIndex = bgIndex;
    m_currentImagePath = imgPath;
    m_currentSolidColor = (color.isValid() ? color : m_currentSolidColor);
    m_isCalendarTransparent = isTransparent;

    // Gọi trực tiếp (vì loadData đã chạy xong)
    changeBackgroundImage(m_currentBackgroundIndex, m_currentImagePath, m_currentSolidColor);
    setCalendarTransparency(m_isCalendarTransparent);
}
