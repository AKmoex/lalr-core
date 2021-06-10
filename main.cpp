#include"LALR.h"
#include<cpp-httplib/httplib.h>
#include"OPA.h"
using namespace httplib;
int main() {

    Server svr;
    cout << "http server已启动" << endl;
    svr.Get("/", [](const Request& req, Response& res) {
        res.set_content("编译原理课程设计", "text/plain");

    });
    svr.Get("/opa", [](const Request& req, Response& res) {


            //web_input("S->a|^|(T)\nT->T,S|S\n#\n","(a,(a,a))#");

        string data =run("E->E+T|T\nT->T*F|F\nF->(E)|i\n#\n","i+i#");

        res.set_content(data, "application/json");

    });

    svr.Get("/lalr", [](const Request& req, Response& res) {

        auto grammar = req.get_param_value("grammar");
        auto expression = req.get_param_value("expression");

        LALR lalr;
        lalr.web_input(grammar, expression);
        //lr.web_input("E->E+T\nE->T\nT->T*F\nT->F\nF->(E)\nF->i","i+i*i");
        //lr->web_input("S->L=R\nS->R\nL->*R\nL->x\nR->L","i+i*i");

        lalr.run();
        string data = lalr.get_data();
        

        res.set_content(data, "application/json");

    });

    svr.listen("127.0.0.1", 5000);
    return 0;
}