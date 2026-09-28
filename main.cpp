// C++ OOP Doubt-Tracker (CLI) - inheritance, encapsulation, file I/O
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// ---------- Base class (encapsulation: private data + getters/setters) ----------
class Doubt {
    int id;
    string topic, difficulty, question, answer;
    bool solved;
public:
    Doubt(int id, string topic, string diff, string q, string a = "", bool s = false)
        : id(id), topic(topic), difficulty(diff), question(q), answer(a), solved(s) {}
    virtual ~Doubt() {}
    virtual string type() const = 0;       // each subclass gives its own type
    virtual string hint() const = 0;       // polymorphism

    int getId() const { return id; }
    string getTopic() const { return topic; }
    string getDifficulty() const { return difficulty; }
    string getQuestion() const { return question; }
    string getAnswer() const { return answer; }
    bool isSolved() const { return solved; }
    void addAnswer(const string& text) {   // answers / follow-ups build a thread
        answer += (answer.empty() ? "" : " || ") + text;
    }
    void setSolved(bool s) { solved = s; }
    void setTopic(const string& t) { topic = t; }
    void setDifficulty(const string& d) { difficulty = d; }
    void setQuestion(const string& q) { question = q; }
};

// ---------- Derived classes (inheritance) ----------
class DsaDoubt : public Doubt {
public:
    using Doubt::Doubt;
    string type() const override { return "DSA"; }
    string hint() const override { return "Think: brute force -> optimize -> complexity."; }
};
class CppDoubt : public Doubt {
public:
    using Doubt::Doubt;
    string type() const override { return "C++"; }
    string hint() const override { return "Try a tiny code example and compile it."; }
};
class GeneralDoubt : public Doubt {
public:
    using Doubt::Doubt;
    string type() const override { return "General"; }
    string hint() const override { return "Search docs / ask a senior / note the source."; }
};

// ---------- Manager: holds doubts + file I/O ----------
class DoubtManager {
    vector<unique_ptr<Doubt>> list;
    string file;
    int nextId = 1;

    static string clean(string s) {        // keep file format safe
        for (char& c : s) if (c == '\t' || c == '\n' || c == '\r') c = ' ';
        return s;
    }
    static unique_ptr<Doubt> make(int id, const string& type, const string& topic,
                                  const string& diff, const string& q,
                                  const string& a, bool s) {
        if (type == "DSA") return make_unique<DsaDoubt>(id, topic, diff, q, a, s);
        if (type == "C++") return make_unique<CppDoubt>(id, topic, diff, q, a, s);
        return make_unique<GeneralDoubt>(id, topic, diff, q, a, s);
    }
public:
    explicit DoubtManager(string f) : file(f) { load(); }

    void load() {
        ifstream in(file);
        string line;
        while (getline(in, line)) {
            vector<string> p; string cur; stringstream ss(line);
            while (getline(ss, cur, '\t')) p.push_back(cur);
            if (p.size() < 7) continue;
            int id = stoi(p[0]);
            list.push_back(make(id, p[1], p[2], p[3], p[5], p[6], p[4] == "1"));
            nextId = max(nextId, id + 1);
        }
    }
    void save() const {
        ofstream out(file);
        for (auto& d : list)
            out << d->getId() << '\t' << d->type() << '\t' << d->getTopic() << '\t'
                << d->getDifficulty() << '\t' << d->isSolved() << '\t'
                << clean(d->getQuestion()) << '\t' << clean(d->getAnswer()) << '\n';
    }
    void add(const string& type, string topic, string diff, string q) {
        list.push_back(make(nextId++, type, clean(topic), clean(diff), clean(q), "", false));
        save();
    }
    bool remove(int id) {
        for (size_t i = 0; i < list.size(); i++)
            if (list[i]->getId() == id) { list.erase(list.begin() + i); save(); return true; }
        return false;
    }
    static int rank(const string& d) { return d == "hard" ? 0 : d == "med" ? 1 : 2; }
    void sortByPriority() {   // unsolved first, then hardest first
        stable_sort(list.begin(), list.end(), [](const unique_ptr<Doubt>& a, const unique_ptr<Doubt>& b) {
            if (a->isSolved() != b->isSolved()) return !a->isSolved();
            return rank(a->getDifficulty()) < rank(b->getDifficulty());
        });
        save();
    }
    Doubt* find(int id) {
        for (auto& d : list) if (d->getId() == id) return d.get();
        return nullptr;
    }
    void print(const string& typeFilter = "", const string& kw = "", int status = -1) const {
        cout << "\n" << left << setw(4) << "ID" << setw(9) << "Type" << setw(14) << "Topic"
             << setw(8) << "Level" << setw(11) << "Status" << setw(30) << "Doubt"
             << "Answer\n" << string(90, '-') << "\n";
        int n = 0;
        for (auto& d : list) {
            if (!typeFilter.empty() && d->type() != typeFilter) continue;
            if (status != -1 && d->isSolved() != (status == 1)) continue;
            if (!kw.empty()) {
                string all = d->getQuestion() + " " + d->getTopic();
                string a = all, k = kw;
                transform(a.begin(), a.end(), a.begin(), ::tolower);
                transform(k.begin(), k.end(), k.begin(), ::tolower);
                if (a.find(k) == string::npos) continue;
            }
            auto cut = [](string s, size_t n) { return s.size() > n ? s.substr(0, n - 2) + ".." : s; };
            cout << left << setw(4) << d->getId() << setw(9) << d->type()
                 << setw(14) << cut(d->getTopic(), 13) << setw(8) << d->getDifficulty()
                 << setw(11) << (d->isSolved() ? "Solved" : "Unsolved")
                 << setw(30) << cut(d->getQuestion(), 29)
                 << (d->getAnswer().empty() ? "-" : cut(d->getAnswer(), 30)) << "\n";
            n++;
        }
        if (!n) cout << "(no doubts found)\n";
    }
    void stats() const {                    // unresolved doubts by topic
        map<string, int> m;
        int solved = 0;
        for (auto& d : list) d->isSolved() ? solved++ : m[d->getTopic()]++;
        cout << "\nTotal: " << list.size() << " | Solved: " << solved
             << " | Unsolved: " << list.size() - solved << "\nUnsolved by topic (revise these first):\n";
        vector<pair<int, string>> v;
        for (auto& [t, c] : m) v.push_back({c, t});
        sort(v.rbegin(), v.rend());
        for (auto& [c, t] : v) cout << "  " << setw(14) << left << t << string(c, '#') << " " << c << "\n";
        if (v.empty()) cout << "  Nothing unsolved. Great!\n";
    }
};

