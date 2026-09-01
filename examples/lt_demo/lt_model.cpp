// ---------------------------------------------------------------------------
// TLM-2.0 loosely-timed demo
//
//   Initiator ──▶ Router ──┬──▶ Memory "mem0"  @ 0x0000-0x0FFF
//                          └──▶ Memory "mem1"  @ 0x1000-0x1FFF
//
// Exercises the loosely-timed coding style as defined by IEEE Std 1666-2023:
// blocking transport + DMI + debug transport (LRM 10.6), with temporal
// decoupling driven by a quantum keeper (LRM 10.3.3, 16.3).
//
// Every non-obvious choice below cites the subclause that requires it.
//
// REQUIRES: an IEEE Std 1666-2023 implementation (SystemC 3.0.x).
// Uses SC_NAMED (LRM 5.2.9, new in 2023 per Annex D item 11) and omits
// SC_HAS_PROCESS, which Annex C item ah lists as deprecated and Annex D
// item 10 records as no longer necessary.
// ---------------------------------------------------------------------------

#include <systemc>

#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/tlm_quantumkeeper.h>

#include <cstring>
#include <iomanip>
#include <iostream>
#include <vector>

using sc_core::sc_time;
using sc_core::SC_NS;
using sc_core::SC_US;
using sc_core::SC_ZERO_TIME;

// ---------------------------------------------------------------------------
// Target: a memory
// ---------------------------------------------------------------------------
struct Memory : sc_core::sc_module
{
    // LRM 16.2.2.2. BUSWIDTH is in bits and is independent of a transaction's
    // data length (LRM 14.12 f).
    tlm_utils::simple_target_socket<Memory, 32> SC_NAMED(tsock);

    Memory(sc_core::sc_module_name n, sc_dt::uint64 size, const sc_time& latency)
      : sc_module(n)
      , m_size(size)
      , m_latency(latency)
      , m_mem(size, 0)
    {
        // LRM 16.2.2.4 f: registering b_transport alone is enough. An incoming
        // nb_transport_fw is converted to this callback by the socket.
        tsock.register_b_transport(this, &Memory::b_transport);
        tsock.register_get_direct_mem_ptr(this, &Memory::get_direct_mem_ptr);
        tsock.register_transport_dbg(this, &Memory::transport_dbg);
    }

