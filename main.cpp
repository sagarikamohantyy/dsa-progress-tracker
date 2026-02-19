#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>

using namespace std;

struct Entry {
    string date;
    int easy = 0;
    int medium = 0;
    int hard = 0;
};

bool safeIntInput(int &x) {
    cin >> x;
    if (cin.fail() || x < 0) {
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Invalid input.\n";
        return false;
    }
    return true;
}

bool isValidDate(const string &s) {
    if (s.size() != 10 || s[4] != '-' || s[7] != '-') return false;
    int y, m, d;
    if (sscanf(s.c_str(), "%d-%d-%d", &y, &m, &d) != 3) return false;

    tm t= {};
    t.tm_year= y - 1900;
    t.tm_mon= m - 1;
    t.tm_mday= d;

    time_t tt = mktime(&t);
    if (tt==-1) return false;

    tm *chk = localtime(&tt);
    return chk->tm_year==t.tm_year &&
           chk->tm_mon==t.tm_mon &&
           chk->tm_mday==t.tm_mday;
}

tm parseDate(const string &s) {
    tm t={};
    sscanf(s.c_str(), "%d-%d-%d", &t.tm_year, &t.tm_mon, &t.tm_mday);
    t.tm_year-=1900;
    t.tm_mon-=1;
    return t;
}

int daysBetween(const tm &a, const tm &b) {
    time_t ta=mktime(const_cast<tm*>(&a));
    time_t tb=mktime(const_cast<tm*>(&b));
    return int(difftime(tb, ta) / (60 * 60 * 24));
}

vector<Entry> loadData(const string &path) {
    vector<Entry> v;
    ifstream in(path);
    string line;

    getline(in, line); 

    while (getline(in, line)) {
        stringstream ss(line);
        Entry e;
        string sEasy, sMedium, sHard;

        if (!getline(ss, e.date, ',')) continue;
        if (!getline(ss, sEasy, ',')) continue;
        if (!getline(ss, sMedium, ',')) continue;
        if (!getline(ss, sHard)) continue;

        try {
            e.easy = stoi(sEasy);
            e.medium = stoi(sMedium);
            e.hard = stoi(sHard);
        } catch (...) {
            continue; 
        }

        v.push_back(e);
    }
    return v;
}

bool dateExists(const vector<Entry> &v, const string &date) {
    for (const auto &e : v)
        if (e.date == date) return true;
    return false;
}

void saveEntry(const string &path, const Entry &e) {
    ofstream out(path, ios::app);
    out << e.date << "," << e.easy << "," << e.medium << "," << e.hard << "\n";
}

void addProgress(const string &path) {
    auto data=loadData(path);
    Entry e;

    cout << "Date (YYYY-MM-DD): ";
    cin >> e.date;

    if (!isValidDate(e.date)) {
        cout << "Invalid date.\n";
        return;
    }

    if (dateExists(data, e.date)) {
        cout << "Entry already exists for this date.\n";
        return;
    }

    cout << "Easy: ";   if (!safeIntInput(e.easy)) return;
    cout << "Medium: "; if (!safeIntInput(e.medium)) return;
    cout << "Hard: ";   if (!safeIntInput(e.hard)) return;

    saveEntry(path, e);
    cout << "Progress saved.\n";
}

int calculateStreak(vector<Entry> v) {
    if (v.empty()) return 0;

    sort(v.begin(), v.end(),
         [](const Entry &a, const Entry &b) { return a.date < b.date; });

    int streak=1;
    for (int i=v.size()-1; i>0; i--) {
        tm d1= parseDate(v[i-1].date);
        tm d2= parseDate(v[i].date);
        if (daysBetween(d1, d2) == 1)
            streak++;
        else
            break;
    }
    return streak;
}

void viewHistory(vector<Entry> v, int lastDays = -1) {
    if (v.empty()) {
        cout << "No history.\n";
        return;
    }

    sort(v.begin(), v.end(),
         [](const Entry &a, const Entry &b) { return a.date < b.date; });

    if (lastDays > 0 && v.size() > size_t(lastDays))
        v.erase(v.begin(), v.end() - lastDays);

    cout << "\n--- HISTORY ---\n";
    for (auto &e : v) {
        cout << e.date
             << " | E:" << e.easy
             << " M:" << e.medium
             << " H:" << e.hard << endl;
    }
}

void showSummary(vector<Entry> v, int dailyGoal) {
    int e = 0, m = 0, h = 0;

    for (auto &x : v) {
        e += x.easy;
        m += x.medium;
        h += x.hard;
    }

    int todaySolved = 0;
    if (!v.empty()) {
        auto latest = max_element(v.begin(), v.end(),
            [](const Entry &a, const Entry &b) { return a.date < b.date; });
        todaySolved = latest->easy + latest->medium + latest->hard;
    }

    cout << "\n--- SUMMARY ---\n";
    cout << "Days logged: " << v.size() << endl;
    cout << "Easy: " << e << "  Medium: " << m << "  Hard: " << h << endl;
    cout << "Current streak: " << calculateStreak(v) << " days\n";
    cout << "Today's goal: " << todaySolved << "/" << dailyGoal << endl;
}

void ensureConfig(int &goal, string &path) {
    ifstream in("config.txt");
    if (in.good()) {
        string line;
        while (getline(in, line)) {
            if (line.find("DAILY_GOAL=") == 0)
                goal = stoi(line.substr(11));
            else if (line.find("DATA_PATH=") == 0)
                path = line.substr(10);
        }
        return;
    }

    ofstream out("config.txt");
    out << "# Daily target problems\nDAILY_GOAL=3\n\n";
    out << "# CSV data file path\nDATA_PATH=progress.csv\n";
}

int main() {
    int dailyGoal = 3;
    string dataPath = "progress.csv";

    ensureConfig(dailyGoal, dataPath);

    ifstream chk(dataPath);
    if (!chk.good()) {
        ofstream out(dataPath);
        out << "date,easy,medium,hard\n";
    }

    while (true) {
        cout << "\n1. Add progress\n2. View history\n3. View last 7 days\n"
             << "4. View summary\n5. Exit\nChoice: ";

        int c;
        if (!(cin >> c)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Invalid input.\n";
            continue;
        }

        auto data = loadData(dataPath);

        if (c==1) addProgress(dataPath);
        else if (c==2) viewHistory(data);
        else if (c==3) viewHistory(data, 7);
        else if (c==4) showSummary(data, dailyGoal);
        else break;
    }
}
