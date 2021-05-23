//
// Created by AKmoex on 2021/5/23.
//

#include "LALR.h"
set<char>Item::Vn;
set<char>Item::Vt;
set<char>Item::Symbol;
const char* LR::actionStatStr[] = {
        "acc",
        "s",
        "r"
};

Prod::Prod(const string& in) {
    left_VN = in[0];
    right = cut(in, 3, in.length()); // A->X1X2X3X4
}

void Item::analyse_grammar_line(const string& prod) {

    // 先将最左边的非终结符加入
    Vn.insert(prod[0]);
    Symbol.insert(prod[0]);

    // 提取终结符、非终结符
    for (int i = 3; i < prod.length(); i++) {
        char ch = prod[i];
        if (isupper(ch)) {
            Vn.insert(ch);
            Symbol.insert(ch);
        }
        else if (ch != '|') {
            Vt.insert(ch);
            Symbol.insert(ch);
        }
    }

    // 提取候选式
    for (int i = 3; i < prod.length(); i++) {
        int j;
        for (j = i + 1; j < prod.length() && prod[j] != '|'; j++);
        Prod p = Prod(string(1, prod[0]) + "->" + Prod::cut(prod, i, j));
        if (find(prods.begin(), prods.end(), p) == prods.end()) // 去重
            prods.push_back(p);
        i = j;
    }
}

void LR::web_input(string grammars, string expression) {
    // 文法串
    string grammar_line = "";
    for (int i = 0; i < grammars.length(); i++) {
        if (grammars[i] != '\n') {
            grammar_line += grammars[i];
        }
        else {
            cout << grammar_line << endl;
            G.analyse_grammar_line(grammar_line);
            grammar_line = "";
        }
    }
    // 特殊处理一下最后一个文法字符串
    G.analyse_grammar_line(grammar_line);
    // 表达式
    inStr.push_back('#');
    status.push_back(0);
    for (int i = expression.length() - 1; i >= 0; --i) {
        inStr.push_back(expression[i]);
    }
}

Item LR::closure(Item I) {
    if (I.prods.size() == 0) return I;

    // 枚举I的产生式
    for (int i = 0; i < I.prods.size(); ++i) {

        Prod prod = I.prods[i];
        unsigned long pointLoc = 0;
        if ((pointLoc = prod.right.find('.')) != string::npos && pointLoc != prod.right.length() - 1) { // 找到.，A->a.Bc,d
            char X = prod.right[pointLoc + 1];
            // 终结符
            if (G.Vt.find(X) != G.Vt.end()) {
                if (X == '@') // @特殊处理
                    swap(I.prods[i].right[pointLoc], I.prods[i].right[pointLoc + 1]);
                continue;
            }

            string f = Prod::cut(prod.right, pointLoc + 2, prod.right.length());

            set<char> ff = {};
            for (const auto& c : prod.prospect) {
                set<char> fs = first(f + c);
                ff.insert(fs.begin(), fs.end());
            }

            for (vector<Prod>::iterator it = G.prods.begin(); it != G.prods.end(); ++it) {
                if (*it == X) { // 找到产生式
                    Prod p = *it;
                    if (p.right[0] == '@') { // 特殊处理.@ => @.
                        p.right = '.' + p.right;
                        swap(p.right[0], p.right[1]);
                    }
                    else
                        p.right = '.' + p.right;

                    vector<Prod>::iterator Iit = find(I.prods.begin(), I.prods.end(), p); // 找I中是否存在产生式
                    if (Iit != I.prods.end())  // 找到
                        Iit->prospect.insert(ff.begin(), ff.end());
                    else {
                        p.prospect.insert(ff.begin(), ff.end());
                        I.prods.push_back(p);
                    }
                }
            }
        }
    }
    // }
    return I;
}

Item LR::Goto(const Item& I, char X) {
    Item J;
    // 项目集为空或者X==@空字
    if (I.prods.size() == 0 || X == '@') return J;

    // 遍历I中的每个项目
    for (const auto& p : I.prods) {
        string right = p.right;
        unsigned long pointLoc = right.find('.');
        if (right[pointLoc + 1] == X) {
            swap(right[pointLoc], right[pointLoc + 1]);
            J.prods.push_back(Prod(p.left_VN, right, p.prospect));
        }
    }
    return closure(J);
}

