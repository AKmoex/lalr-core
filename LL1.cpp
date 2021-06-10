#include "LL1.h"

Grammar::Grammar(const string& in) {
    prod = in;
    isValid = false;
    analyze_grammar_string();
}

bool Grammar::analyze_grammar_string() {	if (prod.length() < 4) return false;
// A->...则left_VN = A
    if (cut(1, 3) == "->" && isupper(prod[0]))
        left_VN = prod[0];
    else return false;
    // 提取终结符、非终结符
    for (int i = 0; i < prod.length(); ++i) {
        char c = prod[i];
        if (isupper(c)) Vn.insert(c);
        else if (c != '|' && !(c == '-' && prod[i + 1] == '>' && ++i)) Vt.insert(c);
    }

    for (int i = 3; i < prod.length(); ++i) { // 提取候选式
        unsigned int j;
        for (j = i + 1; j < prod.length() && prod[j] != '|'; ++j);
        candidates.insert(cut(i, j));
        i = j;
    }

    return isValid = true;
}

string LL1::left_ecursion_and_common_factors(string grammars){

    // 初始化，维护一个所有得非终结符集合
    init(grammars);
    // 消除左递归
    remove_left_ecursion();
    // 提取左公因子
    extract_common_factors();
    // 返回消除左递归和提取左公因子后的文法
    string new_grammars="";
    for (list<list<string>>::iterator it_i = ListVn.begin(); it_i != ListVn.end(); ++it_i){
        list<string>::iterator j = it_i->begin();
        // 拼接左部非终结符
        new_grammars+=*j+"->";
        // 遍历拼接右部候选式
        for (int i = 2; i < it_i->size();i++){
            j++;
            new_grammars+=*j+"|";
        }
        j++;
        new_grammars+=*j+"\n";
    }
    // 返回处理完成的文法
    return new_grammars.substr(0,new_grammars.length()-1);
}

void LL1::remove_left_ecursion(){
    // 按字符扫描
    while (scan());
    ListVn.pop_back();

    // 先添加非终结符
    list<string> vn;
    list<list<string>>::iterator it;
    for (it = ListVn.begin(); it != ListVn.end(); ++it){
        vn.push_back(it->front());
    }
    // 对每个非终结符遍历其产生式
    for (list<list<string>>::iterator it_i = ListVn.begin(); it_i != ListVn.end(); it_i++){
        // 遍历当前非终结符前面的所有非终结符
        for (list<list<string>>::iterator it_j = ListVn.begin(); it_j != it_i; it_j++){
            list<string>::iterator it_k = it_i->begin();
            // 遍历其产生式
            for (it_k++; it_k != it_i->end();){
                string str1 = *it_k;
                string str2 = it_j->front();
                // 存在着Pi = pja
                if (str1.substr(0,1)==str2){
                    list<string>::iterator it_m = it_j->begin();
                    // 将所有非终结符pj用它的产生式进行替换
                    for (it_m++; it_m != it_j->end(); it_m++){
                        it_i->insert(it_k,*it_m+str1.substr(1,str1.length()));
                    }
                    // 删除处理完的非终结符
                    it_k = it_i->erase(it_k);
                    for (int i = 0; i < it_j->size() - 1;i++){
                        it_k--;
                    }
                }
                    // 不存在，扫描下一个非终结符及其产生式
                else{
                    it_k++;
                }
            }
        }
        bool flag = false;
        list<string>::iterator i = it_i->begin();
        string str2 = "";
        string str3 = "";

        // 扫描每个非终结符及其产生式
        for (i++; i != it_i->end();i++){
            // 判断是否存在直接左递归,即R->Ra的情况
            if (i->substr(0,1) == it_i->front()){
                string s = i->substr(1, i->size());
                *i = s;
                str2 = s;
                flag = true;
                break;
            }
        }
        //去重
        it_i->unique();
        //若存在直接左递归，则消除之
        if (flag){
            list<string>::iterator j = it_i->begin();
            // 从upLetter中选取一个为使用的非终结符
            str3=string(1,*upLetter.begin());
            // 此时该非终结符已使用，从upLetter中剔除
            upLetter.erase(*upLetter.begin());
            // 将所有相同前缀替换
            for (j++; j != it_i->end(); j++){
                *j = *j + str3;
            }
            //添加一条新的产生式进文法中
            list<string> la;
            la.push_back(str3);
            la.push_back(str2+str3);
            la.push_back("@");
            ListVn.push_back(la);
        }
    }
}

