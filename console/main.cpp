#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <windows.h>

using namespace std;

class Reminder;
class RecurrenceRule;
class Event;
class Meeting;
class Appointment;
class Holiday;
class Study;

//Hàm hỗ trợ về thời gian
chrono::system_clock::time_point parseDateTime(const string& dateTimeStr) {
    tm t = {}; 
    sscanf(dateTimeStr.c_str(), "%d-%d-%d %d:%d",
        &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min);
    t.tm_year -= 1900;
    t.tm_mon -= 1;
    t.tm_sec = 0;

    time_t tt = mktime(&t);
    return chrono::system_clock::from_time_t(tt);
}

void printDateTime(const chrono::system_clock::time_point& tp) {
    time_t t_c = chrono::system_clock::to_time_t(tp);
    cout << put_time(localtime(&t_c), "%Y-%m-%d %H:%M");
}

//Lớp Reminder
class Reminder{
    public:
        enum ReminderType {Popup, Email, SMS};
    private:
        chrono::system_clock::time_point triggerTime;
        ReminderType type;
    public:
        Reminder();
        Reminder(chrono::system_clock::time_point triggerTime, ReminderType type);
        void inPut();
        void outPut();
//save & load
        void save(ofstream& ofs) const;
        void load(ifstream& ifs);
};

void Reminder::save(ofstream& ofs) const {
    time_t t = chrono::system_clock::to_time_t(triggerTime);
    ofs << t << endl; 
    ofs << static_cast<int>(type) << endl;
}

void Reminder::load(ifstream& ifs) {
    time_t t;
    int noti;
    
    ifs >> t; ifs.ignore();
    triggerTime = chrono::system_clock::from_time_t(t);
    
    ifs >> noti; ifs.ignore();
    type = static_cast<ReminderType>(noti);
}
//-----------------------------

Reminder::Reminder(){
    triggerTime=chrono::system_clock::now();
    type = Popup;
}

Reminder::Reminder(chrono::system_clock::time_point triggerTime, ReminderType type){
    this->triggerTime=triggerTime;
    this->type=type;
}

void Reminder::inPut(){
    cout << "===== Nhập thông báo =====\n";
    cout << "Nhập thời gian thông báo (YYYY-MM-DD HH:MM): ";
    string date; getline(cin, date);
    triggerTime = parseDateTime(date);

    cout << "Nhập loại thông báo (0: Popup, 1: Email, 2: SMS): ";
    int noti; cin >> noti; cin.ignore();
    type = static_cast<ReminderType>(noti);
}

void Reminder::outPut(){
    cout << "==========Thông Báo==========" << endl;

    cout << "Thời gian thông báo: "; printDateTime(triggerTime); cout << endl;

    cout << "Cách thức thông báo: ";
    switch(type){
        case Popup: cout << "PopUp"; break;
        case Email: cout << "Email"; break;
        case SMS: cout << "SMS"; break;
    }

    cout << endl;
}

//Lớp RecurrenceRule
class RecurrenceRule {
    public:
        enum Frequency { None, Daily, Weekly, Monthly, Yearly };
    private:
        Frequency frequency;                          
        chrono::system_clock::time_point dateRepeat;   
    public:
        RecurrenceRule();
        RecurrenceRule(Frequency frequency, chrono::system_clock::time_point dateRepeat);
        void inPut();   
        void outPut() const;
//save & load
        void save(ofstream& ofs) const;
        void load(ifstream& ifs);
};

void RecurrenceRule::save(ofstream& ofs) const {
    time_t t = chrono::system_clock::to_time_t(dateRepeat);
    ofs << t << endl;
    ofs << static_cast<int>(frequency) << endl;
}

void RecurrenceRule::load(ifstream& ifs) {
    time_t t;
    int freq;

    ifs >> t; ifs.ignore();
    dateRepeat = chrono::system_clock::from_time_t(t);
    
    ifs >> freq; ifs.ignore();
    frequency = static_cast<Frequency>(freq);
}
//---------------------------
RecurrenceRule::RecurrenceRule() {
    frequency = None;
    dateRepeat = chrono::system_clock::now();
}

RecurrenceRule::RecurrenceRule(Frequency frequency, chrono::system_clock::time_point dateRepeat) {
    this->frequency = frequency;
    this->dateRepeat = dateRepeat;
}

