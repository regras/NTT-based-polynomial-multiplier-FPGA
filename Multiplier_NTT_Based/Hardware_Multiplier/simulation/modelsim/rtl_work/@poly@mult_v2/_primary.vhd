library verilog;
use verilog.vl_types.all;
entity PolyMult_v2 is
    port(
        clk             : in     vl_logic;
        reset           : in     vl_logic;
        start_transaction: in     vl_logic;
        mode            : in     vl_logic_vector(2 downto 0);
        busy            : out    vl_logic;
        done_all        : out    vl_logic;
        valid_in        : in     vl_logic;
        din             : in     vl_logic_vector(12 downto 0);
        fifo_rd_enable  : out    vl_logic;
        valid_out       : out    vl_logic;
        dout            : out    vl_logic_vector(12 downto 0);
        fifo_wr_enable  : out    vl_logic;
        fifo_full       : in     vl_logic
    );
end PolyMult_v2;
