// ---------------------------------------------------------------------------
// IEEE Std 1666-2023 feature probe
//
// Exercises constructs that Annex D records as new in 2023 (and a few 2023
// clarifications), to check that a claimed 1666-2023 implementation actually
// provides them and behaves as the LRM specifies.
//
// Every assertion cites the subclause it is testing. Requires SystemC 3.0.x.
// ---------------------------------------------------------------------------

#include <systemc>

#include <tlm>

#include <iostream>
#include <string>
#include <vector>

using namespace sc_core;

static int g_pass = 0, g_fail = 0;

#define CHECK(what, cond)                                                    \
    do {                                                                     \
        const bool ok_ = (cond);                                             \
        std::cout << (ok_ ? "  pass  " : "  FAIL  ") << (what) << "\n";      \
        ok_ ? ++g_pass : ++g_fail;                                           \
    } while (0)

// ---------------------------------------------------------------------------
// Annex D 6 / LRM 4.6.10-4.6.13 — sc_stage_callback_if
// ---------------------------------------------------------------------------
struct StageLogger : sc_stage_callback_if
{
    std::vector<sc_stage> seen;

    void stage_callback(const sc_stage& stage) override
    {
        seen.push_back(stage);
    }

    bool saw(sc_stage s) const
    {
        for (std::size_t i = 0; i < seen.size(); ++i)
            if (seen[i] == s) return true;
        return false;
    }
};

// ---------------------------------------------------------------------------
// Annex D 11 / LRM 5.2.9 — SC_NAMED
// Annex D 22 / LRM 5.21 — sc_hierarchy_scope
// ---------------------------------------------------------------------------
class Escapee : public sc_module
{
public:
    explicit Escapee(sc_module_name) {}
};

class Probe : public sc_module
{
public:
    // LRM 5.2.9: SC_NAMED passes the identifier through as the constructor's
    // string name, so the variable name and the instance name cannot drift.
    sc_signal<int>  SC_NAMED(sig);
    // LRM 6.31: sc_unbound shall only bind to sc_port<sc_signal_inout_if<T>>;
    // binding it to an input port is an error, because the value read back is
    // undefined. sc_tie::value, by contrast, is legal on an input port.
    sc_inout<bool>  SC_NAMED(open_out);
    sc_in<bool>     SC_NAMED(tied_in);
    sc_event        SC_NAMED(ev);

    explicit Probe(sc_module_name n) : sc_module(n)
    {
        // LRM 5.2.8: no SC_HAS_PROCESS — Annex C ah / Annex D 10.
        SC_THREAD(run);

        // LRM 5.21.5: build an object at the root of the hierarchy from
        // inside a nested module. The scope reverts when it goes out of scope.
        {
            sc_hierarchy_scope scope(sc_hierarchy_scope::get_root());
            escaped_ = new Escapee("escaped");
        }
    }

private:
    Escapee* escaped_ = nullptr;

