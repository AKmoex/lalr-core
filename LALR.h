//
// Created by AKmoex on 2021/5/23.
//

#ifndef LALR_CORE_LALR_H
#define LALR_CORE_LALR_H

#pragma once

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
class Prod {
    friend class Item;
    friend class LALR;

public:
    string displayStr() const {
        string p = string(1, left_VN) + "->" + right.c_str();
        int i = 0;
        for (const auto& c : prospect)
            if (c != '#') p += string(1, i++ == 0 ? ',' : '|') + c; // #放到最后显示
        if (prospect.find('#') != prospect.end()) p += string(1, i++ == 0 ? ',' : '|') + "#";
        return p;
    }
    vector<string> displayStr2() const {
        vector<string>v;
        v.push_back(string(1, left_VN));
        v.push_back(right.c_str());
        for (const auto& c : prospect){
            if (c != '#') {
                v.push_back(string(1,c));
            }
        }
        if (prospect.find('#') != prospect.end()){
            v.push_back("#");
        }
        return v;
    }
    set<char> prospect; // 搜索符
    static string cut(const string& in, int i, int j) {
        return string(in.begin() + i, in.begin() + j);
    }
    static string replaceAll(const string& in, const string from, const string to);
    Prod(const string& in);
    Prod(const char& VT, const string& right, const set<char>& prospect) :
            left_VN(VT), right(right), prospect(prospect) {}

private:
    char left_VN; // 产生式左部非终结符名字
    string right; // 产生式右部
    friend bool operator == (const Prod& a, const Prod& b) {
        return a.left_VN == b.left_VN && a.right == b.right;
    }
    friend bool operator == (const Prod& a, char c) {
        return a.left_VN == c;
    }

};

// 项目集
class Item {
    friend class LALR;
private:
    vector<Prod> prods; // 项目集
    static set<char> Vn; // 非终结符
    static set<char> Vt; // 终结符
    static set<char> Symbol; // 所有符号
    friend bool operator == (const Item& a, const Item& b) {
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

class LALR {
private:
    Item G; // 文法G
    enum actionStat {
        ACCEPT = 0,
        SHIFT,
        REDUCE,
    };
    static const char* actionStatStr[];

    vector<Item> C; // 项目集规范族
    map<pair<int, char>, int> GOTO; // goto数组，项目集<int, int>=char
    map<pair<int, char>, pair<actionStat, int> > ACTION; // Action数组，Action[(i, a)]=(s|r)j
    map<char, set<char> > FIRST; // first集
    map<char, set<char> > FOLLOW; // follow集
    set<char> first(const string& s); // 求first集
    void follow();
    vector<char> inStr; // 输入串/栈
    vector<int> status; // 状态栈
    vector<char> parse; // 分析栈
    Item closure(Item I); // 求该项目的闭包
    Item Goto(const Item& I, char X); // 求I经过X到达的项目集
    void items(); // 求项目集状态机DFA！!
    void merge(); // 合并同心项目集族
    string type; // 所用的分析方法

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


#endif //LALR_CORE_LALR_H