void RecurrenceRule::inPut() {
    cout << "===== Nhập quy tắc lặp lại =====" << endl;

    cout << "Nhập thời điểm lặp (YYYY-MM-DD HH:MM): ";
    string dateStr;
    getline(cin, dateStr);
    dateRepeat = parseDateTime(dateStr);

    cout << "Chọn kiểu lặp (0: Không lặp lại, 1: Mỗi ngày, 2: Mỗi tuần, 3: Mỗi tháng, 4: Mỗi năm): ";
    int type;
    cin >> type;
    cin.ignore();
    frequency = static_cast<Frequency>(type);
}

void RecurrenceRule::outPut() const {
    time_t t_c = chrono::system_clock::to_time_t(dateRepeat);
    tm local_tm = *localtime(&t_c);

    char buffer[50];

    switch (frequency) {
        case Daily:
            sprintf(buffer, "%02d:%02d mỗi ngày", local_tm.tm_hour, local_tm.tm_min);
            cout << buffer;
            break;

        case Weekly: {
            const char* weekdays[] = {"Chủ nhật", "Thứ 2", "Thứ 3", "Thứ 4", "Thứ 5", "Thứ 6", "Thứ 7"};
            sprintf(buffer, "%s mỗi tuần", weekdays[local_tm.tm_wday]);
            cout << buffer;
            break;
        }

        case Monthly:
            sprintf(buffer, "Ngày %d mỗi tháng", local_tm.tm_mday);
            cout << buffer;
            break;

        case Yearly:
            sprintf(buffer, "%02d/%02d mỗi năm", local_tm.tm_mday, local_tm.tm_mon + 1);
            cout << buffer;
            break;

        case None:
            cout << "Không lặp lại";
            break;
        default:
            cout << "Không lặp lại";
            break;
    }

    cout << endl;
}

//Lớp event
class Event{
    private:
        int id;
        string title;
        string description;
        chrono::system_clock::time_point startTime;
        chrono::system_clock::time_point endTime;
        Reminder* reminder;
        RecurrenceRule* recurrence;
    public:
        Event();
        Event(int id, string title, string description, 
            chrono::system_clock::time_point startTime, 
            chrono::system_clock::time_point endTime);
        virtual ~Event();
        virtual void inPut();
        virtual void outPut();
        int getId();
        chrono::system_clock::time_point getDate();
        void setReminder(Reminder* r);
        void setRecurrence(RecurrenceRule* rr);
        Reminder* getReminder();
        RecurrenceRule* getRecurrence();
//save & load
        virtual void save(ofstream& ofs) const;
        virtual void load(ifstream& ifs);
};

void Event::save(ofstream& ofs) const {
    ofs << id << endl;
    ofs << title << endl;
    ofs << description << endl;
    
    // Lưu startTime và endTime dưới dạng UNIX timestamp
    ofs << chrono::system_clock::to_time_t(startTime) << endl;
    ofs << chrono::system_clock::to_time_t(endTime) << endl;
    
    // Xử lý Reminder
    ofs << (reminder ? 1 : 0) << endl; 
    if (reminder) {
        reminder->save(ofs);
    }

    // Xử lý RecurrenceRule
    ofs << (recurrence ? 1 : 0) << endl; 
    if (recurrence) {
        recurrence->save(ofs);
    }
}

void Event::load(ifstream& ifs) {
    time_t t_start, t_end;
    
    ifs >> id; ifs.ignore();
    getline(ifs, title);
    getline(ifs, description);
    
    // Đọc timestamp và chuyển về time_point
    ifs >> t_start; ifs.ignore();
    startTime = chrono::system_clock::from_time_t(t_start);
    ifs >> t_end; ifs.ignore();
    endTime = chrono::system_clock::from_time_t(t_end);
    
    // Đọc Reminder
    int has_reminder;
    ifs >> has_reminder; ifs.ignore();
    if (has_reminder == 1) {
        reminder = new Reminder();
        reminder->load(ifs);
    }

    // Đọc RecurrenceRule
    int has_recurrence;
    ifs >> has_recurrence; ifs.ignore();
    if (has_recurrence == 1) {
        recurrence = new RecurrenceRule();
        recurrence->load(ifs);
    }
}
//-----------------------

int Event::getId(){
    return id;
}

chrono::system_clock::time_point Event::getDate(){
    return startTime;
}