// 求项目集状态机DFA
void LR::items() {
    Item initial;
    // 拓展文法
    initial.prods.push_back(Prod('^', '.' + string(1, G.prods[0].left_VN), { '#' }));
    // 求初状态对应项目
    C.push_back(closure(initial));

    int size = 0;
    // 当总状态数不再增大

    while (size != C.size()) {
        size = C.size();
        // 对已有每个的项目集求解伸出一条弧后指向的状态(项目集)
        for (int i = 0; i < C.size(); i++) {
            Item I = C[i];
            // 每个文法符号X找出伸出的弧并求项目
            for (const auto& X : G.Symbol) {
                Item next = Goto(I, X);
                // 不为空时才可以增加到总项目集中
                if (next.prods.size() != 0) {
                    auto it = find(C.begin(), C.end(), next);
                    // 下一状态已经存在
                    if (it != C.end()) {
                        // 增加一条弧即可
                        GOTO[make_pair(i, X)] = it - C.begin();
                    }
                    else {
                        // 新增一个状态，再连上弧线
                        C.push_back(next);
                        GOTO[make_pair(i, X)] = C.size() - 1;
                    }
                }
            }
        }
    }
}

void LR::run() {
    build_table();
    draw_graph();
    web_output_table();
    web_output_steps();
}

// 构造Action、GOTO表
void LR::build_table() {
    // 构造DFA状态机
    items();

    for (const auto& I : C) {
        vector<string>s;
        int i = &I - &C[0];
        s.push_back(to_string(i));
        for (const auto& p : I.prods) { // 列出项目
            string res = p.displayStr();
            if (res.find('^') != string::npos)
                res = Prod::replaceAll(res, "^", string(1, G.prods[0].left_VN) + "'");
            s.push_back(res.c_str());
            cout<<endl;
            cout<<res.c_str()<<endl;
        }
    }

    // 遍历每个状态(项目集)
    for (int i = 0; i < C.size(); i++) {
        const Item& item = C[i];
        // 遍历每个产生式
        for (const auto& prod : item.prods) {
            unsigned long pointLoc = prod.right.find('.');
            // 判断.的位置
            // .不在最后
            if (pointLoc < prod.right.length() - 1) {
                char X = prod.right[pointLoc + 1];
                if (G.Vt.find(X) != G.Vt.end() && GOTO.find(make_pair(i, X)) != GOTO.end()) { // 终结符
                    int j = GOTO[make_pair(i, X)];
                    ACTION[make_pair(i, X)] = make_pair(SHIFT, j);
                }
            }
                // .在最后
            else {
                if (prod == Prod('^', string(1, G.prods[0].left_VN) + '.', {}) && prod.prospect == set<char>({ '#' }))  // S'->S.,# acction[i, #] = acc
                    ACTION[make_pair(i, '#')] = make_pair(ACCEPT, 0);
                else if (prod.left_VN != '^') {
                    string right = prod.right;
                    right.erase(pointLoc, 1); // 删除.
                    for (const auto& X : prod.prospect) { // A->a.,b，枚举b
                        vector<Prod>::iterator it = find(G.prods.begin(), G.prods.end(), Prod(prod.left_VN, right, set<char>{}));
                        if (it != G.prods.end())  // 找到了
                            ACTION[make_pair(i, X)] = make_pair(REDUCE, it - G.prods.begin());
                    }
                }
            }
        }
    }
    if (G.Vt.find('@') != G.Vt.end()) { // 删除@，移进#
        G.Vt.erase(G.Vt.find('@'));
        G.Symbol.erase(G.Symbol.find('@'));
    }
    G.Vt.insert('#');
    G.Symbol.insert('#');
}

