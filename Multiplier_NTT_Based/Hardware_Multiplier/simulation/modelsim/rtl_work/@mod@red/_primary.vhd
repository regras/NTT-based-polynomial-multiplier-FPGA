library verilog;
use verilog.vl_types.all;
entity ModRed is
    port(
        clk             : in     vl_logic;
        reset           : in     vl_logic;
        q               : in     vl_logic_vector(12 downto 0);
        P               : in     vl_logic_vector(25 downto 0);
        C               : out    vl_logic_vector(12 downto 0)
    );
end ModRed;
