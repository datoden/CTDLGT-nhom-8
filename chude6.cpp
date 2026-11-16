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
