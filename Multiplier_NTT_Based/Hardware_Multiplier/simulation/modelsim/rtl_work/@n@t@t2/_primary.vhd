library verilog;
use verilog.vl_types.all;
entity NTT2 is
    port(
        clk             : in     vl_logic;
        reset           : in     vl_logic;
        q               : in     vl_logic_vector(12 downto 0);
        NTTin0          : in     vl_logic_vector(12 downto 0);
        NTTin1          : in     vl_logic_vector(12 downto 0);
        MULin           : in     vl_logic_vector(12 downto 0);
        ADDout          : out    vl_logic_vector(12 downto 0);
        SUBout          : out    vl_logic_vector(12 downto 0);
        NTToutEVEN      : out    vl_logic_vector(12 downto 0);
        NTToutODD       : out    vl_logic_vector(12 downto 0)
    );
end NTT2;