int LL1::Getchar(){
    if (*p != EOF){
        ch = *p;
        p++;
        return 1;
    }
    else
        return 0;
}

void LL1::extract_common_factors(){
    // 扫描每个字符及其产生式
    for (list<list<string>>::iterator it_i = ListVn.begin(); it_i != ListVn.end(); ++it_i){
        list<string>::iterator i = it_i->begin();
        list<string>::iterator p = it_i->begin();
        i++;
        list<string> lm;
        for (; i != it_i->end(); i++){
            p = i;
            // 不是非终结符，直接跳过
            if (i->at(0) > 'z' || i->at(0) < 'a'){
                continue;
            }
            for (p++; p != it_i->end(); p++){
                if (p->at(0) == i->at(0)){
                    //末位不是非终结符（即终结符）
                    if (p->at(p->size() - 1) >= 'a'&&p->at(p->size() - 1) <= 'z') {
                        // 从upLetter中选取新的非终结符来替换
                        string ss=string(1,*upLetter.begin());
                        // 更新upLetter非终结符集合
                        upLetter.erase(*upLetter.begin());
                        if (lm.empty()){
                            // 将替换后的产生式添加进来
                            if (i->size() > 1){
                                lm.push_back(i->substr(1, i->size() + 1));
                            }
                            if (p->size() > 1){
                                lm.push_back(p->substr(1, i->size() + 1));
                            }
                            // 单独添加一个产生式即 R->@
                            lm.push_back("@");
                            lm.push_back(ss);
                        }else{
                            // 为空时，直接添加进去就行
                            lm.push_back(*p);
                        }
                        //更新产生式
                        i = it_i->erase(i);
                        *i = i->substr(0, 1) + ss;
                    }
                    else{ continue; }
                }
                else{ continue; }
            }
            // 处理完成，将新的非终结符即产生式添加至原来的文法List中
            if (!lm.empty()){
                string s = lm.back();
                lm.pop_back();
                lm.push_front(s);
                ListVn.push_back(lm);
            }
        }
    }
}

int LL1::scan(){
    if (!Getchar()) return 0;
    //若当前字符为空，或者为制表符（9）或者为换行符（10）或者为换页符（12）则取下一个字符，直到当前字符合法
    while (ch == ' ' || ch == 9  || ch == 12){
        Getchar();
    }

    char ch2=*p;
    if (ListVn.empty()){
        ListVn.push_back(_vn);

    }
    if (ch == '-'&&ch2=='>'){
        ListVn.back().push_back(str);
        str = "";
        Getchar();
        return 1;
    }
    else if (ch == '\n'){
        ListVn.back().push_back(str);
        //(ListVn.back()->data).insertAsLast(str);
        str = "";
        ListVn.push_back(*new list<string>);
        //cout << "新添加一个List" << endl;

    }
    else if (ch=='|'){
        ListVn.back().push_back(str);
        //ListVn.last()->data.insertAsLast(str);
        //cout << "插入成功么：" << ListVn.back().back();
        //cout << endl << "sizeof(listvn):"<<ListVn.size() << endl;
        str = "";
    }
    else{
        str += ch;
    }
    return 1;
}

bool LL1::addProd(const Grammar& prod) {
    if (prod.isValid) {
        vector<Grammar>::iterator it = find(Grammars.begin(), Grammars.end(), prod.left_VN);
        if (it != Grammars.end()) {   // 找到产生式
            it->candidates.insert(prod.candidates.begin(), prod.candidates.end()); // 追加候选式
            for (auto s : prod.candidates)
                it->prod += ("|" + s); // 追加候选式
        }
        else  // 没找到
            Grammars.push_back(prod);
        VN.insert(prod.Vn.begin(), prod.Vn.end());
        VT.insert(prod.Vt.begin(), prod.Vt.end());
        return true;
    }
    else return false;
}

