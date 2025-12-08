library verilog;
use verilog.vl_types.all;
entity AddressGenerator is
    port(
        clk             : in     vl_logic;
        reset           : in     vl_logic;
        start           : in     vl_logic;
        raddr0          : out    vl_logic_vector(6 downto 0);
        waddr0          : out    vl_logic_vector(6 downto 0);
        waddr1          : out    vl_logic_vector(6 downto 0);
        wen0            : out    vl_logic;
        wen1            : out    vl_logic;
        brsel0          : out    vl_logic;
        brsel1          : out    vl_logic;
        brselen0        : out    vl_logic;
        brselen1        : out    vl_logic;
        brscramble0     : out    vl_logic_vector(63 downto 0);
        raddr_tw        : out    vl_logic_vector(7 downto 0);
        stage_count     : out    vl_logic_vector(4 downto 0);
        ntt_finished    : out    vl_logic
    );
end AddressGenerator;