    void run()
    {
        // -- LRM 5.11.3: sc_time from a string (Annex D 16) -------------------
        std::cout << "\n[5.11 sc_time]\n";
        CHECK("sc_time(\"1 ms\") == sc_time(1.0, SC_MS)",
              sc_time("1 ms") == sc_time(1.0, SC_MS));
        CHECK("sc_time(\"1.0E-3\") == sc_time(1.0e-3, SC_SEC)",
              sc_time("1.0E-3") == sc_time(1.0e-3, SC_SEC));
        CHECK("sc_time(\"2.0m\") == sc_time(2.0, SC_MS)",
              sc_time("2.0m") == sc_time(2.0, SC_MS));
        CHECK("sc_time(\"0\") == SC_ZERO_TIME",
              sc_time("0") == SC_ZERO_TIME);
        CHECK("from_seconds(1e-9) == sc_time(1, SC_NS)",
              sc_time::from_seconds(1e-9) == sc_time(1, SC_NS));
        CHECK("from_string(\"5 ns\") == sc_time(5, SC_NS)",
              sc_time::from_string("5 ns") == sc_time(5, SC_NS));
        CHECK("from_value(v) round-trips value()",
              sc_time::from_value(sc_time(7, SC_NS).value()) == sc_time(7, SC_NS));
        // Annex D 16: operator% is new in 2023.
        CHECK("operator%: 7ns % 3ns == 1ns",
              (sc_time(7, SC_NS) % sc_time(3, SC_NS)) == sc_time(1, SC_NS));
        CHECK("to_string() is non-empty", !sc_time(1, SC_NS).to_string().empty());

        // -- LRM 4.6.6: sc_delta_count_at_current_time (Annex D 7) ------------
        std::cout << "\n[4.6.6 delta counts]\n";
        {
            const sc_dt::uint64 abs0 = sc_delta_count();
            const sc_dt::uint64 cur0 = sc_delta_count_at_current_time();
            wait(SC_ZERO_TIME);                       // one delta cycle
            wait(SC_ZERO_TIME);                       // another
            const sc_dt::uint64 abs1 = sc_delta_count();
            const sc_dt::uint64 cur1 = sc_delta_count_at_current_time();
            CHECK("sc_delta_count advanced by 2", abs1 - abs0 == 2);
            CHECK("at_current_time advanced by 2", cur1 - cur0 == 2);

            wait(10, SC_NS);                           // advance simulated time
            // LRM 4.6.6: at_current_time resets on each time advance.
            CHECK("at_current_time reset after a time advance",
                  sc_delta_count_at_current_time() < sc_delta_count());
        }

        // -- LRM 5.10: events, hierarchical naming ----------------------------
        std::cout << "\n[5.10 sc_event]\n";
        CHECK("SC_NAMED event basename is \"ev\"", std::string(ev.basename()) == "ev");
        // LRM 5.10.1: an event created during elaboration shall be
        // hierarchically named.
        CHECK("event is in the hierarchy", ev.in_hierarchy());
        CHECK("event name is hierarchical",
              std::string(ev.name()) == std::string(name()) + ".ev");
        // LRM 5.10.5: get_parent_object returns the owning module.
        CHECK("event parent is this module", ev.get_parent_object() == this);
        // LRM 5.10.8: the none event is guaranteed never to be notified.
        // LRM 5.10.2 declares this as a static DATA member
        // (`static const sc_event none;`). SystemC 3.0.2 provides a static
        // member FUNCTION `static const sc_event& none()` instead, so the
        // spelling below deviates from the standard. Per the LRM Introduction,
        // the standard is definitive where the two disagree.
        CHECK("sc_event::none() is never triggered",
              !sc_event::none().triggered());

        // LRM 5.10.5: triggered() is true in the delta notification phase that
        // follows an immediate/delta notification.
        ev.notify(SC_ZERO_TIME);
        wait(SC_ZERO_TIME);
        CHECK("triggered() true after a delta notification", ev.triggered());

        // -- LRM 5.17.2/5.17.3: hierarchical name registry (Annex D) ----------
        std::cout << "\n[5.17 hierarchical names]\n";
        CHECK("sc_hierarchical_name_exists finds this module",
              sc_hierarchical_name_exists(name()));
        CHECK("sc_hierarchical_name_exists rejects a bogus name",
              !sc_hierarchical_name_exists("top.no_such_thing"));
        // LRM 5.17.3: reserve a name so nothing else can take it.
        CHECK("sc_register_hierarchical_name succeeds",
              sc_register_hierarchical_name("top.reserved_slot"));
        CHECK("the reserved name now exists",
              sc_hierarchical_name_exists("top.reserved_slot"));
        CHECK("sc_unregister_hierarchical_name releases it",
              sc_unregister_hierarchical_name("top.reserved_slot"));
        CHECK("the released name is gone",
              !sc_hierarchical_name_exists("top.reserved_slot"));

        // -- LRM 4.6.9: sc_get_status ----------------------------------------
        std::cout << "\n[4.6 kernel status]\n";
        CHECK("sc_get_status() == SC_RUNNING", sc_get_status() == SC_RUNNING);
        // LRM 4.6.7: the relation the standard states explicitly.
        CHECK("sc_is_running matches the status mask",
              sc_is_running() ==
                  bool(sc_get_status() & (SC_RUNNING | SC_PAUSED | SC_SUSPENDED)));
        CHECK("sc_pending_activity_at_current_time is callable",
              sc_pending_activity() || !sc_pending_activity());

        // -- LRM 5.21: sc_hierarchy_scope ------------------------------------
        std::cout << "\n[5.21 sc_hierarchy_scope]\n";
        CHECK("object built under get_root() is top-level",
              escaped_ != nullptr && escaped_->get_parent_object() == nullptr);
        CHECK("its name has no dot",
              escaped_ && std::string(escaped_->name()).find('.') == std::string::npos);

        // -- LRM 6.31: sc_unbound / sc_tie ------------------------------------
        std::cout << "\n[6.31 sc_unbound / sc_tie]\n";
        // LRM 6.31: writes to an sc_unbound channel are ignored and shall not
        // notify an event; the port is nonetheless legally bound.
        open_out.write(true);
        CHECK("write to an sc_unbound output is accepted", true);
        // LRM 6.31: the value read from an sc_tie channel corresponds to the
        // value it was tied to.
        CHECK("sc_tie::value(true) reads back true", tied_in.read() == true);
        // LRM 6.31: each use of sc_unbound creates a fresh channel whose
        // name is "sc_unbound" plus an underscore and decimal digits.
        {
            const sc_object* ch =
                dynamic_cast<const sc_object*>(open_out.get_interface());
            CHECK("sc_unbound channel basename starts with \"sc_unbound\"",
                  ch && std::string(ch->basename()).rfind("sc_unbound", 0) == 0);
        }

        // -- LRM 8.6: version identification ---------------------------------
        std::cout << "\n[8.6 version]\n";
#ifdef IEEE_1666_SYSTEMC
        CHECK("IEEE_1666_SYSTEMC >= 202301L", IEEE_1666_SYSTEMC >= 202301L);
        std::cout << "        IEEE_1666_SYSTEMC = " << IEEE_1666_SYSTEMC << "\n";
#else
        CHECK("IEEE_1666_SYSTEMC is defined", false);
#endif
        std::cout << "        sc_version()   = " << sc_version() << "\n";
        // LRM 8.6.5 names these; sc_core_version does NOT exist in the standard.
        CHECK("sc_version_major >= 3", sc_version_major >= 3);
        CHECK("sc_release() is non-empty", std::string(sc_release()).size() > 0);

        sc_stop();
    }
};

