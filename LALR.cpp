//
// Created by AKmoex on 2021/5/23.
//

#include "LALR.h"
set<char>Item::Vn;
set<char>Item::Vt;
set<char>Item::Symbol;
const char* LALR::actionStatStr[] = {
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

void LALR::web_input(string grammars, string expression) {
    G.Vt.clear();
    G.Vn.clear();
    G.Symbol.clear();

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

Item LALR::closure(Item I) {
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

Item LALR::Goto(const Item& I, char X) {
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
void LALR::items() {
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

// LALR合并项目集族
void LALR::merge(){

    // 复制一份
    vector<Item> C_copy;
    C_copy.assign(C.begin(), C.end());
    map<pair<int, char>, int> GOTO_copy=GOTO;

    // 打印合并前的项目集族
    cout<<"项目集族内容--------------------------"<<endl;
    for (const auto& I : C) {
        int i = &I - &C[0];
        cout<<i<<endl;
        for (const auto& p : I.prods) { // 列出项目
            string res = p.displayStr();
             cout<<res<<endl;

        }
        cout<<endl;
    }

    // 打印合并前的GOTO表
    cout<<"遍历GOTO表"<<endl;
    for(map<pair<int,char>,int>::iterator it = GOTO.begin(); it != GOTO.end(); it++){
        cout<<it->first.first<<" + "<<it->first.second<<" -> "<<it->second<<endl;
    }
    cout<<"--------------------------------------------------"<<endl<<endl;

    // same字典中记录的是同心集族
    // 如 1：2、8、9
    // 则说明和1同心的有 2、8、9
    map<int,vector<int>>same;

    // skip_item集合表示需要跳过集族（不需要处理它，直接跳过）
    // 比如上面的 2、8、9
    set<int>skip_item;

    // 遍历每个集族，找出所有的同心集族
    for(int i=1;i<C.size();i++){
        // 判断是否需要跳过
        if(skip_item.count(i)==0){
            // 若不需要跳过，则检查它
            cout<<endl<<"检查项目"<<i<<endl;
            // 记录集族i的每个产生式
            set<string>items_i;
            for (const auto& p_i : C[i].prods) {
                vector<string>v_i=p_i.displayStr2();
                string s_i=v_i[0]+"->"+v_i[1];
                items_i.insert(s_i);
            }

            // 检查集族i后面的集族，判断是否存在同心的
            for(int j=i+1;j<C.size();j++){
                // 得到每个产生式
                set<string>items_j;
                for (const auto& p_j : C[j].prods) {
                    vector<string>v_j=p_j.displayStr2();
                    string s_j=v_j[0]+"->"+v_j[1];
                    items_j.insert(s_j);
                }
                // 比较集族i的产生式和集族j的产生式是否相同
                bool isEqual=items_i.size()==items_j.size()&&equal(items_i.begin(), items_i.end(), items_j.begin(), items_j.end());
                // 若相同，则表明集族i和集族j同心
                if(isEqual){
                    cout<<j<<"  ";
                    for(auto x:items_i){
                        cout<<x<<endl;
                    }
                    // 更新same[i]
                    same[i].push_back(j);
                    // 同时将集族j添加至skip_item中，表明需要跳过
                    skip_item.insert(j);
                }
            }
        }

    }

    // 建立映射关系，将新的项目集族和1、2、3、4.。。。。。。建立映射
    // 这里主要是为了解决 前端画状态图
    map<int,int>ys;
    int dd=0;
    for(int d=0;d<C.size();d++){
        if(skip_item.count(d)==1){
            continue;
        }
        else{
            ys[d]=dd;
            dd++;
        }
    }

    // 下面是正式的合并操作

    for(map<int,vector<int>>::iterator it = same.begin(); it != same.end(); it++){
        cout<<it->first<<":";
        for(int i=0;i<it->second.size();i++){

            int same_item=it->second[i];
            vector<Prod>ps=C[same_item].prods;
            // 处理展望符
            for(auto p:ps){
                // 寻找对应产生式
                int yj=0;
                for(int j=0;j<C[it->first].prods.size();j++){
                    if(C[it->first].prods[j]==p){
                        yj=j;
                        break;
                    }
                }
                // 遍历展望符集合，判断是否需要添加进来
                for(auto pp:p.prospect){
                    if(C[it->first].prods[yj].prospect.count(pp)==0){
                        // 将需要添加的展望符添加进来
                        C[it->first].prods[yj].prospect.insert(pp);
                    }
                }
            }

            // 寻找原GOTO表中由same_item射出的弧
            for(map<pair<int,char>,int>::iterator it2 = GOTO.begin(); it2 != GOTO.end(); it2++){
                // 找到射出弧
                if(it2->first.first==same_item){
                    // 判断射出弧的终点是否是合并后的自身项目集
                    // 这里需要判断一下终点是不是合并后的自身
                    // true表示自身
                    bool zhong=false;
                    for(int zz=0;zz<it->second.size();zz++){
                        if(it2->second==it->second[zz]){
                            zhong=true;
                        }
                    }
                    // 终点是自身
                    if(zhong){
                        // 指向自己就可以
                        GOTO[make_pair(it->first,it2->first.second)]=it->first;
                    }
                    // 终点不是自身
                    else{
                        // 修改指向
                        GOTO[make_pair(it->first,it2->first.second)]=it2->second;
                    }
                }
            }

            // 寻找原GOTO表中射向same_item的弧
            for(map<pair<int,char>,int>::iterator it2 = GOTO.begin(); it2 != GOTO.end(); it2++){
                // 找到射向弧
                if(it2->second==same_item){
                    // 全部射向 same字典 对应的键
                    GOTO[make_pair(it2->first.first,it2->first.second)]=it->first;
                }
            }

            // 删除所有与same_item有关的弧
            // 先复制一份
            map<pair<int,char>,int>g=GOTO;
            for(map<pair<int,char>,int>::iterator it2 = g.begin(); it2 != g.end(); it2++){
                // 找到射出弧
                if(it2->first.first==same_item){
                    // 删除它
                    GOTO.erase(make_pair(it2->first.first,it2->first.second));
                }
                // 找到射向弧
                if(it2->second==same_item){
                    GOTO.erase(make_pair(it2->first.first,it2->first.second));
                }
            }

            // 合并展望符

            // 删除项目集族
            // 提取出来单独处理
            //swap(*(begin(C)+same_item),*(end(C)-1));
            //C.pop_back();
        }

    }


    // 删除多余的项目集族
    int delete_item_nums=0;
    for(auto x:skip_item){
        C.erase(C.begin()+x-delete_item_nums);
        delete_item_nums++;
    }
    // 更改GOTO表的映射关系
    map<pair<int,char>,int>gg=GOTO;
    for(map<pair<int,char>,int>::iterator it = gg.begin(); it != gg.end(); it++){
        int x=it->first.first;
        char ch=it->first.second;
        int y=it->second;

        GOTO.erase(make_pair(it->first.first,it->first.second));

        GOTO[make_pair(ys[x],ch)]=ys[y];
    }


    cout<<"--------------------------------------------------"<<endl<<endl;
    // 遍历GOTO
    cout<<"遍历GOTO表"<<endl;
    for(map<pair<int,char>,int>::iterator it = GOTO.begin(); it != GOTO.end(); it++){
        cout<<it->first.first<<" + "<<it->first.second<<" -> "<<it->second<<endl;
    }
    cout<<"--------------------------------------------------"<<endl<<endl;
    cout<<C.size();

//    for (const auto& I : C) {
//        vector<string>s;
//        int i = &I - &C[0];
//        if(i==0){
//            continue;
//        }
//        else{
//            cout<<endl<<"项目"<<i<<endl;
//            for (const auto& p : I.prods) { // 列出项目
//                vector<string>v=p.displayStr2();
//                cout<<v[0]<<"   "<<v[1]<<"  ";
//                for(int m=2;m<v.size();m++){
//                    cout<<v[m]<<",";
//                }
//                cout<<endl;
//            }
//
//            cout<<endl<<endl;
//        }
//    }

    cout<<"项目集族内容--------------------------"<<endl;

    for (const auto& I : C) {
        int i = &I - &C[0];
        cout<<i<<endl;
        for (const auto& p : I.prods) { // 列出项目
            string res = p.displayStr();
            cout<<res<<endl;

        }
        cout<<endl;
    }


    // 检验是否是LALR
    for(int i=0;i<C.size();i++){
            // 保存归约项目
            vector<string>ps;
            // 找出所有归约项目
            for (const auto& p_i : C[i].prods) {
                vector<string>v_i=p_i.displayStr2();
                string s_i=v_i[0]+"->"+v_i[1];
                cout<<s_i<<endl;
                // 该项目是归约项目
                if(s_i[s_i.length()-1]=='.'){
                    // 秩序记录箭头后面的内容就可以了
                    ps.push_back(v_i[1]);
                    cout<<s_i<<endl;
                }

                // 归约项目数目小于等于1，则不可能存在归于-归约错误
                if(ps.size()<=1){
                    continue;
                }

                set<string>ps_set;
                copy(ps.begin(), ps.end(), inserter(ps_set, ps_set.begin()));
                // 如果vector和set的大小不等，则说明有重复项目
                // 即存在归约-归约错误，则改用LR1方法进行分析
                if(ps.size()!=ps_set.size()){
                    // 改用LR1分析法分析
                    // 恢复项目集族、GOTO表、ACTION表

                    C.clear();
                    C.assign(C_copy.begin(), C_copy.end());
                    cout<<C.size()<<endl;
                    GOTO=GOTO_copy;
                    type="LR1";
                    cout<<"用LR1分析"<<endl;
                }

            }

            j["type"]=type;




    }

}

void LALR::run() {
    build_table();

    draw_graph();

    web_output_table();
    web_output_steps();
}

// 构造Action、GOTO表
void LALR::build_table() {
    // 构造DFA状态机
    items();

    //合并项目集族
    merge();

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

set<char> LALR::first(const string& s) { // s不为产生式！
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

void LALR::follow() {
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

void LALR::draw_graph() {

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
        char ch=link.first.second;

        string s1=to_string(i);
        string s2=to_string(jj);
        string s3=string(1,ch);

        vector<string>edge;
        edge.push_back(s1);
        edge.push_back(s2);
        edge.push_back(s3);
        j["edges"].push_back(edge);
    }
}

void LALR::web_output_table() {
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

void LALR::web_output_steps() {
    bool success = false;
    // 先将#入分析栈
    // 不加其实也没关系
    parse.push_back('#');
    while (!success) {
        vector<string>one_step;
        vector<string>tree_node;
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
            tree_node.push_back("e");
            tree_node.push_back(one_step[2]);

            j["Tree"].push_back(tree_node);
            break;
        }

        pair<actionStat, int> act = ACTION[p];

        // 移进
        if (act.first == SHIFT) {
            // 移进，给出树节点
            tree_node.push_back("s");
            tree_node.push_back(string(1, p.second));

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

            // 归约，给出语法树中父子节点
            tree_node.push_back("r");
            tree_node.push_back(p.right);
            tree_node.push_back(string(1, p.left_VN));

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
        j["Tree"].push_back(tree_node);

    }
}

string LALR::get_data() {
    string data = j.dump();
    return data;
}

