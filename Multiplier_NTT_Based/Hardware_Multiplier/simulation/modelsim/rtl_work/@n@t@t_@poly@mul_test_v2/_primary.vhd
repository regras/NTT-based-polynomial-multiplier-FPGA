library verilog;
use verilog.vl_types.all;
entity NTT_PolyMul_test_v2 is
    generic(
        HP              : integer := 5;
        FP              : vl_notype
    );
    attribute mti_svvh_generic_type : integer;
    attribute mti_svvh_generic_type of HP : constant is 1;
    attribute mti_svvh_generic_type of FP : constant is 3;
end NTT_PolyMul_test_v2;