Event::Event(){
    id=0;
    title=" ";
    description=" ";
    startTime=chrono::system_clock::now();
    endTime=chrono::system_clock::now();
    reminder = nullptr;
    recurrence = nullptr;
}

Event::Event(int id, string title, string description, 
            chrono::system_clock::time_point startTime, 
            chrono::system_clock::time_point endTime){
    this->id=id;
    this->title=title;
    this->description=description;
    this->startTime=startTime;
    this->endTime=endTime;
    reminder = nullptr;
    recurrence = nullptr;
}


Event::~Event() {
    if (reminder) {
        delete reminder;
        reminder = nullptr;
    }
    if (recurrence) {
        delete recurrence;
        recurrence = nullptr;
    }
}

void Event::setReminder(Reminder* r) {
    if (reminder) delete reminder; 
    reminder = r;
}

void Event::setRecurrence(RecurrenceRule* rr) {
    if (recurrence) delete recurrence;
    recurrence = rr;
}

Reminder* Event::getReminder(){
    return reminder;
}

RecurrenceRule* Event::getRecurrence(){
    return recurrence;
}

void Event::inPut(){
    cout << "==========Nhập sự kiện==========" << endl;

    cout << "Nhập id: "; cin >> id; cin.ignore();

    cout << "Nhập tiêu đề: "; getline(cin, title); 

    cout << "Nhập mô tả: "; getline(cin, description);

    cout << "Nhập thời gian bắt đầu (YYYY-MM-DD HH:MM): "; 
    string start; getline(cin, start);
    startTime = parseDateTime(start);

    cout << "Nhập thời gian kết thúc (YYYY-MM-DD HH:MM): "; 
    string end; getline(cin, end);
    endTime = parseDateTime(end);

    cout << "Bạn có muốn thêm thông báo không? (1 = Có, 0 = Không): ";
    int addRem; cin >> addRem; cin.ignore();
    if (addRem == 1) {
        reminder = new Reminder();
        reminder->inPut();
    }

    cout << "Bạn có muốn thêm quy tắc lặp không? (1 = Có, 0 = Không): ";
    int addRec; cin >> addRec; cin.ignore();
    if (addRec == 1) {
        recurrence = new RecurrenceRule();
        recurrence->inPut();
    }
}

void Event::outPut(){
    cout << "\n==========Sự kiện==========" << endl;
    cout << "Id: " << id << endl;
    cout << "TIêu đề: " << title << endl;
    cout << "Mô tả: " << description << endl;
    cout << "Thời gia bắt đầu: "; printDateTime(startTime); cout << endl;
    cout << "Thời gian kết thúc: "; printDateTime(endTime); cout << endl;
    if(reminder){ 
        reminder->outPut();
    }
    if(recurrence){ 
        cout << "Quy tắc lặp: "; recurrence->outPut();
    }
}

//Lớp Meeting (kế thừa lớp event)
class Meeting : public Event{
    public:
        enum MeetingStatus { Tentative, Confirmed, Cancelled };
    private:
        string organizer;
        vector<string> attendees;
        MeetingStatus status;
    public:
        Meeting();
        Meeting(int id, string title, string description, 
            chrono::system_clock::time_point startTime, 
            chrono::system_clock::time_point endTime,
            string organizer, vector<string> attendees, MeetingStatus status);
        void inPut() override;
        void outPut() override;
        void addAttendee(string person);
        void removeAttendee(string person);
//save & load
        void save(ofstream& ofs) const override;
        void load(ifstream& ifs) override;
};

void Meeting::save(ofstream& ofs) const {
    Event::save(ofs); // Lưu các trường của lớp cha
    ofs << organizer << endl;
    ofs << static_cast<int>(status) << endl;
    
    // Lưu danh sách người tham dự (số lượng và từng người)
    ofs << attendees.size() << endl;
    for (const auto& person : attendees) {
        ofs << person << endl;
    }
}

void Meeting::load(ifstream& ifs) {
    Event::load(ifs); // Đọc các trường của lớp cha
    getline(ifs, organizer);
    
    int status_meet;
    ifs >> status_meet; ifs.ignore();
    status = static_cast<MeetingStatus>(status_meet);

    // Đọc danh sách người tham dự
    size_t count;
    ifs >> count; ifs.ignore();
    attendees.clear();
    for (size_t i = 0; i < count; ++i) {
        string person;
        getline(ifs, person);
        attendees.push_back(person);
    }
}
//---------------------------------------

