library verilog;
use verilog.vl_types.all;
entity PolyPointwiseMult is
    port(
        clk             : in     vl_logic;
        reset           : in     vl_logic;
        start           : in     vl_logic;
        done            : out    vl_logic;
        wr_en           : in     vl_logic;
        wr_addr         : in     vl_logic_vector(7 downto 0);
        wr_A            : in     vl_logic_vector(15 downto 0);
        wr_B            : in     vl_logic_vector(15 downto 0);
        q               : in     vl_logic_vector(15 downto 0);
        rd_en           : in     vl_logic;
        rd_addr         : in     vl_logic_vector(7 downto 0);
        rd_data         : out    vl_logic_vector(15 downto 0)
    );
end PolyPointwiseMult;