    // -- blocking transport --------------------------------------------------
    // LRM 11.2.2.2. The sc_time argument carries the timing annotation for both
    // the call and the return (LRM 11.2.4.2 c).
    void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay)
    {
        const tlm::tlm_command cmd = trans.get_command();
        const sc_dt::uint64    adr = trans.get_address();
        unsigned char*         ptr = trans.get_data_ptr();
        const unsigned int     len = trans.get_data_length();
        const unsigned char*   be  = trans.get_byte_enable_ptr();
        const unsigned int     wid = trans.get_streaming_width();

        // LRM 14.17.2: the target does one and only one of {execute + OK,
        // set an error response, report + OK}. Each guard below picks the
        // error response the standard recommends for that failure.

        if (adr + len > m_size) {
            // LRM 14.10 / 14.17.1: address outside the memory the target owns.
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            return;
        }
        if (len == 0 && cmd != tlm::TLM_IGNORE_COMMAND) {
            // LRM 14.12 d: data length shall not be 0 for a read or write.
            trans.set_response_status(tlm::TLM_BURST_ERROR_RESPONSE);
            return;
        }
        if (wid < len) {
            // LRM 14.15 f: streaming width 0 is invalid, and a non-streaming
            // transfer should set it to at least the data length. This target
            // does not model streaming, so anything smaller is unsupported.
            // LRM 14.15 i recommends TLM_BURST_ERROR_RESPONSE.
            trans.set_response_status(tlm::TLM_BURST_ERROR_RESPONSE);
            return;
        }
        if (be != 0) {
            // This target does not model byte enables. LRM 14.13 n recommends
            // TLM_BYTE_ENABLE_ERROR_RESPONSE when they cannot be supported.
            trans.set_response_status(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE);
            return;
        }

        switch (cmd) {
        case tlm::TLM_WRITE_COMMAND:
            // LRM 14.7 f: on a write neither interconnect nor target may modify
            // the data array — we only read out of it.
            std::memcpy(&m_mem[adr], ptr, len);
            break;
        case tlm::TLM_READ_COMMAND:
            // LRM 14.7 f: on a read the target is the one component allowed to
            // modify the data array.
            std::memcpy(ptr, &m_mem[adr], len);
            break;
        case tlm::TLM_IGNORE_COMMAND:
            break;
        default:
            trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
            return;
        }

        // LRM 11.2.4.2 d: b_transport may increase the annotation, never
        // decrease it (except after a wait). Loosely-timed style: execute now,
        // hand the cost back to the caller as annotation rather than calling
        // wait — see the first example under LRM 11.2.4.2.
        delay += m_latency;

        // LRM 14.16: a DMI hint. Only meaningful to set it true here because
        // this target does support get_direct_mem_ptr.
        trans.set_dmi_allowed(true);

        // LRM 14.7 g: the target may set the response status any time between
        // receiving the transaction and passing the response back upstream —
        // which for b_transport is the moment we return.
        trans.set_response_status(tlm::TLM_OK_RESPONSE);
    }

    // -- DMI -----------------------------------------------------------------
    // LRM 11.3.3 / 11.3.5.
    bool get_direct_mem_ptr(tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi)
    {
        if (trans.get_address() >= m_size) {
            // LRM 11.3.5 r: to deny a region, describe it and return false.
            dmi.allow_read_write();
            dmi.set_start_address(0);
            dmi.set_end_address(m_size ? m_size - 1 : 0);
            return false;
        }

        // LRM 11.3.5 m: set the granted access to what is actually permitted.
        dmi.allow_read_write();
        // LRM 11.3.5 g: the DMI pointer addresses the storage that corresponds
        // to the start address, in the same organization as the generic
        // payload data array (LRM 11.3.5 h).
        dmi.set_dmi_ptr(&m_mem[0]);
        // LRM 11.3.5 r: first and last byte of the granted region, in this
        // target's own address space. The router translates on the way back
        // (LRM 11.3.5 u).
        dmi.set_start_address(0);
        dmi.set_end_address(m_size - 1);
        // LRM 11.3.5 ac: latency is the average PER BYTE. The initiator
        // multiplies by the number of bytes it would have transferred.
        dmi.set_read_latency(m_latency / 4);
        dmi.set_write_latency(m_latency / 4);
        return true;
    }

    // -- debug transport -----------------------------------------------------
    // LRM 11.4.4: no delay, no side effects, returns the number of bytes
    // actually transferred.
    unsigned int transport_dbg(tlm::tlm_generic_payload& trans)
    {
        const sc_dt::uint64 adr = trans.get_address();
        if (adr >= m_size)
            return 0;

        const unsigned int len =
            std::min<sc_dt::uint64>(trans.get_data_length(), m_size - adr);

        if (trans.get_command() == tlm::TLM_WRITE_COMMAND)
            std::memcpy(&m_mem[adr], trans.get_data_ptr(), len);
        else if (trans.get_command() == tlm::TLM_READ_COMMAND)
            std::memcpy(trans.get_data_ptr(), &m_mem[adr], len);

        return len;
    }

private:
    const sc_dt::uint64        m_size;
    const sc_time              m_latency;
    std::vector<unsigned char> m_mem;
};

// ---------------------------------------------------------------------------
// Interconnect: an address-decoding router
// ---------------------------------------------------------------------------
struct Router : sc_core::sc_module
{
    static const int N_TARGETS = 2;

    tlm_utils::simple_target_socket<Router, 32>           SC_NAMED(tsock);
    // Tagged so one callback can serve every downstream socket and still tell
    // them apart (LRM 16.2.3.1).
    tlm_utils::simple_initiator_socket_tagged<Router, 32>* isock[N_TARGETS];

