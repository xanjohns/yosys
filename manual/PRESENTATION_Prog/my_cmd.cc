#include "kernel/rtlil.h"
#include "kernel/register.h"
#include "kernel/log.h"
#include "kernel/sigtools.h"

struct MyPass : public Pass {
    MyPass() : Pass("my_cmd", "just a simple test") { }
    virtual void execute(std::vector<std::string> args, RTLIL::Design *design)
    {
        log("Arguments to my_cmd:\n");
        for (auto &arg : args)
            log("  %s\n", arg.c_str());

        log("Modules in current design:\n");
        for (auto &mod : design->modules)
            log("  %s (%zd wires, %zd cells)\n", RTLIL::id2cstr(mod.first),
                    mod.second->wires_.size(), mod.second->cells_.size());
    }
} MyPass;


struct Test1Pass : public Pass {
    Test1Pass() : Pass("test1", "creating the absval module") { }
    virtual void execute(std::vector<std::string>, RTLIL::Design *design)
    {
        RTLIL::Module *module = new RTLIL::Module;
        module->name = "\\absval";

        RTLIL::Wire *a = module->new_wire(4, "\\a");
        a->port_input = true;
        a->port_id = 1;

        RTLIL::Wire *y = module->new_wire(4, "\\y");
        y->port_output = true;
        y->port_id = 2;

        RTLIL::Wire *a_inv = module->new_wire(4, NEW_ID);
        module->addNeg(NEW_ID, a, a_inv, true);
        module->addMux(NEW_ID, a, a_inv, RTLIL::SigSpec(a, 1, 3), y);

        log("Name of this module: %s\n", RTLIL::id2cstr(module->name));

        if (design->modules.count(module->name) != 0)
            log_error("A module with the name %s already exists!\n",
                    RTLIL::id2cstr(module->name));

        design->modules[module->name] = module;
    }
} Test1Pass;


struct Test2Pass : public Pass {
    Test2Pass() : Pass("test2", "demonstrating sigmap on test module") { }
    virtual void execute(std::vector<std::string>, RTLIL::Design *design)
    {
        if (design->selection_stack.back().empty())
            log_cmd_error("This command can't operator on an empty selection!\n");

        RTLIL::Module *module = design->modules.at("\\test");

        RTLIL::SigSpec a(module->wires_.at("\\a")), x(module->wires_.at("\\x")),
                                                   y(module->wires_.at("\\y"));
        log("%d %d %d\n", a == x, x == y, y == a); // will print "0 0 0"

        SigMap sigmap(module);
        log("%d %d %d\n", sigmap(a) == sigmap(x), sigmap(x) == sigmap(y),
                          sigmap(y) == sigmap(a)); // will print "1 1 1"

        log("Mapped signal x: %s\n", log_signal(sigmap(x)));

        log_header("Doing important stuff!\n");
        log_push();
        for (int i = 0; i < 10; i++)
            log("Log message #%d.\n", i);
        log_pop();
    }
} Test2Pass;

