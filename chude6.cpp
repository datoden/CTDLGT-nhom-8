#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <iomanip>
#include <limits>
#include <cctype>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <windows.h>

using namespace std;

string intToStr(int n) {
    stringstream ss;
    ss << n;
    return ss.str();
}

int getUTF8Length(const string& str) {
    int len = 0;
    for (size_t i = 0; i < str.length(); ) {
        unsigned char c = (unsigned char)str[i];
        if (c <= 0x7F) i += 1;
        else if ((c & 0xE0) == 0xC0) i += 2;
        else if ((c & 0xF0) == 0xE0) i += 3;
        else if ((c & 0xF8) == 0xF0) i += 4;
        else i += 1;
        len++;
    }
    return len;
}

void printAligned(const string& text, int width) {
    cout << text;
    int actualLength = getUTF8Length(text);
    int spacesToPad = width - actualLength;
    if (spacesToPad > 0) {
        cout << string(spacesToPad, ' ');
    }
}

struct WordEntry {
    string word;
    string meaning;
    string wordClass;
    string pronunciation;
    string topic;
};

struct HistoryRecord {
    string query;
    string mode;
    string status;
    string timestamp;
};

struct TrieNode {
    TrieNode *child[26];
    bool isEnd;
    string meaning;
    string wordClass;
    string pronunciation;
    string topic;

    TrieNode() {
        for (int i = 0; i < 26; i++) {
            child[i] = NULL;
        }
        isEnd = false;
        meaning = "";
        wordClass = "Chua ro";
        pronunciation = "---";
        topic = "Chung";
    }
};

string trimStr(const string &s) {
    string res = s;
    while (!res.empty() && isspace((unsigned char)res[0])) {
        res.erase(0, 1);
    }
    while (!res.empty() && isspace((unsigned char)res[res.size() - 1])) {
        res.erase(res.size() - 1, 1);
    }
    return res;
}

string toLowerStr(const string &s) {
    string res = trimStr(s);
    for (size_t i = 0; i < res.size(); i++) {
        res[i] = (char)tolower((unsigned char)res[i]);
    }
    return res;
}

bool normalizeWord(string &word) {
    word = trimStr(word);
    if (word.empty()) return false;

    for (int i = 0; i < (int)word.size(); i++) {
        unsigned char c = (unsigned char)word[i];
        if (c > 127 || !isalpha(c))
            return false;
        word[i] = (char)tolower(c);
        if (word[i] < 'a' || word[i] > 'z')
            return false;
    }
    return true;
}

string getCurrentTimeStr() {
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char buf[30];
    strftime(buf, sizeof(buf), "%H:%M:%S %d/%m/%Y", ltm);
    return string(buf);
}

int levenshteinDistance(const string &s1, const string &s2) {
    int m = (int)s1.size();
    int n = (int)s2.size();
    vector<vector<int> > dp(m + 1, vector<int>(n + 1));

    for (int i = 0; i <= m; i++) dp[i][0] = i;
    for (int j = 0; j <= n; j++) dp[0][j] = j;

    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            dp[i][j] = min(min(dp[i - 1][j] + 1, dp[i][j - 1] + 1), dp[i - 1][j - 1] + cost);
        }
    }
    return dp[m][n];
}

class Trie {
private:
    TrieNode root;

    const TrieNode* findNode(const string &word) const {
        const TrieNode *cur = &root;
        for (int i = 0; i < (int)word.size(); i++) {
            int idx = word[i] - 'a';
            if (idx < 0 || idx >= 26 || cur->child[idx] == NULL) {
                return NULL;
            }
            cur = cur->child[idx];
        }
        return cur;
    }

    bool hasChildren(const TrieNode *node) const {
        for (int i = 0; i < 26; i++) {
            if (node->child[i] != NULL)
                return true;
        }
        return false;
    }
void deleteTree(TrieNode *node) {
        if (node == NULL) return;
        for (int i = 0; i < 26; i++) {
            deleteTree(node->child[i]);
        }
        delete node;
    }
    bool eraseRec(TrieNode *node, const string &word, int pos, bool &deleted) {
        if (pos == (int)word.size()) {
            if (!node->isEnd)
                return false;

            node->isEnd = false;
            node->meaning.clear();
            node->wordClass = "Chua ro";
            node->pronunciation = "---";
            node->topic = "Chung";
            deleted = true;

            return !hasChildren(node);
        }

        int idx = word[pos] - 'a';
        if (idx < 0 || idx >= 26 || node->child[idx] == NULL) {
            return false;
        }

        bool removeChild = eraseRec(node->child[idx], word, pos + 1, deleted);
        if (removeChild) {
            delete node->child[idx];
            node->child[idx] = NULL;
        }

        return !node->isEnd && !hasChildren(node);
    }