    Router(sc_core::sc_module_name n)
      : sc_module(n)
    {
        for (int i = 0; i < N_TARGETS; ++i) {
            char nm[16];
            std::snprintf(nm, sizeof(nm), "isock_%d", i);
            isock[i] = new tlm_utils::simple_initiator_socket_tagged<Router, 32>(nm);
            isock[i]->register_invalidate_direct_mem_ptr(
                this, &Router::invalidate_direct_mem_ptr, i);
        }

        tsock.register_b_transport(this, &Router::b_transport);
        tsock.register_get_direct_mem_ptr(this, &Router::get_direct_mem_ptr);
        tsock.register_transport_dbg(this, &Router::transport_dbg);

        // LRM 15.2.9 b: routing is deterministic and depends only on the
        // address and command attributes. The map is fixed at construction and
        // shall not change while transactions are in flight.
        m_base[0] = 0x0000; m_size[0] = 0x1000;
        m_base[1] = 0x1000; m_size[1] = 0x1000;
    }

    ~Router()
    {
        for (int i = 0; i < N_TARGETS; ++i)
            delete isock[i];
    }

    // -- forward path --------------------------------------------------------
    void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay)
    {
        const sc_dt::uint64 global = trans.get_address();
        const int port = decode(global);

        if (port < 0) {
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            return;
        }

        // LRM 14.7 d: interconnect may modify the address attribute, but only
        // before passing the transaction on. After the downstream call it must
        // not touch it again — and per LRM 14.7 e the value on return is to be
        // treated as undefined for routing purposes, so we restore it here for
        // the initiator's benefit rather than relying on what comes back.
        trans.set_address(global - m_base[port]);

        // LRM 11.2.2.4 h: an interconnect should pass b_transport down the
        // forward path.
        (*isock[port])->b_transport(trans, delay);

        trans.set_address(global);
        delay += m_hop_latency;
    }

    // -- DMI forward path ----------------------------------------------------
    bool get_direct_mem_ptr(tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi)
    {
        const sc_dt::uint64 global = trans.get_address();
        const int port = decode(global);

        if (port < 0) {
            // LRM 11.3.5 r/aa: describe the denied region rather than leaving
            // it at the default full range, so the initiator does not conclude
            // that DMI is barred across the whole system address space.
            dmi.allow_read_write();
            dmi.set_start_address(global);
            dmi.set_end_address(global);
            return false;
        }

        trans.set_address(global - m_base[port]);
        const bool granted = (*isock[port])->get_direct_mem_ptr(trans, dmi);
        trans.set_address(global);

        // LRM 11.3.5 u: the interconnect must translate the DMI descriptor's
        // start/end addresses on the RETURN path, inverse to the forward-path
        // translation it applied to the address attribute.
        dmi.set_start_address(dmi.get_start_address() + m_base[port]);
        dmi.set_end_address(dmi.get_end_address() + m_base[port]);

        // LRM 11.3.5 v: the interconnect may narrow a granted region. Clip to
        // what this port actually owns.
        clip(dmi, m_base[port], m_base[port] + m_size[port] - 1);

        // LRM 11.3.5 ac: both interconnect and target may increase the
        // latency, so it accumulates on the way back to the initiator.
        dmi.set_read_latency(dmi.get_read_latency() + m_hop_latency / 4);
        dmi.set_write_latency(dmi.get_write_latency() + m_hop_latency / 4);
        return granted;
    }

    // -- DMI backward path ---------------------------------------------------
    // LRM 11.3.6 g: an interconnect is obliged to pass this on down the
    // backward path, translating the address range inversely to the forward
    // path. LRM 11.3.6 h: calling every initiator is the safe implementation.
    void invalidate_direct_mem_ptr(int id, sc_dt::uint64 start, sc_dt::uint64 end)
    {
        tsock->invalidate_direct_mem_ptr(start + m_base[id], end + m_base[id]);
    }

    // -- debug transport -----------------------------------------------------
    unsigned int transport_dbg(tlm::tlm_generic_payload& trans)
    {
        const sc_dt::uint64 global = trans.get_address();
        const int port = decode(global);
        if (port < 0)
            return 0;

        trans.set_address(global - m_base[port]);
        const unsigned int n = (*isock[port])->transport_dbg(trans);
        trans.set_address(global);
        return n;
    }