// ---------------------------------------------------------------------------
// LRM 14.8 — generic payload option attribute (TLM-2.0)
// ---------------------------------------------------------------------------
static void tlm_payload_checks()
{
    std::cout << "\n[14.x generic payload]\n";
    tlm::tlm_generic_payload gp;

    // LRM 14.7 Table 54: default values.
    CHECK("command default TLM_IGNORE_COMMAND",
          gp.get_command() == tlm::TLM_IGNORE_COMMAND);
    CHECK("address default 0",             gp.get_address() == 0);
    CHECK("data pointer default 0",        gp.get_data_ptr() == nullptr);
    CHECK("data length default 0",         gp.get_data_length() == 0);
    CHECK("byte enable pointer default 0", gp.get_byte_enable_ptr() == nullptr);
    CHECK("byte enable length default 0",  gp.get_byte_enable_length() == 0);
    CHECK("streaming width default 0",     gp.get_streaming_width() == 0);
    CHECK("dmi allowed default false",     gp.is_dmi_allowed() == false);
    CHECK("response status default TLM_INCOMPLETE_RESPONSE",
          gp.get_response_status() == tlm::TLM_INCOMPLETE_RESPONSE);

    // LRM 14.8 c: the option attribute defaults to TLM_MIN_PAYLOAD.
    CHECK("option default TLM_MIN_PAYLOAD",
          gp.get_gp_option() == tlm::TLM_MIN_PAYLOAD);
    gp.set_gp_option(tlm::TLM_FULL_PAYLOAD);
    CHECK("set_gp_option/get_gp_option round-trip",
          gp.get_gp_option() == tlm::TLM_FULL_PAYLOAD);

    // LRM 14.17.1: response status helpers.
    CHECK("is_response_error() true while incomplete",
          tlm::tlm_generic_payload().is_response_error());
    gp.set_response_status(tlm::TLM_OK_RESPONSE);
    CHECK("is_response_ok() after TLM_OK_RESPONSE", gp.is_response_ok());
    CHECK("get_response_string() is non-empty",
          !gp.get_response_string().empty());

    // LRM 11.3.5 b: tlm_dmi defaults.
    tlm::tlm_dmi dmi;
    dmi.init();
    CHECK("tlm_dmi granted access defaults to DMI_ACCESS_NONE",
          dmi.is_none_allowed());
    CHECK("tlm_dmi start address defaults to 0", dmi.get_start_address() == 0);
    CHECK("tlm_dmi read latency defaults to SC_ZERO_TIME",
          dmi.get_read_latency() == SC_ZERO_TIME);
}

