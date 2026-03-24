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

    tm t = {};
    t.tm_year = y - 1900;
    t.tm_mon = m - 1;
    t.tm_mday = d;

    time_t tt = mktime(&t);
    if (tt == -1) return false;

    tm *chk = localtime(&tt);
    return chk->tm_year == t.tm_year &&
           chk->tm_mon == t.tm_mon &&
           chk->tm_mday == t.tm_mday;
}

tm parseDate(const string &s) {
    tm t = {};
    sscanf(s.c_str(), "%d-%d-%d", &t.tm_year, &t.tm_mon, &t.tm_mday);
    t.tm_year -= 1900;
    t.tm_mon -= 1;
    return t;
}

int daysBetween(const tm &a, const tm &b) {
    time_t ta = mktime(const_cast<tm*>(&a));
    time_t tb = mktime(const_cast<tm*>(&b));
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

void saveEntry(const string &path, const Entry &e) {
    ofstream out(path, ios::app);
    out << e.date << "," << e.easy << "," << e.medium << "," << e.hard << "\n";
}

void rewriteFile(const string &path, const vector<Entry> &data) {
    ofstream out(path);
    out << "date,easy,medium,hard\n";
    for (const auto &e : data)
        out << e.date << "," << e.easy << "," << e.medium << "," << e.hard << "\n";
}

void addOrEditProgress(const string &path) {
    auto data = loadData(path);
    string date;

    cout << "Date (YYYY-MM-DD): ";
    cin >> date;

    if (!isValidDate(date)) {
        cout << "Invalid date.\n";
        return;
    }

    for (auto &e : data) {
        if (e.date == date) {
            cout << "Entry exists — E:" << e.easy << " M:" << e.medium << " H:" << e.hard << "\n";
            cout << "Edit? (y/n): ";
            char choice;
            cin >> choice;
            if (choice != 'y') return;

            cout << "New Easy: ";   if (!safeIntInput(e.easy)) return;
            cout << "New Medium: "; if (!safeIntInput(e.medium)) return;
            cout << "New Hard: ";   if (!safeIntInput(e.hard)) return;

            rewriteFile(path, data);
            cout << "Entry updated.\n";
            return;
        }
    }

    Entry e;
    e.date = date;
    cout << "Easy: ";   if (!safeIntInput(e.easy)) return;
    cout << "Medium: "; if (!safeIntInput(e.medium)) return;
    cout << "Hard: ";   if (!safeIntInput(e.hard)) return;

    saveEntry(path, e);
    cout << "Progress saved.\n";
}

void deleteEntry(const string &path) {
    auto data = loadData(path);
    string date;
    cout << "Enter date to delete (YYYY-MM-DD): ";
    cin >> date;

    auto it = remove_if(data.begin(), data.end(),
        [&date](const Entry &e) { return e.date == date; });

    if (it == data.end()) {
        cout << "Entry not found.\n";
        return;
    }

    data.erase(it, data.end());
    rewriteFile(path, data);
    cout << "Entry deleted.\n";
}

pair<int,int> getStreaks(vector<Entry> v) {
    if (v.empty()) return {0, 0};

    sort(v.begin(), v.end(),
         [](const Entry &a, const Entry &b) { return a.date < b.date; });

    int streak = 1;
    for (int i = (int)v.size() - 1; i > 0; i--) {
        tm d1 = parseDate(v[i-1].date);
        tm d2 = parseDate(v[i].date);
        if (daysBetween(d1, d2) == 1)
            streak++;
        else
            break;
    }

    int best = 1, current = 1;
    for (int i = 1; i < (int)v.size(); i++) {
        tm d1 = parseDate(v[i-1].date);
        tm d2 = parseDate(v[i].date);
        if (daysBetween(d1, d2) == 1)
            current++;
        else
            current = 1;
        best = max(best, current);
    }

    return {streak, best};
}

int missedDays(vector<Entry> v) {
    if (v.size() < 2) return 0;

    sort(v.begin(), v.end(),
         [](const Entry &a, const Entry &b) { return a.date < b.date; });

    int missed = 0;
    for (int i = 1; i < (int)v.size(); i++) {
        tm d1 = parseDate(v[i-1].date);
        tm d2 = parseDate(v[i].date);
        int gap = daysBetween(d1, d2);
        if (gap > 1) missed += gap - 1;
    }
    return missed;
}

void viewHistory(vector<Entry> v) {
    if (v.empty()) { cout << "No history.\n"; return; }

    sort(v.begin(), v.end(),
         [](const Entry &a, const Entry &b) { return a.date < b.date; });

    cout << "Show last how many days? (0 = all): ";
    int days;
    if (!safeIntInput(days)) return;

    if (days > 0 && v.size() > size_t(days))
        v.erase(v.begin(), v.end() - days);

    cout << "\nHISTORY\n";
    for (auto &e : v) {
        int total = e.easy + e.medium + e.hard;
        cout << e.date
             << " | E:" << e.easy
             << " M:" << e.medium
             << " H:" << e.hard
             << " | Total:" << total << "\n";
    }
}

void showSummary(vector<Entry> v, int dailyGoal) {
    int e = 0, m = 0, h = 0;
    for (auto &x : v) {
        e += x.easy;
        m += x.medium;
        h += x.hard;
    }

    int total = e + m + h;

    int todaySolved = 0;
    if (!v.empty()) {
        auto latest = max_element(v.begin(), v.end(),
            [](const Entry &a, const Entry &b) { return a.date < b.date; });
        todaySolved = latest->easy + latest->medium + latest->hard;
    }

    pair<int,int> streaks = getStreaks(v);
    int curStreak = streaks.first;
    int topStreak = streaks.second;

    cout << "\n--- SUMMARY ---\n";
    cout << "Days logged:    " << v.size() << "\n";
    cout << "Total solved:   " << total << "\n";
    cout << "Easy:           " << e << "\n";
    cout << "Medium:         " << m << "\n";
    cout << "Hard:           " << h << "\n";

    if (total > 0) {
        cout << "Difficulty mix: "
             << "E " << (e * 100 / total) << "% "
             << "M " << (m * 100 / total) << "% "
             << "H " << (h * 100 / total) << "%\n";
    }

    cout << "Current streak: " << curStreak << " days\n";
    cout << "Best streak:    " << topStreak << " days\n";
    cout << "Missed days:    " << missedDays(v) << "\n";
    cout << "Today's goal:   " << todaySolved << "/" << dailyGoal << "\n";
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
        cout << "DSA Progress Tracker\n";
        cout << "1. Add / Edit progress\n";
        cout << "2. View history\n";
        cout << "3. View summary\n";
        cout << "4. Delete entry\n";
        cout << "5. Exit\n";
        cout << "Choice: ";

        int c;
        if (!(cin >> c)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Invalid input.\n";
            continue;
        }

        auto data = loadData(dataPath);

        if      (c == 1) addOrEditProgress(dataPath);
        else if (c == 2) viewHistory(data);
        else if (c == 3) showSummary(data, dailyGoal);
        else if (c == 4) deleteEntry(dataPath);
        else if (c == 5) break;
        else cout << "Invalid choice.\n";
    }

    cout << "Goodbye!\n";
    return 0;
}