private:
    int decode(sc_dt::uint64 addr) const
    {
        for (int i = 0; i < N_TARGETS; ++i)
            if (addr >= m_base[i] && addr < m_base[i] + m_size[i])
                return i;
        return -1;
    }

    static void clip(tlm::tlm_dmi& dmi, sc_dt::uint64 lo, sc_dt::uint64 hi)
    {
        if (dmi.get_start_address() < lo) dmi.set_start_address(lo);
        if (dmi.get_end_address()   > hi) dmi.set_end_address(hi);
    }

    sc_dt::uint64 m_base[N_TARGETS];
    sc_dt::uint64 m_size[N_TARGETS];
    const sc_time m_hop_latency = sc_time(2, SC_NS);
};

// ---------------------------------------------------------------------------
// Initiator
// ---------------------------------------------------------------------------
struct Initiator : sc_core::sc_module
{
    tlm_utils::simple_initiator_socket<Initiator, 32> SC_NAMED(isock);

    Initiator(sc_core::sc_module_name n)
      : sc_module(n)
    {
        // LRM 11.2.2.4 b: b_transport shall not be called from a method
        // process, so the traffic generator has to be a thread process
        // (LRM 5.2.11). No SC_HAS_PROCESS: Annex C ah / Annex D 10.
        SC_THREAD(run);

        // LRM 16.2.2.4 k: an initiator should register an nb_transport_bw
        // callback; without one, a call would be a run-time error.
        isock.register_nb_transport_bw(this, &Initiator::nb_transport_bw);
        // LRM 16.2.2.4 l: optional, but a DMI user must honour invalidation
        // (LRM 11.3.6 d).
        isock.register_invalidate_direct_mem_ptr(
            this, &Initiator::invalidate_direct_mem_ptr);

        // LRM 16.3.5 a: the constructor of tlm_quantumkeeper does not compute
        // the local quantum, so reset() must be called before use.
        m_qk.set_global_quantum(sc_time(1, SC_US));
        m_qk.reset();
    }

