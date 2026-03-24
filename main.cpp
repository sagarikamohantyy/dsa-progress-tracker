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

bool safeInt(int &x) {
    cin >> x;
    if (cin.fail() || x < 0) {
        cin.clear();
        cin.ignore(10000,'\n');
        cout << "Invalid.\n";
        return false;
    }
    return true;
}

bool validDate(const string &s) {
    if (s.size() != 10 || s[4] != '-' || s[7] != '-') return false;

    int y, m, d;
    sscanf(s.c_str(), "%d-%d-%d", &y, &m, &d);

    tm t = {};
    t.tm_year = y - 1900;
    t.tm_mon  = m - 1;
    t.tm_mday = d;

    time_t tt = mktime(&t);
    if (tt == -1) return false;

    tm *chk = localtime(&tt);
    return chk->tm_year == t.tm_year
        && chk->tm_mon  == t.tm_mon
        && chk->tm_mday == t.tm_mday;
}

vector<Entry> load(const string &path) {
    vector<Entry> v;
    ifstream f(path);
    string line;
    getline(f, line); 

    while (getline(f, line)) {
        stringstream ss(line);
        Entry e;
        string a, b, c;

        if (!getline(ss, e.date, ',')) continue;
        if (!getline(ss, a, ','))      continue;
        if (!getline(ss, b, ','))      continue;
        if (!getline(ss, c))           continue;

        try {
            e.easy   = stoi(a);
            e.medium = stoi(b);
            e.hard   = stoi(c);
        } catch (...) { continue; }

        v.push_back(e);
    }
    return v;
}

void append(const string &path, const Entry &e) {
    ofstream f(path, ios::app);
    f << e.date << "," << e.easy << "," << e.medium << "," << e.hard << "\n";
}

void rewrite(const string &path, const vector<Entry> &data) {
    ofstream f(path);
    f << "date,easy,medium,hard\n";
    for (auto &e : data)
        f << e.date << "," << e.easy << "," << e.medium << "," << e.hard << "\n";
}

void addOrEdit(const string &path) {
    auto data = load(path);
    string date;

    cout << "Date (YYYY-MM-DD): ";
    cin >> date;

    if (!validDate(date)) { cout << "Bad date.\n"; return; }

    for (auto &e : data) {
        if (e.date != date) continue;

        cout << "Already logged:" << e.easy
             << " M:" << e.medium << " H:" << e.hard << "\n";
        cout << "Edit? (y/n): ";
        char ch; cin >> ch;
        if (ch != 'y') return;

        cout << "Easy: ";   if (!safeInt(e.easy))   return;
        cout << "Medium: "; if (!safeInt(e.medium)) return;
        cout << "Hard: ";   if (!safeInt(e.hard))   return;

        rewrite(path, data);
        cout << "Updated.\n";
        return;
    }

    // new entry
    Entry e;
    e.date = date;
    cout << "Easy: ";   if (!safeInt(e.easy))   return;
    cout << "Medium: "; if (!safeInt(e.medium)) return;
    cout << "Hard: ";   if (!safeInt(e.hard))   return;

    append(path, e);
    cout << "Saved.\n";
}

void deleteEntry(const string &path) {
    auto data = load(path);
    string date;
    cout << "Date to delete: ";
    cin >> date;

    auto it = remove_if(data.begin(), data.end(),
        [&](const Entry &e) { return e.date == date; });

    if (it == data.end()) { cout << "Not found.\n"; return; }

    data.erase(it, data.end());
    rewrite(path, data);
    cout << "Deleted.\n";
}

void history(vector<Entry> v) {
    if (v.empty()) { cout << "Nothing yet.\n"; return; }

    sort(v.begin(), v.end(),
        [](const Entry &a, const Entry &b) { return a.date < b.date; });

    cout << "Last how many days? (0 = all): ";
    int n; if (!safeInt(n)) return;

    if (n > 0 && v.size() > size_t(n))
        v.erase(v.begin(), v.end() - n);

    cout << "\n";
    for (auto &e : v)
        cout << e.date
             << "\tE:" << e.easy
             << "\tM:"  << e.medium
             << "\tH:"  << e.hard
             << "\tTotal:" << e.easy + e.medium + e.hard << "\n";
}

void summary(vector<Entry> v, int goal) {
    int e = 0, m = 0, h = 0;
    for (auto &x : v) { e += x.easy; m += x.medium; h += x.hard; }
    int total = e + m + h;

    int today = 0;
    if (!v.empty()) {
        auto it = max_element(v.begin(), v.end(),
            [](const Entry &a, const Entry &b) { return a.date < b.date; });
        today = it->easy + it->medium + it->hard;
    }

    cout << "\ndays:   " << v.size()  << "\n";
    cout << "total:  " << total << "  (E:" << e << " M:" << m << " H:" << h << ")\n";

    if (total > 0)
        cout << "mix:\nE " << e*100/total << "%"
             << "  M " << m*100/total << "%"
             << "  H " << h*100/total << "%\n";

    cout << "today:" << today << "/" << goal << "\n";
}

int main() {
    const string CSV = "progress.csv";
    int goal = 3;

    ifstream chk(CSV);
    if (!chk.good()) {
        ofstream f(CSV);
        f << "date,easy,medium,hard\n";
    }

    while (true) {
        cout << "\nDSA Tracker\n"
             << "1. Add/Edit\n"
             << "2. History\n"
             << "3. Summary\n"
             << "4. Delete\n"
             << "5. Quit\n"
             << "> ";

        int c;
        if (!(cin >> c)) {
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        auto data = load(CSV);

        if      (c == 1) addOrEdit(CSV);
        else if (c == 2) history(data);
        else if (c == 3) summary(data, goal);
        else if (c == 4) deleteEntry(CSV);
        else if (c == 5) break;
    }
}