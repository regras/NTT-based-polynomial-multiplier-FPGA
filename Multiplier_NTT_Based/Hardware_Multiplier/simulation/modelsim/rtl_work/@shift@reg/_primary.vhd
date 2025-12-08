library verilog;
use verilog.vl_types.all;
entity ShiftReg is
    generic(
        SHIFT           : integer := 0;
        DATA            : integer := 32
    );
    port(
        clk             : in     vl_logic;
        reset           : in     vl_logic;
        data_in         : in     vl_logic_vector;
        data_out        : out    vl_logic_vector
    );
    attribute mti_svvh_generic_type : integer;
    attribute mti_svvh_generic_type of SHIFT : constant is 1;
    attribute mti_svvh_generic_type of DATA : constant is 1;
end ShiftReg;