set<char> LR::first(const string& s) { // s不为产生式！
    if (s.length() == 0)
        return set<char>({ '@' });
    else if (s.length() == 1) {
        if (G.Vt.find(s[0]) != G.Vt.end() || s[0] == '#') // 终结符
            return set<char>({ s[0] });
        else
        if (FIRST[s[0]].size() != 0) return FIRST[s[0]];
        else {
            for (vector<Prod>::iterator it = G.prods.begin(); it != G.prods.end(); ++it)
                if (*it == s[0]) {
                    // 防止直接左递归
                    int xPos = it->right.find(it->left_VN);
                    if (xPos != string::npos) { // 找到X->aXb
                        if (xPos == 0) continue; // X->Xb
                        else { // X->aXb
                            string a = Prod::cut(it->right, 0, xPos);
                            if (first(a) == set<char>{'@'}) continue;
                        }
                    }
                    set<char> f = first(it->right);
                    FIRST[s[0]].insert(f.begin(), f.end());
                }


            return FIRST[s[0]];
        }
    }
    else { // first(X1X2X3X4)...
        set<char> ret;
        for (unsigned int i = 0; i < s.length(); ++i) {
            set<char> f = first(string(1, s[i])); // 逐个符号求first(Xi)集
            if (f.find('@') != f.end() && s.length() - 1 != i) { // 发现@
                f.erase(f.find('@')); // 减去@
                ret.insert(f.begin(), f.end()); // 放入first集合
            }
            else { // 无@或者最后一个Xi，则不需要求下去了
                ret.insert(f.begin(), f.end());
                break;
            }
        }
        return ret;
    }
}

void LR::follow() {
    FOLLOW[G.prods[0].left_VN].insert('#'); // 开始符号放'#'
    for (auto pp : G.prods) { // 直到follow(X)不在增大
        unsigned int size = 0;
        while (size != FOLLOW[pp.left_VN].size()) {
            size = FOLLOW[pp.left_VN].size();
            for (auto prod : G.prods) { // 求出所有非终结符的follow集合
                char X = prod.left_VN;
                for (auto p : G.prods) {// 求出X的follow集合
                    string s = p.right;
                    unsigned long loc = 0;
                    if ((loc = s.find(X)) != string::npos) { // 找到非终结符X
                        set<char> f = first(string(s.begin() + loc + 1, s.end())); // 求first(b)
                        FOLLOW[X].insert(f.begin(), f.end()); // 加入到follow(X)中
                        if (f.find('@') != f.end()) {// 找到@
                            FOLLOW[X].erase(FOLLOW[X].find('@')); // 删除@
                            set<char> fw = FOLLOW[p.left_VN]; // 把follow(A)放入follow(X)
                            FOLLOW[X].insert(fw.begin(), fw.end());
                        }
                    }
                }
            }
        }
    }
}

string Prod::replaceAll(const string& in, const string from, const string to) {
    size_t replacePos = in.find(from);
    string res = in;
    if (replacePos != string::npos)
        res.replace(replacePos, from.length(), to);
    return res;
}

void LR::draw_graph() {

    // 画节点
    // 遍历项目集
    j["n"];
    for (const auto& I : C) {
        vector<string>s;
        int i = &I - &C[0];
        s.push_back(to_string(i));
        for (const auto& p : I.prods) { // 列出项目
            string res = p.displayStr();
            if (res.find('^') != string::npos)
                res = Prod::replaceAll(res, "^", string(1, G.prods[0].left_VN) + "'");
            s.push_back(res.c_str());

        }
        j["n"].push_back(s);
    }


    // 状态节点
    for (const auto& I : C) {
        int i = &I - &C[0];
        j["nodes"].push_back(i);
    }

    // 状态边
    for (const auto& link : GOTO) {
        int i = link.first.first;
        int jj = link.second;
        vector<int>edge;
        edge.push_back(i);
        edge.push_back(jj);
        j["edges"].push_back(edge);
    }
}