Meeting::Meeting() : Event(){
    organizer = " ";
    attendees.clear();
    status = Tentative;
}

Meeting::Meeting(int id, string title, string description, 
            chrono::system_clock::time_point startTime, 
            chrono::system_clock::time_point endTime,
            string organizer, vector<string> attendees, MeetingStatus status) : Event(id, title, description, startTime, endTime){
    this->organizer=organizer;
    this->attendees=attendees;
    this->status=status;            
}

void Meeting::inPut() {
    Event::inPut();
    cout << "==========Nhập thông tin cuộc họp==========" << endl;

    cout << "Chủ cuộc họp: ";
    getline(cin,organizer);

    int status_meet;
    cout << "Trạng thái cuộc họp (0: Dự kiến, 1: Đã xác nhận, 2: Đã hủy): ";
    cin >> status_meet;
    cin.ignore();
    status=static_cast<MeetingStatus>(status_meet);

    string attendee;
    cout << "*Nhập người tham dự*" << endl;
    while(true){
        cout << "Người tham dự (q để kết thúc): ";
        getline(cin,attendee);
        if (attendee == "q") {
            break;
        }
        attendees.push_back(attendee);
    }

    cout << endl;
}

void Meeting::outPut(){
    Event::outPut();

    cout << "==========Thông tin cuộc họp==========" << endl;

    cout << "Người tổ chức: " << organizer << endl;

    cout << "Người tham dự: ";
    if(attendees.empty()){
        cout << "Không có ai tham gia." << endl;
    }
    else{
        for(const auto& person : attendees){
            cout << person << " ";
        }
        cout << endl;
    }

    cout << "Trạng thái: ";
    switch(status){
        case Tentative: cout << "Dự kiến"; break;
        case Confirmed: cout << "Đã xác thực"; break;
        case Cancelled: cout << "Đã hủy"; break;
    }

    cout << endl;
}

void Meeting::addAttendee(string person){
    attendees.push_back(person);
}

void Meeting::removeAttendee(string person){
    auto tmp = remove(attendees.begin(), attendees.end(), person);
    if(tmp != attendees.end()){
        attendees.erase(tmp,attendees.end());
        cout << "Đã xóa " << person << endl;
    }
    else{
        cout << "Không tìm thấy " << person << endl;
    }
}

//Lớp Appointment (kế thừa lớp event)
class Appointment : public Event {
    private:
        string location;
        bool isPrivate;
    public:
        Appointment();
        Appointment(int id, string title, string description, 
            chrono::system_clock::time_point startTime, 
            chrono::system_clock::time_point endTime,
            string location, bool isPrivate);
        void inPut() override;
        void outPut() override;
//save & load
        void save(ofstream& ofs) const override;
        void load(ifstream& ifs) override;
};

void Appointment::save(ofstream& ofs) const {
    Event::save(ofs); 
    ofs << location << endl;
    ofs << (isPrivate ? 1 : 0) << endl;
}

void Appointment::load(ifstream& ifs) {
    Event::load(ifs);
    getline(ifs, location);
    
    int is_private_int;
    ifs >> is_private_int; ifs.ignore();
    isPrivate = (is_private_int == 1);
}
//---------------------------------

Appointment::Appointment() : Event(){
    location = " ";
    isPrivate = false;
}

Appointment::Appointment(int id, string title, string description, 
            chrono::system_clock::time_point startTime, 
            chrono::system_clock::time_point endTime,
            string location, bool isPrivate) : Event(id, title, description, startTime, endTime){
    this->location=location;
    this->isPrivate=isPrivate;            
}

void Appointment::inPut(){
    Event::inPut();

    cout << "=======Nhập thông tin cuộc hẹn=======" << endl;
    cout << "Địa điểm: "; getline(cin,location);
    int x;
    cout << "Riêng tư (1 = Có, 0 = Không): "; cin >> x; cin.ignore();
    isPrivate = (x==1);
}

void Appointment::outPut(){
    Event::outPut();

    cout << "=======Thông tin cuộc hẹn=======" << endl;
    cout << "Địa điểm: " << location << endl;
    cout << "Tính riêng tư: ";
    if(isPrivate){
        cout << "Có" << endl;
    }
    else{
        cout << "Không" << endl;
    }
}