// FIRST集
set<char> LL1::first(const string& s) {
    Grammar prod = Grammar(s);
    // 右部产生式合法
    if (prod.isValid) {
        if (FIRST[prod.left_VN].size() != 0) return FIRST[prod.left_VN];
        // 遍历该产生式中的所有候选式
        for (auto sel : prod.candidates) {
            // 递归求出FIRST集合
            set<char> fi = first(sel);
            // 将求得的FIRST集保存
            FIRST[prod.left_VN].insert(fi.begin(), fi.end());
        }
        return FIRST[prod.left_VN];
    }
        // 空串的FIRST集合即@
    else if (s.length() == 0){
        return set<char>({ '@' });
    }
        // 长度为1的候选式
    else if (s.length() == 1) {
        // 终结符
        if (VT.find(s[0]) != VT.end()){
            // 终结符的FIRST集即终结符本身
            return set<char>({ s[0] });
        }
            // 非终结符
        else {
            // 若这个非终结符的FIRST在之前已经求出来了
            // 则直接取出就可以
            if (FIRST[s[0]].size() != 0){
                return FIRST[s[0]];
            }
            else {
                // 求出非终结符的产生式
                vector<Grammar>::iterator it = find(Grammars.begin(), Grammars.end(), s[0]);
                // 找到产生式
                if (it != Grammars.end()) {
                    // 递归求解FIRST集
                    set<char> fi = first(it->prod);
                    // 将求出来的FIRST集保存
                    FIRST[s[0]].insert(fi.begin(), fi.end());
                }
                return FIRST[s[0]];
            }
        }
    }
        // 长度大于1的候选式
    else {
        set<char> ret;
        for (int i = 0; i < s.length(); ++i) {
            // 逐个符号求FIRST(Xi)集
            set<char> f = first(string(1, s[i]));
            // 判断是否存在空字@
            if (f.find('@') != f.end() && s.length() - 1 != i) {
                // 剔除@
                f.erase(f.find('@'));
                // 将剔除@后的集合添加至FIRST集合
                ret.insert(f.begin(), f.end());
            }
                // 没有空字@
            else {
                // 直接全部添加至FIRST集合就行
                ret.insert(f.begin(), f.end());
                break;
            }
        }
        // 返回FIRST集
        return ret;
    }
}

// FOLLOW集求解
void LL1::follow() {
    // 开始符号放'#'
    FOLLOW[Grammars[0].left_VN].insert('#');
    // 遍历所有的文法
    for (auto pp : Grammars) {
        int size = 0;
        // 直到FOLLOW(X)不在增大
        while (size != FOLLOW[pp.left_VN].size()) {
            size = FOLLOW[pp.left_VN].size();
            // 处理所有的候选式
            for (auto prod : Grammars) {
                char X = prod.left_VN;
                // 求出X的FOLLOW集合
                for (auto p : Grammars){
                    // 逐个候选式找X
                    for (auto s : p.candidates) {
                        unsigned long loc = 0;
                        // 找到非终结符X
                        if ((loc = s.find(X)) != string::npos) {
                            // 求FIRST(b)
                            set<char> f = first(string(s.begin() + loc + 1, s.end()));
                            // 将FIRST(b)加入到FOLLOW(X)中
                            FOLLOW[X].insert(f.begin(), f.end());
                            // 找到@
                            if (f.find('@') != f.end()) {
                                // 剔除@
                                FOLLOW[X].erase(FOLLOW[X].find('@'));
                                // 把FOLLOW(A)放入FOLLOW(X)
                                set<char> fw = FOLLOW[p.left_VN];
                                FOLLOW[X].insert(fw.begin(), fw.end());
                            }
                        }
                    }
                }
            }
        }
    }
}

// 预测分析表的求解
void LL1::table() {
    // 遍历非终结符及其产生式
    for (auto prod : Grammars) {
        // 每个产生式的每个候选式
        for (auto sel : prod.candidates) {
            // 求它的FIRST集
            set<char> f = first(sel);
            // 对于FIRST集中的元素，添加产生式
            for (auto VN : f){
                // 存在@，将FOLLOW(x)的每个终结符放入表中
                if (VN == '@'){
                    for (auto term : FOLLOW[prod.left_VN]){
                        M[make_pair(prod.left_VN, term)] = sel;
                    }
                }
                else{
                    // 不存在@，则不用检查FOLLOW集，直接在FIRST集中元素所在列添加
                    M[make_pair(prod.left_VN, VN)] = sel;
                }
            }
        }
    }
}

void LL1::build() {
    // 求解非终结符地FIRST集
    for (auto prod : Grammars) first(prod.prod);

    // 复制一份终结符集合
    for (auto vt : VT) VT_SAVE.insert(vt);

    // 求解非终结符的FOLL集合
    follow();

    // 求解预测分析标
    table();

    // 求完表后，将@替换为#
    if (VT.find('@') != VT.end())
        VT.erase(VT.find('@'));
    VT.insert('#');

    //tableInfo();

    terminal_output();

}