    void run()
    {
        std::cout << "\n--- phase 1: writes through the router ---\n";
        for (sc_dt::uint64 a = 0x0000; a < 0x2000; a += 0x400) {
            unsigned int word = static_cast<unsigned int>(a) ^ 0xA5A5A5A5u;
            access(tlm::TLM_WRITE_COMMAND, a, word);
        }

        std::cout << "\n--- phase 2: read back and verify ---\n";
        for (sc_dt::uint64 a = 0x0000; a < 0x2000; a += 0x400) {
            unsigned int word = 0;
            access(tlm::TLM_READ_COMMAND, a, word);
            const unsigned int expect = static_cast<unsigned int>(a) ^ 0xA5A5A5A5u;
            if (word != expect) {
                std::ostringstream os;
                os << "read mismatch at 0x" << std::hex << a
                   << ": got 0x" << word << ", expected 0x" << expect;
                SC_REPORT_ERROR("lt_demo", os.str().c_str());
            }
        }

        std::cout << "\n--- phase 3: unmapped address (expect an error response) ---\n";
        {
            unsigned int word = 0;
            access(tlm::TLM_READ_COMMAND, 0x8000, word, /*tolerate_error=*/true);
        }

        std::cout << "\n--- phase 4: debug transport (no delay, no side effects) ---\n";
        {
            const sc_time before = sc_core::sc_time_stamp();
            tlm::tlm_generic_payload trans;
            unsigned char buf[16] = {0};
            // LRM 11.4.4: the debug interface uses command, address, data
            // pointer and data length only.
            trans.set_command(tlm::TLM_READ_COMMAND);
            trans.set_address(0x0000);
            trans.set_data_ptr(buf);
            trans.set_data_length(sizeof(buf));

            const unsigned int n = isock->transport_dbg(trans);
            std::cout << "  transport_dbg returned " << n << " bytes:";
            for (unsigned int i = 0; i < n && i < 8; ++i)
                std::cout << " " << std::hex << std::setw(2) << std::setfill('0')
                          << static_cast<unsigned>(buf[i]) << std::dec;
            std::cout << "\n  simulation time unchanged: "
                      << (sc_core::sc_time_stamp() == before ? "yes" : "NO") << "\n";
        }

        std::cout << "\n--- phase 5: 64 accesses over DMI ---\n";
        for (int i = 0; i < 64; ++i) {
            unsigned int word = 0xDEAD0000u + i;
            access(tlm::TLM_WRITE_COMMAND, 0x0100 + 4 * i, word);
        }
        std::cout << "  DMI hits: " << m_dmi_hits
                  << ", transport calls: " << m_transport_calls << "\n";

        // LRM 16.3.5 n/o: nothing forces a final synchronization, so drain the
        // outstanding local time before finishing.
        m_qk.sync();

        std::cout << "\nfinished at " << sc_core::sc_time_stamp() << "\n";
        sc_core::sc_stop();
    }

private:
    // One access, taking the DMI fast path when it is available.
    void access(tlm::tlm_command cmd, sc_dt::uint64 addr, unsigned int& word,
                bool tolerate_error = false)
    {
        if (try_dmi(cmd, addr, word))
            return;

        tlm::tlm_generic_payload trans;

        // LRM 14.7 a: the initiator is responsible for setting every generic
        // payload attribute except the extension pointers before passing the
        // transaction into an interface method call. Reusing a transaction
        // object (LRM 11.2.2.4 c) makes that a live hazard, so set all of them
        // explicitly every time.
        trans.set_command(cmd);
        trans.set_address(addr);
        trans.set_data_ptr(reinterpret_cast<unsigned char*>(&word));
        trans.set_data_length(4);                 // LRM 14.12 c/d: never 0
        trans.set_byte_enable_ptr(0);             // LRM 14.13: 0 = unused
        trans.set_byte_enable_length(0);
        trans.set_streaming_width(4);             // LRM 14.15 f: 0 is invalid
        trans.set_dmi_allowed(false);             // LRM 14.16: clear the hint
        trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);  // LRM 14.17.1
        // The option attribute is left at its TLM_MIN_PAYLOAD default:
        // LRM 14.8 g requires exactly that for the transport interfaces.

        // LRM 16.3.4 i: pass the local time offset as the annotation.
        sc_time delay = m_qk.get_local_time();

        isock->b_transport(trans, delay);
        ++m_transport_calls;

        // LRM 16.3.5 p: b_transport may itself yield, so it is the initiator's
        // job to write the returned annotation back into the quantum keeper and
        // then test whether a sync is due.
        m_qk.set(delay);

        // LRM 14.7 i: only now may the initiator assume the response status,
        // the DMI hint and (for a read) the data array hold the target's values.
        if (trans.is_response_error()) {
            if (tolerate_error) {
                std::cout << "  0x" << std::hex << addr << std::dec
                          << " -> " << trans.get_response_string() << " (expected)\n";
            } else {
                SC_REPORT_ERROR("lt_demo", trans.get_response_string().c_str());
            }
        } else if (trans.is_dmi_allowed()) {
            request_dmi(addr);
        }

        // Cost of the initiator's own work for this access.
        m_qk.inc(sc_time(10, SC_NS));