//Lớp Holiday (kế thừa lớp Event)
class Holiday : public Event {
    public:
        enum HolidayType {International, National, Religious, Custom};
    private:
        chrono::system_clock::time_point date;
        HolidayType type;
    public:
        Holiday();
        Holiday(int id, string title, string description, 
            chrono::system_clock::time_point startTime, 
            chrono::system_clock::time_point endTime,
            chrono::system_clock::time_point date, HolidayType type);
        void inPut() override;
        void outPut() override;
//save & load
        void save(ofstream& ofs) const override;
        void load(ifstream& ifs) override;
};

void Holiday::save(ofstream& ofs) const {
    Event::save(ofs); 
    ofs << chrono::system_clock::to_time_t(date) << endl;
    ofs << static_cast<int>(type) << endl;
}

void Holiday::load(ifstream& ifs) {
    Event::load(ifs);
    
    time_t t_date;
    ifs >> t_date; ifs.ignore();
    date = chrono::system_clock::from_time_t(t_date);

    int type_int;
    ifs >> type_int; ifs.ignore();
    type = static_cast<HolidayType>(type_int);
}
//----------------------------

Holiday::Holiday() : Event(){
    this->date=chrono::system_clock::now();
    this->type = Custom;
}

Holiday::Holiday(int id, string title, string description, 
            chrono::system_clock::time_point startTime, 
            chrono::system_clock::time_point endTime, 
            chrono::system_clock::time_point date, HolidayType type) : Event(id, title, description, startTime, endTime){
    this->date=date;
    this->type=type;            
}

void Holiday::inPut(){
    Event::inPut();

    cout << "=====Nhập thông tin ngày lễ=====" << endl;

    cout << "Nhập ngày lễ (YYYY-MM-DD HH:MM): ";
    string dateTMP; getline(cin, dateTMP);
    this->date = parseDateTime(dateTMP);
    
    int type_date;
    cout << "Loại ngày lễ: (0: Quốc tế, 1: Quốc gia, 2: Tôn giáo, 3: Cá nhân): ";
    cin >> type_date;
    cin.ignore();
    type=static_cast<HolidayType>(type_date);
}

void Holiday::outPut(){
    Event::outPut();

    cout << "==========Ngày lễ==========" << endl;

    cout << "Ngày diễn ra: "; printDateTime(date); cout << endl;

    cout << "Loại ngày lễ: "; 
    switch(type){
        case International: cout << "Quốc tế"; break;
        case National: cout << "Quốc gia"; break;
        case Religious: cout << "Tôn giáo"; break;
        case Custom: cout << "Cá nhân"; break;
    }
    
    cout << endl;
}


//Lớp Study (kế thừa lớp Event)
class Study : public Event {
    public:
        enum Materials {OnCampus, Online, ExtraClass, SelfStudy};
    private:
        string subject;
        Materials StudyType;
    public:
        Study();
        Study(int id, string title, string description, 
            chrono::system_clock::time_point startTime, 
            chrono::system_clock::time_point endTime,
            string subject, Materials StudyType);
        void inPut() override;
        void outPut() override;
//save & load
        void save(ofstream& ofs) const override;
        void load(ifstream& ifs) override;
};

void Study::save(ofstream& ofs) const {
    Event::save(ofs); 
    ofs << subject << endl;
    ofs << static_cast<int>(StudyType) << endl;
}

void Study::load(ifstream& ifs) {
    Event::load(ifs);
    getline(ifs, subject);
    
    int type_int;
    ifs >> type_int; ifs.ignore();
    StudyType = static_cast<Materials>(type_int);
}
//-------------------------------

Study::Study() : Event(){
    subject = " ";
    StudyType = OnCampus;
}

Study::Study(int id, string title, string description, 
            chrono::system_clock::time_point startTime, 
            chrono::system_clock::time_point endTime, 
            string subject, Materials StudyType) : Event(id, title, description, startTime, endTime ){
    this->subject=subject;
    this->StudyType=StudyType;
}

void Study::inPut(){
    Event::inPut();

    cout << "=====Nhập thông sự kiện học tập=====" << endl;

    cout << "Môn học: "; getline(cin,subject);

    cout << "Hình thức học (0: Tại trường, 1: Tại nhà, 2: Học thêm, 3: Tự học): ";
    int Type; cin >> Type; cin.ignore();
    StudyType = static_cast<Materials>(Type);
}