// ---------- helpers (with input validation) ----------
string ask(const string& msg) { cout << msg; string s; if (!getline(cin, s)) exit(0); return s; }
string askNonEmpty(const string& msg) {
    while (true) { string s = ask(msg); if (!s.empty()) return s; cout << "  Cannot be empty, try again.\n"; }
}
string pickType(bool required) {
    while (true) {
        cout << "Type: 1) DSA  2) C++ concept  3) General\n";
        string c = ask(required ? "Choose (1-3): " : "Choose (1-3, blank = all): ");
        if (c == "1") return "DSA";
        if (c == "2") return "C++";
        if (c == "3") return "General";
        if (c.empty() && !required) return "";
        cout << "  Please enter 1, 2 or 3.\n";
    }
}
string askDifficulty() {
    while (true) {
        string c = ask("Difficulty: 1) easy  2) med  3) hard : ");
        if (c == "1") return "easy";
        if (c == "2") return "med";
        if (c == "3") return "hard";
        cout << "  Please enter 1, 2 or 3.\n";
    }
}
int readInt(const string& msg) {   // returns -1 if not a valid number
    string s = ask(msg);
    try { size_t pos; int v = stoi(s, &pos); if (pos == s.size()) return v; } catch (...) {}
    cout << "  Please enter a valid number.\n";
    return -1;
}

int main() {
    DoubtManager mgr("doubts.txt");
    while (true) {
        cout << "\n=== DOUBT TRACKER ===\n1 Add doubt\n2 View all / filter by type & status\n"
                "3 Add answer / follow-up + set solved/unsolved\n4 Mark solved/unsolved\n"
                "5 Search keyword\n6 Stats\n7 Edit doubt\n8 Delete doubt\n"
                "9 Sort (unsolved & hardest first)\n0 Exit\n";
        string ch = ask("Choice: ");
        if (ch == "1") {
            string t = pickType(true);
            string topic = askNonEmpty("Topic (e.g. arrays, pointers): ");
            string diff = askDifficulty();
            string q = askNonEmpty("Your doubt: ");
            mgr.add(t, topic, diff, q);
            cout << "Saved!\n";
        } else if (ch == "2") {
            string t = pickType(false);
            string st = ask("Status: 1) Solved  2) Unsolved  (blank = all): ");
            mgr.print(t, "", st == "1" ? 1 : st == "2" ? 0 : -1);
        } else if (ch == "3") {
            mgr.print();
            Doubt* d = mgr.find(readInt("Doubt ID: "));
            if (!d) { cout << "Not found.\n"; continue; }
            cout << "Q: " << d->getQuestion() << "\nAnswer so far: " << d->getAnswer()
                 << "\nHint: " << d->hint() << "\n";
            string a = ask("Write answer or follow-up: ");
            if (!a.empty()) d->addAnswer(a);
            string sv = ask("Is this doubt solved? 1) Solved  2) Unsolved  (blank = keep): ");
            if (sv == "1") d->setSolved(true);
            else if (sv == "2") d->setSolved(false);
            mgr.save();
            cout << "Updated. Status: " << (d->isSolved() ? "Solved" : "Unsolved") << "\n";
        } else if (ch == "4") {
            Doubt* d = mgr.find(readInt("Doubt ID: "));
            if (!d) { cout << "Not found.\n"; continue; }
            d->setSolved(!d->isSolved()); mgr.save();
            cout << "Now: " << (d->isSolved() ? "Solved" : "Unsolved") << "\n";
        } else if (ch == "5") {
            mgr.print("", ask("Keyword: "));
        } else if (ch == "6") {
            mgr.stats();
        } else if (ch == "7") {
            mgr.print();
            Doubt* d = mgr.find(readInt("Doubt ID to edit: "));
            if (!d) { cout << "Not found.\n"; continue; }
            string t = ask("New topic (blank = keep): ");
            if (!t.empty()) d->setTopic(t);
            string q = ask("New doubt text (blank = keep): ");
            if (!q.empty()) d->setQuestion(q);
            if (ask("Change difficulty? (y/n): ") == "y") d->setDifficulty(askDifficulty());
            mgr.save();
            cout << "Edited.\n";
        } else if (ch == "8") {
            mgr.print();
            int id = readInt("Doubt ID to delete: ");
            if (id < 0 || !mgr.find(id)) { cout << "Not found.\n"; continue; }
            if (ask("Really delete? (y/n): ") == "y") { mgr.remove(id); cout << "Deleted.\n"; }
        } else if (ch == "9") {
            mgr.sortByPriority();
            mgr.print();
        } else if (ch == "0") break;
        else cout << "Invalid choice.\n";
    }
    cout << "Bye!\n";
}
