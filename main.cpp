#include"LALR.h"
#include<cpp-httplib/httplib.h>
#include<json/single_include/nlohmann/json.hpp>
using namespace httplib;
using json = nlohmann::json;
int main() {

    Server svr;
    cout << "http server已启动" << endl;
    svr.Get("/", [](const Request& req, Response& res) {
        res.set_content("编译原理课程设计", "text/plain");

    });


    svr.Get("/lr1", [](const Request& req, Response& res) {

        auto grammar = req.get_param_value("grammar");
        auto expression = req.get_param_value("expression");

        //LR lr;
        //ClassName *object=new ClassName(param);
        LR lr;
        lr.web_input(grammar, expression);
        //lr.web_input("E->E+T\nE->T\nT->T*F\nT->F\nF->(E)\nF->i","i+i*i");
        //lr->web_input("S->L=R\nS->R\nL->*R\nL->x\nR->L","i+i*i");

        lr.run();
        string data = lr.get_data();
        

        res.set_content(data, "application/json");

    });

    svr.listen("127.0.0.1", 5000);
    return 0;
}