library verilog;
use verilog.vl_types.all;
entity FA is
    port(
        x               : in     vl_logic;
        y               : in     vl_logic;
        z               : in     vl_logic;
        c               : out    vl_logic;
        s               : out    vl_logic
    );
end FA;