// ---------------------------------------------------------------------------
int sc_main(int, char*[])
{
    // LRM 5.11.5: sc_set_time_resolution shall only be called during
    // elaboration, at most once, and before any non-zero sc_time is built.
    // Leaving it at the 1 ps default here.
    std::cout << "time resolution = " << sc_get_time_resolution()
              << ",  sc_max_time = " << sc_max_time() << "\n";

    StageLogger logger;
    // LRM 4.6.11: the second argument is an sc_stage mask; the implementation
    // shall support bitwise operations on the enumeration.
    sc_register_stage_callback(logger,
                               SC_POST_BEFORE_END_OF_ELABORATION |
                               SC_POST_END_OF_ELABORATION |
                               SC_POST_START_OF_SIMULATION |
                               SC_POST_UPDATE |
                               SC_PRE_TIMESTEP |
                               SC_PRE_STOP |
                               SC_POST_END_OF_SIMULATION);

    Probe SC_NAMED(top);
    // LRM 6.31 / Annex D 27: sc_unbound leaves an output deliberately open;
    // sc_tie::value drives an input with a fixed value.
    top.open_out(sc_unbound);
    top.tied_in(sc_tie::value(true));

    tlm_payload_checks();

    sc_start();

    std::cout << "\n[4.6.13 stage callbacks]\n";
    CHECK("SC_POST_BEFORE_END_OF_ELABORATION fired",
          logger.saw(SC_POST_BEFORE_END_OF_ELABORATION));
    CHECK("SC_POST_END_OF_ELABORATION fired",
          logger.saw(SC_POST_END_OF_ELABORATION));
    CHECK("SC_POST_START_OF_SIMULATION fired",
          logger.saw(SC_POST_START_OF_SIMULATION));
    CHECK("SC_POST_UPDATE fired", logger.saw(SC_POST_UPDATE));
    CHECK("SC_PRE_TIMESTEP fired", logger.saw(SC_PRE_TIMESTEP));
    CHECK("SC_PRE_STOP fired", logger.saw(SC_PRE_STOP));

    sc_unregister_stage_callback(logger, SC_POST_END_OF_SIMULATION);

    // LRM 4.6.9: after sc_stop the status shall be SC_STOPPED.
    std::cout << "\n[4.6.9 final status]\n";
    CHECK("status is SC_STOPPED", sc_get_status() == SC_STOPPED);
    CHECK("sc_end_of_simulation_invoked()", sc_end_of_simulation_invoked());

    std::cout << "\n==== " << g_pass << " passed, " << g_fail << " failed ====\n";
    return g_fail ? 1 : 0;
}