void Study::outPut(){
    Event::outPut();

    cout << "========Sự kiện học tập========" << endl;
    
    cout << "Môn học: " << subject << endl;

    cout << "Hình thức học: ";
    switch(StudyType){
        case OnCampus: cout << "Tại trường"; break;
        case Online: cout << "Trực tuyến"; break;
        case ExtraClass: cout << "Học thêm"; break;
        case SelfStudy: cout << "Tự học"; break;
    }

    cout << endl;
}


//DSLK sự kiện
struct EventNode{
    Event *event;
    EventNode *next;
    EventNode(Event *e): event(e), next(nullptr) {}
};

class Calendar{
    private:
        string nameCalendar;
        EventNode *head;
    public:
        Calendar();
        Calendar(string name);
        ~Calendar();
        void addEvent(Event *e);
        bool deleteEventById(const int& id);
        void sortByStartTime();
        Event *findEventById(const int& id);
        vector<Event*>findEventByDate(chrono::system_clock::time_point date);
        void displayAll();
        int size();
//save & load
        void saveToFile(const string& filename = "events.txt");
        void loadFromFile(const string& filename = "events.txt");
};

void Calendar::saveToFile(const string& filename) {
    ofstream ofs(filename);
    if (!ofs.is_open()) {
        cerr << "LỖI: Không thể mở file " << filename << " để ghi.\n";
        return;
    }

    EventNode *tmp = head;
    while (tmp) {
        Event* e = tmp->event;
        // Ghi loại sự kiện
        if (dynamic_cast<Meeting*>(e)) {
            ofs << "MEETING" << endl;
        } else if (dynamic_cast<Appointment*>(e)) {
            ofs << "APPOINTMENT" << endl;
        } else if (dynamic_cast<Holiday*>(e)) {
            ofs << "HOLIDAY" << endl;
        } else if (dynamic_cast<Study*>(e)) {
            ofs << "STUDY" << endl;
        } else {
            ofs << "EVENT" << endl;
        }
        
        // Ghi dữ liệu chi tiết (sử dụng hàm ảo save)
        e->save(ofs);
        tmp = tmp->next;
    }
    ofs.close();
    cout << "\nĐã lưu " << filename << " thành công.\n";
}

void Calendar::loadFromFile(const string& filename) {
    ifstream ifs(filename);
    if (!ifs.is_open()) {
        cout << "Không tìm thấy file " << filename << ". Bắt đầu với lịch trống.\n";
        return;
    }

    // Xóa lịch cũ trước khi tải dữ liệu mới để tránh rò rỉ bộ nhớ
    EventNode *tmp = head;
    while(tmp){
        EventNode *temp = tmp;
        tmp = tmp->next;
        delete temp->event;
        delete temp;
    }
    head = nullptr; 

    string type_str;
    // Đọc loại sự kiện (dòng đầu tiên)
    while (getline(ifs, type_str)) {
        Event* e = nullptr;
        
        // Tạo đối tượng dựa trên loại sự kiện
        if (type_str == "MEETING") {
            e = new Meeting();
        } else if (type_str == "APPOINTMENT") {
            e = new Appointment();
        } else if (type_str == "HOLIDAY") {
            e = new Holiday();
        } else if (type_str == "STUDY") {
            e = new Study();
        } else {
            // Nếu không khớp loại, tiếp tục đọc dòng tiếp theo
            continue; 
        }

        if (e) {
            e->load(ifs); // Đọc dữ liệu chi tiết (sử dụng hàm ảo load)
            addEvent(e);  // Thêm vào lịch
        }
    }

    ifs.close();
    cout << "Đã tải dữ liệu lịch thành công từ " << filename << ".\n";
}
//--------------------------

Calendar::Calendar(){
    nameCalendar= " ";
    head = nullptr;
}

Calendar::Calendar(string name){
    nameCalendar = name;
    head = nullptr;
}

Calendar::~Calendar(){
    EventNode *tmp = head;
    while(tmp!=nullptr){
        EventNode *temp = tmp;
        tmp = tmp->next;
        delete temp->event;
        delete temp;
    }
}

void Calendar::addEvent(Event *e){
    EventNode *newNode = new EventNode(e);
    if(!head){
        head = newNode;
    }
    else{
        EventNode *tmp = head;
        while(tmp->next!=nullptr){
            tmp=tmp->next;
        }
        tmp->next=newNode;
    }
}

