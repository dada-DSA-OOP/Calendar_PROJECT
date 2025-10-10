#include "mainwindow.h"
#include "ui_mainwindow.h"

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

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    bool compact = width() < 900; //Nếu cửa sổ < 900px thì ẩn chữ

    for (auto button : this->findChildren<QToolButton*>()) {
        button->setToolButtonStyle(compact ?
        Qt::ToolButtonIconOnly :
        Qt::ToolButtonTextBesideIcon);
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

    // Hàm tiện ích: tạo item tick kèm submenu (sử dụng QWidgetAction)
    auto makeCheckableWithSubmenu = [&](const QString &text, QMenu *submenu) {
        QWidget *w = new QWidget;
        QHBoxLayout *layout = new QHBoxLayout(w);
        layout->setContentsMargins(10, 2, 6, 2);

        QCheckBox *chk = new QCheckBox(text);
        chk->setChecked(true);
        QPushButton *btn = new QPushButton("▼");
        btn->setFlat(true);
        btn->setFixedSize(20, 20);
        btn->setFocusPolicy(Qt::NoFocus);

        layout->addWidget(chk);
        layout->addStretch();
        layout->addWidget(btn);

        QWidgetAction *act = new QWidgetAction(filterMenu);
        act->setDefaultWidget(w);
        filterMenu->addAction(act);

        QObject::connect(btn, &QPushButton::clicked, [submenu, btn]() {
            submenu->exec(btn->mapToGlobal(QPoint(0, btn->height())));
        });
        return chk;
    };

    QAction *actAppointment = filterMenu->addAction("Cuộc hẹn");
    actAppointment->setCheckable(true);
    actAppointment->setChecked(true);


    // === Các mục có submenu === //
    QMenu *menuMeetings = new QMenu("Cuộc họp", filterMenu);
    addShadowEffect(menuMeetings);
    QAction *actClearAll = menuMeetings->addAction("Bỏ chọn tất cả");
    QObject::connect(actClearAll, &QAction::triggered, [menuMeetings]() {
        for (QAction *a : menuMeetings->actions())
            if (a->isCheckable()) a->setChecked(false);
    });
    menuMeetings->addSeparator();
    QAction *header1 = menuMeetings->addAction("Tôi là người tổ chức");
    header1->setEnabled(false);
    header1->setFont(QFont("Segoe UI", 9, QFont::Bold));
    for (const QString &t : QStringList{"Đã gửi", "Bản thảo"}) {
        QAction *a = menuMeetings->addAction("  " + t);
        a->setCheckable(true);
        a->setChecked(true);
    }
    menuMeetings->addSeparator();
    QAction *header2 = menuMeetings->addAction("Tôi là người dự");
    header2->setEnabled(false);
    header2->setFont(QFont("Segoe UI", 9, QFont::Bold));
    for (const QString &t : QStringList{"Đã chấp nhận", "Đã từ chối", "Dự định", "Đã hủy bỏ", "Chưa trả lời"}) {
        QAction *a = menuMeetings->addAction("  " + t);
        a->setCheckable(true);
        a->setChecked(true);
    }

    QMenu *menuCategory = new QMenu("Thể loại", filterMenu);
    addShadowEffect(menuCategory);

    // Nút "Bỏ chọn tất cả"
    QAction *actUncheckCategory = menuCategory->addAction("Bỏ chọn tất cả");
    QObject::connect(actUncheckCategory, &QAction::triggered, [menuCategory]() {
        for (QAction *a : menuCategory->actions())
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
    QObject::connect(actUncheckDisplayAs, &QAction::triggered, [menuDisplayAs]() {
        for (QAction *a : menuDisplayAs->actions())
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
    makeCheckableWithSubmenu("Cuộc họp", menuMeetings);
    makeCheckableWithSubmenu("Thể loại", menuCategory);
    makeCheckableWithSubmenu("Hiển thị như", menuDisplayAs);
    makeCheckableWithSubmenu("Lặp lại", menuRepeat);
    makeCheckableWithSubmenu("Trực tiếp", menuDirect);

    btnFilter->setMenu(filterMenu);
    homeLayout->addWidget(btnFilter);


    homeLayout->addWidget(btnFilter);

    //Gạch dọc chia
    homeLayout->addWidget(makeSeparator());

    homeLayout->addWidget(makeBtn("In", "resource/icons/printer.png"));
    homeLayout->addStretch();

    toolbarStack->addWidget(homePage);

    // --- View toolbar ---
    QWidget *viewPage = new QWidget;
    QHBoxLayout *viewLayout = new QHBoxLayout(viewPage);
    viewLayout->setContentsMargins(10,6,10,6);
    viewLayout->addWidget(new QLabel("View tools here"));
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
    QWidget *mainContent = new QWidget;
    mainContent->setStyleSheet("background: white;");

    // ===== Tổng layout =====
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(topBar, 0, Qt::AlignTop);
    mainLayout->addWidget(mainContent, 1);
    setCentralWidget(central);

    // ===== Kết nối =====
    connect(tabBar, &QTabBar::currentChanged, toolbarStack, &QStackedWidget::setCurrentIndex);

    tabBar->setCurrentIndex(0);
    toolbarStack->setCurrentIndex(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}