        // LRM 16.3.5 n: yield once the local offset reaches the local quantum.
        if (m_qk.need_sync())
            m_qk.sync();
    }

    // LRM 11.3.2 / 11.3.3.
    void request_dmi(sc_dt::uint64 addr)
    {
        tlm::tlm_generic_payload trans;
        // LRM 11.3.4: DMI uses the command and address attributes.
        trans.set_command(tlm::TLM_READ_COMMAND);
        trans.set_address(addr);

        tlm::tlm_dmi dmi;
        // LRM 11.3.5 d: the descriptor shall be in its default state when
        // passed in. init() guarantees that even if the object were pooled.
        dmi.init();

        if (isock->get_direct_mem_ptr(trans, dmi))
            m_dmi.push_back(dmi);
    }

    bool try_dmi(tlm::tlm_command cmd, sc_dt::uint64 addr, unsigned int& word)
    {
        for (std::size_t i = 0; i < m_dmi.size(); ++i) {
            const tlm::tlm_dmi& d = m_dmi[i];
            if (addr < d.get_start_address() || addr + 3 > d.get_end_address())
                continue;
            // LRM 11.3.5 p: the initiator is responsible for using only the
            // access mode that was actually granted.
            if (cmd == tlm::TLM_READ_COMMAND  && !d.is_read_allowed())  continue;
            if (cmd == tlm::TLM_WRITE_COMMAND && !d.is_write_allowed()) continue;

            unsigned char* p = d.get_dmi_ptr() + (addr - d.get_start_address());
            if (cmd == tlm::TLM_READ_COMMAND)
                std::memcpy(&word, p, 4);
            else
                std::memcpy(p, &word, 4);

            // LRM 11.3.5 ac/ad: the latency is per byte and the initiator is
            // responsible for honouring it, or the model loses timing accuracy.
            const sc_time& per_byte = (cmd == tlm::TLM_READ_COMMAND)
                                    ? d.get_read_latency() : d.get_write_latency();
            m_qk.inc(per_byte * 4);
            ++m_dmi_hits;

            if (m_qk.need_sync())
                m_qk.sync();
            return true;
        }
        return false;
    }

    // LRM 11.3.6 d: on invalidation the initiator shall immediately discard
    // every DMI region that overlaps the given range.
    void invalidate_direct_mem_ptr(sc_dt::uint64 start, sc_dt::uint64 end)
    {
        std::vector<tlm::tlm_dmi> kept;
        for (std::size_t i = 0; i < m_dmi.size(); ++i) {
            const bool overlaps = !(m_dmi[i].get_end_address() < start ||
                                    m_dmi[i].get_start_address() > end);
            if (!overlaps)
                kept.push_back(m_dmi[i]);
        }
        m_dmi.swap(kept);
        std::cout << "  DMI invalidated over [0x" << std::hex << start
                  << ", 0x" << end << "]\n" << std::dec;
    }

    // LRM 16.2.2.4 k requires this to be registered. A loosely-timed initiator
    // that only ever calls b_transport should never see it, so make a call
    // loud rather than silently returning a plausible value.
    tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload&,
                                       tlm::tlm_phase&, sc_time&)
    {
        SC_REPORT_FATAL("lt_demo",
                        "nb_transport_bw called on a loosely-timed initiator");
        return tlm::TLM_COMPLETED;
    }

    tlm_utils::tlm_quantumkeeper m_qk;
    std::vector<tlm::tlm_dmi>    m_dmi;
    unsigned long                m_dmi_hits = 0;
    unsigned long                m_transport_calls = 0;
};

// ---------------------------------------------------------------------------
// Top level
// ---------------------------------------------------------------------------
struct Top : sc_core::sc_module
{
    Initiator cpu;
    Router    bus;
    Memory    mem0;
    Memory    mem1;

    Top(sc_core::sc_module_name n)
      : sc_module(n)
      , cpu("cpu")
      , bus("bus")
      , mem0("mem0", 0x1000, sc_time(20, SC_NS))
      , mem1("mem1", 0x1000, sc_time(50, SC_NS))
    {
        // LRM 13.2.5: binding an initiator socket to a target socket binds the
        // forward and backward paths together.
        cpu.isock.bind(bus.tsock);
        bus.isock[0]->bind(mem0.tsock);
        bus.isock[1]->bind(mem1.tsock);
    }
};

int sc_main(int, char*[])   // LRM 4.4.5.2 fixes this signature
{
    // LRM 4.4.5.2: elaboration runs from here to just before the first
    // sc_start.
    Top top("top");

    sc_core::sc_start();

    // LRM 4.4.5.3: applications are recommended to call sc_stop before
    // returning from sc_main so that the end_of_simulation callbacks run.
    // The initiator already called it; a second call would be a warning
    // (LRM 4.6.4), so check the status first (LRM 4.6.9).
    if (sc_core::sc_get_status() != sc_core::SC_STOPPED)
        sc_core::sc_stop();

    return 0;
}