bool Calendar::deleteEventById(const int& id){
    if(!head) return 0;

    EventNode *tmp = head, *prev = nullptr;
    while(tmp){
        Event *e = tmp->event;
        int eventId = e->getId();
        if(eventId==id){
            if(prev==nullptr){
                head=tmp->next;
            }
            else{
                prev->next = tmp->next;
            }
            delete tmp->event;
            delete tmp;
            return 1;
        }
        prev=tmp;
        tmp=tmp->next;
    }
    return 0;
}

void Calendar::sortByStartTime(){
    if(!head || !head->next) return;

    for(EventNode *i = head; i->next != nullptr; i=i->next){
        for(EventNode *j = i->next; j != nullptr; j=j->next){
            if(i->event->getDate() > j->event->getDate()){
                Event *tmp = i->event;
                i->event=j->event;
                j->event=tmp;
            }
        }
    }
}

Event *Calendar::findEventById(const int& id){
    EventNode *tmp = head;
    while(tmp){
        if(tmp->event->getId() == id){
            return tmp->event;
        }
        tmp=tmp->next;
    }
    cout << "Không tìm thấy ID\n";
    return nullptr;
}

vector<Event*> Calendar::findEventByDate(chrono::system_clock::time_point date){
    vector<Event*> result;
    EventNode *tmp = head;

    time_t input_t = chrono::system_clock::to_time_t(date);
    tm input_tm = *localtime(&input_t);

    while(tmp){
        time_t et = chrono::system_clock::to_time_t(tmp->event->getDate());
        tm etm = *localtime(&et);

        if(etm.tm_year==input_tm.tm_year &&
           etm.tm_mon==input_tm.tm_mon &&
           etm.tm_mday==input_tm.tm_mday){
            result.push_back(tmp->event);
        }
        tmp=tmp->next;
    }
    return result;
}

void Calendar::displayAll(){
    if(!head){ cout << "Không có sự kiện nào trong lịch\n"; return;}

    cout << "=====Danh sách sự kiện trong lịch=====\n";
    EventNode *tmp = head;
    while(tmp){
        tmp->event->outPut();
        tmp = tmp ->next;
    }
}

