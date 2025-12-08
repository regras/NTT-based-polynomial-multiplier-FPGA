library verilog;
use verilog.vl_types.all;
entity NTTN is
    port(
        clk             : in     vl_logic;
        reset           : in     vl_logic;
        load_w          : in     vl_logic;
        load_data       : in     vl_logic;
        start           : in     vl_logic;
        start_intt      : in     vl_logic;
        din             : in     vl_logic_vector(12 downto 0);
        done            : out    vl_logic;
        dout            : out    vl_logic_vector(12 downto 0)
    );
end NTTN;
