library verilog;
use verilog.vl_types.all;
entity CSA is
    port(
        x               : in     vl_logic_vector(31 downto 0);
        y               : in     vl_logic_vector(31 downto 0);
        z               : in     vl_logic_vector(31 downto 0);
        c               : out    vl_logic_vector(31 downto 0);
        s               : out    vl_logic_vector(31 downto 0)
    );
end CSA;