    WordEntry makeEntry(const string &word, const TrieNode *node) const {
        WordEntry e;
        e.word = word;
        e.meaning = node->meaning;
        e.wordClass = node->wordClass;
        e.pronunciation = node->pronunciation;
        e.topic = node->topic;
        return e;
    }

    void collectRec(const TrieNode *node, string &current, vector<WordEntry> &out, int limit) const {
        if (node == NULL || (int)out.size() >= limit)
            return;

        if (node->isEnd) {
            out.push_back(makeEntry(current, node));
        }

        for (int i = 0; i < 26 && (int)out.size() < limit; i++) {
            if (node->child[i] != NULL) {
                current += (char)('a' + i);
                collectRec(node->child[i], current, out, limit);
                current.erase(current.size() - 1, 1);
            }
        }
    }

    void listRec(const TrieNode *node, string &current, vector<WordEntry> &out) const {
        if (node->isEnd) {
            out.push_back(makeEntry(current, node));
        }

        for (int i = 0; i < 26; i++) {
            if (node->child[i] != NULL) {
                current += (char)('a' + i);
                listRec(node->child[i], current, out);
                current.erase(current.size() - 1, 1);
            }
        }
    }

    void saveRec(const TrieNode *node, string &current, ostream &out) const {
        if (node->isEnd) {
            if (node->wordClass == "Chua ro" && node->pronunciation == "---" && node->topic == "Chung") {
                out << current << "|" << node->meaning << endl;
            } else {
                out << current << "|" << node->meaning << "|"
                    << node->wordClass << "|" << node->pronunciation << "|" << node->topic << endl;
            }
        }

        for (int i = 0; i < 26; i++) {
            if (node->child[i] != NULL) {
                current += (char)('a' + i);
                saveRec(node->child[i], current, out);
current.erase(current.size() - 1, 1);
            }
        }
    }

public:
    ~Trie() {
        for (int i = 0; i < 26; i++) {
            deleteTree(root.child[i]);
            root.child[i] = NULL;
        }
    }

    bool insert(const string &word, const string &meaning,
                const string &wordClass = "Chua ro",
                const string &pronunciation = "---",
                const string &topic = "Chung") {
        TrieNode *cur = &root;

        for (int i = 0; i < (int)word.size(); i++) {
            int idx = word[i] - 'a';
            if (cur->child[idx] == NULL) {
                cur->child[idx] = new TrieNode();
            }
            cur = cur->child[idx];
        }

        bool isNew = !cur->isEnd;
        cur->isEnd = true;
        cur->meaning = meaning;
        cur->wordClass = wordClass.empty() ? "Chua ro" : wordClass;
        cur->pronunciation = pronunciation.empty() ? "---" : pronunciation;
        cur->topic = topic.empty() ? "Chung" : topic;

        return isNew;
    }

    bool searchEntry(const string &word, WordEntry &entry) const {
        const TrieNode *node = findNode(word);
        if (node == NULL || !node->isEnd)
            return false;

        entry = makeEntry(word, node);
        return true;
    }

    vector<WordEntry> searchVietnamese(const string &vnQuery) const {
        vector<WordEntry> all = listAll();
        vector<WordEntry> result;
        string target = toLowerStr(vnQuery);
        if (target.empty()) return result;

        for (size_t i = 0; i < all.size(); i++) {
            if (toLowerStr(all[i].meaning).find(target) != string::npos) {
                result.push_back(all[i]);
            }
        }
        return result;
    }

    vector<WordEntry> searchApproximate(const string &query, int maxDist = 2) const {
        vector<WordEntry> all = listAll();
        vector<pair<int, WordEntry> > scored;
        string q = toLowerStr(query);

        for (size_t i = 0; i < all.size(); i++) {
            int dist = levenshteinDistance(q, all[i].word);
            if (dist <= maxDist || all[i].word.find(q) != string::npos) {
                scored.push_back(make_pair(dist, all[i]));
            }
        }

        for (size_t i = 0; i < scored.size(); i++) {
            for (size_t j = i + 1; j < scored.size(); j++) {
                if (scored[j].first < scored[i].first) {
                    swap(scored[i], scored[j]);
                }
            }
        }

        vector<WordEntry> result;
        for (size_t i = 0; i < scored.size() && i < 10; i++) {
            result.push_back(scored[i].second);
        }
        return result;
    }
vector<WordEntry> filterByClass(const string &wClass) const {
        vector<WordEntry> all = listAll();
        vector<WordEntry> result;
        string target = toLowerStr(wClass);

        for (size_t i = 0; i < all.size(); i++) {
            if (toLowerStr(all[i].wordClass).find(target) != string::npos) {
                result.push_back(all[i]);
            }
        }
        return result;
    }