void LR::web_output_table() {
    // 终结符
    for (auto vt : G.Vt) {
        if (vt != '#') {
            j["VT"].push_back(string(1, vt));
        }
    }
    j["VT"].push_back(string(1, '#'));

    // 非终结符
    for (auto vn : G.Vn) {
        j["VN"].push_back(string(1, vn));
    }

    // 表
    for (int i = 0; i < C.size(); i++) {

        vector<string>one_tb;

        // 对于普通终结符
        for (const auto& X : G.Vt) {
            if (X != '#') {
                pair<int, char> p = make_pair(i, X);
                // 存在这一项
                if (ACTION.find(p) != ACTION.end()) {
                    pair<actionStat, int> res = ACTION[p];
                    string second=to_string(res.second);
                    string str = actionStatStr[res.first] + second;
                    one_tb.push_back(str);
                }
                else {
                    one_tb.push_back("");
                }
            }
        }
        // #统一放在最后
        pair<int, char> p = make_pair(i, '#');
        // 找到了
        if (ACTION.find(p) != ACTION.end()) {
            pair<actionStat, int> res = ACTION[p];
            string str=actionStatStr[res.first];
            if (res.first != ACCEPT) {
                str += to_string(res.second);
            }
            one_tb.push_back(str);
        }
        else {
            one_tb.push_back("");
        }


        // 对于非终结符
        for (const auto& X : G.Vn) {
            pair<int, char> p = make_pair(i, X);
            string str = "";
            if (GOTO.find(p) != GOTO.end()) {
                str += to_string(GOTO[make_pair(i, X)]);
            }
            one_tb.push_back(str);
        }

        j["Body"].push_back(one_tb);
    }

}

void LR::web_output_steps() {
    bool success = false;
    // 先将#入分析栈
    // 不加其实也没关系
    parse.push_back('#');
    while (!success) {
        vector<string>one_step;
        // 状态栈
        string status_string = "";
        for (int i = 0; i < status.size();i++) {
            status_string += to_string(status[i]);
        }
        one_step.push_back(status_string);

        // 分析栈
        string analyse_string = "";
        for (int i = 0; i < parse.size(); i++) {
            analyse_string += string(1, parse[i]);
        }
        one_step.push_back(analyse_string);

        // 输入串
        string expression_string = "";
        for (int i = inStr.size() - 1; i >= 0; i--) {
            expression_string+= string(1, inStr[i]);
        }
        one_step.push_back(expression_string);


        // 分别取分析栈和输入串栈顶
        int sTop = status.size() - 1;
        int iTop = inStr.size() - 1;
        pair<int, char> p = make_pair(status[sTop], inStr[iTop]);

        // 出错,结束
        if (ACTION.find(p) == ACTION.end()) {
            one_step.push_back("ERROR");
            j["Process"].push_back(one_step);
            break;
        }

        pair<actionStat, int> act = ACTION[p];

        // 移进
        if (act.first == SHIFT) {

            string s = "ACTION[";
            s += to_string(p.first) + ",";
            s += string(1, p.second) + "]=S";
            s += to_string(act.second) + ",";
            s += " Status" + to_string(act.second) + "into Stack";

            status.push_back(act.second);
            parse.push_back(inStr[iTop]);
            inStr.pop_back();
            one_step.push_back(s);
        }
            // 归约
        else if (act.first == REDUCE) {
            string s = "r" + to_string(act.second) + ":";
            Prod p = G.prods[act.second];
            s += string(1, p.left_VN) + "->" + p.right + " Reduce, GOTO(" + to_string(status[status.size() - 1]) + "," + string(1, p.left_VN) + ")=";
            // 空串，无需出栈，直接规约
            if (p.right != "@") {
                for (unsigned i = 0; i < p.right.size(); ++i) {
                    status.pop_back();
                    parse.pop_back();
                }
            }
            parse.push_back(p.left_VN);
            status.push_back(GOTO[make_pair(status[status.size() - 1], p.left_VN)]);
            s += to_string(status[status.size() - 1]) + "into Stack";
            one_step.push_back(s);

        }
            // 接受
        else if (act.first == ACCEPT) {
            success = true;
            string s = "acc,Success";
            one_step.push_back(s);
        }
        j["Process"].push_back(one_step);
    }
}

string LR::get_data() {
    string data = j.dump();
    return data;
}