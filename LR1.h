//
// Created by AKmoex on 2021/6/10.
//

#ifndef LALR_CORE_LR1_H
#define LALR_CORE_LR1_H


#include <iostream>
#include <ctype.h>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <set>
#include <exception>
#include "./json//single_include//nlohmann//json.hpp"

using json = nlohmann::json;

using namespace std;


// 产生式
class Prod_LR1 {
    friend class Item_LR1;
    friend class LR;

public:
    string displayStr() const {
        string p = string(1, left_VN) + "->" + right.c_str();
        int i = 0;
        for (const auto& c : prospect)
            if (c != '#') p += string(1, i++ == 0 ? ',' : '|') + c; // #放到最后显示
        if (prospect.find('#') != prospect.end()) p += string(1, i++ == 0 ? ',' : '|') + "#";
        return p;
    }
    set<char> prospect; // 搜索符
    static string cut(const string& in, int i, int j) {
        return string(in.begin() + i, in.begin() + j);
    }
    static string replaceAll(const string& in, const string from, const string to);
    Prod_LR1(const string& in);
    Prod_LR1(const char& VT, const string& right, const set<char>& prospect) :
            left_VN(VT), right(right), prospect(prospect) {}

private:
    char left_VN; // 产生式左部非终结符名字
    string right; // 产生式右部
    friend bool operator == (const Prod_LR1& a, const Prod_LR1& b) {
        return a.left_VN == b.left_VN && a.right == b.right;
    }
    friend bool operator == (const Prod_LR1& a, char c) {
        return a.left_VN == c;
    }

};

class Item_LR1 { // 项目集
    friend class LR;
private:
    vector<Prod_LR1> prods; // 项目集
    static set<char> Vn; // 非终结符
    static set<char> Vt; // 终结符
    static set<char> Symbol; // 所有符号
    friend bool operator == (const Item_LR1& a, const Item_LR1& b) {
        if (a.prods.size() != b.prods.size()) return false;
        else {
            for (const auto& p : a.prods) {// 选择a的每个产生式
                auto it = find(b.prods.begin(), b.prods.end(), p);
                if (it == b.prods.end())  // 找不到
                    return false;
                else {// 找到的话判断搜索符是否都相等
                    if (p.prospect != it->prospect)
                        return false;
                }
            }
            return true;
        }
    }
public:
    void analyse_grammar_line(const string& prod);

};

class LR {
private:
    Item_LR1 G; // 文法G
    enum actionStat {
        ACCEPT = 0,
        SHIFT,
        REDUCE,
    };
    static const char* actionStatStr[];

    vector<Item_LR1> C; // 项目集规范族
    map<pair<int, char>, int> GOTO; // goto数组，项目集<int, int>=char
    map<pair<int, char>, pair<actionStat, int> > ACTION; // Action数组，Action[(i, a)]=(s|r)j
    map<char, set<char> > FIRST; // first集
    map<char, set<char> > FOLLOW; // follow集
    set<char> first(const string& s); // 求first集
    void follow();
    vector<char> inStr; // 输入串/栈
    vector<int> status; // 状态栈
    vector<char> parse; // 分析栈
    Item_LR1 closure(Item_LR1 I); // 求该项目的闭包
    Item_LR1 Goto(const Item_LR1& I, char X); // 求I经过X到达的项目集
    void items(); // 求项目集状态机DFA！!

    json j;
public:
    void web_input(string grammars, string expression);
    // 构造Action、GOTO表
    void build_table();
    void run();
    void draw_graph(); // 画出DFA！

    void web_output_table();
    void web_output_steps();
    string get_data();
};


#endif //LALR_CORE_LR1_H
