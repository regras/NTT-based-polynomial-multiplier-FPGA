library verilog;
use verilog.vl_types.all;
entity PolyMult is
    port(
        clk             : in     vl_logic;
        reset_n         : in     vl_logic;
        start           : in     vl_logic;
        busy            : out    vl_logic;
        done_out        : out    vl_logic
    );
end PolyMult;