// LL1分析过程
void LL1::parser() {
    // 分析栈中先添加 #
    parse.push_back('#');
    // 将文法的初始文法添加进分析栈
    parse.push_back(Grammars[0].left_VN); // 文法开始符号


    int ptop, itop;
    string prod = ""; // 候选式
    string one_step = "";
    string action = "init";
    while ((ptop = parse.size() - 1) >= 0) {
        itop = indata.size() - 1;

        // 取出当前分析栈中内容
        string s_parse = "";
        for (vector<char>::iterator it = parse.begin(); it != parse.end(); ++it) {
            s_parse += string(1, *it);
        }

        one_step += s_parse + " ";

        // 取出当前输入栈中内容
        string s_indata = "";
        for (vector<char>::reverse_iterator it = indata.rbegin(); it != indata.rend(); ++it) {

            s_indata += string(1, *it);
        }

        one_step += s_indata + " ";
        one_step += prod;
        string prod_s = prod;

        one_step += " "+action;
        Steps.push_back(one_step);
        one_step = "";
        // begin
        prod = "";
        char X = parse[ptop];
        char curc = indata[itop];
        parse.pop_back();
        if (VT.find(X) != VT.end()) { // 终结符
            // 不等于当前表达式符号就出错
            if (X != curc) {
                Steps.push_back("error error error error");
                success = false;
                break;
            }
            else {
                // 读入下一个符号
                indata.pop_back();
                prod = "";
                action = "mergr " + string(1, X) + ",get_next";
            }
        }
        else if (X == '@')
            continue;
        else { // 非终结符
            prod = M[make_pair(X, curc)];
            if (prod.size()) {
                // 不等于空才能入栈
                if (prod != "@") {
                    string s_ = prod;
                    reverse(s_.begin(), s_.end());
                    cout << s_ << endl;
                    action = "pop,push";
                    //prod_s = prod_s.substr(3, prod_s.length()-4);
                    cout <<"你好aaa " <<prod_s << endl;
                    for (int i = prod.size() - 1; i >= 0; --i)
                        parse.push_back(prod[i]);

                }
                else {
                    action = "pop";
                }

                prod = string(1, X) + "->" + prod;

            }
            else {

                // 恢复错误处理
                //set<char>FIRST_AND_FOLOOW;
                //for (auto x : FIRST[X]) {
                //	FIRST_AND_FOLOOW.insert(x);
                //}

                //for (auto x : FOLLOW[X]) {
                //	FIRST_AND_FOLOOW.insert(x);
                //}
                //// 说明可以跳过
                //while (find(FIRST_AND_FOLOOW.begin(), FIRST_AND_FOLOOW.end(), curc) == FIRST_AND_FOLOOW.end()) {
                //	indata.pop_back();
                //}
                //stringstream sss1;
                //sss1 << step;
                //string str_err = sss1.str() + " skip skip skip";
                //Steps.push_back(str_err);
                //continue;


                Steps.push_back("error error error error");
                success = false;
                break;
            }

        }
    }
}

string LL1::run(string grammar, string expression) {
    //file_input("C:\\Users\\AKmoex\\Desktop\\grammar.txt");
    string new_grammars=left_ecursion_and_common_factors(grammar+"\n");
    web_input(new_grammars,  expression);
    build();
    parser();
    return web_output();
    //puts("}");
}

void LL1::file_input(string path) {
    ifstream fin(path.c_str());
    string grammar_line;
    bool flag = true;
    if (fin.is_open()) {
        // 读取文法数目
        getline(fin, grammar_line);
        int grammar_count= stoi(grammar_line);
        // 逐条读取文法
        for (int i = 0; i < grammar_count;i++) {
            getline(fin, grammar_line);
            addProd(Grammar(grammar_line));
        }

        // 读取表达式
        getline(fin, grammar_line);


        indata.push_back('#');
        for (int i = grammar_line.length() - 1; i >= 0; --i){
            indata.push_back(grammar_line[i]);
        }
    };
    fin.close();
}

