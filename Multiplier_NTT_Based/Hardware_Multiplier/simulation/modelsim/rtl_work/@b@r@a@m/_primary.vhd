library verilog;
use verilog.vl_types.all;
entity BRAM is
    generic(
        DLEN            : integer := 32;
        HLEN            : integer := 9
    );
    port(
        clk             : in     vl_logic;
        wen             : in     vl_logic;
        waddr           : in     vl_logic_vector;
        din             : in     vl_logic_vector;
        raddr           : in     vl_logic_vector;
        dout            : out    vl_logic_vector
    );
    attribute mti_svvh_generic_type : integer;
    attribute mti_svvh_generic_type of DLEN : constant is 1;
    attribute mti_svvh_generic_type of HLEN : constant is 1;
end BRAM;