int main() {
    SetConsoleOutputCP(65001); 
    SetConsoleCP(65001);

    Calendar calendar("Lịch của tôi");

    calendar.loadFromFile(); 

    //test
    //meeting
    Meeting *m = new Meeting(
        100, 
        "Cuộc họp công ty",
        "Họp kế hoạch tháng 11",
        parseDateTime("2025-11-21 09:00"),
        parseDateTime("2025-11-21 10:00"),
        "Nguyễn Văn A",
        vector<string>{"Ngọc", "Minh", "Tuấn"},
        Meeting::Confirmed
    );
    m->setReminder(new Reminder(parseDateTime("2025-11-21 08:00"), Reminder::Popup));
    calendar.addEvent(m);
//apointment
    Appointment* a = new Appointment(
        101,
        "Lịch khám bệnh",
        "Khám sức khoẻ tổng quát",
        parseDateTime("2025-11-25 14:00"),
        parseDateTime("2025-11-25 15:00"),
        "Bệnh viện Quận 1",
        false
    );
    a->setReminder(new Reminder(parseDateTime("2025-11-25 13:00"), Reminder::Email));
    calendar.addEvent(a);
//Holiday
    Holiday* h1 = new Holiday(
        102,
        "Quốc tế Lao động",
        "Ngày lễ 1/5",
        parseDateTime("2025-05-01 00:00"),
        parseDateTime("2025-05-01 23:59"),
        parseDateTime("2025-05-01 00:00"),
        Holiday::International
    );
    RecurrenceRule* r1 = new RecurrenceRule(
        RecurrenceRule::Yearly,
        parseDateTime("2025-05-01 00:00")
    );
    h1->setRecurrence(r1);
    calendar.addEvent(h1);

    Holiday* h2 = new Holiday(
        103,
        "Quốc khánh",
        "Ngày quốc khánh Việt Nam",
        parseDateTime("2025-09-02 00:00"),
        parseDateTime("2025-09-02 23:59"),
        parseDateTime("2025-09-02 00:00"),
        Holiday::National
    );
    RecurrenceRule* r2 = new RecurrenceRule(
        RecurrenceRule::Yearly,
        parseDateTime("2025-09-02 00:00")
    );
    h2->setRecurrence(r2);
    calendar.addEvent(h2);
    //study
    Study* s = new Study(
        104,
        "Học C++",
        "Ôn tập OOP nâng cao",
        parseDateTime("2025-11-18 19:00"),
        parseDateTime("2025-11-18 21:00"),
        "Lập trình C++",
        Study::SelfStudy
    );
    s->setReminder(new Reminder(parseDateTime("2025-11-18 18:30"), Reminder::SMS));
    s->setRecurrence(new RecurrenceRule(
        RecurrenceRule::Weekly,
        parseDateTime("2025-11-18 19:00")
    ));
    calendar.addEvent(s);
    //khác
    Event* e1 = new Event(
        105,
        "Chạy bộ",
        "Chạy bộ buổi sáng",
        parseDateTime("2025-11-19 06:00"),
        parseDateTime("2025-11-19 06:45")
    );
    calendar.addEvent(e1);

    Event* e2 = new Event(
        106,
        "Xem phim",
        "Xem phim cuối tuần",
        parseDateTime("2025-11-23 20:00"),
        parseDateTime("2025-11-23 22:30")
    );
    calendar.addEvent(e2);

    
    while(true){
        cout << "\n===== MENU =====\n";
        cout << "1. Thêm sự kiện\n";
        cout << "2. Xóa sự kiện theo ID\n";
        cout << "3. Tìm sự kiện theo ID\n";
        cout << "4. Tìm sự kiện theo ngày\n";
        cout << "5. Sắp xếp theo thời gian\n";
        cout << "6. Hiện tất cả sự kiện\n";
        cout << "0. Thoát\n";
        cout << "Chọn: ";

        int choice;
        if(!(cin >> choice) || choice > 6 || choice < 0){
            cout << "Không hợp lệ (Chỉ có thể là số 0 đến 6)\n";
            cin.clear();
            cin.ignore(9999,'\n');
            continue;
        }
        cin.ignore(9999, '\n');

        if(choice == 0) {
            calendar.saveToFile(); 
            break;
        }

        if(choice == 1){
            cout << "Chọn loại sự kiện:\n";
            cout << "1. Meeting\n";
            cout << "2. Appointment\n";
            cout << "3. Holiday\n";
            cout << "4. Study\n";
            cout << "5. Khác\n";
            cout << "Chọn: ";

            int type;
            if(!(cin >> type) || type > 5 || type < 1){
                cout << "Không hợp lệ (Chỉ có thể là số 1 đến 4)\n";
                cin.clear();
                cin.ignore(9999,'\n');
                continue;
            }
            cin.ignore(9999, '\n');

            Event* e = nullptr;

            if(type == 1) e = new Meeting();
            if(type == 2) e = new Appointment();
            if(type == 3) e = new Holiday();
            if(type == 4) e = new Study();
            if(type == 5) e = new Event();

            if(!e){
                cout << "Loại sự kiện không hợp lệ\n";
                continue;
            }

            e->inPut();
            calendar.addEvent(e);
        }

        else if(choice == 2){
            int id;
            cout << "Nhập ID cần xóa: ";
            if(!(cin >> id)){
                cout << "ID không hợp lệ (ID chỉ có thể là số)\n";
                cin.clear();
                cin.ignore(9999,'\n');
                continue;
            }
            cin.ignore(9999, '\n');

            if(calendar.deleteEventById(id))
                cout << "Xóa thành công.\n";
            else
                cout << "Không tìm thấy.\n";
        }

        else if(choice == 3){
            int id;
            cout << "Nhập ID cần tìm: ";
            if(!(cin >> id)){
                cout << "ID không hợp lệ (ID chỉ có thể là số)\n";
                cin.clear();
                cin.ignore(9999,'\n');
                continue;
            }
            cin.ignore(9999, '\n');

            Event* e = calendar.findEventById(id);
            if(e) e->outPut();
        }

        else if(choice == 4){
            string date;
            cout << "Nhập ngày (YYYY-MM-DD 00:00): ";
            getline(cin, date);

            auto results = calendar.findEventByDate(parseDateTime(date));
            for(auto e : results){
                e->outPut();
            }
        }

        else if(choice == 5){
            calendar.sortByStartTime();
            cout << "Đã sắp xếp.\n";
        }

        else if(choice == 6){
            calendar.displayAll();
        }
    }

    return 0;
}