void LL1::terminal_output() {

    // 打印非终结符集合
    cout << "*********************  非终结符  *********************" << endl << endl;
    bool flag = true;
    cout << "VN = {";
    for (auto vn : VN) {
        if (flag) {
            cout << " " << vn;
            flag = false;
        }
        else {
            cout << " , " << vn;
        }
    }
    cout << " }" << endl << endl;


    // 打印终结符集合
    cout << "*********************   终结符   *********************" << endl << endl;
    flag = true;
    cout << "VT = {";
    for (auto vt : VT_SAVE) {
        if (flag) {
            cout << " "<<vt;
            flag = false;
        }
        else {
            cout << " , "<<vt ;
        }
    }
    cout << " }" << endl << endl;

    // 打印FIRST集合
    cout << "*********************   FIRST集  *********************" << endl << endl;
    flag = true;
    for (auto vn : VN) {
        cout << "FIRST(" << vn << ")={ ";
        for (auto f : FIRST[vn]) {
            if (flag) {
                cout << f;
                flag = false;
            }
            else {
                cout << " ," << f;
            }
        }
        cout << " }" << endl;
        flag = true;
    }
    cout << endl;

    // 打印FOLLOW集合
    cout << "*********************  FOLLOW集  *********************" << endl << endl;
    flag = true;
    for (auto vn : VN) {
        cout << "FOLLOW(" << vn << ")={ ";
        for (auto f : FOLLOW[vn]) {
            if (flag) {
                cout << f;
                flag = false;
            }
            else {
                cout << " ," << f;
            }
        }
        cout << " }" << endl;
        flag = true;
    }
    cout << endl;

    // 打印预测分析表
    cout << "********************  预测分析表  ********************" << endl << endl;
    flag = true;
}

string LL1::web_output() {
    json j;

    // 是否成功
    j["success"] = success;
    //新的文法
    for (auto grammar : new_grammars) {
        j["new_grammars"].push_back(grammar);
    }

    // 非终结符
    for (auto vn : VN) {
        j["VN"].push_back(string(1, vn));
    }

    // 终结符
    for (auto vt : VT) {
        j["VT"].push_back(string(1, vt));
    }

    // FIRST集合
    for (auto vn : VN) {
        for (auto f : FIRST[vn]) {
            j["FIRST"][string(1, vn)].push_back(string(1, f));
        }
    }

    // FOLLOW集合
    for (auto vn : VN) {
        j["FOLLOW"][string(1, vn)] = {};
        for (auto f : FOLLOW[vn]) {
            j["FOLLOW"][string(1, vn)].push_back(string(1, f));
        }
    }

    // Table_header 表头
    for (auto vt : VT_SAVE)
        if (vt != '@') {
            j["Table_header"].push_back(string(1, vt));
        }
    j["Table_header"].push_back("#");

    // Table_body 表内容
    for (auto prod : Grammars) {
        j["Table_body"][string(1, prod.left_VN)];

        for (auto c : VT) {
            if (c != '#') {
                j["Table_body"][string(1, prod.left_VN)].push_back(M[make_pair(prod.left_VN, c)].c_str());
            }
        }
        j["Table_body"][string(1, prod.left_VN)].push_back(M[make_pair(prod.left_VN, '#')].c_str());
    }

    for (auto x : Steps) {
        j["Steps"].push_back(x);
    }



    string ss = j.dump();


    cout << j.dump(4) << std::endl;
    return ss;
}

void LL1::web_input(string grammar,string expression) {
    string grammar_line = "";
    for (int i = 0; i < grammar.length(); i++) {
        if (grammar[i] != '\n') {
            grammar_line += grammar.substr(i, 1);
        }
        else {
            new_grammars.push_back(grammar_line);
            addProd(Grammar(grammar_line));
            grammar_line = "";
        }
    }
    addProd(Grammar(grammar_line));
    new_grammars.push_back(grammar_line);

    indata.push_back('#');
    for (int i = expression.length() - 1; i >= 0; --i){
        indata.push_back(expression[i]);
    }

}

void LL1::init(string grammars){
    // 遍历文法的每一个字符
    for(int i=0;i<grammars.length();i++){
        if(grammars[i]>='A'&&grammars[i]<='Z'){
            // 从upLetter中剔除已出现的非终结符
            upLetter.erase(grammars[i]);
        }
    }
    p = buf;
    // 依次读取字符
    for(int i=0;i<grammars.length();i++){
        *p=grammars[i];
        p++;
    }
    *p=EOF;
    p=buf;
}