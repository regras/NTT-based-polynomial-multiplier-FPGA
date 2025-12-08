library verilog;
use verilog.vl_types.all;
entity ModRed_sub is
    generic(
        CURR_DATA       : integer := 0;
        NEXT_DATA       : integer := 0
    );
    port(
        clk             : in     vl_logic;
        reset           : in     vl_logic;
        qH              : in     vl_logic_vector(3 downto 0);
        T1              : in     vl_logic_vector;
        C               : out    vl_logic_vector
    );
    attribute mti_svvh_generic_type : integer;
    attribute mti_svvh_generic_type of CURR_DATA : constant is 1;
    attribute mti_svvh_generic_type of NEXT_DATA : constant is 1;
end ModRed_sub;