    vector<WordEntry> filterByTopic(const string &topic) const {
        vector<WordEntry> all = listAll();
        vector<WordEntry> result;
        string target = toLowerStr(topic);

        for (size_t i = 0; i < all.size(); i++) {
            if (toLowerStr(all[i].topic).find(target) != string::npos) {
                result.push_back(all[i]);
            }
        }
        return result;
    }

    bool erase(const string &word) {
        bool deleted = false;
        eraseRec((TrieNode*)&root, word, 0, deleted);
        return deleted;
    }

    vector<WordEntry> suggest(const string &prefix, int limit = 10) const {
        vector<WordEntry> result;
        const TrieNode *node = findNode(prefix);
        if (node == NULL || limit == 0)
            return result;

        string current = prefix;
        collectRec(node, current, result, limit);
        return result;
    }

    vector<WordEntry> listAll() const {
        vector<WordEntry> result;
        string current = "";
        listRec(&root, current, result);
        return result;
    }

    bool saveToFile(const string &filename) const {
        ofstream out(filename.c_str());
        if (!out)
            return false;

        string current = "";
        saveRec(&root, current, out);
        return (bool)out;
    }

    int loadFromFile(const string &filename) {
        ifstream in(filename.c_str());
        if (!in)
            return 0;

        string line;
        int count = 0;

        while (getline(in, line)) {
            if (trimStr(line).empty()) continue;

            vector<string> parts;
            stringstream ss(line);
            string item;
            while (getline(ss, item, '|')) {
                parts.push_back(item);
            }

            if (parts.size() < 2) continue;

            string word = parts[0];
            string meaning = trimStr(parts[1]);
            string wClass = (parts.size() >= 3) ? trimStr(parts[2]) : "Chua ro";
            string pron   = (parts.size() >= 4) ? trimStr(parts[3]) : "---";
            string topic  = (parts.size() >= 5) ? trimStr(parts[4]) : "Chung";

            if (meaning.empty() || !normalizeWord(word))
                continue;

            insert(word, meaning, wClass, pron, topic);
            count++;
        }

        return count;
    }
};
void showEntries(const vector<WordEntry> &items) {
    if (items.empty()) {
        cout << "\nKhong co tu vung nao!\n";
        return;
    }
cout << "\n========================================================================================================\n";
    cout << "                                      BANG TU VUNG ANH - VIET\n";
    cout << "========================================================================================================\n";

    printAligned("STT", 5);
    printAligned("TU VUNG", 16);
    printAligned("TU LOAI", 14);
    printAligned("PHIEN AM", 18);
    printAligned("CHU DE", 16);
    cout << "NGHIA TIENG VIET\n";

    cout << "--------------------------------------------------------------------------------------------------------\n";

    for (size_t i = 0; i < items.size(); i++) {
        printAligned(intToStr((int)i + 1), 5);
        printAligned(items[i].word, 16);
        printAligned(items[i].wordClass, 14);
        printAligned(items[i].pronunciation, 18);
        printAligned(items[i].topic, 16);
        cout << items[i].meaning << endl;
    }

    cout << "========================================================================================================\n";
    cout << "Tong so tu vung: " << items.size() << endl;
}
void printSingleEntry(const WordEntry &e) {
    cout << "\n+----------------------------------------------------+\n";
    cout << "| Tu vung   : "; printAligned(e.word, 39); cout << "|\n";
    cout << "| Phien am  : "; printAligned(e.pronunciation, 39); cout << "|\n";
    cout << "| Tu loai   : "; printAligned(e.wordClass, 39); cout << "|\n";
    cout << "| Chu de    : "; printAligned(e.topic, 39); cout << "|\n";
    cout << "| Nghia TV  : "; printAligned(e.meaning, 39); cout << "|\n";
    cout << "+----------------------------------------------------+\n";
}
