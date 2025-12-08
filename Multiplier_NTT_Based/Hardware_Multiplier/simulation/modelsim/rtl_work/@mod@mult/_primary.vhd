library verilog;
use verilog.vl_types.all;
entity ModMult is
    port(
        clk             : in     vl_logic;
        reset           : in     vl_logic;
        A               : in     vl_logic_vector(12 downto 0);
        B               : in     vl_logic_vector(12 downto 0);
        q               : in     vl_logic_vector(12 downto 0);
        C               : out    vl_logic_vector(12 downto 0)
    );
end ModMult;
